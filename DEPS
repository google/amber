use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'b91a236ee1c3e9fa068df058164385732cb46bba',
  'clspv_revision': '3a11614ee40907c6f9edd99bd7d23b123111e947',
  'cppdap_revision': '1fd23dda91e01550be1a421de307e6fedb2035a9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '141a5c2ec5f77733f301f9f332bc3b0aefbc2c62',
  'glslang_revision': 'f4f1d8a352ca1908943aea2ad8c54b39b4879080',
  'googletest_revision': '3c95bf552405fd0cc63cea0ca2f6c4cd89c8d356',
  'json_revision': '350ff4f7ced7c4117eae2fb93df02823c8021fcb',
  'lodepng_revision': '7fdcc96a5e5864eee72911c3ca79b1d9f0d12292',
  'shaderc_revision': '011139094ec790ff7f32ea2d80286255fc9ed18b',
  'spirv_headers_revision': 'c43a43c7cc3af55910b9bec2a71e3e8a622443cf',
  'spirv_tools_revision': '7e1825a59619f3decd9b46f25d457b1a8878ca89',
  'swiftshader_revision': 'df17a76102dfabb3f1bd6e51449cece9f77b45e3',
  'vulkan_headers_revision': '320af06cbdd29848e1d7100d9b8e4e517db1dfd5',
  'vulkan_loader_revision': '970c132746b1d8bc2baeeb2bcdb5edf490b47f94',
  'vulkan_validationlayers_revision': '8c5a10d952f4122898205854b61f9e8de46d2363',
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
