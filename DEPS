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

  'clspv_llvm_revision': '4a2ebd6661cf1c929c280e863e4299164800413e',
  'clspv_revision': '05fe1edbafb8d98ce798e7d9feb4a219bea114ef',
  'cpplint_revision': 'fa12a0bbdafa15291276ddd2a2dcd2ac7a2ce4cb',
  'dxc_revision': '773b01272719e07ea369bc17f5ddfce248751c7a',
  'directx_headers_revision': '980971e835876dc0cde415e8f9bc646e64667bf7',
  'glslang_revision': '340bf88f3fdb4f4a25b7071cd2c1205035fc6eaa',
  'googletest_revision': '16f637fbf4ffc3f7a01fa4eceb7906634565242f',
  'json_revision': '4f8fba14066156b73f1189a2b8bd568bde5284c5',
  'lodepng_revision': '5601b8272a6850b7c5d693dd0c0e16da50be8d8d',
  'shaderc_revision': '690d259384193c90c01b52288e280b05a8481121',
  'spirv_headers_revision': 'ddd2c099be25e3fec7cdc14106c17aca99512082',
  'spirv_tools_revision': '9f8ff357d91d20c2a2cc18e9551ac5f7fd3db749',
  'swiftshader_revision': 'da334852e70510d259bfa8cbaa7c5412966b2f41',
  'vulkan_headers_revision': '450bd2232225d6c7728a4108055ac2e37cef6475',
  'vulkan_loader_revision': '7a07afe04ad134d4eabe25f62720177f60ed6627',
  'vulkan_utility_libraries_revision': '9a3f4105c16bde4e2717eeab0fcc5a49236f7294',
  'vulkan_validationlayers_revision': '76cedc09801f23d543d39851d686bec31a4ac3e3',
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
