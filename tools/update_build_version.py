#!/usr/bin/env python

# Copyright 2018 The Amber Authors. All rights reserved.
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

# Generates build-versions.h in the src/ directory.
#
# Args:  <output_dir> <amber-dir> <spirv-tools-dir> <spirv-headers> <glslang-dir> <shaderc-dir>

from __future__ import print_function

import datetime
import os.path
import re
import subprocess
import sys
import time

OUTFILE = 'src/build-versions.h'


def command_output(cmd, directory):
  p = subprocess.Popen(cmd,
                       cwd=directory,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
  (stdout, _) = p.communicate()
  if p.returncode != 0:
    raise RuntimeError('Failed to run {} in {}'.format(cmd, directory))
  return stdout


def describe(directory):
  if not os.path.exists(directory):
    return "-"
  return command_output(
      ['git', 'log', '-1', '--format=%h'], directory).rstrip().decode()


def get_version_string(project, directory):
  name = project.upper().replace('-', '_')
  return "#define {}_VERSION \"{}\"".format(name, describe(directory))


def main():
  if len(sys.argv) != 4:
    print('usage: {} <outdir> <amber-dir> <third_party>'.format(
      sys.argv[0]))
    sys.exit(1)

  outdir = sys.argv[1]
  srcdir = sys.argv[3]

  projects = ['spirv-tools', 'spirv-headers', 'glslang', 'shaderc']
  new_content = get_version_string('amber', sys.argv[2]) + "\n"
  new_content = new_content + ''.join([
    '{}\n'.format(get_version_string(p, os.path.join(srcdir, p)))
    for p in projects
  ])

  file = outdir + "/" + OUTFILE
  if os.path.isfile(file):
    with open(file, 'r') as f:
      if new_content == f.read():
        return
  with open(file, 'w') as f:
    f.write(new_content)


if __name__ == '__main__':
  main()
