use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'martinus_git': 'https://github.com/martinus',

  'clspv_llvm_revision': '1f1c71aeacc1c4eab385c074714508b6e7121f73',
  'clspv_revision': 'a0d39a143135ce0a9610298b123ad1f8fb798132',
  'cppdap_revision': '88e89520148b2f95e17ca9348587a28215ffc921',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '5fba0c36b35685bf54a08676f9cb9335bc846b26',
  'glslang_revision': '6624e1367309630b2f6df3cf93a5f864e89973f9',
  'googletest_revision': '16f637fbf4ffc3f7a01fa4eceb7906634565242f',
  'json_revision': '4f8fba14066156b73f1189a2b8bd568bde5284c5',
  'lodepng_revision': '5601b8272a6850b7c5d693dd0c0e16da50be8d8d',
  'shaderc_revision': 'e72186b66bb90ed06aaf15cbdc9a053581a0616b',
  'spirv_headers_revision': 'b8047fbe45f426f5918fadc67e8408f5b108c3c9',
  'spirv_tools_revision': '8a40f6de57d7b78bc431678d90aa8a570d1631f2',
  'swiftshader_revision': '0fa19bd6c285e8ad3459a3f58ca903ceb2d7ab00',
  'vulkan_headers_revision': '0873a22a11ec7e3f13762900ad0da39206189886',
  'vulkan_loader_revision': '0fd1cf08c9b10db8f7de99290b4eb88662112725',
  'vulkan_validationlayers_revision': '610d3a664792372540d3093e0ffe372318b1e437',
  'robin_hood_hashing_revision': '24b3f50f9532153edc23b29ae277dcccfd75a462',
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

  'third_party/robin-hood-hashing': Var('martinus_git') + '/robin-hood-hashing.git@' +
      Var('robin_hood_hashing_revision'),
}
