use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'microsoft_git': 'https://github.com/Microsoft',

  'dj2_git': 'https://github.com/dj2',

  'clspv_llvm_revision': '97aa42f5dfcd10ca6df230caf9ca7868da5f25af',
  'clspv_revision': '8132fc9122f7709149dd82b25283e244bbe666a6',
  'cpplint_revision': '5651966e0275572a9956199418d89c9ccc7b2b1a',
  'dxc_revision': 'ec912b2ec95feb50925704dd631ef7abee1a5f09',
  'glslang_revision': 'fe0b2bd694bb07004a2db859c5714c321c26b751',
  'googletest_revision': '33a0d4f6d76a0ed6061e612848532cba82d42870',
  'lodepng_revision': 'ba9fc1f084f03b5fbf8c9a5df9448173f27544b1',
  'shaderc_revision': '5903ef1f95a0acdbbd3ae645af1a6d6b30320f10',
  'spirv_headers_revision': '38cafab379e5d16137cb97a485b9385191039b92',
  'spirv_tools_revision': 'c0e9807094ef6e345ef0a4d5f17af81af063cd27',
  'swiftshader_revision': 'ef44b4402722658648ec9d10a76bd990776be1c0',
  'vulkan_headers_revision': '42ad3f90faec009b9435383ee89910846d6a91ba',
  'vulkan_loader_revision': 'a28ea6d6342e6cd2bbb76480addae8af12993fcd',
}

deps = {
  'third_party/clspv': vars['google_git'] + '/clspv.git@' +
      vars['clspv_revision'],

  'third_party/clspv-llvm': vars['llvm_git'] + '/llvm-project.git@' +
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

  'third_party/vulkan-loader': vars['dj2_git'] + '/Vulkan-Loader.git@' +
      vars['vulkan_loader_revision'],
}
