use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '01966003674d49e06632495fec2a5a7b3fd58a80',
  'clspv_revision': 'b3e2b6d9d8c2b16ae8c48b2c175b3e76268b268c',
  'cppdap_revision': 'cc93ba9747201007c8ff90e7d924152485462fbc',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '316d802e377a39f4cc4c0c2155086051e65863f6',
  'glslang_revision': 'f5ed7a69d5d64bd3ac802712c24995c6c12d23f8',
  'googletest_revision': '356f2d264a485db2fcc50ec1c672e0d37b6cb39b',
  'json_revision': 'fbec662afab55019654e471b65a846a47a696722',
  'lodepng_revision': '34628e89e80cd007179b25b0b2695e6af0f57fac',
  'shaderc_revision': 'd8eca133b4b18e4fb8b2ab9b9f04dc53d5ce2537',
  'spirv_headers_revision': '11d7637e7a43cd88cfd4e42c99581dcb682936aa',
  'spirv_tools_revision': 'bd2a9ea85210d3bb474bc5adb9ff4b0bb536b4fc',
  'swiftshader_revision': '3121585acce20f5cc2074088563e3c1a076b8e48',
  'vulkan_headers_revision': 'db06fce926b0fa5034ed8be30e84fce6fc645e83',
  'vulkan_loader_revision': 'a173c025f8fea41d5d5e6bbd78c84647ce67fd21',
  'vulkan_validationlayers_revision': '5c38b18ab4600465dc324b5c30a193eb20c557f9',
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
