use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'a6ac2b32fbab9679c8f2fa97a3b1123e3a9654c8',
  'clspv_revision': 'e0406e7053d1bb46b4bbeb57f0f2bbfca32f5612',
  'cppdap_revision': '1fd23dda91e01550be1a421de307e6fedb2035a9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'f1f60648df03f20b1281bfbfbd356c4f6d9f3b48',
  'glslang_revision': 'beec2e4a7c4d846eeb4097faebd74d020602faa3',
  'googletest_revision': '0555b0eacbc56df1fd762c6aa87bb84be9e4ce7e',
  'json_revision': '350ff4f7ced7c4117eae2fb93df02823c8021fcb',
  'lodepng_revision': '7fdcc96a5e5864eee72911c3ca79b1d9f0d12292',
  'shaderc_revision': '88f9156d7f6a2a30baed1ace196faa3bc5eccc05',
  'spirv_headers_revision': '5ab5c96198f30804a6a29961b8905f292a8ae600',
  'spirv_tools_revision': '1f2fcddd3963b9c29bf360daf7656c5977c2aadd',
  'swiftshader_revision': 'e4c1a25cc6797a47043fc415c53b8edcd7b3e37e',
  'vulkan_headers_revision': '11c6670b4a4f766ed4f1e777d1b3c3dc082dfa5f',
  'vulkan_loader_revision': 'be6ccb9ecaf77dfef59246a1e8502e99c8e1a511',
  'vulkan_validationlayers_revision': '9d3ef3258715573b17e8195855c76626600998be',
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
