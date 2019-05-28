use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm-mirror',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'clspv_clang_revision': '151e674ab9981c986990e45c8a0a97815cac2021',
  'clspv_llvm_revision': '86b49e2b741eec98bc7afc6e075ace823e616f50',
  'clspv_revision': '3201427d026fd52952af59f6af94ec161720d184',
  'cpplint_revision': '9f41862c0efa7681e2147910d39629c73a2b2702',
  'dxc_revision': '7342a3b9be25bd4787fd24a4041795796e7ec49f',
  'glslang_revision': 'f44b17ee135d5e153ce000e88b806b5377812b11',
  'googletest_revision': 'd5932506d6eed73ac80b9bcc47ed723c8c74eb1e',
  'lodepng_revision': 'ba9fc1f084f03b5fbf8c9a5df9448173f27544b1',
  'shaderc_revision': '53c776f776821bc037b31b8b3b79db2fa54b4ce7',
  'spirv_headers_revision': 'de99d4d834aeb51dd9f099baa285bd44fd04bb3d',
  'spirv_tools_revision': '001e823b65345145bcaaeb94d39290b10f8661b3',
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
