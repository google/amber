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

Amber, by default, enables testing, SPIRV-Tools and Shaderc. Each of these can
be disabled by using the appropriate flags to CMake. Note, disabling SPIRV-Tools
will disable Shaderc automatically.

The available flags which can be defined are:
 * AMBER_SKIP_TESTS
 * AMBER_SKIP_SPIRV_TOOLS
 * AMBER_SKIP_SHADERC

```
cmake -DAMBER_SKIP_TESTS=True -DAMBER_SKIP_SPIRV_TOOLS=True -GNinja ../..
```

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

 * First: If an enclosing CMake project includes the
   [Vulkan-Headers][Vulkan-Headers]
   CMake project, then headers will be picked up from there.

   In this case the CMake variable `Vulkan_LIBRARIES` can name the
   Vulkan library, or a default of `vulkan` will be used.

 * Second: If you have CMake 3.7 or later, then the Vulkan implementation will
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

## Amber Sample

The build will generate an `out/Debug/amber` executable which can be used to
run amber scripts. The script can be used as
`out/Debug/amber <path to amber file>`. Where, currently, the amber file is
in the [VkScript](docs/vk_script.md) format.

```
out/Debug/amber tests/cases/clear.vkscript
```

The sample app returns a value of 0 on success or non-zero on error. Any issues
encountered should be displayed on the console.

## Contributing

Please see the [CONTRIBUTING](CONTRIBUTING.md) and
[CODE_OF_CONDUCT](CODE_OF_CONDUCT.md) files on how to contribute to Amber.


[Dawn]: https://dawn.googlesource.com/dawn/
[Talvos]: https://talvos.github.io/
[Vulkan-Headers]: https://github.com/KhronosGroup/Vulkan-Headers
[VkRunner]: https://github.com/igalia/vkrunner
