#!/usr/bin/env python

# Copyright 2019 The Amber Authors. All rights reserved.
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

# Generates vk-wrappers.inc in the src/ directory.

from __future__ import print_function

import os.path
import re
import sys
import xml.etree.ElementTree as ET
from string import Template

def read_inc(file):
  methods = []
  pattern = re.compile(r"AMBER_VK_FUNC\((\w+)\)")
  with open(file, 'r') as f:
    for line in f:
      match = pattern.search(line)
      if match == None:
        raise Exception("FAILED TO MATCH PATTERN");

      methods.append(match.group(1))
  return methods


def read_vk(file):
  methods = {}
  tree = ET.parse(file)
  root = tree.getroot();
  for command in root.iter("command"):
    proto = command.find('proto')
    if proto == None:
      continue

    return_type = proto.find('type').text
    name = proto.find('name').text

    param_list = []
    for param in command.findall('param'):
      param_val = "".join(param.itertext())
      param_name = param.find('name').text
      param_list.append({
        'def': param_val,
        'name': param_name
      })

    methods[name] = {
      'return_type': return_type,
      'name': name,
      'params': param_list
    }

  return methods


def gen_wrappers(methods, xml):
  content = ""
  for method in methods:
    data = xml[method]
    if data == None:
      raise Exception("Failed to find {}".format(method))

    param_vals = []
    param_names = []
    for param in data['params']:
      param_vals.append(param['def'])
      param_names.append(param['name'])

    signature = ', '.join(str(x) for x in param_vals)
    arguments = ', '.join(str(x) for x in param_names)
    return_type = data['return_type']
    return_variable = ''
    call_prefix = ''
    if return_type != 'void':
      return_variable = 'ret'
      call_prefix = return_type + ' ' + return_variable + ' = '

    template = Template(R'''{
  PFN_${method} ptr = reinterpret_cast<PFN_${method}>(getInstanceProcAddr(instance_, "${method}"));
  if (!ptr) {
    return Result("Vulkan: Unable to load ${method} pointer");
  }
  if (delegate && delegate->LogGraphicsCalls()) {
    ptrs_.${method} = [ptr, delegate](${signature}) -> ${return_type} {
      delegate->Log("${method}");
      uint64_t timestamp_start = 0;
      if (delegate->LogGraphicsCallsTime()) {
        timestamp_start = delegate->GetTimestampNs();
      }
      ${call_prefix}ptr(${arguments});
      if (delegate->LogGraphicsCallsTime()) {
        uint64_t timestamp_end = delegate->GetTimestampNs();
        uint64_t duration = timestamp_end - timestamp_start;
        std::ostringstream out;
        out << "time ";
        // name of method on 40 characters
        out << std::left << std::setw(40) << "${method}";
        // duration in nanoseconds on 12 characters, right-aligned
        out << std::right << std::setw(12) << duration;
        out << " ns";
        delegate->Log(out.str());
      }
      return ${return_variable};
    };
  } else {
    ptrs_.${method} = [ptr](${signature}) -> ${return_type} {
      ${call_prefix}ptr(${arguments});
      return ${return_variable};
    };
  }
}
''')

    content += template.substitute(method=method,
                                   signature=signature,
                                   arguments=arguments,
                                   return_type=return_type,
                                   return_variable=return_variable,
                                   call_prefix=call_prefix)

  return content


def gen_headers(methods, xml):
  content = ""
  for method in methods:
    data = xml[method]
    if data == None:
      raise Exception("Failed to find {}".format(method))

    param_vals = []
    param_names = []
    for param in data['params']:
      param_vals.append(param['def'])
      param_names.append(param['name'])

    content += "std::function<{}({})> {};\n".format(data['return_type'],
        ', '.join(str(x) for x in param_vals), method)

  return content


def gen_direct(methods):
  content = "";

  template = Template(R'''
if (!(ptrs_.${method} = reinterpret_cast<PFN_${method}>(getInstanceProcAddr(instance_, "${method}")))) {
  return Result("Vulkan: Unable to load ${method} pointer");
}
''')

  for method in methods:
    content += template.substitute(method=method)

  return content


def gen_direct_headers(methods):
  content = ""
  for method in methods:
    content += "PFN_{} {};\n".format(method, method);

  return content


def main():
  if len(sys.argv) != 3:
    print('usage: {} <outdir> <src_dir>'.format(
      sys.argv[0]))
    sys.exit(1)

  outdir = sys.argv[1]
  srcdir = sys.argv[2]

  vkfile = os.path.join(srcdir, 'third_party', 'vulkan-headers', 'registry', 'vk.xml')
  incfile = os.path.join(srcdir, 'src', 'vulkan', 'vk-funcs.inc')

  data = read_inc(incfile)

  wrapper_content = ''
  header_content = ''
  if os.path.isfile(vkfile):
    vk_data = read_vk(vkfile)
    wrapper_content = gen_wrappers(data, vk_data)
    header_content = gen_headers(data, vk_data)
  else:
    wrapper_content = gen_direct(data)
    header_content = gen_direct_headers(data)

  outfile = os.path.join(outdir, 'vk-wrappers.inc')
  if os.path.isfile(outfile):
    with open(outfile, 'r') as f:
      if wrapper_content == f.read():
        return
  with open(outfile, 'w') as f:
    f.write(wrapper_content)

  hdrfile = os.path.join(outdir, 'vk-wrappers.h')
  if os.path.isfile(hdrfile):
    with open(hdrfile, 'r') as f:
      if header_content == f.read():
        return
  with open(hdrfile, 'w') as f:
    f.write(header_content)


if __name__ == '__main__':
  main()
