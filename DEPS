use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': 'b37f6d3af100dacf550888aef21787c2b2494ad0',
  'clspv_revision': '50e3cd23a763372a0ccf5c9bbfc21b6c5e2df57d',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '0532d8cd08c9535af2ca5407a3c6289766312f53',
  'glslang_revision': 'b0ada80356ca7b560c600b93a596af1331442542',
  'googletest_revision': '8b4817e3df3746a20502a84580f661ac448821be',
  'lodepng_revision': '5a0dba103893e6b8084be13945a826663917d00a',
  'shaderc_revision': 'c6a2ef12dffcd0c13f4d8c90b1a2f23ccda7931a',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '323a81fc5e30e43a04e5e22af4cba98ca2a161e6',
  'swiftshader_revision': '11cb891a01a2b47cc2135ef19ca18046662476e0',
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
