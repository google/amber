use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': '6f3effbbf054e75039030d389752608efd5a0221',
  'clspv_revision': '50e3cd23a763372a0ccf5c9bbfc21b6c5e2df57d',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '0532d8cd08c9535af2ca5407a3c6289766312f53',
  'glslang_revision': '3ed344dd784ecbbc5855e613786f3a1238823e56',
  'googletest_revision': '8b4817e3df3746a20502a84580f661ac448821be',
  'lodepng_revision': '2febfe0d105822575328759dd950c8a24b0ad6b3',
  'shaderc_revision': 'f53792645f0696b8954cfdb3c213f96799dd89b2',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '323a81fc5e30e43a04e5e22af4cba98ca2a161e6',
  'swiftshader_revision': '4fd7fccd6db5ebb7a1f9cc7853afc2b6fea16b97',
  'vulkan_headers_revision': '881bbb347a08d1b5aa77f61a52a30b506de9f2bf',
  'vulkan_validationlayers_revision': '41a96d15fd99c024f6df17761c2c46c31a5fa9ae',
  'vulkan_loader_revision': 'af8c7e040f93a58e368ce477a409238d9728f46c',
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
