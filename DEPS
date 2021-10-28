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
  'cppdap_revision': '5f3169421ebf151d5583252618a0631e23a89066',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '5fba0c36b35685bf54a08676f9cb9335bc846b26',
  'glslang_revision': 'd1608ab1ef17f1488bdcbfe11f2c3c96ac482fce',
  'googletest_revision': '16f637fbf4ffc3f7a01fa4eceb7906634565242f',
  'json_revision': 'fec56a1a16c6e1c1b1f4e116a20e79398282626c',
  'lodepng_revision': '8c6a9e30576f07bf470ad6f09458a2dcd7a6a84a',
  'shaderc_revision': 'f6d6dddfabfec1041c0dfb8e7ff3608a5f82227c',
  'spirv_headers_revision': '1380cbbec10756b492e9397d03c4250887e15090',
  'spirv_tools_revision': 'd997c83b103ed1f3af09ed65e1cbf89fbc6d9451',
  'swiftshader_revision': '0fa19bd6c285e8ad3459a3f58ca903ceb2d7ab00',
  'vulkan_headers_revision': 'd594f70127b4198286b3472a48bee56e341259cd',
  'vulkan_loader_revision': '830a0724aa281d7cad98eda59b850871f024bb41',
  'vulkan_validationlayers_revision': 'b9168891cb55c00e2054220082775f6e0c114df0',
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
