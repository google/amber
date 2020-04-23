#!/bin/bash
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

set -e  # fail on error
set -x  # show commands

BUILD_ROOT=$PWD
SRC=$PWD/github/amber
CONFIG=$1
COMPILER=$2

BUILD_TYPE="Debug"

CMAKE_C_CXX_COMPILER=""
if [ $COMPILER = "clang" ]
then
  sudo ln -s /usr/bin/clang-3.8 /usr/bin/clang
  sudo ln -s /usr/bin/clang++-3.8 /usr/bin/clang++
  CMAKE_C_CXX_COMPILER="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
fi

# Possible configurations are:
# DEBUG, RELEASE

if [ $CONFIG = "RELEASE" ]
then
  BUILD_TYPE="RelWithDebInfo"
fi

# removing the old version
echo y | sudo apt-get purge --auto-remove cmake

# Update CMake
sudo apt purge -y --auto-remove cmake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main'
sudo apt-get -qq update
sudo apt-get -qq install -y cmake

echo $(date): $(cmake --version)

# Get ninja
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip -q ninja-linux.zip
export PATH="$PWD:$PATH"

# Make a directory for Dawn dependencies
mkdir -p $SRC/build/out/dawn-deps && cd $SRC/build/out/dawn-deps

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

cd $SRC
./tools/git-sync-deps

cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...

cmake -GNinja ..\
 $CMAKE_C_CXX_COMPILER\
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
