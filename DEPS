use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': '5e5e99c041e48a69615eefd123dac23d9d0c7f73',
  'clspv_revision': 'ae7cff6ee4b5e3048e8cd3d01e499d67cae18d5d',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '2749458081b6211fde8fc6de5294f1460798b49b',
  'glslang_revision': 'ebf634bcaa3e46ca8a912ed05b87281c731b2391',
  'googletest_revision': 'd166e09483845b9b6a658dccc3d3dbb293676b62',
  'lodepng_revision': '2e541f53ebedf6ebae375a381ca2fd6d82b460bc',
  'shaderc_revision': '4399459c192085e44270b3dea25c0d3224ad77df',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '38d7fbaad0a376c777fa5ee95338c2c90e02d416',
  'swiftshader_revision': 'ff3d55c60356103457f626a2f39cbda160e84709',
  'vulkan_headers_revision': '0e57fc1cfa56a203efe43e4dfb9b3c9e9b105593',
  'vulkan_validationlayers_revision': '2b19f3916726243552178be6f8d38454bbb1db63',
  'vulkan_loader_revision': '2069798558ec7eb9b489ffc69fd1d27eebb0c84e',
}

deps = {
  'third_party/clspv': Var('google_git') + '/clspv.git@' +
      Var('clspv_revision'),

  'third_party/clspv-llvm': Var('llvm_git') + '/llvm-project.git@' +
      Var('clspv_llvm_revision'),

  'third_party/cpplint': Var('google_git') + '/styleguide.git@' +
      Var('cpplint_revision'),

  'third_party/dxc': Var('microsoft_git') + '/DirectXShaderCompiler.git@' +
      Var('dxc_revision'),

  'third_party/googletest': Var('google_git') + '/googletest.git@' +
      Var('googletest_revision'),

  'third_party/glslang': Var('khronos_git') + '/glslang.git@' +
      Var('glslang_revision'),

  'third_party/lodepng': Var('lvandeve_git') + '/lodepng.git@' +
      Var('lodepng_revision'),

  'third_party/shaderc': Var('google_git') + '/shaderc.git@' +
      Var('shaderc_revision'),

  'third_party/spirv-headers': Var('khronos_git') + '/SPIRV-Headers.git@' +
      Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('khronos_git') + '/SPIRV-Tools.git@' +
      Var('spirv_tools_revision'),

  'third_party/swiftshader': Var('swiftshader_git') + '/SwiftShader.git@' +
      Var('swiftshader_revision'),

  'third_party/vulkan-headers': Var('khronos_git') + '/Vulkan-Headers.git@' +
      Var('vulkan_headers_revision'),

  'third_party/vulkan-validationlayers': Var('khronos_git') + '/Vulkan-ValidationLayers.git@' +
      Var('vulkan_validationlayers_revision'),

  'third_party/vulkan-loader': Var('khronos_git') + '/Vulkan-Loader.git@' +
      Var('vulkan_loader_revision'),
}
