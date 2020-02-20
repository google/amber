use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'fa221fc6b8ac5ecd344daac634f0db8f22ebf435',
  'clspv_revision': 'd4fab77af8e4e0a8bc175a495ee6edfcfb51471c',
  'cppdap_revision': 'c4358807e2e61c6fbc34f17074b8e52223e156f9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '40982295834f3b34ffc373b6dd60d607b56c2ade',
  'glslang_revision': 'c12493ff69e21800fb08b6d6e92eb0b9c5cb5efb',
  'googletest_revision': '23b2a3b1cf803999fb38175f6e9e038a4495c8a5',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'ffe95ec86647a343c333f756dcd6f0fd1e0348fb',
  'shaderc_revision': '738f1655a7bbaec8d9302b9c3daefa7109117649',
  'spirv_headers_revision': '5dbc1c32182e17b8ab8e8158a802ecabaf35aad3',
  'spirv_tools_revision': '79f8caf9154a0328a87424354bd10ab69e811185',
  'swiftshader_revision': '74d3f371f2015deeedb985a8b24ba242ea65caef',
  'vulkan_headers_revision': '9bd3f561bcee3f01d22912de10bb07ce4e23d378',
  'vulkan_loader_revision': 'c357af84c7cb3de7bd2dfcd68212fd213eeea802',
  'vulkan_validationlayers_revision': '6112b121e1043655a6d576d06056e4f3fbc3b239',
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
