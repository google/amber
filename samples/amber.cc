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

#include "amber/amber.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "amber/recipe.h"
#include "samples/config_helper.h"
#include "samples/ppm.h"
#include "samples/timestamp.h"
#include "src/build-versions.h"
#include "src/make_unique.h"

#if AMBER_ENABLE_LODEPNG
#include "samples/png.h"
#endif  // AMBER_ENABLE_LODEPNG

namespace {

const char* kGeneratedColorBuffer = "framebuffer";

struct Options {
  std::vector<std::string> input_filenames;

  std::vector<std::string> image_filenames;
  std::string buffer_filename;
  std::vector<std::string> fb_names;
  std::vector<amber::BufferInfo> buffer_to_dump;
  uint32_t engine_major = 1;
  uint32_t engine_minor = 0;
  int32_t fence_timeout = -1;
  bool parse_only = false;
  bool pipeline_create_only = false;
  bool disable_validation_layer = false;
  bool quiet = false;
  bool show_help = false;
  bool show_version_info = false;
  bool log_graphics_calls = false;
  bool log_graphics_calls_time = false;
  bool log_execute_calls = false;
  bool disable_spirv_validation = false;
  amber::EngineType engine = amber::kEngineTypeVulkan;
  std::string spv_env;
};

const char kUsage[] = R"(Usage: amber [options] SCRIPT [SCRIPTS...]

