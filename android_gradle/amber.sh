#!/usr/bin/env bash

# Copyright 2019 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

args=()
index=1

# The @Q expansion operator requires Bash 4.4.

for arg in "$@"
do
args+=("-e")
args+=("arg${index}")
args+=("${arg}")
index=$((index+1))
done

adb shell rm -f /sdcard/Android/data/com.google.amber/cache/amber_stdout.txt
adb shell rm -f /sdcard/Android/data/com.google.amber/cache/amber_stderr.txt
adb shell am instrument -w \
  -e stdout /sdcard/Android/data/com.google.amber/cache/amber_stdout.txt \
  -e stderr /sdcard/Android/data/com.google.amber/cache/amber_stderr.txt \
  "${args[@]@Q}" \
  com.google.amber.test/androidx.test.runner.AndroidJUnitRunner
adb shell cat /sdcard/Android/data/com.google.amber/cache/amber_stdout.txt
adb shell cat /sdcard/Android/data/com.google.amber/cache/amber_stderr.txt
