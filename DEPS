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

  'clspv_llvm_revision': '7f4d66f23e3e8a2a03e83a2df0ee3a56790e2b64',
  'clspv_revision': 'fed6f59b2dff3abae8d0c0215d3586765ed01e5f',
  'cppdap_revision': '88e89520148b2f95e17ca9348587a28215ffc921',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '0917aa9c0c20528b5cfb051f277eb01a222e51f2',
  'glslang_revision': '6624e1367309630b2f6df3cf93a5f864e89973f9',
  'googletest_revision': '16f637fbf4ffc3f7a01fa4eceb7906634565242f',
  'json_revision': '4f8fba14066156b73f1189a2b8bd568bde5284c5',
  'lodepng_revision': '5601b8272a6850b7c5d693dd0c0e16da50be8d8d',
  'shaderc_revision': 'e72186b66bb90ed06aaf15cbdc9a053581a0616b',
  'spirv_headers_revision': 'b8047fbe45f426f5918fadc67e8408f5b108c3c9',
  'spirv_tools_revision': '8a40f6de57d7b78bc431678d90aa8a570d1631f2',
  'swiftshader_revision': 'f6bdbed95b3703d782c2a2a537092afb39455442',
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
