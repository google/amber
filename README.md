# Amber

Amber is a multi-API shader test framework.

Amber lets you capture and communicate shader bugs with the fluidity and ease of
a scripting flow:

* No graphics API programming is required.
  * WIP: Supports Vulkan and [Dawn][Dawn] graphics APIs.
* A single text string (or file) maps to a single graphics API pipeline test
  case. The text includes:
  * Input data, including buffers and images.
  * Shaders.
  * Expectations for the result of running the pipeline.
* Shaders can be expressed in binary form (as hex), in SPIR-V assembly, or in a
  higher level shader language.
* After executing the pipeline, result buffers and images can be saved to output
  files.

Amber is influenced by [Talvos][Talvos] and [VkRunner][VkRunner].
The [VkScript](docs/vk_script.md) syntax matches the format used by VkRunner.

This is not an officially supported Google product.

## Writing Amber Tests
Working with Amber involves writing input test files. Some example files can be
see in the [tests/cases](tests/cases) folder.

The main input format is [Amberscript](docs/amber_script.md). New features are
added to AmberScript as Amber is enhanced. This is the preferred format in which
new script files are written.

### Clear test as AmberScript

```groovy
SHADER vertex vtex_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
#version 430

layout(location = 0) in vec4 color_in;
layout(location = 0) out vec4 color_out;

void main() {
  color_out = color_in;
}
END

BUFFER img_buf FORMAT B8G8R8A8_UNORM

PIPELINE graphics my_pipeline
  ATTACH vtex_shader
  ATTACH frag_shader

  FRAMEBUFFER_SIZE 256 256
  BIND BUFFER img_buf AS color LOCATION 0
END

CLEAR my_pipeline
EXPECT img_buf IDX 0 0 SIZE 256 256 EQ_RGBA 0 0 0 0
```

The [VkScript](docs/vk_script.md) format is supported for historic reasons. It
is based off, and very closely matches, the format accepted by VkRunner. There
are no new features being added to VkScript, it is for historical use.

### Clear test as VkScript

```
[require]
VK_KHR_get_physical_device_properties2

[vertex shader passthrough]

[fragment shader]
#version 430

layout(location = 0) in vec4 color_in;
layout(location = 0) out vec4 color_out;

void main() {
  color_out = color_in;
}

[test]
clear
relative probe rect rgba (0.0, 0.0, 1.0, 1.0) (0, 0, 0, 0)
```

