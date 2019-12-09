use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': 'd96ea47c75fdf4a62358eae49a0026bcf031bde6',
  'clspv_revision': '33d8fce7dd2ffb9d105afb5db74067b648cf5271',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'c5df9d33e62ea3ed2fc0ff5162ee52d4b0291726',
  'glslang_revision': '0de87ee9a5bf5d094a3faa1a71fd9080e80b6be0',
  'googletest_revision': 'ae8d1fc81b1469905b3d0fa6f8a077f58fc4b250',
  'lodepng_revision': 'dc3f19b5aeaa26ff94f48f52a1c7cb5b1ef4ed3c',
  'shaderc_revision': 'f9278b9cb69d93c8afb6b7a68a92c5c10bf1897c',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': 'e82a428605f6ce0a07337b36f8ba3935c9f165ac',
  'swiftshader_revision': '2b9e22ac6c800a9581dff8e2f284bd4d49a4f814',
  'vulkan_headers_revision': '2b89fd4e2734b728ca0be72a13f2265c5f5aa88e',
  'vulkan_validationlayers_revision': 'e72b61c7c20dd2443f955b956f96b2976ccee004',
  'vulkan_loader_revision': '2d6f74c6d4319e94cf1fa33954c619ab4428f2b8',
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