 options:
  -p                        -- Parse input files only; Don't execute.
  -ps                       -- Parse input files, create pipelines; Don't execute.
  -q                        -- Disable summary output.
  -d                        -- Disable validation layers.
  -f <value>                -- Sets the fence timeout value to |value|
  -t <spirv_env>            -- The target SPIR-V environment e.g., spv1.3, vulkan1.1.
                               If a SPIR-V environment, assume the lowest version of Vulkan that
                               requires support of that version of SPIR-V.
                               If a Vulkan environment, use the highest version of SPIR-V required
                               to be supported by that version of Vulkan.
                               Use vulkan1.1spv1.4 for SPIR-V 1.4 with Vulkan 1.1.
                               Defaults to spv1.0.
  -i <filename>             -- Write rendering to <filename> as a PNG image if it ends with '.png',
                               or as a PPM image otherwise.
  -I <buffername>           -- Name of framebuffer to dump. Defaults to 'framebuffer'.
  -b <filename>             -- Write contents of a UBO or SSBO to <filename>.
  -B [<desc set>:]<binding> -- Descriptor set and binding of buffer to write.
                               Default is [0:]0.
  -e <engine>               -- Specify graphics engine: vulkan, dawn. Default is vulkan.
  -v <engine version>       -- Engine version (eg, 1.1 for Vulkan). Default 1.0.
  -V, --version             -- Output version information for Amber and libraries.
  --log-graphics-calls      -- Log graphics API calls (only for Vulkan so far).
  --log-graphics-calls-time -- Log timing of graphics API calls timing (Vulkan only).
  --log-execute-calls       -- Log each execute call before run.
  --disable-spirv-val       -- Disable SPIR-V validation.
  -h                        -- This help text.
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
      opts->image_filenames.push_back(args[i]);

    } else if (arg == "-I") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -I argument." << std::endl;
        return false;
      }
      opts->fb_names.push_back(args[i]);

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
      opts->buffer_to_dump.emplace_back();
      opts->buffer_to_dump.back().buffer_name = args[i];
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
    } else if (arg == "-f") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for -f argument." << std::endl;
        return false;
      }

      int32_t val = std::stoi(std::string(args[i]));
      if (val < 0) {
        std::cerr << "Fence timeout must be non-negative" << std::endl;
        return false;
      }
      opts->fence_timeout = val;

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
    } else if (arg == "-ps") {
      opts->pipeline_create_only = true;
    } else if (arg == "-d") {
      opts->disable_validation_layer = true;
    } else if (arg == "-s") {
      // -s is deprecated but still recognized, it inverts the quiet flag.
      opts->quiet = false;
    } else if (arg == "-q") {
      opts->quiet = true;
    } else if (arg == "--log-graphics-calls") {
      opts->log_graphics_calls = true;
    } else if (arg == "--log-graphics-calls-time") {
      opts->log_graphics_calls_time = true;
    } else if (arg == "--log-execute-calls") {
      opts->log_execute_calls = true;
    } else if (arg == "--disable-spirv-val") {
      opts->disable_spirv_validation = true;
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
    fclose(file);
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

class SampleDelegate : public amber::Delegate {
 public:
  SampleDelegate() = default;
  ~SampleDelegate() override = default;

  void Log(const std::string& message) override {
    std::cout << message << std::endl;
  }

  bool LogGraphicsCalls() const override { return log_graphics_calls_; }
  void SetLogGraphicsCalls(bool log_graphics_calls) {
    log_graphics_calls_ = log_graphics_calls;
  }

  bool LogExecuteCalls() const override { return log_execute_calls_; }
  void SetLogExecuteCalls(bool log_execute_calls) {
    log_execute_calls_ = log_execute_calls;
  }

  bool LogGraphicsCallsTime() const override {
    return log_graphics_calls_time_;
  }
  void SetLogGraphicsCallsTime(bool log_graphics_calls_time) {
    log_graphics_calls_time_ = log_graphics_calls_time;
    if (log_graphics_calls_time) {
      // Make sure regular logging is also enabled
      log_graphics_calls_ = true;
    }
  }

  uint64_t GetTimestampNs() const override {
    return timestamp::SampleGetTimestampNs();
  }

 private:
  bool log_graphics_calls_ = false;
  bool log_graphics_calls_time_ = false;
  bool log_execute_calls_ = false;
};

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

    if (options.fence_timeout > -1)
      recipe->SetFenceTimeout(static_cast<uint32_t>(options.fence_timeout));

    recipe_data.emplace_back();
    recipe_data.back().file = file;
    recipe_data.back().recipe = std::move(recipe);
  }

  if (options.parse_only)
    return 0;

  SampleDelegate delegate;
  if (options.log_graphics_calls)
    delegate.SetLogGraphicsCalls(true);
  if (options.log_graphics_calls_time)
    delegate.SetLogGraphicsCallsTime(true);
  if (options.log_execute_calls)
    delegate.SetLogExecuteCalls(true);

  amber::Options amber_options;
  amber_options.engine = options.engine;
  amber_options.spv_env = options.spv_env;
  amber_options.execution_type = options.pipeline_create_only
                                     ? amber::ExecutionType::kPipelineCreateOnly
                                     : amber::ExecutionType::kExecute;
  amber_options.delegate = &delegate;
  amber_options.disable_spirv_validation = options.disable_spirv_validation;

  std::set<std::string> required_features;
  std::set<std::string> required_device_extensions;
  std::set<std::string> required_instance_extensions;
  for (const auto& recipe_data_elem : recipe_data) {
    const auto features = recipe_data_elem.recipe->GetRequiredFeatures();
    required_features.insert(features.begin(), features.end());

    const auto device_extensions =
        recipe_data_elem.recipe->GetRequiredDeviceExtensions();
    required_device_extensions.insert(device_extensions.begin(),
                                      device_extensions.end());

    const auto inst_extensions =
        recipe_data_elem.recipe->GetRequiredInstanceExtensions();
    required_instance_extensions.insert(inst_extensions.begin(),
                                        inst_extensions.end());
  }

  sample::ConfigHelper config_helper;
  std::unique_ptr<amber::EngineConfig> config;

  amber::Result r = config_helper.CreateConfig(
      amber_options.engine, options.engine_major, options.engine_minor,
      std::vector<std::string>(required_features.begin(),
                               required_features.end()),
      std::vector<std::string>(required_instance_extensions.begin(),
                               required_instance_extensions.end()),
      std::vector<std::string>(required_device_extensions.begin(),
                               required_device_extensions.end()),
      options.disable_validation_layer, options.show_version_info, &config);

  if (!r.IsSuccess()) {
    std::cout << r.Error() << std::endl;
    return 1;
  }

  amber_options.config = config.get();

  if (!options.buffer_filename.empty()) {
    // Have a filename to dump, but no explicit buffer, set the default of 0:0.
    if (options.buffer_to_dump.empty()) {
      options.buffer_to_dump.emplace_back();
      options.buffer_to_dump.back().buffer_name = "0:0";
    }

    amber_options.extractions.insert(amber_options.extractions.end(),
                                     options.buffer_to_dump.begin(),
                                     options.buffer_to_dump.end());
  }

  if (options.image_filenames.size() - options.fb_names.size() > 1) {
    std::cerr << "Need to specify framebuffer names using -I for each output "
                 "image specified by -i."
              << std::endl;
    return 1;
  }

  // Use default frame buffer name when not specified.
  while (options.image_filenames.size() > options.fb_names.size())
    options.fb_names.push_back(kGeneratedColorBuffer);

  for (const auto& fb_name : options.fb_names) {
    amber::BufferInfo buffer_info;
    buffer_info.buffer_name = fb_name;
    buffer_info.is_image_buffer = true;
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
      // Note, we continue after failure to allow dumping the buffers which may
      // give clues as to the failure.
    }

    for (size_t i = 0; i < options.image_filenames.size(); ++i) {
      std::vector<uint8_t> out_buf;
      auto image_filename = options.image_filenames[i];
      auto pos = image_filename.find_last_of('.');
      bool usePNG =
          pos != std::string::npos && image_filename.substr(pos + 1) == "png";
      for (const amber::BufferInfo& buffer_info : amber_options.extractions) {
        if (buffer_info.buffer_name == options.fb_names[i]) {
          if (usePNG) {
#if AMBER_ENABLE_LODEPNG
            result = png::ConvertToPNG(buffer_info.width, buffer_info.height,
                                       buffer_info.values, &out_buf);
#else   // AMBER_ENABLE_LODEPNG
            result = amber::Result("PNG support not enabled");
#endif  // AMBER_ENABLE_LODEPNG
          } else {
            ppm::ConvertToPPM(buffer_info.width, buffer_info.height,
                              buffer_info.values, &out_buf);
            result = {};
          }
          break;
        }
      }
      if (result.IsSuccess()) {
        std::ofstream image_file;
        image_file.open(image_filename, std::ios::out | std::ios::binary);
        if (!image_file.is_open()) {
          std::cerr << "Cannot open file for image dump: ";
          std::cerr << image_filename << std::endl;
          continue;
        }
        image_file << std::string(out_buf.begin(), out_buf.end());
        image_file.close();
      } else {
        std::cerr << result.Error() << std::endl;
      }
    }

    if (!options.buffer_filename.empty()) {
      std::ofstream buffer_file;
      buffer_file.open(options.buffer_filename, std::ios::out);
      if (!buffer_file.is_open()) {
        std::cerr << "Cannot open file for buffer dump: ";
        std::cerr << options.buffer_filename << std::endl;
      } else {
        for (const amber::BufferInfo& buffer_info : amber_options.extractions) {
          // Skip frame buffers.
          if (std::any_of(options.fb_names.begin(), options.fb_names.end(),
                          [&](std::string s) {
                            return s == buffer_info.buffer_name;
                          }) ||
              buffer_info.buffer_name == kGeneratedColorBuffer) {
            continue;
          }

          buffer_file << buffer_info.buffer_name << std::endl;
          const auto& values = buffer_info.values;
          for (size_t i = 0; i < values.size(); ++i) {
            buffer_file << " " << std::setfill('0') << std::setw(2) << std::hex
                        << values[i].AsUint32();
            if (i % 16 == 15)
              buffer_file << std::endl;
          }
          buffer_file << std::endl;
        }
        buffer_file.close();
      }
    }
  }

  if (!options.quiet) {
    if (!failures.empty()) {
      std::cout << "\nSummary of Failures:" << std::endl;

      for (const auto& failure : failures)
        std::cout << "  " << failure << std::endl;
    }

    std::cout << "\nSummary: "
              << (options.input_filenames.size() - failures.size()) << " pass, "
              << failures.size() << " fail" << std::endl;
  }

  return !failures.empty();
}