## Requirements

 * Recommended: Configure at least one backend. See [Backends](#backends) below.
 * Git
 * CMake (version 3.7+ enables automatic discovery of an installed Vulkan SDK)
 * Ninja (or other build tool)
 * Python, for fetching dependencies and building Vulkan wrappers


## Building
```
git clone https://github.com/google/amber.git
cd amber
./tools/git-sync-deps
mkdir -p out/Debug
cd out/Debug
cmake -GNinja ../..
ninja
```

Alternatives:

* On Windows, Amber normally statically links against the C runtime library.
  To override this and link against a shared C runtime, CMake option
  `-DAMBER_ENABLE_SHARED_CRT`.
  This will cause Amber to be built with `/MD` for release builds or `/MDd` for
  debug builds.

### Android

* Android build needs Android SDK 28, Android NDK 16, Java 8. If you prefer
  other versions of Android SDK, Android NDK, Java, then you can change
  `ANDROID_PLATFORM` and `ANDROID_BUILD_TOOL_VERSION` in
  `tools/build-amber-sample.sh`.
* Set up Android SDK path by running
  `export ANDROID_SDK_HOME=path/to/Android/SDK` in your shell.
* Set up Android NDK path by running
  `export ANDROID_NDK_HOME=path/to/Android/NDK` in your shell.
* Generate a KeyStore using `keytool` command and set up `KEY_STORE_PATH`
  env variable for the KeyStore file path.
* Run `./tools/build-amber-sample.sh [build output directory path]`.

#### Android plain executable

It is possible to obtain a plain executable for Android, as opposed to an APK,
with the following:

```
git clone https://github.com/google/amber.git
cd amber
./tools/git-sync-deps

./tools/update_build_version.py . samples/ third_party/
./tools/update_vk_wrappers.py . .

mkdir build
cd build
mkdir app
mkdir libs
${ANDROID_NDK_HOME}/ndk-build -C ../samples NDK_PROJECT_PATH=. NDK_LIBS_OUT=`pwd`/libs NDK_APP_OUT=`pwd`/app
```

The list of target ABIs can be configured in `samples/jni/Application.mk` by
editing the APP_ABI entry:

```
APP_ABI := arm64-v8a armeabi-v7a x86 x86_64
```

The resulting executable will be produced as
`build/app/local/<abi>/amber_ndk`. This executable can be run via the adb shell
on your device, e.g. under `/data/local/tmp` (`/sdcard` is generally not
suitable because it is mounted with a non-executable flag). Also, vulkan layers
may not be available to this executable as it is not an app, so make sure to use
the `-d` flag to disable Vulkan layers:

```
adb push build/app/local/<abi>/amber_ndk /data/local/tmp
adb shell
# Now on device shell
cd /data/local/tmp
./amber_ndk -d <shader-test-files>
```

### Optional Components

The components which build up Amber can be enabled or disabled as needed. Any
option with `_SKIP_` in the name is on by default, any with `_USE_` is off by
default.

The available flags which can be defined are:
 * AMBER_SKIP_TESTS -- Skip building Amber unit tests
 * AMBER_SKIP_SAMPLES -- Skip building the Amber sample applications
 * AMBER_SKIP_SPIRV_TOOLS -- Disable the SPIRV-Tools integration
 * AMBER_SKIP_SHADERC -- Disable the ShaderC integration
 * AMBER_SKIP_LODEPNG -- Disable the LodePNG integration
 * AMBER_USE_DXC -- Enables DXC as a shader compiler
 * AMBER_USE_LOCAL_VULKAN -- Does not try to find the Vulkan SDK, builds needed
                             components locally
 * AMBER_USE_CLSPV -- Enables CLSPV as a shader compiler
 * AMBER_USE_SWIFTSHADER -- Builds Swiftshader so it can be used as a Vulkan ICD

```
cmake -DAMBER_SKIP_TESTS=True -DAMBER_SKIP_SPIRV_TOOLS=True -GNinja ../..
```

#### DXC

DXC can be enabled in Amber by adding the `-DAMBER_USE_DXC=true` flag when
running cmake.

## Build Bots

There are a number of build bots to verify Amber continues to compile and run
on the various targets. Due to bot limitations, the integration tests are not
being run on the bots, just the unit tests.

## Backends

Amber is designed to run against different graphics APIs.
Amber will build if no graphics API is found, but will only allow verifying the
syntax of the amber script files.

Currently the Vulkan and Dawn graphics APIs are supported.

### Using Vulkan as a backend

A Vulkan implementation is found by CMake in the following priority order:

 * If `AMBER_USE_LOCAL_VULKAN` is enable the headers, loader and layers will be
   built locally and not found on the system.
 * If an enclosing CMake project includes the
   [Vulkan-Headers][Vulkan-Headers]
   CMake project, then headers will be picked up from there.

   In this case the CMake variable `Vulkan_LIBRARIES` can name the
   Vulkan library, or a default of `vulkan` will be used.

 * If you have CMake 3.7 or later, then the Vulkan implementation will
   be found from a Vulkan SDK as published by LunarG.

   Environment variables:
   * `VULKAN_SDK` should point to the platform-specific SDK directory
     that contains the `include` and `lib` directories.
   * `VK_ICD_FILENAMES` should point to the ICD JSON file.
   * `VK_LAYER_PATH` should point to the explicit\_layer.d folder.
   * `LD_LIBRARY_PATH` must contain the $VULKAN_SDK/lib/ folder for the
     validation libraries.

   ```
   export VULKAN_SDK=$HOME/vulkan-macos-1.1.85.0/macOS
   export VK_ICD_FILENAMES=$VULKAN_SDK/etc/vulkan/icd.d/MoltenVK_icd.json
   export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d
   export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
   ```

### Using Dawn as a backend

We assume you have built [Dawn][Dawn] from source, and have access to both the
source and build trees. To build a Dawn backend for Amber, set the following
CMake variables when configuring Amber:

  * `Dawn_INCLUDE_DIR`: The directory containing `dawn/dawn_export.h`
    (in the source tree).
  * `Dawn_GEN_INCLUDE_DIR`: The directory containing generated header
    `dawn/dawncpp.h` (in the build output tree).
  * `Dawn_LIBRARY_DIR`: The directory containing the `dawn_native` library (in
    the build output tree).

## Amber Samples

The build will generate an `out/Debug/amber` executable which can be used to
run amber scripts. The script can be used as
`out/Debug/amber <path to amber file>`. Where, currently, the amber file is
in the [VkScript](docs/vk_script.md) format.

```
out/Debug/amber tests/cases/clear.amber
```

The sample app returns a value of 0 on success or non-zero on error. Any issues
encountered should be displayed on the console.

By default, `out/Debug/amber` supports saving the output image into '.png'
file. You can disable this by passing `-DAMBER_SKIP_LODEPNG=true` to cmake.

The `image_diff` program will also be created. This allows comparing two images
using the Amber buffer comparison methods.

## Contributing

Please see the [CONTRIBUTING](CONTRIBUTING.md) and
[CODE_OF_CONDUCT](CODE_OF_CONDUCT.md) files on how to contribute to Amber.


[Dawn]: https://dawn.googlesource.com/dawn/
[Talvos]: https://talvos.github.io/
[Vulkan-Headers]: https://github.com/KhronosGroup/Vulkan-Headers
[VkRunner]: https://github.com/igalia/vkrunner

### Using SwiftShader as a backend

SwiftShader if installed it can be used by by exporting the `VK_ICD_FILENAMES`
environment variable and using it directly. If SwiftShader is not installed it
can be built with Amber by setting `AMBER_ENABLE_SWIFTSHADER` during the
configure step of CMake.


```
mkdir out/sw
cd out/sw
cmake -GNinja -DAMBER_ENABLE_SWIFTSHADER=TRUE ../..
ninja
export VK_ICD_FILENAMES=$PWD/Linux/vk_swiftshader_icd.json
./amber -d -V    # Should see SwiftShader listed as device
./amber -d ../../tests/cases/clear.amber
```
