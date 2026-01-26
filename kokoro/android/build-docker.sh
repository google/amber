#!/bin/bash
# Copyright 2025 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
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
# Opt out of the ownership check; we need this for the Amber repo and
# third_party/spirv-tools and possibly others.
git config --global --add safe.directory '*'

. /bin/using.sh
using ndk-r29    #  Also sets ANDROID_NDK_HOME
using ninja-1.10.0
using python-3.12.4
using cmake-3.31.2

./tools/git-sync-deps

TOOLCHAIN_PATH="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
ANDROID_STL="c++_static"
ANDROID_PLATFORM="android-24"
ANDROID_ABI="arm64-v8a"

BUILD_TYPE="Release"

[ -d build ] && rm -rf build
mkdir build
cd build

# Invoke the build.
echo "$(date): Starting build..."
cmake -GNinja \
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" \
    "-DANDROID_ABI=$ANDROID_ABI" \
    "-DANDROID_PLATFORM=$ANDROID_PLATFORM" \
    "-DANDROID_NDK=$ANDROID_NDK_HOME" \
    "-DANDROID_STL=$ANDROID_STL" \
    "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PATH" \
     ..

echo "$(date): Build everything..."
ninja
echo "$(date): Build completed."
