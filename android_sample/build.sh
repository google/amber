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

if [[ $1 == "" ]]; then
  echo "Usage: $0 [build directory]"
  exit 1
fi

BUILD_DIR=$1
if [[ `ls $BUILD_DIR 2> /dev/null` == "" ]]; then
  mkdir -p $BUILD_DIR
fi

if [[ $ANDROID_SDK_HOME == "" ]]; then
  echo "Error: set ANDROID_SDK_HOME environment variable and run it again e.g.,"
  echo "       $ export ANDROID_SDK_HOME=path/to/Android/SDK"
  exit 1
fi

if [[ $ANDROID_NDK_HOME == "" ]]; then
  echo "Error: set ANDROID_NDK_HOME environment variable and run it again e.g.,"
  echo "       $ export ANDROID_NDK_HOME=path/to/Android/NDK"
  exit 1
fi

APK_NAME=Amber.apk
ANDROID_PLATFORM=android-28
ANDROID_BUILD_TOOL_VERSION=28.0.0
ABI=arm64-v8a
BUILD_TYPE=Debug

AAPT=$ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/aapt
AAPT_ADD="$AAPT add"
AAPT_PACK="$AAPT package -f -I
           $ANDROID_SDK_HOME/platforms/$ANDROID_PLATFORM/android.jar"

DX="$ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/dx --dex"

JAVAC="javac -classpath
       $ANDROID_SDK_HOME/platforms/$ANDROID_PLATFORM/android.jar
       -sourcepath $BUILD_DIR/gen -d $BUILD_DIR"

ANDROID_CMAKE=`find $ANDROID_SDK_HOME -name 'cmake' -executable -type f`
ANDROID_NINJA=`find $ANDROID_SDK_HOME -name 'ninja' -executable -type f`

KEYTOOL_PASSWORD=changeit

mkdir -p $BUILD_DIR/gen $BUILD_DIR/output/lib/$ABI $BUILD_DIR/$BUILD_TYPE

echo "Step 1:"
echo "Run cmake to build $BUILD_DIR/output/lib/$ABI/libamber_android.so ......"
echo

pushd $BUILD_DIR/$BUILD_TYPE
$ANDROID_CMAKE \
  -DANDROID_ABI=$ABI \
  -DANDROID_PLATFORM=$ANDROID_PLATFORM \
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=../output/lib/$ABI \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DANDROID_NDK=$ANDROID_NDK_HOME \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DCMAKE_MAKE_PROGRAM=$ANDROID_NINJA \
  -GNinja \
  -DANDROID_TOOLCHAIN=clang \
  -DANDROID_STL=c++_static \
  ../../
$ANDROID_NINJA
popd

echo
echo "Step 2:"
echo "Copy shared libraries for vulkan validation layer ......"
echo

cp $ANDROID_NDK_HOME/sources/third_party/vulkan/src/build-android/jniLibs/$ABI/*.so \
  $BUILD_DIR/output/lib/$ABI

echo
echo "Step 3:"
echo "Generate R.java and build/pack it to $BUILD_DIR/$APK_NAME ......"
echo

$AAPT_PACK --non-constant-id -m \
  -M AndroidManifest.xml \
  -S res \
  -J $BUILD_DIR/gen/ \
  --generate-dependencies

$AAPT_PACK -m \
  -M AndroidManifest.xml \
  -A assets \
  -S res \
  -J "$BUILD_DIR/gen" \
  -F "$BUILD_DIR/$APK_NAME" \
  --shared-lib $BUILD_DIR/output

$JAVAC $BUILD_DIR/gen/com/google/amber/*.java
$DX --output="$BUILD_DIR/classes.dex" $BUILD_DIR

cd $BUILD_DIR
$AAPT_ADD $APK_NAME classes.dex

if [[ `ls keystore.jks 2> /dev/null` == "" ]]; then
  echo
  echo "Extra step:"
  echo "Generate signing key ......"
  echo

  keytool -genkey -alias amber \
    -keyalg RSA \
    -keypass $KEYTOOL_PASSWORD \
    -storepass $KEYTOOL_PASSWORD \
    -keystore keystore.jks
fi

echo
echo "Step 4:"
echo "Sign $BUILD_DIR/$APK_NAME ......"
echo

echo $KEYTOOL_PASSWORD | \
  $ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/apksigner sign \
  --min-sdk-version 28 --ks keystore.jks $APK_NAME

echo
echo "Done. Your app must be $BUILD_DIR/$APK_NAME"
