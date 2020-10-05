use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '394db2259575ef3cac8d3d37836b11eb2373c435',
  'clspv_revision': '86ce19c0130bd13a70862a50a9aa9676eba6548c',
  'cppdap_revision': '1fd23dda91e01550be1a421de307e6fedb2035a9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'b0c029eebda9f8e212d1e447c5b1f8a969bb9db6',
  'glslang_revision': '3ee5f2f1d3316e228916788b300d786bb574d337',
  'googletest_revision': 'a781fe29bcf73003559a3583167fe3d647518464',
  'json_revision': 'fbec662afab55019654e471b65a846a47a696722',
  'lodepng_revision': '34628e89e80cd007179b25b0b2695e6af0f57fac',
  'shaderc_revision': 'ba92b11e1fcaf4c38a64f84d643d6429175bf650',
  'spirv_headers_revision': '3fdabd0da2932c276b25b9b4a988ba134eba1aa6',
  'spirv_tools_revision': 'd91afd8de27a064f938a196fdea2ace234971faa',
  'swiftshader_revision': 'cbb5a10e03fb655a3617988ff5d71bf37892761c',
  'vulkan_headers_revision': '83825d55c7d522931124696ecb07ed48f2693e5c',
  'vulkan_loader_revision': 'bfe4f378aee6a2825b8112429cd46529d936babf',
  'vulkan_validationlayers_revision': 'c3215f8f89bde067386ff8358e0b30774691c8a5',
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
