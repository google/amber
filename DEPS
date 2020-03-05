use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': 'c359f9537ffb17c4f40a933980ddb515d7ee923b',
  'clspv_revision': '0925af43a12096f8bb0748c97695cf0bf5d61aeb',
  'cppdap_revision': 'c4358807e2e61c6fbc34f17074b8e52223e156f9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'a5cf0efe30d266dbb0db13787e9d57900758b40f',
  'glslang_revision': 'c6a4c6d3d8b5b85b93308336534adf9c1ef0ae66',
  'googletest_revision': 'e588eb1ff9ff6598666279b737b27f983156ad85',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'ffe95ec86647a343c333f756dcd6f0fd1e0348fb',
  'shaderc_revision': '14f128e8dc29b40b1853a9c89481b72fec1044bb',
  'spirv_headers_revision': 'd9ce9ca52ffea72d45b313d06867d5312f222aaa',
  'spirv_tools_revision': '66a682b6a8f973ba79b0e250221d53a4fbd11f4c',
  'swiftshader_revision': '51b2800bb317d9ab6026e6123c62f013dd5cf5e4',
  'vulkan_headers_revision': '9bd3f561bcee3f01d22912de10bb07ce4e23d378',
  'vulkan_loader_revision': '136207a63495917347ea8aec5bfeff267c207eb8',
  'vulkan_validationlayers_revision': '97484d689869b39d82925708645d53f6d86f3b2b',
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
