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
  'clspv_revision': 'e0406e7053d1bb46b4bbeb57f0f2bbfca32f5612',
  'cppdap_revision': '1fd23dda91e01550be1a421de307e6fedb2035a9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '489c2e4d32417cd6693db5673ab071d82e1f5974',
  'glslang_revision': '7f6559d2802d0653541060f0909e33d137b9c8ba',
  'googletest_revision': '0555b0eacbc56df1fd762c6aa87bb84be9e4ce7e',
  'json_revision': '350ff4f7ced7c4117eae2fb93df02823c8021fcb',
  'lodepng_revision': '7fdcc96a5e5864eee72911c3ca79b1d9f0d12292',
  'shaderc_revision': '88f9156d7f6a2a30baed1ace196faa3bc5eccc05',
  'spirv_headers_revision': '5ab5c96198f30804a6a29961b8905f292a8ae600',
  'spirv_tools_revision': '1f2fcddd3963b9c29bf360daf7656c5977c2aadd',
  'swiftshader_revision': '04515da400d5fbc22d852af1369c4d46bd54991e',
  'vulkan_headers_revision': '11c6670b4a4f766ed4f1e777d1b3c3dc082dfa5f',
  'vulkan_loader_revision': 'be6ccb9ecaf77dfef59246a1e8502e99c8e1a511',
  'vulkan_validationlayers_revision': '0cb8cc8cfcb2b86a767c9516ac2d62edb4e38ebe',
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
