use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '9244be7b0514c41e0fa8a2880595fa663cc2f85f',
  'clspv_revision': '616ffa2804e18513f608e83c8d3cf778353ba562',
  'cppdap_revision': 'cc93ba9747201007c8ff90e7d924152485462fbc',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'b7626e97eebaff425239a7ac32d3e9698259758d',
  'glslang_revision': '78a3c915a1d70cf236588cc3c88f4db0566494a4',
  'googletest_revision': '859bfe8981d6724c4ea06e73d29accd8588f3230',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': '486d165ed70999cd575a9996a7f2551c7b238c81',
  'shaderc_revision': 'd8eca133b4b18e4fb8b2ab9b9f04dc53d5ce2537',
  'spirv_headers_revision': '11d7637e7a43cd88cfd4e42c99581dcb682936aa',
  'spirv_tools_revision': '636f449e1529a10d259eb7dc37d97192cf2820f8',
  'swiftshader_revision': 'b0c00e8dd6d408fbc2c6a464b95fad93bf0dc812',
  'vulkan_headers_revision': 'db1a98c6cc430725669ea10eb6a35b3584d5f3ab',
  'vulkan_loader_revision': '006586926adece57adea3e006140b5df19826371',
  'vulkan_validationlayers_revision': '515b4041cc32085f99d4ab8f87aa1c40c7450467',
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
