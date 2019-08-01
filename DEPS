use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm-mirror',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_clang_revision': '834a93f953ac6789f39dbeb86d8144f847807353',
  'clspv_llvm_revision': 'e35805b8192297a81e795573614274b904785a7d',
  'clspv_revision': '64a51387967ea2a26e4a52a22ad8640148194edc',
  'cpplint_revision': '9f41862c0efa7681e2147910d39629c73a2b2702',
  'dxc_revision': 'ec912b2ec95feb50925704dd631ef7abee1a5f09',
  'glslang_revision': '8aa9a7bb8f74869ab9cfd2c9bd51cef1741004e7',
  'googletest_revision': '947aeab281f1b160d2db5045064be73c984f1ae6',
  'lodepng_revision': 'ba9fc1f084f03b5fbf8c9a5df9448173f27544b1',
  'shaderc_revision': '747518d43f4b463cf0d6720fdf9968c55b2921fe',
  'spirv_headers_revision': 'e4322e3be589e1ddd44afb20ea842a977c1319b8',
  'spirv_tools_revision': '31590104ec860ff364c0c768df7f55c7887aaa69',
  'swiftshader_revision': 'ef44b4402722658648ec9d10a76bd990776be1c0',
  'vulkan_headers_revision': '8e2c4cd554b644592a6d904f2c8000ebbd4aa77f',
  'vulkan_loader_revision': '15fa85d92454f7823febeb68b56038d427e2a7a4',
}

deps = {
  'third_party/clspv': vars['google_git'] + '/clspv.git@' +
      vars['clspv_revision'],

  'third_party/clspv-clang': vars['llvm_git'] + '/clang.git@' +
      vars['clspv_clang_revision'],

  'third_party/clspv-llvm': vars['llvm_git'] + '/llvm.git@' +
      vars['clspv_llvm_revision'],

  'third_party/cpplint': vars['google_git'] + '/styleguide.git@' +
      vars['cpplint_revision'],

  'third_party/dxc': vars['microsoft_git'] + '/DirectXShaderCompiler.git@' +
      vars['dxc_revision'],

  'third_party/googletest': vars['google_git'] + '/googletest.git@' +
      vars['googletest_revision'],

  'third_party/glslang': vars['khronos_git'] + '/glslang.git@' +
      vars['glslang_revision'],

  'third_party/lodepng': vars['lvandeve_git'] + '/lodepng.git@' +
      vars['lodepng_revision'],

  'third_party/shaderc': vars['google_git'] + '/shaderc.git@' +
      vars['shaderc_revision'],

  'third_party/spirv-headers': vars['khronos_git'] + '/SPIRV-Headers.git@' +
      vars['spirv_headers_revision'],

  'third_party/spirv-tools': vars['khronos_git'] + '/SPIRV-Tools.git@' +
      vars['spirv_tools_revision'],

  'third_party/swiftshader': vars['swiftshader_git'] + '/SwiftShader.git@' +
      vars['swiftshader_revision'],

  'third_party/vulkan-headers': vars['khronos_git'] + '/Vulkan-Headers.git@' +
      vars['vulkan_headers_revision'],

  'third_party/vulkan-loader': vars['khronos_git'] + '/Vulkan-Loader.git@' +
      vars['vulkan_loader_revision'],
}
