#!/bin/bash

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
       -sourcepath build/gen -d build"

ANDROID_CMAKE=`find $ANDROID_SDK_HOME -name 'cmake' -executable -type f`
ANDROID_NINJA=`find $ANDROID_SDK_HOME -name 'ninja' -executable -type f`

KEYTOOL_PASSWORD=changeit

mkdir -p build/gen build/output/lib/$ABI build/$BUILD_TYPE

echo "Step 1:"
echo "Run cmake to build build/output/lib/$ABI/libamber_android.so ......"
echo

pushd build/$BUILD_TYPE
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
  build/output/lib/$ABI

echo
echo "Step 3:"
echo "Generate R.java and build/pack it to build/$APK_NAME ......"
echo

$AAPT_PACK --non-constant-id -m \
  -M AndroidManifest.xml \
  -S res \
  -J build/gen/ \
  --generate-dependencies

$AAPT_PACK -m \
  -M AndroidManifest.xml \
  -A assets \
  -S res \
  -J "build/gen" \
  -F "build/$APK_NAME" \
  --shared-lib build/output

$JAVAC build/gen/com/google/amber/*.java
$DX --output="build/classes.dex" build

cd build
$AAPT_ADD $APK_NAME classes.dex

if [[ `ls keystore.jks` == "" ]]; then
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
echo "Sign build/$APK_NAME ......"
echo

echo $KEYTOOL_PASSWORD | \
  $ANDROID_SDK_HOME/build-tools/$ANDROID_BUILD_TOOL_VERSION/apksigner sign \
  --min-sdk-version 28 --ks keystore.jks $APK_NAME

echo
echo "Done. Your app must be build/$APK_NAME"
