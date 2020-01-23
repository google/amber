use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': 'ce23515f5ab01161c98449d833b3ae013b553aa8',
  'clspv_revision': '50e3cd23a763372a0ccf5c9bbfc21b6c5e2df57d',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '0532d8cd08c9535af2ca5407a3c6289766312f53',
  'glslang_revision': 'b0ada80356ca7b560c600b93a596af1331442542',
  'googletest_revision': '10b1902d893ea8cc43c69541d70868f91af3646b',
  'lodepng_revision': '5a0dba103893e6b8084be13945a826663917d00a',
  'shaderc_revision': '9ad78aa7dba268ef8b808d23b135ea636fd893b1',
  'spirv_headers_revision': 'dc77030acc9c6fe7ca21fff54c5a9d7b532d7da6',
  'spirv_tools_revision': 'ab7ac60f14ae66006bed5c989a2cfd4c4881704c',
  'swiftshader_revision': 'cb4d2c38a3430437b1fceeb1d066e2d98d1916b1',
  'vulkan_headers_revision': '7264358702061d3ed819d62d3d6fd66ab1da33c3',
  'vulkan_validationlayers_revision': '8317b28e672a6bff682f8406575e5daf446a80f3',
  'vulkan_loader_revision': '37d3a235af2cc822c6aaf62616cf19db43d9bc8b',
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
