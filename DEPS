use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'fb47ffc618eaf0e716eafa7e719fc67bf41cef3b',
  'clspv_revision': '04f3a95d01078d644f6ea9710061e3e0aac854d1',
  'cppdap_revision': '4dcca5775616ada2796ff7f84c3a4843eee9b506',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'd3af7f1237c1bafd130dfb79062e978f51007712',
  'glslang_revision': 'e157435c1e777aa1052f446dafed162b4a722e03',
  'googletest_revision': '67cc66080d64e3fa5124fe57ed0cf15e2cecfdeb',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'e34ac04553e51a6982ae234d98ce6b76dd57a6a1',
  'shaderc_revision': 'aface10e934e4b2b5b83979879a6f1e3e37431d3',
  'spirv_headers_revision': 'f8bf11a0253a32375c32cad92c841237b96696c0',
  'spirv_tools_revision': 'fd773eb50d628c1981338addc093df879757c2cf',
  'swiftshader_revision': '5cf1e9a31c90ccd9ec40d0c5ef1357f6e0ec1cfd',
  'vulkan_headers_revision': '0e78ffd1dcfc3e9f14a966b9660dbc59bd967c5c',
  'vulkan_loader_revision': 'ba07fa0be82cf1e3e7e8c2e0e01f5109a5847d3a',
  'vulkan_validationlayers_revision': '2f3087802bcd79bd71c35c127713f594f0f6e1f2',
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
