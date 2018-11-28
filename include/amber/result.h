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

/// Holds the results for an operation.
class Result {
 public:
  /// Creates a result which succeeded.
  Result();
  /// Creates a result which failed and will return |err|.
  explicit Result(const std::string& err);
  Result(const Result&);
  ~Result();

  Result& operator=(const Result&);

  /// Returns true if the result is a success.
  bool IsSuccess() const { return succeeded_; }

  /// Returns the error string if |IsSuccess| is false.
  const std::string& Error() const { return error_; }

 private:
  bool succeeded_;
  std::string error_;
};

}  // namespace amber

#endif  // AMBER_RESULT_H_
