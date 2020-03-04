use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',
  'llvm_git': 'https://github.com/llvm',
  'lvandeve_git':  'https://github.com/lvandeve',
  'microsoft_git': 'https://github.com/Microsoft',
  'nlohmann_git': 'https://github.com/nlohmann',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'clspv_llvm_revision': '1bedb2340774099fd3faa6610a78119a4f802955',
  'clspv_revision': 'bbbda97e8d946e7900a69f7e979be30af828899b',
  'cppdap_revision': 'c4358807e2e61c6fbc34f17074b8e52223e156f9',
  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'dxc_revision': 'ed6b888f17c960ad05bb94a64813a542bc9eb7bc',
  'glslang_revision': '8985fc91089f3117d338f0ebd683596a197b2d92',
  'googletest_revision': 'e588eb1ff9ff6598666279b737b27f983156ad85',
  'json_revision': '456478b3c50d60100dbb1fb9bc931f370a2c1c28',
  'lodepng_revision': 'ffe95ec86647a343c333f756dcd6f0fd1e0348fb',
  'shaderc_revision': '6e9087162a1cd9b67d66ebd3cda0054765f27ee9',
  'spirv_headers_revision': '0a7fc45259910f07f00c5a3fa10be5678bee1f83',
  'spirv_tools_revision': 'e1688b60caf77e7efd9e440e57cca429ca7c5a1e',
  'swiftshader_revision': '126720bd2e578fec077a57d877ac64c46b18cd52',
  'vulkan_headers_revision': '9bd3f561bcee3f01d22912de10bb07ce4e23d378',
  'vulkan_loader_revision': '136207a63495917347ea8aec5bfeff267c207eb8',
  'vulkan_validationlayers_revision': '1f337d1a25b84b631e0e2bc2d9f4bfc850e777f1',
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
