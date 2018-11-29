use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'cpplint_revision': '9f41862c0efa7681e2147910d39629c73a2b2702',
  'glslang_revision': 'f44b17ee135d5e153ce000e88b806b5377812b11',
  'googletest_revision': 'd5932506d6eed73ac80b9bcc47ed723c8c74eb1e',
  'shaderc_revision': '53c776f776821bc037b31b8b3b79db2fa54b4ce7',
  'spirv_headers_revision': '282879ca34563020dbe73fd8f7d45bed6755626a',
  'spirv_tools_revision': '703305b1a5a2a06ea75510b954dea1dd2489fa46',
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
