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

set -e  # fail on error
set -x  # display commands

BUILD_ROOT=$PWD
SRC=$PWD/github/amber
SRC=$PWD

# NDK Path
export ANDROID_NDK=/opt/android-ndk-r15c

# Get NINJA.
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip -q ninja-linux.zip
export PATH="$PWD:$PATH"

cd $SRC
./tools/git-sync-deps
 
mkdir -p build/libs build/app
cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting ndk-build ...
$ANDROID_NDK/ndk-build \
  -C $SRC/android_test \
  NDK_PROJECT_PATH=.   \
  NDK_LIBS_OUT=./libs  \
  NDK_APP_OUT=./app    \
  -j8

echo $(date): ndk-build completed.
