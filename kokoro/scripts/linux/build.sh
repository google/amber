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

# Get ninja
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip -q ninja-linux.zip
export PATH="$PWD:$PATH"

cd $SRC
./tools/git-sync-deps

mkdir build && cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...
cmake -GNinja -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_C_CXX_COMPILER -DAMBER_USE_LOCAL_VULKAN=1 ..

echo $(date): Build everything...
ninja
echo $(date): Build completed.

echo $(date): Starting amber_unittests...
./amber_unittests
echo $(date): amber_unittests completed.

#echo $(date): Starting integration tests..
#../../test/run_tests.py
#echo $(date): integration tests completed.
