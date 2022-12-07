#!/usr/bin/env python
# Copyright 2018 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import base64
import difflib
import optparse
import os
import platform
import re
import subprocess
import sys
import tempfile

SUPPRESSIONS = {
  "Darwin": [
    # No geometry shader on MoltenVK
    "draw_triangle_list_using_geom_shader.vkscript",
    # No tessellation shader on MoltenVK
    "draw_triangle_list_using_tessellation.vkscript",
    # No std140 support for matrices in SPIRV-Cross
    "compute_mat2x2.vkscript",
    "compute_mat2x2float.vkscript",
    "compute_mat2x2.amber",
    "compute_mat3x2.vkscript",
    "compute_mat3x2float.vkscript",
    "compute_mat3x2.amber",
    # Metal vertex shaders cannot simultaneously write to a buffer and return
    # a value to the rasterizer rdar://48348476
    # https://github.com/KhronosGroup/MoltenVK/issues/527
    "multiple_ssbo_update_with_graphics_pipeline.vkscript",
    "multiple_ubo_update_with_graphics_pipeline.vkscript",
  ],
  "Linux": [
  ],
  "Win": [
  ]
}

SUPPRESSIONS_SWIFTSHADER = [
  # Incorrect rendering: github.com/google/amber/issues/727
  "draw_array_instanced.vkscript",
  # Exceeds device limit maxComputeWorkGroupInvocations
  "draw_sampled_image.amber",
  # No geometry shader support
  "draw_triangle_list_using_geom_shader.vkscript",
  # No tessellation shader support
  "draw_triangle_list_using_tessellation.vkscript",
  # Vertex buffer format not supported
  "draw_triangle_list_in_r8g8b8a8_srgb_color_frame.vkscript",
  "draw_triangle_list_in_r32g32b32a32_sfloat_color_frame.vkscript",
  "draw_triangle_list_in_r16g16b16a16_uint_color_frame.vkscript",
  # Color attachment format is not supported
  "draw_triangle_list_in_r16g16b16a16_snorm_color_frame.vkscript",
  "draw_triangle_list_in_r8g8b8a8_snorm_color_frame.vkscript",
  # No supporting device for Float16Int8Features
  "float16.amber",
  "int8.amber",
  # No supporting device for the required 16-bit storage features
  "storage16.amber",
  # Exceeded maxBoundDescriptorSets limit of physical device
  "multiple_ssbo_with_sparse_descriptor_set_in_compute_pipeline.vkscript",
  # shaderStorageImageWriteWithoutFormat but is not enabled on the device
  "opencl_read_and_write_image3d_rgba32i.amber",
  "opencl_write_image.amber",
  "glsl_read_and_write_image3d_rgba32i.amber",
  # shaderStorageImageMultisample feature not supported
  "draw_storageimage_multisample.amber",
  # Unsupported depth/stencil formats
  "draw_rectangles_depth_test_d24s8.amber",
  "draw_rectangles_depth_test_x8d24.amber",
  # Tessellation not supported
  "tessellation_isolines.amber",
  # 8 bit indices not supported
  "draw_indexed_uint8.amber",
]

OPENCL_CASES = [
  "opencl_bind_buffer.amber",
  "opencl_c_copy.amber",
  "opencl_generated_push_constants.amber",
  "opencl_read_and_write_image3d_rgba32i.amber",
  "opencl_read_image.amber",
  "opencl_read_image_literal_sampler.amber",
  "opencl_set_arg.amber",
  "opencl_write_image.amber",
 ]

DXC_CASES = [
  "draw_triangle_list_hlsl.amber",
  "relative_includes_hlsl.amber",
]

