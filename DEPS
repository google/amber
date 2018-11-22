use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'cpplint_revision': '9f41862c0efa7681e2147910d39629c73a2b2702',
  'glslang_revision': 'd2a7b07a64811bb4a734bd66ef4e0b4c7d7fe1af',
  'googletest_revision': 'd5932506d6eed73ac80b9bcc47ed723c8c74eb1e',
  'shaderc_revision': '75441559fdb99856be9c02c0abd9c8e23b405033',
  'spirv_headers_revision': '7cb43009d543e90698dd300eb26dfd6d9a9bb100',
  'spirv_tools_revision': '981763ec74fc2d59e2bc98e8f15377d2de9b73fd',
}

deps = {
  'third_party/cpplint': vars['google_git'] + '/styleguide.git@' +
      vars['cpplint_revision'],

  'third_party/googletest': vars['google_git'] + '/googletest.git@' +
      vars['googletest_revision'],

  'third_party/glslang': vars['khronos_git'] + '/glslang.git@' +
      vars['glslang_revision'],

  'third_party/shaderc': vars['google_git'] + '/shaderc.git@' +
      vars['shaderc_revision'],

  'third_party/spirv-headers': vars['khronos_git'] + '/SPIRV-Headers.git@' +
      vars['spirv_headers_revision'],

  'third_party/spirv-tools': vars['khronos_git'] + '/SPIRV-Tools.git@' +
      vars['spirv_tools_revision'],
}
