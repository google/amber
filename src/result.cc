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

#include "amber/result.h"

#include <sstream>

namespace amber {

Result::Result(const std::string& err) {
  errors_.emplace_back(err);
}

Result& Result::operator+=(const Result& res) {
  errors_.insert(std::end(errors_), std::begin(res.errors_),
                 std::end(res.errors_));
  return *this;
}

Result& Result::operator+=(const std::string& err) {
  errors_.emplace_back(err);
  return *this;
}

std::string Result::Error() const {
  static const char* kNoErrorMsg = "<no error message given>";
  switch (errors_.size()) {
    case 0:
      return "";
    case 1:
      return errors_[0].size() > 0 ? errors_[0] : kNoErrorMsg;
    default: {
      std::stringstream ss;
      ss << errors_.size() << " errors:";
      for (size_t i = 0; i < errors_.size(); i++) {
        auto& err = errors_[i];
        ss << "\n";
        ss << " (" << (i + 1) << ") ";
        ss << (err.size() > 0 ? err : kNoErrorMsg);
      }
      return ss.str();
    }
  }
}

}  // namespace amber
