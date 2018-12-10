// Copyright 2018 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "amber/amber.h"
#include "amber/recipe.h"
#include "src/build-versions.h"

namespace {

struct Options {
  std::string input_filename;

  std::string image_filename;
  std::string buffer_filename;
  int64_t buffer_binding_index = 0;
  bool parse_only = false;
  bool show_help = false;
  bool show_version_info = false;
  amber::EngineType engine = amber::EngineType::kVulkan;
};

const char kUsage[] = R"(Usage: amber [options] SCRIPT

 options:
  -p             -- Parse input files only; Don't execute
  -i <filename>  -- Write rendering to <filename> as a PPM image.
  -b <filename>  -- Write contents of a UBO or SSBO to <filename>.
  -B <buffer>    -- Index of buffer to write. Defaults buffer 0.
  -e <engine>    -- Specify graphics engine: vulkan, dawn. Default is vulkan.
  -V, --version  -- Output version information for Amber and libraries.
  -h             -- This help text.
)";

bool ParseArgs(const std::vector<std::string>& args, Options* opts) {
  for (size_t i = 1; i < args.size(); ++i) {
    const std::string& arg = args[i];
    if (arg == "-i") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -i argument." << std::endl;
        return false;
      }
      opts->image_filename = args[i];

    } else if (arg == "-b") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -b argument." << std::endl;
        return false;
      }
      opts->buffer_filename = args[i];

    } else if (arg == "-B") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -B argument." << std::endl;
        return false;
      }
      opts->buffer_binding_index =
          static_cast<int64_t>(strtol(args[i].c_str(), nullptr, 10));

      if (opts->buffer_binding_index < 0U) {
        std::cerr << "Invalid value for -B, must be 0 or greater." << std::endl;
        return false;
      }

    } else if (arg == "-e") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -e argument." << std::endl;
        return false;
      }
      const std::string& engine = args[i];
      if (engine == "vulkan") {
        opts->engine = amber::EngineType::kVulkan;
      } else if (engine == "dawn") {
        opts->engine = amber::EngineType::kDawn;
      } else {
        std::cerr
            << "Invalid value for -e argument. Must be one of: vulkan dawn"
            << std::endl;
        return false;
      }

    } else if (arg == "-h" || arg == "--help") {
      opts->show_help = true;
    } else if (arg == "-V" || arg == "--version") {
      opts->show_version_info = true;
    } else if (arg == "-p") {
      opts->parse_only = true;
    } else if (arg.size() > 0 && arg[0] == '-') {
      std::cerr << "Unrecognized option " << arg << std::endl;
      return false;
    } else {
      opts->input_filename = args[i];
    }
  }

  return true;
}

std::string ReadFile(const std::string& input_file) {
  FILE* file = nullptr;
#if defined(_MSC_VER)
  fopen_s(&file, input_file.c_str(), "rb");
#else
  file = fopen(input_file.c_str(), "rb");
#endif
  if (!file) {
    std::cerr << "Failed to open " << input_file << std::endl;
    return {};
  }

  fseek(file, 0, SEEK_END);
  uint64_t tell_file_size = static_cast<uint64_t>(ftell(file));
  if (tell_file_size <= 0) {
    std::cerr << "Input file of incorrect size: " << input_file << std::endl;
    return {};
  }
  fseek(file, 0, SEEK_SET);

  size_t file_size = static_cast<size_t>(tell_file_size);

  std::vector<char> data;
  data.resize(file_size);

  size_t bytes_read = fread(data.data(), sizeof(char), file_size, file);
  fclose(file);
  if (bytes_read != file_size) {
    std::cerr << "Failed to read " << input_file << std::endl;
    return {};
  }

  return std::string(data.begin(), data.end());
}

}  // namespace

int main(int argc, const char** argv) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;

  if (!ParseArgs(args, &options)) {
    std::cerr << "Failed to parse arguments." << std::endl;
    return 1;
  }

  if (options.show_version_info) {
    std::cout << "Amber        : " << AMBER_VERSION << std::endl;
#ifndef AMBER_DISABLE_SPIRV_TOOLS
    std::cout << "SPIRV-Tools  : " << SPIRV_TOOLS_VERSION << std::endl;
    std::cout << "SPIRV-Headers: " << SPIRV_HEADERS_VERSION << std::endl;
#endif  // AMBER_DISABLE_SPIRV_TOOLS
#ifndef AMBER_DISABLE_SHADERC
    std::cout << "GLSLang      : " << GLSLANG_VERSION << std::endl;
    std::cout << "Shaderc      : " << SHADERC_VERSION << std::endl;
#endif  // AMBER_DISABLE_SHADERC
  }

  if (options.show_help) {
    std::cout << kUsage << std::endl;
    return 0;
  }

  if (options.input_filename.empty()) {
    std::cerr << "Input file must be provided." << std::endl;
    return 2;
  }

  auto data = ReadFile(options.input_filename);
  if (data.empty())
    return 1;

  amber::Amber am;
  amber::Recipe recipe;
  amber::Result result = am.Parse(data, &recipe);
  if (!result.IsSuccess()) {
    std::cerr << result.Error() << std::endl;
    return 1;
  }

  if (options.parse_only)
    return 0;

  amber::Options amber_options;
  amber_options.engine = options.engine;
  result = am.Execute(&recipe, amber_options);
  if (!result.IsSuccess()) {
    std::cerr << result.Error() << std::endl;
    return 1;
  }

  if (!options.buffer_filename.empty()) {
    // TODO(dsinclair): Write buffer file
    assert(false);
  }

  if (!options.image_filename.empty()) {
    // TODO(dsinclair): Write image file
    assert(false);
  }

  return 0;
}
