#!/bin/bash

# Copyright 2020 The Amber Authors.
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

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

set -x  # show commands

# Disable git's "detected dubious ownership" error - kokoro checks out the repo with a different
# user, and we don't care about this warning.
git config --global --add safe.directory '*'

using cmake-3.17.2
using ninja-1.10.0

if [ ! -z "$COMPILER" ]; then
    using "$COMPILER"
fi

# Possible configurations are:
# DEBUG, RELEASE

BUILD_TYPE="Debug"
if [ $CONFIG = "RELEASE" ]
then
  BUILD_TYPE="RelWithDebInfo"
fi

# Make a directory for Dawn dependencies
mkdir -p $ROOT_DIR/build/out/dawn-deps && cd $ROOT_DIR/build/out/dawn-deps

# Get depot tools
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PWD/depot_tools:$PATH"

# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn
DAWN=$PWD

# Bootstrap the gclient configuration
cp scripts/standalone.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync

# Generate build files
mkdir -p out/Release
touch out/Release/args.gn
gn gen out/Release

# build dawn
ninja -C out/Release

cd $ROOT_DIR
./tools/git-sync-deps

cd $ROOT_DIR/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...

cmake -GNinja ..\
 -DCMAKE_BUILD_TYPE=$BUILD_TYPE\
 -DDawn_INCLUDE_DIR=$DAWN/src/include\
 -DDawn_GEN_INCLUDE_DIR=$DAWN/out/Release/gen/src/include\
 -DDawn_LIBRARY_DIR=$DAWN/out/Release

echo $(date): Build everything...
ninja
echo $(date): Build completed.

echo $(date): Starting amber_unittests...
./amber_unittests
echo $(date): amber_unittests completed.
