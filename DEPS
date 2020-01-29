use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'ac9b2a6297420a461f7b9db9e2dbd67f5f07f301',
  'clspv_revision': '50e3cd23a763372a0ccf5c9bbfc21b6c5e2df57d',
  'cppdap_revision': 'de7dffaf6635ffa3c78553bb6b9e11a50c9b86ad',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '32168eac845b1dca4b0b3bd086434ec1503a6ae7',
  'glslang_revision': '07a55839eed550d84ef62e0c7f503e0d67692708',
  'googletest_revision': '10b1902d893ea8cc43c69541d70868f91af3646b',
  'json_revision': 'ad383f66bc48bac18ddf8785d63ef2c6a32aa770',
  'lodepng_revision': '5a0dba103893e6b8084be13945a826663917d00a',
  'shaderc_revision': '821d564bc7896fdd5d68484e10aac30ea192568f',
  'spirv_headers_revision': 'dc77030acc9c6fe7ca21fff54c5a9d7b532d7da6',
  'spirv_tools_revision': '97f1d485b76303ea7094fa164c0cc770b79f6f78',
  'swiftshader_revision': '5ba2a5b9a43cc07f1d3042d649907ace92e4cb58',
  'vulkan_headers_revision': '7264358702061d3ed819d62d3d6fd66ab1da33c3',
  'vulkan_loader_revision': '44ac9b2f406f863c69a297a77bd23c28fa29e78d',
  'vulkan_validationlayers_revision': 'c51d450ef72c36014e821a289f38f8a5b5ea1010',
}

deps = {
  'third_party/clspv': Var('google_git') + '/clspv.git@' +
      Var('clspv_revision'),

  'third_party/clspv-llvm': Var('llvm_git') + '/llvm-project.git@' +
      Var('clspv_llvm_revision'),

  'third_party/cppdap': Var('google_git') + '/cppdap.git@' +
      Var('cppdap_revision'),

  'third_party/cpplint': Var('google_git') + '/styleguide.git@' +
      Var('cpplint_revision'),

  'third_party/dxc': Var('microsoft_git') + '/DirectXShaderCompiler.git@' +
      Var('dxc_revision'),

  'third_party/googletest': Var('google_git') + '/googletest.git@' +
      Var('googletest_revision'),

  'third_party/json': Var('nlohmann_git') + '/json.git@' +
      Var('json_revision'),

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
