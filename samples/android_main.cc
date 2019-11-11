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

#include <sstream>
#include <string>
#include <vector>

#include <jni.h>

extern int main(int argc, const char** argv);

extern "C" JNIEXPORT JNICALL int Java_com_google_amber_Amber_androidMain(
    JNIEnv* env,
    jobject claz,
    jstring args_jstring) {
  // Redirect std output to a file
  freopen("/sdcard/amberlog.txt", "w", stdout);
  freopen("/sdcard/amberlog.txt", "a", stderr);

  std::string args(env->GetStringUTFChars(args_jstring, NULL));

  // Parse argument string and add -d by default
  std::stringstream ss(args);
  std::vector<std::string> argv_string{std::istream_iterator<std::string>{ss},
                                       std::istream_iterator<std::string>{}};
  std::vector<const char*> argv;
  argv.push_back("amber");
  argv.push_back("-d");

  for (auto s : argv_string)
    argv.push_back(s.c_str());

  return main(argv.size(), argv.data());
}
