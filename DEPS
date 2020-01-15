use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_llvm_revision': '252c4dce618926311bcb4715eae6955f1bc71f13',
  'clspv_revision': '242e3b6cbea0628c94453d24802bae55864a9b64',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': '80ec7c6bc7e3c40da52110dd2eb9b85192153775',
  'glslang_revision': 'd203754bc1160cbb14e80de238042a2b9b439917',
  'googletest_revision': 'd854bd6acc47f7f6e168007d58b5f509e4981b36',
  'lodepng_revision': '2febfe0d105822575328759dd950c8a24b0ad6b3',
  'shaderc_revision': '99ca03e1ac3a78c7d0e1ca7b3a1f6d973e0c1fc7',
  'spirv_headers_revision': '204cd131c42b90d129073719f2766293ce35c081',
  'spirv_tools_revision': 'f8d7df760c81604ddc4ae3dac7f19333336f64c3',
  'swiftshader_revision': '63468081e52e4f482adf1c4152b7196dc261144b',
  'vulkan_headers_revision': '881bbb347a08d1b5aa77f61a52a30b506de9f2bf',
  'vulkan_validationlayers_revision': 'e91181dfeb79db57687121a06369f7af599934d5',
  'vulkan_loader_revision': 'a0099c50284635fd7242541b6d335d01a90dcc26',
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
