use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'glslang_revision': '7274bbc27c07406b7f670fde997af560386a1d77',
  'googletest_revision': 'd5932506d6eed73ac80b9bcc47ed723c8c74eb1e',
  'shaderc_revision': '823901b2803d9d70c4e0975daa8600c6500b5274',
  'spirv_headers_revision': 'a2c529b5dda18838ab4b52f816acfebd774eaab3',
  'spirv_tools_revision': '2b1f6b373ca9ee776e969948b883d93c8b4f304c',
}

deps = {
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
