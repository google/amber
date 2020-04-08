use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'a2bb19ca420d0801f5b9a5363dec1d7bcb829030',
  'clspv_revision': '4e6c283e15420ba6e949d44850929b8d78b45ad9',
  'cppdap_revision': '4dcca5775616ada2796ff7f84c3a4843eee9b506',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'ef73f1da8ad79a42405e8c75cf7d66ed8a0bb345',
  'glslang_revision': '0b66fa3b62cb36a3bc86f5018cf92a5211b27156',
  'googletest_revision': '61f010d703b32de9bfb20ab90ece38ab2f25977f',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'e34ac04553e51a6982ae234d98ce6b76dd57a6a1',
  'shaderc_revision': 'f175adffa9d2da6ae18a5f8d54e7b5f27df0f1c2',
  'spirv_headers_revision': 'f8bf11a0253a32375c32cad92c841237b96696c0',
  'spirv_tools_revision': 'e95fbfb1f509ad7a7fdfb72ac35fe412d72fc4a4',
  'swiftshader_revision': 'f99302c4efe6f32297a619d407b4410ec3ee6412',
  'vulkan_headers_revision': '0e78ffd1dcfc3e9f14a966b9660dbc59bd967c5c',
  'vulkan_loader_revision': 'bb74deab0a4dc88ec0f6c808d4f9874e8629ce5e',
  'vulkan_validationlayers_revision': '9a283ae3457fcbcd7990b3b985a4fbaddee19925',
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
