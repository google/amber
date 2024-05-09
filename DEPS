use_relative_paths = True

vars = {
  'cpplint_git':  'https://github.com/cpplint',
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'martinus_git': 'https://github.com/martinus',

  'clspv_llvm_revision': 'b70366c9c430e1eadd59d5a1dfbb9c4d84f83de5',
  'clspv_revision': 'f99809bdab1710846633b4ec24f5448263e75da7',
  'cpplint_revision': 'fa12a0bbdafa15291276ddd2a2dcd2ac7a2ce4cb',
  'dxc_revision': '773b01272719e07ea369bc17f5ddfce248751c7a',
  'directx_headers_revision': '980971e835876dc0cde415e8f9bc646e64667bf7',
  'glslang_revision': 'e8dd0b6903b34f1879520b444634c75ea2deedf5',
  'googletest_revision': '16f637fbf4ffc3f7a01fa4eceb7906634565242f',
  'json_revision': '4f8fba14066156b73f1189a2b8bd568bde5284c5',
  'lodepng_revision': '5601b8272a6850b7c5d693dd0c0e16da50be8d8d',
  'shaderc_revision': 'f59f0d11b80fd622383199c867137ededf89d43b',
  'spirv_headers_revision': '5e3ad389ee56fca27c9705d093ae5387ce404df4',
  'spirv_tools_revision': '9241a58a8028c49510bc174b6c970e3c2b4b8e51',
  'swiftshader_revision': 'da334852e70510d259bfa8cbaa7c5412966b2f41',
  'vulkan_headers_revision': '4bc77c26ff9ce89cf4a4f79e1c24a44604132d53',
  'vulkan_loader_revision': 'e69a59a96b241038f24a0e425445d001ea099b2c',
  'vulkan_utility_libraries_revision': '358a107a6ff284906dcccbabe5b0183c03fd85b6',
  'vulkan_validationlayers_revision': '23455c903e2ab21db2b4497331d80d610841cae1',
}

deps = {
  'third_party/clspv': Var('google_git') + '/clspv.git@' +
      Var('clspv_revision'),

  'third_party/clspv-llvm': Var('llvm_git') + '/llvm-project.git@' +
      Var('clspv_llvm_revision'),

  'third_party/cpplint': Var('cpplint_git') + '/cpplint.git@' +
      Var('cpplint_revision'),

  'third_party/dxc': Var('microsoft_git') + '/DirectXShaderCompiler.git@' +
      Var('dxc_revision'),

  'third_party/DirectX-Headers': Var('microsoft_git') + '/DirectX-Headers.git@' +
      Var('directx_headers_revision'),

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

  'third_party/vulkan-utility-libraries': Var('khronos_git') + '/Vulkan-Utility-Libraries.git@' +
      Var('vulkan_utility_libraries_revision'),
}
