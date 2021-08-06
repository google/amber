use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '7e30989dabce9ddbca0cbad7a8f25fb4e756d334',
  'clspv_revision': '3970681ca8144e9a8d3cdd3f0d37c12465434211',
  'cppdap_revision': 'be5b677c7b85b52f7570c572e99833514e754b62',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '9f771648e6feee85e70d3a92bd31ec053a4d82b3',
  'glslang_revision': 'e0f3fdf43385061a1e3a049208e98527ee6af4af',
  'googletest_revision': '5b40153003d1a5ad7b8f40cffcd09434afda3428',
  'json_revision': '350ff4f7ced7c4117eae2fb93df02823c8021fcb',
  'lodepng_revision': '8c6a9e30576f07bf470ad6f09458a2dcd7a6a84a',
  'shaderc_revision': 'fadb0edb247a1daa74f9a206a27e9a1c0417ce49',
  'spirv_headers_revision': 'e7b49d7fb59808a650618e0a4008d4bae927e112',
  'spirv_tools_revision': '175ecd49ed66cb3cb2593a4c6dd24468563bf17d',
  'swiftshader_revision': 'b2af6a85583d1adf61033e82eaa5d067d764ece9',
  'vulkan_headers_revision': '9fe958cdabcaf87650a4517b27df1ec2034d051f',
  'vulkan_loader_revision': '4c901a731a63baf5e6049e1a976a6655fb83be01',
  'vulkan_validationlayers_revision': '28bd6d60be55549ce2c617b9c065d44dddf9f146',
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
