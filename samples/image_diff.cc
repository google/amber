// Copyright 2019 The Amber Authors.
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

#include <iostream>
#include <sstream>
#include <vector>

#include "src/buffer.h"
#include "src/format.h"
#include "src/type_parser.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
#include "third_party/lodepng/lodepng.h"
#pragma clang diagnostic pop

namespace {

enum class CompareAlgorithm { kRMSE = 0, kHISTOGRAM_EMD = 1 };

struct Options {
  std::vector<std::string> input_filenames;
  bool show_help = false;
  float tolerance = 1.0f;
  CompareAlgorithm compare_algorithm = CompareAlgorithm::kRMSE;
};

const char kUsage[] = R"(Usage: image_diff [options] image1.png image2.png

Exactly one algorithm (and its parameters) must be specified.

Algorithms:

  --rmse TOLERANCE
               Compare using the Root Mean Square Error (RMSE) algorithm with
               a floating point TOLERANCE value in the range 0..255, where 0
               indicates identical images and 255 indicates images where every
               color channel has the maximum difference.

  --histogram_emd TOLERANCE
               Compare the per-channel color histograms of the images using a
               variant of the Earth Mover's Distance (EMD) algorithm with a
               floating point TOLERANCE value in the range 0.0..1.0, where 0.0
               indicates identical histograms and 1.0 indicates completely
               different histograms for at least one of the color channels.
               E.g. an image with red=255 for every pixel vs. an image with
               red=0 for every pixel.

Other options:

  -h | --help  This help text.
)";

bool ParseArgs(const std::vector<std::string>& args, Options* opts) {
  int num_algorithms = 0;
  for (size_t i = 1; i < args.size(); ++i) {
    const std::string& arg = args[i];
    if (arg == "-h" || arg == "--help") {
      opts->show_help = true;
      return true;
    } else if (arg == "--rmse") {
      num_algorithms++;
      opts->compare_algorithm = CompareAlgorithm::kRMSE;
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing tolerance value for RMSE comparison."
                  << std::endl;
        return false;
      }
      std::stringstream sstream(args[i]);
      sstream >> opts->tolerance;
      if (sstream.fail()) {
        std::cerr << "Invalid tolerance value " << args[i] << std::endl;
        return false;
      }
      if (opts->tolerance < 0 || opts->tolerance > 255) {
        std::cerr << "Tolerance must be in the range 0..255." << std::endl;
        return false;
      }

    } else if (arg == "--histogram_emd") {
      num_algorithms++;
      opts->compare_algorithm = CompareAlgorithm::kHISTOGRAM_EMD;
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing tolerance value for histogram EMD comparison."
                  << std::endl;
        return false;
      }
      std::stringstream sstream(args[i]);
      sstream >> opts->tolerance;
      if (sstream.fail()) {
        std::cerr << "Invalid tolerance value " << args[i] << std::endl;
        return false;
      }
      if (opts->tolerance < 0 || opts->tolerance > 1) {
        std::cerr << "Tolerance must be in the range 0..1." << std::endl;
        return false;
      }
    } else if (!arg.empty()) {
      opts->input_filenames.push_back(arg);
    }
  }
  if (num_algorithms == 0) {
    std::cerr << "No comparison algorithm specified." << std::endl;
    return false;
  } else if (num_algorithms > 1) {
    std::cerr << "Only one comparison algorithm can be specified." << std::endl;
    return false;
  }

  return true;
}

amber::Result LoadPngToBuffer(const std::string& filename,
                              amber::Buffer* buffer) {
  std::vector<unsigned char> image;
  uint32_t width;
  uint32_t height;
  uint32_t error = lodepng::decode(image, width, height, filename.c_str());

  if (error) {
    std::string result = "PNG decode error: ";
    result += lodepng_error_text(error);
    return amber::Result(result);
  }

  std::vector<amber::Value> values;
  values.resize(image.size());
  for (size_t i = 0; i < image.size(); ++i) {
    values[i].SetIntValue(image[i]);
  }

  buffer->SetData(values);

  return {};
}

}  // namespace

int main(int argc, const char** argv) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;

  if (!ParseArgs(args, &options)) {
    return 1;
  }

  if (options.show_help) {
    std::cout << kUsage << std::endl;
    return 0;
  }

  if (options.input_filenames.size() != 2) {
    std::cerr << "Two input file names are required." << std::endl;
    return 1;
  }

  amber::TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UNORM");
  amber::Format fmt(type.get());

  amber::Buffer buffers[2];
  for (size_t i = 0; i < 2; ++i) {
    buffers[i].SetFormat(&fmt);
    amber::Result res =
        LoadPngToBuffer(options.input_filenames[i], &buffers[i]);
    if (!res.IsSuccess()) {
      std::cerr << "Error loading " << options.input_filenames[i] << ": "
                << res.Error() << std::endl;
      return 1;
    }
  }

  amber::Result res;
  if (options.compare_algorithm == CompareAlgorithm::kRMSE)
    res = buffers[0].CompareRMSE(&buffers[1], options.tolerance);
  else if (options.compare_algorithm == CompareAlgorithm::kHISTOGRAM_EMD)
    res = buffers[0].CompareHistogramEMD(&buffers[1], options.tolerance);

  if (res.IsSuccess())
    std::cout << "Images similar" << std::endl;
  else
    std::cout << "Images differ: " << res.Error() << std::endl;

  return !res.IsSuccess();
}
