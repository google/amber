use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': '97aa42f5dfcd10ca6df230caf9ca7868da5f25af',
  'clspv_revision': '8132fc9122f7709149dd82b25283e244bbe666a6',
  'clspv_llvm_revision': '6ba5cbf3ea2315acf1b7f1c39c6fec6cca5560ca',
  'clspv_revision': 'f26dac9ae5d6fab98ca027a26e375937074770cf',
  'cpplint_revision': '2589002f53fa8866c9f0b48b9854eb4c8dbf4fa9',
  'dxc_revision': '005576d73d96b58dbbc0a2542d509650065ebb7f',
  'glslang_revision': 'f4d4668529f1dad0e475295456b601353fe7cf33',
  'googletest_revision': '200ff599496e20f4e39566feeaf2f6734ca7570f',
  'lodepng_revision': 'dc3f19b5aeaa26ff94f48f52a1c7cb5b1ef4ed3c',
  'shaderc_revision': '091a11f81dce59f126c4b5ef4da8f034e1d8bf8e',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '45dde9ad6dde133db9bd6942c46df692ca539186',
  'swiftshader_revision': 'f2c9ce1e08ed30ed42f16df61c0ea91579941134',
  'vulkan_headers_revision': 'ba6cbb0478684580d6f6f3465d8b2c0ea594b642',
  'vulkan_validationlayers_revision': '53902309eda9b202649e5ce576192c73cf9aa793',
  'vulkan_loader_revision': '79e03670c2a328bea3c1a3f80ea913f296a487e6',
}

deps = {
  'third_party/clspv': Var('google_git') + '/clspv.git@' +
      Var('clspv_revision'),

  'third_party/clspv-llvm': Var('llvm_git') + '/llvm-project.git@' +
      Var('clspv_llvm_revision'),

  'third_party/cpplint': Var('google_git') + '/styleguide.git@' +
      Var('cpplint_revision'),

  'third_party/dxc': Var('microsoft_git') + '/DirectXShaderCompiler.git@' +
      Var('dxc_revision'),

  'third_party/googletest': Var('google_git') + '/googletest.git@' +
      Var('googletest_revision'),

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
