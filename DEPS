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
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '815358229dc74a43f57905cbf10fd805ab26c049',
  'glslang_revision': '38b4db48f98c4e3a9cc405de3a76547b857e1c37',
  'googletest_revision': '679bfec6db73c021b0226e386c65ec1baee7a09f',
  'lodepng_revision': 'dc3f19b5aeaa26ff94f48f52a1c7cb5b1ef4ed3c',
  'shaderc_revision': 'ff260580c07e8f9eae471b3d02dd2fe589c73198',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '85f3e93d13f32d45bd7f9999aa51baddf2452aae',
  'swiftshader_revision': '663dcefa22ea5eec1b108feebaf40a683fb104ff',
  'vulkan_headers_revision': '2b89fd4e2734b728ca0be72a13f2265c5f5aa88e',
  'vulkan_validationlayers_revision': '8c954d5a413d4fa724c0c3a6c8e37a7132c68e72',
  'vulkan_loader_revision': 'fc962d08075e295cb563ce586f0c94c815c4e847',
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
