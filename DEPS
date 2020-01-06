use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': 'de735247c8b638efa8ce5783ac8c7c2e0b7cf3eb',
  'clspv_revision': 'bb237bfb75af4ed757548281240cc46d87ecc874',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '9c89a1c2c6baa76dabc154f126408973848b0069',
  'glslang_revision': '6334d594f68c2ba36e3e9bf91aac185ac3875717',
  'googletest_revision': '306f3754a71d6d1ac644681d3544d06744914228',
  'lodepng_revision': '9652b36175737fbec20c3cfbfcaaa4b4807ea26f',
  'shaderc_revision': '82a9846c4377773dea03fde16ae4271ba9ddd8c1',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': '8aa423930db37e37086665efcc55944d577c06e5',
  'swiftshader_revision': '10a900e5ffaffdffe2806b1507af43a74acdfe9e',
  'vulkan_headers_revision': 'f63dd5c9d874310c8403f3aef9302b761efedd18',
  'vulkan_validationlayers_revision': '2be3fe0f84713f6ce914886279f10f3ba16aaaff',
  'vulkan_loader_revision': '2069798558ec7eb9b489ffc69fd1d27eebb0c84e',
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