SUPPRESSIONS_DAWN = [
  # Dawn does not support push constants
  "graphics_push_constants.amber",
  "graphics_push_constants.vkscript",
  "compute_push_const_mat2x2.vkscript",
  "compute_push_const_mat2x2float.vkscript",
  "compute_push_const_mat2x3.vkscript",
  "compute_push_const_mat2x3float.vkscript",
  "compute_push_const_mat3x2.vkscript",
  "compute_push_const_mat3x2float.vkscript",
  "compute_push_const_mat3x3.vkscript",
  "compute_push_const_mat3x3float.vkscript",
  "compute_push_const_mat3x4.vkscript",
  "compute_push_const_mat3x4float.vkscript",
  "compute_push_const_mat4x3.vkscript",
  "compute_push_const_mat4x3float.vkscript",
  "compute_push_constant_and_ssbo.amber",
  "compute_push_constant_and_ssbo.vkscript",
  # Dawn does not support tessellation or geometry shader
  "draw_triangle_list_using_geom_shader.vkscript",
  "draw_triangle_list_using_tessellation.vkscript",
  # Dawn requires a fragmentStage now and in the medium term.
  # issue #556 (temp dawn limitation)
  "position_to_ssbo.amber",
  # DrawRect command is not supported in a pipeline with more than one vertex
  # buffer attached
  "draw_array_after_draw_rect.vkscript",
  "draw_rect_after_draw_array.vkscript",
  "draw_rect_and_draw_array_mixed.vkscript",
  # Dawn DoCommands require a pipeline
  "probe_no_compute_with_multiple_ssbo_commands.vkscript",
  "probe_no_compute_with_ssbo.vkscript",
  # Max number of descriptor sets is 4 in Dawn
  "multiple_ssbo_with_sparse_descriptor_set_in_compute_pipeline.vkscript",
  # Dawn entry point has to be "main" as a result it does not support
  # doEntryPoint or opencl (in opencl using "main" as entry point is invalid).
  # issue #607 (temp dawn limitation)
  "compute_ssbo_with_entrypoint_command.vkscript",
  "entry_point.amber",
  "non_default_entry_point.amber",
  "opencl_bind_buffer.amber",
  "opencl_c_copy.amber",
  "opencl_set_arg.amber",
  "shader_specialization.amber",
  # framebuffer format is not supported according to table "Mandatory format
  # support" in Vulkan spec: VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT = 0
  "draw_triangle_list_in_r16g16b16a16_snorm_color_frame.vkscript",
  "draw_triangle_list_in_r16g16b16a16_uint_color_frame.vkscript",
  "draw_triangle_list_in_r32g32b32a32_sfloat_color_frame.vkscript",
  "draw_triangle_list_in_r8g8b8a8_snorm_color_frame.vkscript",
  "draw_triangle_list_in_r8g8b8a8_srgb_color_frame.vkscript",
  # Dawn does not support vertexPipelineStoresAndAtomics
  "multiple_ubo_update_with_graphics_pipeline.vkscript",
  "multiple_ssbo_update_with_graphics_pipeline.vkscript",
  # Currently not working, under investigation
  "draw_triangle_list_with_depth.vkscript",
  # draw_grid not implemented for dawn yet
  "draw_grid.amber",
  "draw_grid_multiple_color_attachment.amber",
  "draw_grid_multiple_pipeline.amber",
  "draw_grids.amber",
  "draw_grid.vkscript",
  "draw_grid_with_buffer.amber",
  "draw_grid_with_two_vertex_data_attached.expect_fail.amber",
]

class TestCase:
  def __init__(self, input_path, parse_only, use_dawn, use_opencl, use_dxc,
               use_swiftshader):
    self.input_path = input_path
    self.parse_only = parse_only
    self.use_dawn = use_dawn
    self.use_opencl = use_opencl
    self.use_dxc = use_dxc
    self.use_swiftshader = use_swiftshader

    self.results = {}

  def IsExpectedFail(self):
    fail_re = re.compile('^.+[.]expect_fail[.][amber|vkscript]')
    return fail_re.match(self.GetInputPath())

  def IsSuppressed(self):
    system = platform.system()

    base = os.path.basename(self.input_path)
    is_dawn_suppressed = base in SUPPRESSIONS_DAWN
    if self.use_dawn and is_dawn_suppressed:
      return True

    is_swiftshader_suppressed = base in SUPPRESSIONS_SWIFTSHADER
    if self.use_swiftshader and is_swiftshader_suppressed:
      return True

    is_opencl_test = base in OPENCL_CASES
    if not self.use_opencl and is_opencl_test:
      return True

    is_dxc_test = base in DXC_CASES
    if not self.use_dxc and is_dxc_test:
      return True

    if system in SUPPRESSIONS.keys():
      is_system_suppressed = base in SUPPRESSIONS[system]
      return is_system_suppressed

    return False

  def IsParseOnly(self):
    return self.parse_only

  def IsUseDawn(self):
    return self.use_dawn

  def GetInputPath(self):
    return self.input_path

  def GetResult(self, fmt):
    return self.results[fmt]


