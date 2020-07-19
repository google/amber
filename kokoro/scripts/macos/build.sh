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
BUILD_TYPE=$1
shift
EXTRA_CONFIG=$@

# Get ninja
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-mac.zip
unzip -q ninja-mac.zip
chmod +x ninja
export PATH="$PWD:$PATH"

echo $(date): $(cmake --version)

DEPS_ARGS=""
if [[ "$EXTRA_CONFIG" =~ "ENABLE_SWIFTSHADER=TRUE" ]]; then
  DEPS_ARGS+=" --with-swiftshader"
fi

cd $SRC
./tools/git-sync-deps $DEPS_ARGS

mkdir build && cd $SRC/build

CMAKE_C_CXX_COMPILER="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...
cmake -GNinja -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_C_CXX_COMPILER \
  -DAMBER_USE_LOCAL_VULKAN=1 \
  -DAMBER_ENABLE_SWIFTSHADER=1 \
  ..

echo $(date): Build everything...
ninja
echo $(date): Build completed.

echo $(date): Starting amber_unittests...
./amber_unittests
echo $(date): amber_unittests completed.

# Tests currently fail on Debug build on the bots
if [[ "$EXTRA_CONFIG" =~ "ENABLE_SWIFTSHADER=TRUE" ]]; then
  echo $(date): Starting integration tests..
  export LD_LIBRARY_PATH=build/third_party/vulkan-loader/loader
  export VK_LAYER_PATH=build/third_party/vulkan-validationlayers/layers
  export VK_ICD_FILENAMES=build/Darwin/vk_swiftshader_icd.json
  cd $SRC
  ./tests/run_tests.py --build-dir $SRC/build --use-swiftshader
  echo $(date): integration tests completed.
fi
