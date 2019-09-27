# Copyright The Amber Authors.
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

set -e  # Fail on error
set -x  # Display commands as run

BUILD_ROOT=$PWD
SRC=$PWD/github/amber
BUILD_TYPE=Release

export ANDROID_NDK=/opt/android-ndk-r15c
ANDROID_STL=c++_static
ANDROID_PLATFORM="android-14"
ANDROID_ABI="armeabi-v7a with NEON"

TOOLCHAIN_PATH=$ANDROID_NDK/build/cmake/android.toolchain.cmake

# removing the old version
echo y | sudo apt-get purge --auto-remove cmake

# Installing the 3.10.2 version
wget http://www.cmake.org/files/v3.10/cmake-3.10.2.tar.gz
tar -xvzf cmake-3.10.2.tar.gz
cd cmake-3.10.2/
./configure
make
sudo make install
echo $(date): $(cmake --version)

# Get NINJA.
wget -q https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip -q ninja-linux.zip
export PATH="$PWD:$PATH"

cd $SRC
./tools/git-sync-deps

mkdir build && cd $SRC/build

# Invoke the build.
BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
echo $(date): Starting build...
cmake -GNinja \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_PLATFORM=$ANDROID_PLATFORM \
    -DANDROID_NDK=$ANDROID_NDK \
    -DANDROID_STL=$ANDROID_STL \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PATH \
     ..

echo $(date): Build everything...
ninja
echo $(date): Build completed.
