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
  'dxc_revision': '23b84f584c12f4bc16246f42fbe35164cee60c01',
  'glslang_revision': '38b4db48f98c4e3a9cc405de3a76547b857e1c37',
  'googletest_revision': '679bfec6db73c021b0226e386c65ec1baee7a09f',
  'lodepng_revision': 'dc3f19b5aeaa26ff94f48f52a1c7cb5b1ef4ed3c',
  'shaderc_revision': 'da52fae116b3968baa4996e86a05aa45ec1f06ec',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '85f3e93d13f32d45bd7f9999aa51baddf2452aae',
  'swiftshader_revision': 'aee3c369516a24caa8ebc9a38fc57da708ceccbc',
  'vulkan_headers_revision': '24347673152e093a48efbf65dfd3b06026b6ed33',
  'vulkan_validationlayers_revision': '5efc3922e3e863cc69fb487b040c1d1e563de1bc',
  'vulkan_loader_revision': '08d344208e60e0bb8c8fbe25b9b1f2bf63801e2f',
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
