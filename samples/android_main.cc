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
    jobjectArray args,
    jstring stdoutFile,
    jstring stderrFile) {
  const char* stdout_file_cstr = env->GetStringUTFChars(stdoutFile, NULL);
  const char* stderr_file_cstr = env->GetStringUTFChars(stderrFile, NULL);

  // Redirect std output to a file
  freopen(stdout_file_cstr, "w", stdout);
  freopen(stderr_file_cstr, "w", stderr);

  env->ReleaseStringUTFChars(stdoutFile, stdout_file_cstr);
  env->ReleaseStringUTFChars(stderrFile, stderr_file_cstr);

  int arg_count = env->GetArrayLength(args);

  std::vector<std::string> argv_string;
  for (int i = 0; i < arg_count; i++) {
    jstring js = static_cast<jstring>(env->GetObjectArrayElement(args, i));
    const char* arg_cstr = env->GetStringUTFChars(js, NULL);
    argv_string.push_back(arg_cstr);
    env->ReleaseStringUTFChars(js, arg_cstr);
  }

  std::vector<const char*> argv;
  argv.push_back("amber");

  for (size_t i = 0; i < argv_string.size(); i++)
    argv.push_back(argv_string[i].c_str());

  return main(argv.size(), argv.data());
}
