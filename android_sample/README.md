# Build

### Set up Android SDK path

Add `sdk.dir = path/to/Android/SDK` to `local.properties`
or run `export ANDROID_SDK_HOME=path/to/Android/SDK` in your shell.

### Set up Android NDK path

Add `ndk.dir = path/to/Android/NDK` to `local.properties`
or run `export ANDROID_NDK_HOME=path/to/Android/NDK` in your shell.

### Install

Use gradlew commands e.g.,

```
./gradlew installDebug
```

### License

- Following files were copied from [Google Vulkan tutorial for Android](
https://github.com/googlesamples/android-vulkan-tutorials) and updated:
app/CMakeLists.txt, app/build.gradle, app/src/main/jni/vulkan\_wrapper.cpp,
app/src/main/jni/vulkan\_wrapper.h, app/src/main/AndroidManifest.xml.
These files follow Apache License, Version 2.0. See License.txt.

- Other files has the same license as Amber does.
