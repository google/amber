#!/bin/bash

# Copyright 2019 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -x

if [[ $1 == "" ]]; then
  echo "Usage: $0 [build directory]"
  exit 1
fi

BUILD_DIR=$(readlink -f $1)
if [[ $(ls $BUILD_DIR 2> /dev/null) == "" ]]; then
  mkdir -p $BUILD_DIR
fi

if [[ $ANDROID_SDK_HOME == "" ]]; then
  echo "Error: ANDROID_SDK_HOME missing, please set env variable e.g.,"
  echo "       $ export ANDROID_SDK_HOME=path/to/Android/SDK"
  exit 1
fi

if [[ $ANDROID_NDK_HOME == "" ]]; then
  echo "Error: ANDROID_NDK_HOME missing, please set env variable e.g.,"
  echo "       $ export ANDROID_NDK_HOME=path/to/Android/NDK"
  exit 1
fi

if [[ $(command -v javac) == "" ]]; then
  echo "Error: Install Java. Recommended version is Java 8."
  exit 1
fi

if [[ $KEY_STORE_PATH == "" ]]; then
  echo "Error: KEY_STORE_PATH missing, please set env variable."
  exit 1
fi

ANDROID_SOURCE_DIR=$(dirname $(readlink -f $0))/../android_sample

APK_NAME=AmberSample.apk
ANDROID_PLATFORM=android-28
ANDROID_BUILD_TOOL_VERSION=28.0.0
ABI=arm64-v8a
BUILD_TYPE=Release

AAPT=$ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/aapt
AAPT_ADD="$AAPT add"
AAPT_PACK="$AAPT package -f -I
           $ANDROID_SDK_HOME/platforms/$ANDROID_PLATFORM/android.jar"

DX="$ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/dx --dex"

JAVAC="javac -classpath
       $ANDROID_SDK_HOME/platforms/$ANDROID_PLATFORM/android.jar
       -sourcepath $BUILD_DIR/gen -d $BUILD_DIR"

mkdir -p $BUILD_DIR/gen $BUILD_DIR/output/lib/$ABI $BUILD_DIR/$BUILD_TYPE

pushd $BUILD_DIR/$BUILD_TYPE
cmake \
  -DANDROID_ABI=$ABI \
  -DANDROID_PLATFORM=$ANDROID_PLATFORM \
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=$BUILD_DIR/output/lib/$ABI \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DANDROID_NDK=$ANDROID_NDK_HOME \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DCMAKE_MAKE_PROGRAM=$(which ninja) \
  -GNinja \
  -DANDROID_TOOLCHAIN=clang \
  -DANDROID_STL=c++_static \
  $ANDROID_SOURCE_DIR
ninja
popd

ANDROID_VULKAN=$ANDROID_NDK_HOME/sources/third_party/vulkan
for f in $(find $ANDROID_VULKAN/src/build-android/jniLibs/$ABI/ -name '*.so')
do
  LINK=$BUILD_DIR/output/lib/$ABI/$(basename $f)
  if [[ $(ls $LINK 2> /dev/null) == "" ]]; then
    ln -s $f $LINK
  fi
done

$AAPT_PACK --non-constant-id -m \
  -M $ANDROID_SOURCE_DIR/AndroidManifest.xml \
  -S $ANDROID_SOURCE_DIR/res \
  -J $BUILD_DIR/gen/ \
  --generate-dependencies

$AAPT_PACK -m \
  -M $ANDROID_SOURCE_DIR/AndroidManifest.xml \
  -A $ANDROID_SOURCE_DIR/assets \
  -S $ANDROID_SOURCE_DIR/res \
  -J "$BUILD_DIR/gen" \
  -F "$BUILD_DIR/$APK_NAME" \
  --shared-lib $BUILD_DIR/output

$JAVAC $BUILD_DIR/gen/com/google/amber/*.java
$DX --output="$BUILD_DIR/classes.dex" $BUILD_DIR

cd $BUILD_DIR
$AAPT_ADD $APK_NAME classes.dex

$ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/apksigner sign \
--min-sdk-version 28 --ks $KEY_STORE_PATH $APK_NAME

echo "Successfully built $BUILD_DIR/$APK_NAME"
