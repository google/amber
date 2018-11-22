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

#ifndef AMBER_RESULT_H_
#define AMBER_RESULT_H_

#include <string>
#include <vector>

namespace amber {

class Result {
 public:
  Result();
  explicit Result(const std::string& err);
  Result(const Result&);
  ~Result();

  Result& operator=(const Result&);

  bool IsSuccess() const { return succeeded_; }
  const std::string& Error() const { return error_; }

  void SetImageData(const std::vector<uint8_t>& data) { image_data_ = data; }
  const std::vector<uint8_t>& ImageData() const { return image_data_; }

  void SetBufferData(const std::vector<uint8_t>& data) { buffer_data_ = data; }
  const std::vector<uint8_t>& BufferData() const { return buffer_data_; }

 private:
  bool succeeded_;
  std::string error_;
  std::vector<uint8_t> image_data_;
  std::vector<uint8_t> buffer_data_;
};

}  // namespace amber

#endif  // AMBER_RESULT_H_
