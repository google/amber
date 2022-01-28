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

#include <unordered_map>
#include <utility>

#include "clspv/ArgKind.h"
#include "clspv/Compiler.h"
#include "clspv/Sampler.h"
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"
#include "spirv/unified1/NonSemanticClspvReflection.h"
#include "spirv/unified1/spirv.hpp"

using amber::Pipeline;

namespace {

struct ReflectionHelper {
  Pipeline::ShaderInfo* shader_info = nullptr;
  Pipeline* pipeline = nullptr;
  uint32_t uint_id = 0;
  std::unordered_map<uint32_t, std::string> strings;
  std::unordered_map<uint32_t, uint32_t> constants;
  std::string error_message;
};

Pipeline::ShaderInfo::DescriptorMapEntry::Kind GetArgKindFromExtInst(
    uint32_t value) {
  switch (static_cast<NonSemanticClspvReflectionInstructions>(value)) {
    case NonSemanticClspvReflectionArgumentStorageBuffer:
    case NonSemanticClspvReflectionConstantDataStorageBuffer:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
    case NonSemanticClspvReflectionArgumentUniform:
    case NonSemanticClspvReflectionConstantDataUniform:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::UBO;
    case NonSemanticClspvReflectionArgumentPodStorageBuffer:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
    case NonSemanticClspvReflectionArgumentPodUniform:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO;
    case NonSemanticClspvReflectionArgumentPodPushConstant:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_PUSHCONSTANT;
    case NonSemanticClspvReflectionArgumentSampledImage:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::RO_IMAGE;
    case NonSemanticClspvReflectionArgumentStorageImage:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::WO_IMAGE;
    case NonSemanticClspvReflectionArgumentSampler:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SAMPLER;
    case NonSemanticClspvReflectionArgumentWorkgroup:
    default:
      return Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
  }
}

spv_result_t ParseExtendedInst(ReflectionHelper* helper,
                               const spv_parsed_instruction_t* inst) {
  auto ext_inst = inst->words[inst->operands[3].offset];
  switch (ext_inst) {
    case NonSemanticClspvReflectionKernel: {
      // Remap the name string to the declaration's result id.
      const auto& name = helper->strings[inst->words[inst->operands[5].offset]];
      helper->strings[inst->result_id] = name;
      break;
    }
    case NonSemanticClspvReflectionArgumentInfo: {
      // Remap the name string to the info's result id.
      const auto& name = helper->strings[inst->words[inst->operands[4].offset]];
      helper->strings[inst->result_id] = name;
      break;
    }
    case NonSemanticClspvReflectionArgumentStorageBuffer:
    case NonSemanticClspvReflectionArgumentUniform:
    case NonSemanticClspvReflectionArgumentSampledImage:
    case NonSemanticClspvReflectionArgumentStorageImage:
    case NonSemanticClspvReflectionArgumentSampler: {
      // These arguments have descriptor set and binding.
      auto kernel_id = inst->words[inst->operands[4].offset];
      auto ordinal_id = inst->words[inst->operands[5].offset];
      auto ds_id = inst->words[inst->operands[6].offset];
      auto binding_id = inst->words[inst->operands[7].offset];
      std::string arg_name;
      if (inst->num_operands == 9) {
        arg_name = helper->strings[inst->words[inst->operands[8].offset]];
      }
      auto kind = GetArgKindFromExtInst(ext_inst);
      Pipeline::ShaderInfo::DescriptorMapEntry entry{
          arg_name,
          kind,
          helper->constants[ds_id],
          helper->constants[binding_id],
          helper->constants[ordinal_id],
          /* offset */ 0,
          /* size */ 0};
      helper->shader_info->AddDescriptorEntry(helper->strings[kernel_id],
                                              std::move(entry));
      break;
    }
    case NonSemanticClspvReflectionArgumentPodStorageBuffer:
    case NonSemanticClspvReflectionArgumentPodUniform: {
      // These arguments have descriptor set, binding, offset and size.
      auto kernel_id = inst->words[inst->operands[4].offset];
      auto ordinal_id = inst->words[inst->operands[5].offset];
      auto ds_id = inst->words[inst->operands[6].offset];
      auto binding_id = inst->words[inst->operands[7].offset];
      auto offset_id = inst->words[inst->operands[8].offset];
      auto size_id = inst->words[inst->operands[9].offset];
      std::string arg_name;
      if (inst->num_operands == 11) {
        arg_name = helper->strings[inst->words[inst->operands[10].offset]];
      }
      auto kind = GetArgKindFromExtInst(ext_inst);
      Pipeline::ShaderInfo::DescriptorMapEntry entry{
          arg_name,
          kind,
          helper->constants[ds_id],
          helper->constants[binding_id],
          helper->constants[ordinal_id],
          helper->constants[offset_id],
          helper->constants[size_id]};
      helper->shader_info->AddDescriptorEntry(helper->strings[kernel_id],
                                              std::move(entry));
      break;
    }
    case NonSemanticClspvReflectionArgumentPodPushConstant: {
      // These arguments have offset and size.
      auto kernel_id = inst->words[inst->operands[4].offset];
      auto ordinal_id = inst->words[inst->operands[5].offset];
      auto offset_id = inst->words[inst->operands[6].offset];
      auto size_id = inst->words[inst->operands[7].offset];
      std::string arg_name;
      if (inst->num_operands == 9) {
        arg_name = helper->strings[inst->words[inst->operands[8].offset]];
      }
      auto kind = GetArgKindFromExtInst(ext_inst);
      Pipeline::ShaderInfo::DescriptorMapEntry entry{
          arg_name,
          kind,
          /* descriptor set */ 0,
          /* binding */ 0,
          helper->constants[ordinal_id],
          helper->constants[offset_id],
          helper->constants[size_id]};
      helper->shader_info->AddDescriptorEntry(helper->strings[kernel_id],
                                              std::move(entry));
      break;
    }
    case NonSemanticClspvReflectionArgumentWorkgroup:
      helper->error_message = "Workgroup arguments are not currently supported";
      return SPV_ERROR_INVALID_DATA;
    case NonSemanticClspvReflectionConstantDataStorageBuffer:
    case NonSemanticClspvReflectionConstantDataUniform:
      helper->error_message =
          "Constant descriptor entries are not currently supported";
      return SPV_ERROR_INVALID_DATA;
    case NonSemanticClspvReflectionSpecConstantWorkgroupSize:
    case NonSemanticClspvReflectionSpecConstantGlobalOffset:
    case NonSemanticClspvReflectionSpecConstantWorkDim:
      // Nothing to do. Amber currently requires script authors to know the
      // spec ids and use them directly.
      break;
    case NonSemanticClspvReflectionPushConstantGlobalOffset: {
      auto offset_id = inst->words[inst->operands[4].offset];
      auto size_id = inst->words[inst->operands[5].offset];
      Pipeline::ShaderInfo::PushConstant push_constant{
          Pipeline::ShaderInfo::PushConstant::PushConstantType::kGlobalOffset,
          helper->constants[offset_id], helper->constants[size_id]};
      helper->shader_info->AddPushConstant(std::move(push_constant));
      break;
    }
    case NonSemanticClspvReflectionPushConstantRegionOffset: {
      auto offset_id = inst->words[inst->operands[4].offset];
      auto size_id = inst->words[inst->operands[5].offset];
      Pipeline::ShaderInfo::PushConstant push_constant{
          Pipeline::ShaderInfo::PushConstant::PushConstantType::kRegionOffset,
          helper->constants[offset_id], helper->constants[size_id]};
      helper->shader_info->AddPushConstant(std::move(push_constant));
      break;
    }
    case NonSemanticClspvReflectionPushConstantEnqueuedLocalSize:
    case NonSemanticClspvReflectionPushConstantGlobalSize:
    case NonSemanticClspvReflectionPushConstantNumWorkgroups:
    case NonSemanticClspvReflectionPushConstantRegionGroupOffset:
      helper->error_message = "Unsupported push constant";
      return SPV_ERROR_INVALID_DATA;
    case NonSemanticClspvReflectionLiteralSampler: {
      auto ds_id = inst->words[inst->operands[4].offset];
      auto binding_id = inst->words[inst->operands[5].offset];
      auto mask_id = inst->words[inst->operands[6].offset];
      helper->pipeline->AddSampler(helper->constants[mask_id],
                                   helper->constants[ds_id],
                                   helper->constants[binding_id]);
      break;
    } break;
  }

  return SPV_SUCCESS;
}

spv_result_t ParseReflection(void* user_data,
                             const spv_parsed_instruction_t* inst) {
  auto* helper = reinterpret_cast<ReflectionHelper*>(user_data);
  switch (inst->opcode) {
    case spv::OpTypeInt:
      if (inst->words[inst->operands[1].offset] == 32 &&
          inst->words[inst->operands[2].offset] == 0) {
        // Track the result id of OpTypeInt 32 0.
        helper->uint_id = inst->result_id;
      }
      break;
    case spv::OpConstant:
      if (inst->words[inst->operands[0].offset] == helper->uint_id) {
        // Record the values for all uint32_t constants.
        uint32_t value = inst->words[inst->operands[2].offset];
        helper->constants[inst->result_id] = value;
      }
      break;
    case spv::OpString: {
      // Track all strings.
      std::string value =
          reinterpret_cast<const char*>(inst->words + inst->operands[1].offset);
      helper->strings[inst->result_id] = value;
      break;
    }
    case spv::OpExtInst:
      if (inst->ext_inst_type ==
          SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION) {
        return ParseExtendedInst(helper, inst);
      }
      break;
  }
  return SPV_SUCCESS;
}

}  // anonymous namespace

