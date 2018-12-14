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

 * Recommended: Configure at least one target graphics API. See below.
 * Git
 * CMake
 * Ninja (or other build tool)
 * Recommended: Python, for fetching dependencies


## Building
```
git clone git@github.com:google/amber
cd amber
./tools/git-sync-deps
mkdir -p out/Debug
cd out/Debug
cmake -GNinja ../..
ninja
```

### Android

* Set up Android SDK path by adding `sdk.dir = path/to/Android/SDK` to
`local.properties` or run `export ANDROID_SDK_HOME=path/to/Android/SDK` in your
shell.
* Set up Android NDK path by adding `ndk.dir = path/to/Android/NDK` to
`local.properties` or run `export ANDROID_NDK_HOME=path/to/Android/NDK` in your
shell.
* Run `./gradlew build`.

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
     Example: `VULKAN_SDK=$HOME/vulkan-macos-1.1.85.0/macOS`
   * `VK_ICD_FILENAMES` should point to the ICD JSON file.
     Example: `VK_ICD_FILENAMES=$VULKAN_SDK/etc/vulkan/icd/MoltenVK_icd.json`

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

## Contributing

Please see the [CONTRIBUTING](CONTRIBUTING.md) and
[CODE_OF_CONDUCT](CODE_OF_CONDUCT.md) files on how to contribute to Amber.


[Dawn]: https://dawn.googlesource.com/dawn/
[Talvos]: https://talvos.github.io/
[Vulkan-Headers]: https://github.com/KhronosGroup/Vulkan-Headers
[VkRunner]: https://github.com/igalia/vkrunner
