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
shift
COMPILER=$1
shift
EXTRA_CONFIG=$@

BUILD_TYPE="Debug"

# Always update gcc so we get a newer standard library.
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get -qq update
sudo aptitude install -y gcc-7 g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 100 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --set gcc "/usr/bin/gcc-7"

CMAKE_C_CXX_COMPILER=""
if [ $COMPILER = "clang" ]
then
  CMAKE_C_CXX_COMPILER="-DCMAKE_C_COMPILER=/usr/bin/clang-5.0 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-5.0"
else
  CMAKE_C_CXX_COMPILER="-DCMAKE_C_COMPILER=/usr/bin/gcc-7 -DCMAKE_CXX_COMPILER=/usr/bin/g++-7"
fi

# Possible configurations are:
# DEBUG, RELEASE

if [ $CONFIG = "RELEASE" ]
then
  BUILD_TYPE="RelWithDebInfo"
fi

# removing the old version
echo y | sudo apt-get purge --auto-remove cmake

# Installing the 3.10.2 version
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

DEPS_ARGS=""
if [[ "$EXTRA_CONFIG" =~ "USE_CLSPV=TRUE" ]]; then
  DEPS_ARGS+=" --with-clspv"
fi
if [[ "$EXTRA_CONFIG" =~ "USE_DXC=TRUE" ]]; then
  DEPS_ARGS+=" --use-dxc"
fi
if [[ "$EXTRA_CONFIG" =~ "ENABLE_SWIFTSHADER=TRUE" ]]; then
  DEPS_ARGS+=" --use-swiftshader"
fi

cd $SRC
./tools/git-sync-deps $DEPS_ARGS

mkdir build && cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...
cmake -GNinja -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_C_CXX_COMPILER \
  -DAMBER_USE_LOCAL_VULKAN=1 \
  $EXTRA_CONFIG ..

echo $(date): Build everything...
ninja
echo $(date): Build completed.

echo $(date): Starting amber_unittests...
./amber_unittests
echo $(date): amber_unittests completed.

# Swiftshader is only built with gcc, so only run the integration tests with gcc
if [[ "$EXTRA_CONFIG" =~ "ENABLE_SWIFTSHADER=TRUE" ]]; then
  OPTS=
  if [[ $EXTRA_CONFIG =~ "USE_CLSPV=ON" ]]; then
    OPTS="--use-opencl"
  fi
  if [[ "$EXTRA_CONFIG" =~ "USE_DXC=TRUE" ]]; then
    OPTS+=" --use-dxc"
  fi

  echo $(date): Starting integration tests..
  export LD_LIBRARY_PATH=build/third_party/vulkan-loader/loader
  export VK_LAYER_PATH=build/third_party/vulkan-validationlayers/layers
  export VK_ICD_FILENAMES=build/Linux/vk_swiftshader_icd.json
  cd $SRC
  ./tests/run_tests.py --build-dir $SRC/build --use-swiftshader $OPTS
  echo $(date): integration tests completed.
fi
