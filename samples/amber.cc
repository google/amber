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
#include <fstream>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "amber/amber.h"
#include "amber/recipe.h"
#include "samples/config_helper.h"
#include "samples/ppm.h"
#include "src/build-versions.h"
#include "src/make_unique.h"

namespace {

struct Options {
  std::vector<std::string> input_filenames;

  std::string image_filename;
  std::string buffer_filename;
  int64_t buffer_binding_index = 0;
  uint32_t engine_major = 1;
  uint32_t engine_minor = 0;
  bool parse_only = false;
  bool disable_validation_layer = false;
  bool show_summary = false;
  bool show_help = false;
  bool show_version_info = false;
  amber::EngineType engine = amber::kEngineTypeVulkan;
  std::string spv_env;
};

const char kUsage[] = R"(Usage: amber [options] SCRIPT [SCRIPTS...]

 options:
  -p                  -- Parse input files only; Don't execute.
  -s                  -- Print summary of pass/failure.
  -d                  -- Disable validation layers.
  -t <spirv_env> -- The target SPIR-V environment. Defaults to SPV_ENV_UNIVERSAL_1_0.
  -i <filename>       -- Write rendering to <filename> as a PPM image.
  -b <filename>       -- Write contents of a UBO or SSBO to <filename>.
  -B <buffer>         -- Index of buffer to write. Defaults buffer 0.
  -e <engine>         -- Specify graphics engine: vulkan, dawn. Default is vulkan.
  -v <engine version> -- Engine version (eg, 1.1 for Vulkan). Default 1.0.
  -V, --version       -- Output version information for Amber and libraries.
  -h                  -- This help text.
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
        opts->engine = amber::kEngineTypeVulkan;
      } else if (engine == "dawn") {
        opts->engine = amber::kEngineTypeDawn;
      } else {
        std::cerr
            << "Invalid value for -e argument. Must be one of: vulkan dawn"
            << std::endl;
        return false;
      }
    } else if (arg == "-t") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -t argument." << std::endl;
        return false;
      }
      opts->spv_env = args[i];
    } else if (arg == "-h" || arg == "--help") {
      opts->show_help = true;
    } else if (arg == "-v") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -v argument." << std::endl;
        return false;
      }
      const std::string& ver = args[i];

      size_t dot_pos = 0;
      int32_t val = std::stoi(ver, &dot_pos);
      if (val < 0) {
        std::cerr << "Version major must be non-negative" << std::endl;
        return false;
      }

      opts->engine_major = static_cast<uint32_t>(val);
      if (dot_pos != std::string::npos && (dot_pos + 1) < ver.size()) {
        val = std::stoi(ver.substr(dot_pos + 1));
        if (val < 0) {
          std::cerr << "Version minor must be non-negative" << std::endl;
          return false;
        }
        opts->engine_minor = static_cast<uint32_t>(val);
      }
    } else if (arg == "-V" || arg == "--version") {
      opts->show_version_info = true;
    } else if (arg == "-p") {
      opts->parse_only = true;
    } else if (arg == "-d") {
      opts->disable_validation_layer = true;
    } else if (arg == "-s") {
      opts->show_summary = true;
    } else if (arg.size() > 0 && arg[0] == '-') {
      std::cerr << "Unrecognized option " << arg << std::endl;
      return false;
    } else if (!arg.empty()) {
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
  struct RecipeData {
    std::string file;
    std::unique_ptr<amber::Recipe> recipe;
  };
  std::vector<RecipeData> recipe_data;
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
      std::cerr << file << ": " << result.Error() << std::endl;
      failures.push_back(file);
      continue;
    }

    recipe_data.emplace_back();
    recipe_data.back().file = file;
    recipe_data.back().recipe = std::move(recipe);
  }

  if (options.parse_only)
    return 0;

  amber::Options amber_options;
  amber_options.engine = options.engine;
  amber_options.spv_env = options.spv_env;

  std::set<std::string> required_features;
  std::set<std::string> required_extensions;
  for (const auto& recipe_data_elem : recipe_data) {
    const auto features = recipe_data_elem.recipe->GetRequiredFeatures();
    required_features.insert(features.begin(), features.end());

    const auto extensions = recipe_data_elem.recipe->GetRequiredExtensions();
    required_extensions.insert(extensions.begin(), extensions.end());
  }

  sample::ConfigHelper config_helper;
  std::unique_ptr<amber::EngineConfig> config;

  amber::Result r = config_helper.CreateConfig(
      amber_options.engine, options.engine_major, options.engine_minor,
      std::vector<std::string>(required_features.begin(),
                               required_features.end()),
      std::vector<std::string>(required_extensions.begin(),
                               required_extensions.end()),
      options.disable_validation_layer, &config);

  if (!r.IsSuccess()) {
    std::cout << r.Error() << std::endl;
    return 1;
  }

  amber_options.config = config.get();

  if (!options.buffer_filename.empty()) {
    // TODO(dsinclair): Write buffer file
    assert(false);
  }

  if (!options.image_filename.empty()) {
    amber::BufferInfo buffer_info;
    buffer_info.buffer_name = "framebuffer";
    amber_options.extractions.push_back(buffer_info);
  }

  for (const auto& recipe_data_elem : recipe_data) {
    const auto* recipe = recipe_data_elem.recipe.get();
    const auto& file = recipe_data_elem.file;

    amber::Amber am;
    result = am.Execute(recipe, &amber_options);
    if (!result.IsSuccess()) {
      std::cerr << file << ": " << result.Error() << std::endl;
      failures.push_back(file);
      continue;
    }

    if (!options.image_filename.empty()) {
      std::string image;
      for (amber::BufferInfo buffer_info : amber_options.extractions) {
        if (buffer_info.buffer_name == "framebuffer") {
          std::tie(result, image) = ppm::ConvertToPPM(
              buffer_info.width, buffer_info.height, buffer_info.values);
          break;
        }
      }
      if (!result.IsSuccess()) {
        std::cerr << result.Error() << std::endl;
        continue;
      }
      std::ofstream image_file;
      image_file.open(options.image_filename, std::ios::out | std::ios::binary);
      if (!image_file.is_open()) {
        std::cerr << "Cannot open file for image dump: ";
        std::cerr << options.image_filename << std::endl;
        continue;
      }
      image_file << image;
      image_file.close();
    }
  }

  if (options.show_summary) {
    if (!failures.empty()) {
      std::cout << "\nSummary of Failures:" << std::endl;

      for (const auto& failure : failures)
        std::cout << "  " << failure << std::endl;
    }

    std::cout << "\nSummary: "
              << (options.input_filenames.size() - failures.size()) << " pass, "
              << failures.size() << " fail" << std::endl;
  }

  config_helper.Shutdown();

  return !failures.empty();
}
