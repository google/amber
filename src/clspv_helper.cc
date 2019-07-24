// Copyright 2019 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/clspv_helper.h"

#include <utility>

#include "clspv/ArgKind.h"
#include "clspv/Compiler.h"

namespace amber {
namespace clspvhelper {

Result Compile(Pipeline::ShaderInfo* shader_info,
               std::vector<uint32_t>* generated_binary) {
  std::vector<clspv::version0::DescriptorMapEntry> entries;
  const auto& src_str = shader_info->GetShader()->GetData();
  std::string options;
  for (const auto& option : shader_info->GetCompileOptions()) {
    options += option + " ";
  }
  if (clspv::CompileFromSourceString(src_str, /* sampler map */ "", options,
                                     generated_binary, &entries)) {
    return Result("Clspv compile failed");
  }

  for (auto& entry : entries) {
    if (entry.kind != clspv::version0::DescriptorMapEntry::KernelArg) {
      return Result(
          "Only kernel argument descriptor entries are currently supported");
    }

    Pipeline::ShaderInfo::DescriptorMapEntry descriptor_entry;
    descriptor_entry.descriptor_set = entry.descriptor_set;
    descriptor_entry.binding = entry.binding;
    descriptor_entry.pod_offset = 0;
    descriptor_entry.pod_arg_size = 0;
    switch (entry.kernel_arg_data.arg_kind) {
      case clspv::ArgKind::Buffer:
        descriptor_entry.kind =
            Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
        break;
      case clspv::ArgKind::BufferUBO:
        descriptor_entry.kind =
            Pipeline::ShaderInfo::DescriptorMapEntry::Kind::UBO;
        break;
      case clspv::ArgKind::Pod:
        descriptor_entry.kind =
            Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
        break;
      case clspv::ArgKind::PodUBO:
        descriptor_entry.kind =
            Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO;
        break;
      case clspv::ArgKind::Local:
        // Local arguments are handled via specialization constants.
        break;
      default:
        return Result("Unsupported kernel argument descriptor entry");
    }

    if (entry.kernel_arg_data.arg_kind == clspv::ArgKind::Pod ||
        entry.kernel_arg_data.arg_kind == clspv::ArgKind::PodUBO) {
      descriptor_entry.pod_offset = entry.kernel_arg_data.pod_offset;
      descriptor_entry.pod_arg_size = entry.kernel_arg_data.pod_arg_size;
    }

    descriptor_entry.arg_name = entry.kernel_arg_data.arg_name;
    descriptor_entry.arg_ordinal = entry.kernel_arg_data.arg_ordinal;

    shader_info->AddDescriptorEntry(entry.kernel_arg_data.kernel_name,
                                    std::move(descriptor_entry));
  }

  return Result();
}

}  // namespace clspvhelper
}  // namespace amber
