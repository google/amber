use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '7cefd1b4cd77cd37128965264051e617c7005593',
  'clspv_revision': '1c32720c4c5b3e0e4c78e97a50f886cb42198e0c',
  'cppdap_revision': '9e313445bb75bae71b5d003f7264ee55ae296023',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '36ecb43206b9de059a3ab2a1e2ab1dde25c98c5b',
  'glslang_revision': '0ab78114a9e1b28a48cd37252ec59fdcf85e187d',
  'googletest_revision': '011959aafddcd30611003de96cfd8d7a7685c700',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'e34ac04553e51a6982ae234d98ce6b76dd57a6a1',
  'shaderc_revision': '00ac5d82178cfb5679fe19194d5794fdb01cdd00',
  'spirv_headers_revision': 'c0df742ec0b8178ad58c68cff3437ad4b6a06e26',
  'spirv_tools_revision': '18ba3d9a353b7c59903d8ef712d66ea7f3ee76f1',
  'swiftshader_revision': 'b6e8c3f0f4830887d69ba765a922ac3c40e81dd9',
  'vulkan_headers_revision': '09531f27933bf04bffde9074acb302e026e8f181',
  'vulkan_loader_revision': '3e390a15976143060eb232acc04380209f9ed08e',
  'vulkan_validationlayers_revision': '67eac5116ba382ba157db3ddaa061d53433f5dbf',
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