namespace amber {
namespace clspvhelper {

Result Compile(Pipeline::ShaderInfo* shader_info,
               Pipeline* pipeline,
               spv_target_env env,
               std::vector<uint32_t>* generated_binary) {
  const auto& src_str = shader_info->GetShader()->GetData();
  std::string options;
  for (const auto& option : shader_info->GetCompileOptions()) {
    options += option + " ";
  }
  std::string error_log;
  if (clspv::CompileFromSourceString(src_str, /* sampler map */ "", options,
                                     generated_binary, &error_log)) {
    return Result("Clspv compile error: " + error_log);
  }

  // Parse the reflection instructions.
  ReflectionHelper helper;
  helper.shader_info = shader_info;
  helper.pipeline = pipeline;
  spv_context context(spvContextCreate(env));
  if (spvBinaryParse(context, &helper, generated_binary->data(),
                     generated_binary->size(), nullptr, ParseReflection,
                     nullptr)) {
    return Result(helper.error_message);
  }

  // Strip the reflection instructions to avoid requiring the implementation to
  // support VK_KHR_shader_non_semantic_info.
  spvtools::Optimizer opt(env);
  opt.RegisterPass(spvtools::CreateStripReflectInfoPass());
  std::vector<uint32_t> stripped;
  if (!opt.Run(generated_binary->data(), generated_binary->size(), &stripped)) {
    return Result("failed to strip reflection instructions");
  }
  generated_binary->swap(stripped);

  return Result();
}

}  // namespace clspvhelper
}  // namespace amber
