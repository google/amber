use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '9671f6e8d5edb07cb0dd0bd38fc2c3d29960458f',
  'clspv_revision': 'c4579bb1d990d10d9211cf9c4d0c9fca72e54708',
  'cppdap_revision': '4dcca5775616ada2796ff7f84c3a4843eee9b506',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'a841ddde5ac858dca82059a4a876801c8eca2a79',
  'glslang_revision': 'f03cb290ac10414dfc96017b26ebfaee8f3afb3e',
  'googletest_revision': 'dcc92d0ab6c4ce022162a23566d44f673251eee4',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'e34ac04553e51a6982ae234d98ce6b76dd57a6a1',
  'shaderc_revision': '41f271e6139ceb6a54457fb2da14571f66100a9a',
  'spirv_headers_revision': 'c0df742ec0b8178ad58c68cff3437ad4b6a06e26',
  'spirv_tools_revision': 'd0a87194f7b9a3b7659e837b08cd404ccc8af222',
  'swiftshader_revision': '941293d512fe2c3e6737554ea50802e3326c0196',
  'vulkan_headers_revision': '4c19ae6b95e44c821d5306adf94842defa57ba21',
  'vulkan_loader_revision': '3336e65e880101e28b38836733c1f02fd9214bf2',
  'vulkan_validationlayers_revision': 'a977d65576b5f0e1bd6cf7536115b355fdbdc5ba',
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
