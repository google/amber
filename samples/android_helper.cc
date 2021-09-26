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

extern int android_main(int argc, const char** argv);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

extern "C" JNIEXPORT JNICALL int Java_com_google_amber_Amber_androidHelper(
    JNIEnv* env,
    jobject,
    jobjectArray args,
    jstring stdoutFile,
    jstring stderrFile) {
  const char* stdout_file_cstr = env->GetStringUTFChars(stdoutFile, nullptr);
  const char* stderr_file_cstr = env->GetStringUTFChars(stderrFile, nullptr);

  // Redirect std output to a file
  freopen(stdout_file_cstr, "w", stdout);
  freopen(stderr_file_cstr, "w", stderr);

  env->ReleaseStringUTFChars(stdoutFile, stdout_file_cstr);
  env->ReleaseStringUTFChars(stderrFile, stderr_file_cstr);

  jsize arg_count = env->GetArrayLength(args);

  std::vector<std::string> argv_string;
  argv_string.push_back("amber");

  for (jsize i = 0; i < arg_count; i++) {
    jstring js = static_cast<jstring>(env->GetObjectArrayElement(args, i));
    const char* arg_cstr = env->GetStringUTFChars(js, nullptr);
    argv_string.push_back(arg_cstr);
    env->ReleaseStringUTFChars(js, arg_cstr);
  }

  std::vector<const char*> argv;

  for (const std::string& arg : argv_string)
    argv.push_back(arg.c_str());

  return android_main(static_cast<int>(argv.size()), argv.data());
}

#pragma clang diagnostic pop
