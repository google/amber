#!/bin/bash
# Copyright 2018 The Amber Authors.
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

# ROOT_DIR is set to the repo source root directory
# The current directory is ROOT_DIR.
#
set -e  # fail on error
set -x  # display commands

# Check the assumption about the current directory
grep "Amber is a multi-API shader test framework" README.md >/dev/null

# This is required to run any git command in the docker since owner will
# have changed between the clone environment, and the docker container.
# Marking the root of the repo as safe for ownership changes.
git config --global --add safe.directory $ROOT_DIR

. /bin/using.sh
using ndk-r29
using python-3.12.4

./tools/git-sync-deps
./tools/update_build_version.py . samples/ third_party/
./tools/update_vk_wrappers.py . .

mkdir -p build/libs build/app
cd build

# Invoke the build.

echo "$(date): Starting ndk-build for android_test ..."
  ndk-build                   \
  -C "$ROOT_DIR/android_test" \
  NDK_PROJECT_PATH=.          \
  "NDK_LIBS_OUT=$(pwd)/libs"  \
  "NDK_APP_OUT=$(pwd)/app"    \
  -j8

echo "$(date): ndk-build for android_test completed."

echo "$(date): Starting ndk-build for samples ..."
  ndk-build                   \
  -C "$ROOT_DIR/samples"      \
  NDK_PROJECT_PATH=.          \
  "NDK_LIBS_OUT=$(pwd)/libs"  \
  "NDK_APP_OUT=$(pwd)/app"    \
  -j8
echo "$(date): ndk-build for samples completed."
