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

#include <jni.h>

#include <sstream>
#include <string>
#include <vector>

extern int main(int argc, const char** argv);

extern "C" JNIEXPORT JNICALL int Java_com_google_amber_Amber_androidMain(
    JNIEnv* env,
    jobject,
    jstring args_jstring) {
  const char* args_cstr = env->GetStringUTFChars(args_jstring, NULL);
  std::string args(args_cstr);

  // Parse argument string
  std::stringstream ss(args);
  std::vector<std::string> argv_string{std::istream_iterator<std::string>{ss},
                                       std::istream_iterator<std::string>{}};
  std::vector<const char*> argv;
  argv.push_back("amber");

  std::string stdout_file = "/sdcard/amber_stdout.txt";
  std::string stderr_file = "/sdcard/amber_stderr.txt";

  for (size_t i = 0; i < argv_string.size(); i++) {
    auto s = argv_string[i];

    if (s == "--stdout") {
      i++;
      stdout_file = argv_string[i];
      continue;
    }

    if (s == "--stderr") {
      i++;
      stderr_file = argv_string[i];
      continue;
    }

    argv.push_back(argv_string[i].c_str());
  }

  // Redirect std output to a file
  freopen(stdout_file.c_str(), "w", stdout);
  freopen(stderr_file.c_str(), "w", stderr);

  auto ret = main(argv.size(), argv.data());

  env->ReleaseStringUTFChars(args_jstring, args_cstr);

  return ret;
}