class TestRunner:
  def RunTest(self, tc):
    print("Testing {}".format(tc.GetInputPath()))

    cmd = [self.options.test_prog_path, '-q']
    if tc.IsParseOnly():
      cmd += ['-p']
    if tc.IsUseDawn():
      cmd += ['-e', 'dawn']
    cmd += [tc.GetInputPath()]

    try:
      err = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
      if len(err) != 0 and not tc.IsExpectedFail() and not tc.IsSuppressed():
        sys.stdout.write(err.decode('utf-8'))
        return False

    except Exception as e:
      if not tc.IsExpectedFail() and not tc.IsSuppressed():
        print("{}".format("".join(map(chr, bytearray(e.output)))))
        print(e)
      return False

    return True


  def RunTests(self):
    for tc in self.test_cases:
      result = self.RunTest(tc)

      if tc.IsSuppressed():
        self.suppressed.append(tc.GetInputPath())
      else:
        if not tc.IsExpectedFail() and not result:
          self.failures.append(tc.GetInputPath())
        elif tc.IsExpectedFail() and result:
          print("Expected: " + tc.GetInputPath() + " to fail but passed.")
          self.failures.append(tc.GetInputPath())

  def SummarizeResults(self):
    if len(self.failures) > 0:
      self.failures.sort()

      print('\nSummary of Failures:')
      for failure in self.failures:
        print(failure)

    if len(self.suppressed) > 0:
      self.suppressed.sort()

      print('\nSummary of Suppressions:')
      for suppression in self.suppressed:
        print(suppression)

    print('')
    print('Test cases executed: {}'.format(len(self.test_cases)))
    print('  Successes:  {}'.format((len(self.test_cases) - len(self.suppressed) - len(self.failures))))
    print('  Failures:   {}'.format(len(self.failures)))
    print('  Suppressed: {}'.format(len(self.suppressed)))
    print('')


  def Run(self):
    base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

    usage = 'usage: %prog [options] (file)'
    parser = optparse.OptionParser(usage=usage)
    parser.add_option('--build-dir',
                      default=os.path.join(base_path, 'out', 'Debug'),
                      help='path to build directory')
    parser.add_option('--test-dir',
                      default=os.path.join(os.path.dirname(__file__), 'cases'),
                      help='path to directory containing test files')
    parser.add_option('--test-prog-path', default=None,
                      help='path to program to test')
    parser.add_option('--parse-only',
                      action="store_true", default=False,
                      help='only parse test cases; do not execute')
    parser.add_option('--use-dawn',
                      action="store_true", default=False,
                      help='Use dawn as the backend; Default is Vulkan')
    parser.add_option('--use-opencl',
                      action="store_true", default=False,
                      help='Enable OpenCL tests')
    parser.add_option('--use-dxc',
                      action="store_true", default=False,
                      help='Enable DXC tests')
    parser.add_option('--use-swiftshader',
                      action="store_true", default=False,
                      help='Tells test runner swiftshader is the device')

    self.options, self.args = parser.parse_args()

    if self.options.test_prog_path == None:
      test_prog = os.path.abspath(os.path.join(self.options.build_dir, 'amber'))
      if not os.path.isfile(test_prog):
        print("Cannot find test program {}".format(test_prog))
        return 1

      self.options.test_prog_path = test_prog

    if not os.path.isfile(self.options.test_prog_path):
      print("--test-prog-path must point to an executable")
      return 1

    input_file_re = re.compile('^.+[\.](amber|vkscript)')
    self.test_cases = []

    if self.args:
      for filename in self.args:
        input_path = os.path.join(self.options.test_dir, filename)
        if not os.path.isfile(input_path):
          print("Cannot find test file '{}'".format(filename))
          return 1

        self.test_cases.append(TestCase(input_path, self.options.parse_only,
            self.options.use_dawn, self.options.use_opencl,
            self.options.use_dxc, self.options.use_swiftshader))

    else:
      for file_dir, _, filename_list in os.walk(self.options.test_dir):
        for input_filename in filename_list:
          if input_file_re.match(input_filename):
            input_path = os.path.join(file_dir, input_filename)
            if os.path.isfile(input_path):
              self.test_cases.append(
                  TestCase(input_path, self.options.parse_only,
                      self.options.use_dawn, self.options.use_opencl,
                      self.options.use_dxc, self.options.use_swiftshader))

    self.failures = []
    self.suppressed = []

    self.RunTests()
    self.SummarizeResults()

    return len(self.failures) != 0

def main():
  runner = TestRunner()
  return runner.Run()


if __name__ == '__main__':
  sys.exit(main())
