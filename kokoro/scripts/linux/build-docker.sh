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

DEPS_ARGS=""
if [[ "$EXTRA_CONFIG" =~ "USE_CLSPV=TRUE" ]]; then
  DEPS_ARGS+=" --with-clspv"
fi
if [[ "$EXTRA_CONFIG" =~ "USE_DXC=TRUE" ]]; then
  DEPS_ARGS+=" --with-dxc"
fi
if [[ "$EXTRA_CONFIG" =~ "ENABLE_SWIFTSHADER=TRUE" ]]; then
  DEPS_ARGS+=" --with-swiftshader"
fi

cd $ROOT_DIR
./tools/git-sync-deps $DEPS_ARGS

mkdir -p build
cd $ROOT_DIR/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...
cmake -GNinja -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
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
  export LD_LIBRARY_PATH=$ROOT_DIR/build/third_party/vulkan-loader/loader
  export VK_LAYER_PATH=$ROOT_DIR/build/third_party/vulkan-validationlayers/layers
  export VK_ICD_FILENAMES=$ROOT_DIR/build/Linux/vk_swiftshader_icd.json
  cd $ROOT_DIR
  python3 ./tests/run_tests.py --build-dir $ROOT_DIR/build --use-swiftshader $OPTS
  echo $(date): integration tests completed.
fi
