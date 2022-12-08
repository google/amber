use_relative_paths = True

vars = {
  'cpplint_git':  'https://github.com/cpplint',
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'martinus_git': 'https://github.com/martinus',

  'clspv_llvm_revision': 'b70366c9c430e1eadd59d5a1dfbb9c4d84f83de5',
  'clspv_revision': 'f99809bdab1710846633b4ec24f5448263e75da7',
  'cpplint_revision': 'fa12a0bbdafa15291276ddd2a2dcd2ac7a2ce4cb',
  'dxc_revision': 'c45db48d565a9edc14b025e43b90e62264d06eea',
  'glslang_revision': 'ed257e2bdf34fa2c33946ab120915cd6e038b83d',
  'googletest_revision': '16f637fbf4ffc3f7a01fa4eceb7906634565242f',
  'json_revision': '4f8fba14066156b73f1189a2b8bd568bde5284c5',
  'lodepng_revision': '5601b8272a6850b7c5d693dd0c0e16da50be8d8d',
  'shaderc_revision': '9806490ff5908aec0012f89df342c1a8985b1263',
  'spirv_headers_revision': '0bcc624926a25a2a273d07877fd25a6ff5ba1cfb',
  'spirv_tools_revision': '9c6a925c87d4c6e203e440ecc8a57018e5bb8c4b',
  'swiftshader_revision': 'bca23447ad4667a7b79973569ab5d8d905d211ac',
  'vulkan_headers_revision': '1dace16d8044758d32736eb59802d171970e9448',
  'vulkan_loader_revision': '8aad559a09388ceb5b968af64a2b965d3886e5a0',
  'vulkan_validationlayers_revision': 'a6c1ddca49331d8addde052554487180ee8aec13',
  'robin_hood_hashing_revision': '24b3f50f9532153edc23b29ae277dcccfd75a462',
}

deps = {
  'third_party/clspv': Var('google_git') + '/clspv.git@' +
      Var('clspv_revision'),

  'third_party/clspv-llvm': Var('llvm_git') + '/llvm-project.git@' +
      Var('clspv_llvm_revision'),

  'third_party/cpplint': Var('cpplint_git') + '/cpplint.git@' +
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
