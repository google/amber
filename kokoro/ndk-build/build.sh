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

# NDK Path
export ANDROID_NDK=/opt/android-ndk-r16b

# Get NINJA.
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip -q ninja-linux.zip
export PATH="$PWD:$PATH"

cd $SRC
./tools/git-sync-deps
./tools/update_build_version.py . samples/ third_party/
./tools/update_vk_wrappers.py . .

mkdir -p build/libs build/app
cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}

echo $(date): Starting ndk-build for android_test ...
$ANDROID_NDK/ndk-build     \
  -C $SRC/android_test     \
  NDK_PROJECT_PATH=.       \
  NDK_LIBS_OUT=`pwd`/libs  \
  NDK_APP_OUT=`pwd`/app    \
  -j8
echo $(date): ndk-build for android_test completed.

echo $(date): Starting ndk-build for samples ...
$ANDROID_NDK/ndk-build     \
  -C $SRC/samples          \
  NDK_PROJECT_PATH=.       \
  NDK_LIBS_OUT=`pwd`/libs  \
  NDK_APP_OUT=`pwd`/app    \
  -j8
echo $(date): ndk-build for samples completed.
