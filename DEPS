use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '5e111c5df8efde39c62d5e6906f590311782e30b',
  'clspv_revision': '616ffa2804e18513f608e83c8d3cf778353ba562',
  'cppdap_revision': 'c9187480d12ed094d4cea06e4da40b54ce2c8aaf',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '7e780aef6fc71936d8f3a6fa11a63e66fb349236',
  'glslang_revision': 'd39b8afc47a1f700b5670463c0d1068878acee6f',
  'googletest_revision': '859bfe8981d6724c4ea06e73d29accd8588f3230',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': '486d165ed70999cd575a9996a7f2551c7b238c81',
  'shaderc_revision': 'b4a735cc1adadd57068c083989e5a345fab4a250',
  'spirv_headers_revision': 'ed09fc149cbecb97ebba7d34e7a4aa2f170a01c6',
  'spirv_tools_revision': 'f050cca7ec474fc71873f4d68375d3916c969322',
  'swiftshader_revision': '471c120ac92a584e1007f5a3cf22619647bc90a1',
  'vulkan_headers_revision': 'db1a98c6cc430725669ea10eb6a35b3584d5f3ab',
  'vulkan_loader_revision': '006586926adece57adea3e006140b5df19826371',
  'vulkan_validationlayers_revision': '500f77901f6d8d077dc268dfe18cc5266ab27bf2',
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
