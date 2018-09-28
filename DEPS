use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'glslang_revision': 'a08f465d5398518e9a6aeebd4775604a4c10e381',
  'googletest_revision': '6463ee81ae7ea8ee3dcaf341cb727d278f8cfe6b',
  'shaderc_revision': '92efe4583fcd936252eae61684bcecde8627c9fc',
  'spirv_headers_revision': '801cca8104245c07e8cc53292da87ee1b76946fe',
  'spirv_tools_revision': '18fe6d59e5e8dd8c5ccf8baa40f57bda838e7dfa',
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
