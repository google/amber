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
#include <utility>
#include <vector>

#include "amber/amber.h"
#include "amber/recipe.h"
#include "samples/config_helper.h"
#include "src/build-versions.h"
#include "src/make_unique.h"

namespace {

struct Options {
  std::vector<std::string> input_filenames;

  std::string image_filename;
  std::string buffer_filename;
  int64_t buffer_binding_index = 0;
  bool parse_only = false;
  bool show_help = false;
  bool show_version_info = false;
  amber::EngineType engine = amber::EngineType::kVulkan;
};

const char kUsage[] = R"(Usage: amber [options] SCRIPT [SCRIPTS...]

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
      if (!arg.empty())
        opts->input_filenames.push_back(arg);
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
#if AMBER_ENABLE_SPIRV_TOOLS
    std::cout << "SPIRV-Tools  : " << SPIRV_TOOLS_VERSION << std::endl;
    std::cout << "SPIRV-Headers: " << SPIRV_HEADERS_VERSION << std::endl;
#endif  // AMBER_ENABLE_SPIRV_TOOLS
#if AMBER_ENABLE_SHADERC
    std::cout << "GLSLang      : " << GLSLANG_VERSION << std::endl;
    std::cout << "Shaderc      : " << SHADERC_VERSION << std::endl;
#endif  // AMBER_ENABLE_SHADERC
  }

  if (options.show_help) {
    std::cout << kUsage << std::endl;
    return 0;
  }

  if (options.input_filenames.empty()) {
    std::cerr << "Input file must be provided." << std::endl;
    return 2;
  }

  amber::Result result;
  std::vector<std::string> failures;
  std::vector<std::pair<std::unique_ptr<amber::Recipe>, std::string>>
      recipe_and_files;
  for (const auto& file : options.input_filenames) {
    auto data = ReadFile(file);
    if (data.empty()) {
      std::cerr << file << " is empty." << std::endl;
      failures.push_back(file);
      continue;
    }

    amber::Amber am;
    std::unique_ptr<amber::Recipe> recipe = amber::MakeUnique<amber::Recipe>();

    result = am.Parse(data, recipe.get());
    if (!result.IsSuccess()) {
      std::cerr << "case " << file << ": parse fail" << std::endl;
      std::cerr << result.Error() << std::endl << std::endl;
      failures.push_back(file);
      continue;
    }

    recipe_and_files.push_back(
        std::make_pair(std::move(recipe), std::string(file)));
  }

  if (options.parse_only)
    return 0;

  sample::ConfigHelper config_helper;
  amber::Options amber_options;
  amber_options.engine = options.engine;
  if (options.input_filenames.size() > 1UL) {
    std::vector<const amber::Recipe*> recipes;
    for (const auto& recipe_and_file : recipe_and_files)
      recipes.push_back(recipe_and_file.first.get());

    amber_options.config =
        config_helper.CreateConfig(amber_options.engine, recipes);
  }

  for (const auto& recipe_and_file : recipe_and_files) {
    const auto* recipe = recipe_and_file.first.get();
    const auto& file = recipe_and_file.second;

    amber::Amber am;
    result = am.Execute(recipe, amber_options);
    if (!result.IsSuccess()) {
      std::cerr << "case " << file << ": run fail" << std::endl;
      std::cerr << result.Error() << std::endl << std::endl;
      failures.push_back(file);
      continue;
    }

    if (!options.buffer_filename.empty()) {
      // TODO(dsinclair): Write buffer file
      assert(false);
    }

    if (!options.image_filename.empty()) {
      // TODO(dsinclair): Write image file
      assert(false);
    }
  }

  if (!failures.empty())
    std::cout << "\nSummary of Failures:" << std::endl;

  for (const auto& failure : failures)
    std::cout << "  " << failure << std::endl;

  std::cout << "\nSummary: "
            << (options.input_filenames.size() - failures.size()) << " pass, "
            << failures.size() << " fail" << std::endl;

  config_helper.Shutdown();

  return failures.empty() ? 0 : 1;
}
