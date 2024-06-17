// Copyright 2018 The Amber Authors.
// Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#ifndef SRC_PIPELINE_H_
#define SRC_PIPELINE_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "src/acceleration_structure.h"
#include "src/buffer.h"
#include "src/command_data.h"
#include "src/pipeline_data.h"
#include "src/sampler.h"
#include "src/shader.h"

namespace amber {

enum class PipelineType { kCompute = 0, kGraphics, kRayTracing };

/// Stores all information related to a pipeline.
class Pipeline {
 public:
  /// Information on a shader attached to this pipeline.
  class ShaderInfo {
   public:
    ShaderInfo(Shader*, ShaderType type);
    ShaderInfo(const ShaderInfo&);
    ~ShaderInfo();

    ShaderInfo& operator=(const ShaderInfo&) = default;

    // Set the optimization options for this shader. Optimizations are
    // specified like command-line arguments to spirv-opt (see its --help).
    // Parsing is done by spvtools::Optimizer::RegisterPassesFromFlags (see
    // SPIRV-Tools include/spirv-tools/optimizer.hpp).
    void SetShaderOptimizations(const std::vector<std::string>& opts) {
      shader_optimizations_ = opts;
    }
    const std::vector<std::string>& GetShaderOptimizations() const {
      return shader_optimizations_;
    }

    void SetCompileOptions(const std::vector<std::string>& options) {
      compile_options_ = options;
    }
    const std::vector<std::string>& GetCompileOptions() const {
      return compile_options_;
    }

    enum class RequiredSubgroupSizeSetting : uint32_t {
      kNotSet = 0,
      kSetToSpecificSize,
      kSetToMinimumSize,
      kSetToMaximumSize
    };

    void SetRequiredSubgroupSizeSetting(RequiredSubgroupSizeSetting setting,
                                        uint32_t size) {
      required_subgroup_size_setting_ = setting;
      required_subgroup_size_ = size;
    }
    RequiredSubgroupSizeSetting GetRequiredSubgroupSizeSetting() const {
      return required_subgroup_size_setting_;
    }
    uint32_t GetRequiredSubgroupSize() const { return required_subgroup_size_; }

    void SetVaryingSubgroupSize(const bool isSet) {
      varying_subgroup_size_ = isSet;
    }
    bool GetVaryingSubgroupSize() const { return varying_subgroup_size_; }

    void SetRequireFullSubgroups(const bool isSet) {
      require_full_subgroups_ = isSet;
    }
    bool GetRequireFullSubgroups() const { return require_full_subgroups_; }

    void SetShader(Shader* shader) { shader_ = shader; }
    const Shader* GetShader() const { return shader_; }

    void SetEntryPoint(const std::string& ep) { entry_point_ = ep; }
    const std::string& GetEntryPoint() const { return entry_point_; }

    void SetShaderType(ShaderType type) { shader_type_ = type; }
    ShaderType GetShaderType() const { return shader_type_; }

    const std::vector<uint32_t> GetData() const { return data_; }
    void SetData(std::vector<uint32_t>&& data) { data_ = std::move(data); }

    const std::map<uint32_t, uint32_t>& GetSpecialization() const {
      return specialization_;
    }
    void AddSpecialization(uint32_t spec_id, uint32_t value) {
      specialization_[spec_id] = value;
    }

    /// Descriptor information for an OpenCL-C shader.
    struct DescriptorMapEntry {
      std::string arg_name = "";

      enum class Kind : int {
        UNKNOWN,
        SSBO,
        UBO,
        POD,
        POD_UBO,
        POD_PUSHCONSTANT,
        RO_IMAGE,
        WO_IMAGE,
        SAMPLER,
      } kind;

      uint32_t descriptor_set = 0;
      uint32_t binding = 0;
      uint32_t arg_ordinal = 0;
      uint32_t pod_offset = 0;
      uint32_t pod_arg_size = 0;
    };

    void AddDescriptorEntry(const std::string& kernel,
                            DescriptorMapEntry&& entry) {
      descriptor_map_[kernel].emplace_back(std::move(entry));
    }
    const std::unordered_map<std::string, std::vector<DescriptorMapEntry>>&
    GetDescriptorMap() const {
      return descriptor_map_;
    }

    /// Push constant information for an OpenCL-C shader.
    struct PushConstant {
      enum class PushConstantType {
        kDimensions = 0,
        kGlobalOffset,
        kRegionOffset,
      };
      PushConstantType type;
      uint32_t offset = 0;
      uint32_t size = 0;
    };

    void AddPushConstant(PushConstant&& pc) {
      push_constants_.emplace_back(std::move(pc));
    }
    const std::vector<PushConstant>& GetPushConstants() const {
      return push_constants_;
    }

   private:
    Shader* shader_ = nullptr;
    ShaderType shader_type_;
    std::vector<std::string> shader_optimizations_;
    std::string entry_point_;
    std::vector<uint32_t> data_;
    std::map<uint32_t, uint32_t> specialization_;
    std::unordered_map<std::string, std::vector<DescriptorMapEntry>>
        descriptor_map_;
    std::vector<PushConstant> push_constants_;
    std::vector<std::string> compile_options_;
    RequiredSubgroupSizeSetting required_subgroup_size_setting_;
    uint32_t required_subgroup_size_;
    bool varying_subgroup_size_;
    bool require_full_subgroups_;
  };

  /// Information on a buffer attached to the pipeline.
  ///
  /// The BufferInfo will have either (descriptor_set, binding) or location
  /// attached.
  struct BufferInfo {
    BufferInfo() = default;
    explicit BufferInfo(Buffer* buf) : buffer(buf) {}

    Buffer* buffer = nullptr;
    uint32_t descriptor_set = 0;
    uint32_t binding = 0;
    uint32_t location = 0;
    uint32_t base_mip_level = 0;
    uint32_t dynamic_offset = 0;
    std::string arg_name = "";
    uint32_t arg_no = 0;
    BufferType type = BufferType::kUnknown;
    InputRate input_rate = InputRate::kVertex;
    Format* format;
    uint32_t offset = 0;
    uint32_t stride = 0;
    Sampler* sampler = nullptr;
    uint64_t descriptor_offset = 0;
    uint64_t descriptor_range = ~0ULL;  // ~0ULL == VK_WHOLE_SIZE
  };

  /// Information on a sampler attached to the pipeline.
  struct SamplerInfo {
    SamplerInfo() = default;
    explicit SamplerInfo(Sampler* samp) : sampler(samp) {}

    Sampler* sampler = nullptr;
    uint32_t descriptor_set = 0;
    uint32_t binding = 0;
    std::string arg_name = "";
    uint32_t arg_no = 0;
    uint32_t mask = 0;
  };

  /// Information on a top level acceleration structure at the pipeline.
  struct TLASInfo {
    TLASInfo() = default;
    explicit TLASInfo(TLAS* as) : tlas(as) {}

    TLAS* tlas = nullptr;
    uint32_t descriptor_set = 0;
    uint32_t binding = 0;
  };
  static const char* kGeneratedColorBuffer;
  static const char* kGeneratedDepthBuffer;
  static const char* kGeneratedPushConstantBuffer;

  explicit Pipeline(PipelineType type);
  ~Pipeline();

  std::unique_ptr<Pipeline> Clone() const;

  bool IsGraphics() const { return pipeline_type_ == PipelineType::kGraphics; }
  bool IsCompute() const { return pipeline_type_ == PipelineType::kCompute; }
  bool IsRayTracing() const {
    return pipeline_type_ == PipelineType::kRayTracing;
  }

  PipelineType GetType() const { return pipeline_type_; }

  void SetName(const std::string& name) { name_ = name; }
  const std::string& GetName() const { return name_; }

  void SetFramebufferWidth(uint32_t fb_width) {
    fb_width_ = fb_width;
    UpdateFramebufferSizes();
  }
  uint32_t GetFramebufferWidth() const { return fb_width_; }

  void SetFramebufferHeight(uint32_t fb_height) {
    fb_height_ = fb_height;
    UpdateFramebufferSizes();
  }
  uint32_t GetFramebufferHeight() const { return fb_height_; }

  /// Adds |shader| of |type| to the pipeline.
  Result AddShader(Shader* shader, ShaderType type);
  /// Returns information on all bound shaders in this pipeline.
  std::vector<ShaderInfo>& GetShaders() { return shaders_; }
  /// Returns information on all bound shaders in this pipeline.
  const std::vector<ShaderInfo>& GetShaders() const { return shaders_; }

  /// Returns the ShaderInfo for |shader| or nullptr.
  const ShaderInfo* GetShader(Shader* shader) const {
    for (const auto& info : shaders_) {
      if (info.GetShader() == shader)
        return &info;
    }
    return nullptr;
  }

  /// Adds |shaders| to the pipeline.
  /// Designed to support libraries
  Result AddShaders(const std::vector<ShaderInfo>& lib_shaders) {
    shaders_.reserve(shaders_.size() + lib_shaders.size());
    shaders_.insert(std::end(shaders_), std::begin(lib_shaders),
                    std::end(lib_shaders));

    return {};
  }

  /// Returns a success result if |shader| found and the shader index is
  /// returned in |out|. Returns failure otherwise.
  Result GetShaderIndex(Shader* shader, uint32_t* out) const {
    for (size_t index = 0; index < shaders_.size(); index++) {
      if (shaders_[index].GetShader() == shader) {
        *out = static_cast<uint32_t>(index);
        return {};
      }
    }
    return Result("Referred shader not found in group");
  }

  /// Sets the |type| of |shader| in the pipeline.
  Result SetShaderType(const Shader* shader, ShaderType type);
  /// Sets the entry point |name| for |shader| in this pipeline.
  Result SetShaderEntryPoint(const Shader* shader, const std::string& name);
  /// Sets the optimizations (|opts|) for |shader| in this pipeline.
  Result SetShaderOptimizations(const Shader* shader,
                                const std::vector<std::string>& opts);
  /// Sets the compile options for |shader| in this pipeline.
  Result SetShaderCompileOptions(const Shader* shader,
                                 const std::vector<std::string>& options);
  /// Sets required subgroup size.
  Result SetShaderRequiredSubgroupSize(const Shader* shader,
                                       const uint32_t subgroupSize);
  /// Sets required subgroup size to the device minimum supported subgroup size.
  Result SetShaderRequiredSubgroupSizeToMinimum(const Shader* shader);

  /// Sets required subgroup size to the device maximum supported subgroup size.
  Result SetShaderRequiredSubgroupSizeToMaximum(const Shader* shader);

  /// Sets varying subgroup size property.
  Result SetShaderVaryingSubgroupSize(const Shader* shader, const bool isSet);

  /// Sets require full subgroups property.
  Result SetShaderRequireFullSubgroups(const Shader* shader, const bool isSet);
  /// Returns a list of all colour attachments in this pipeline.
  const std::vector<BufferInfo>& GetColorAttachments() const {
    return color_attachments_;
  }
  /// Adds |buf| as a colour attachment at |location| in the pipeline.
  /// Uses |base_mip_level| as the mip level for output.
  Result AddColorAttachment(Buffer* buf,
                            uint32_t location,
                            uint32_t base_mip_level);
  /// Retrieves the location that |buf| is bound to in the pipeline. The
  /// location will be written to |loc|. An error result will be return if
  /// something goes wrong.
  Result GetLocationForColorAttachment(Buffer* buf, uint32_t* loc) const;

  /// Returns a list of all resolve targets in this pipeline.
  const std::vector<BufferInfo>& GetResolveTargets() const {
    return resolve_targets_;
  }

  /// Adds |buf| as a multisample resolve target in the pipeline.
  Result AddResolveTarget(Buffer* buf);

  /// Sets |buf| as the depth/stencil buffer for this pipeline.
  Result SetDepthStencilBuffer(Buffer* buf);
  /// Returns information on the depth/stencil buffer bound to the pipeline. If
  /// no depth buffer is bound the |BufferInfo::buffer| parameter will be
  /// nullptr.
  const BufferInfo& GetDepthStencilBuffer() const {
    return depth_stencil_buffer_;
  }

  /// Returns pipeline data.
  PipelineData* GetPipelineData() { return &pipeline_data_; }

  /// Returns information on all vertex buffers bound to the pipeline.
  const std::vector<BufferInfo>& GetVertexBuffers() const {
    return vertex_buffers_;
  }
  /// Adds |buf| as a vertex buffer at |location| in the pipeline using |rate|
  /// as the input rate, |format| as vertex data format, |offset| as a starting
  /// offset for the vertex buffer data, and |stride| for the data stride in
  /// bytes.
  Result AddVertexBuffer(Buffer* buf,
                         uint32_t location,
                         InputRate rate,
                         Format* format,
                         uint32_t offset,
                         uint32_t stride);

  /// Binds |buf| as the index buffer for this pipeline.
  Result SetIndexBuffer(Buffer* buf);
  /// Returns the index buffer bound to this pipeline or nullptr if no index
  /// buffer bound.
  Buffer* GetIndexBuffer() const { return index_buffer_; }

  /// Adds |buf| of |type| to the pipeline at the given |descriptor_set|,
  /// |binding|, |base_mip_level|, |descriptor_offset|, |descriptor_range| and
  /// |dynamic_offset|.
  void AddBuffer(Buffer* buf,
                 BufferType type,
                 uint32_t descriptor_set,
                 uint32_t binding,
                 uint32_t base_mip_level,
                 uint32_t dynamic_offset,
                 uint64_t descriptor_offset,
                 uint64_t descriptor_range);
  /// Adds |buf| to the pipeline at the given |arg_name|.
  void AddBuffer(Buffer* buf, BufferType type, const std::string& arg_name);
  /// Adds |buf| to the pipeline at the given |arg_no|.
  void AddBuffer(Buffer* buf, BufferType type, uint32_t arg_no);
  /// Returns information on all buffers in this pipeline.
  const std::vector<BufferInfo>& GetBuffers() const { return buffers_; }
  /// Clears all buffer bindings for given |descriptor_set| and |binding|.
  void ClearBuffers(uint32_t descriptor_set, uint32_t binding);

  /// Adds |sampler| to the pipeline at the given |descriptor_set| and
  /// |binding|.
  void AddSampler(Sampler* sampler, uint32_t descriptor_set, uint32_t binding);
  /// Adds |sampler| to the pipeline at the given |arg_name|.
  void AddSampler(Sampler* sampler, const std::string& arg_name);
  /// Adds |sampler| to the pieline at the given |arg_no|.
  void AddSampler(Sampler* sampler, uint32_t arg_no);
  /// Adds an entry for an OpenCL literal sampler.
  void AddSampler(uint32_t sampler_mask,
                  uint32_t descriptor_set,
                  uint32_t binding);
  /// Clears all sampler bindings for given |descriptor_set| and |binding|.
  void ClearSamplers(uint32_t descriptor_set, uint32_t binding);

  /// Returns information on all samplers in this pipeline.
  const std::vector<SamplerInfo>& GetSamplers() const { return samplers_; }

  /// Adds |tlas| to the pipeline at the given |descriptor_set| and
  /// |binding|.
  void AddTLAS(TLAS* tlas, uint32_t descriptor_set, uint32_t binding);

  /// Returns information on all bound TLAS in the pipeline.
  std::vector<TLASInfo>& GetTLASes() { return tlases_; }

  /// Adds |sbt| to the list of known shader binding tables.
  /// The |sbt| must have a unique name within pipeline.
  Result AddSBT(std::unique_ptr<SBT> sbt) {
    if (name_to_sbt_.count(sbt->GetName()) > 0)
      return Result("duplicate SBT name provided");

    sbts_.push_back(std::move(sbt));
    name_to_sbt_[sbts_.back()->GetName()] = sbts_.back().get();

    return {};
  }

  /// Retrieves the SBT with |name|, |nullptr| if not found.
  SBT* GetSBT(const std::string& name) const {
    auto it = name_to_sbt_.find(name);
    return it == name_to_sbt_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all SBTs.
  const std::vector<std::unique_ptr<SBT>>& GetSBTs() const { return sbts_; }

  /// Adds |group| to the list of known shader groups.
  /// The |group| must have a unique name within pipeline.
  Result AddShaderGroup(std::shared_ptr<ShaderGroup> group) {
    if (name_to_shader_group_.count(group->GetName()) > 0)
      return Result("shader group name already exists");

    shader_groups_.push_back(std::move(group));
    name_to_shader_group_[shader_groups_.back()->GetName()] =
        shader_groups_.back().get();

    return {};
  }

  /// Retrieves the Shader Group with |name|, |nullptr| if not found.
  ShaderGroup* GetShaderGroup(const std::string& name) const {
    auto it = name_to_shader_group_.find(name);
    return it == name_to_shader_group_.end() ? nullptr : it->second;
  }
  /// Retrieves a Shader Group at given |index|.
  ShaderGroup* GetShaderGroupByIndex(uint32_t index) const {
    return shader_groups_[index].get();
  }
  /// Retreives index of shader group specified by |name|
  uint32_t GetShaderGroupIndex(const std::string& name) const {
    ShaderGroup* shader_group = GetShaderGroup(name);

    for (size_t i = 0; i < shader_groups_.size(); i++) {
      if (shader_groups_[i].get() == shader_group) {
        return static_cast<uint32_t>(i);
      }
    }

    return static_cast<uint32_t>(-1);
  }
  /// Retrieves a list of all Shader Groups.
  const std::vector<std::shared_ptr<ShaderGroup>>& GetShaderGroups() const {
    return shader_groups_;
  }

  /// Updates the descriptor set and binding info for the OpenCL-C kernel bound
  /// to the pipeline. No effect for other shader formats.
  Result UpdateOpenCLBufferBindings();

  /// Returns the buffer which is currently bound to this pipeline at
  /// |descriptor_set| and |binding|.
  Buffer* GetBufferForBinding(uint32_t descriptor_set, uint32_t binding) const;

  Result SetPushConstantBuffer(Buffer* buf);
  const BufferInfo& GetPushConstantBuffer() const {
    return push_constant_buffer_;
  }

  /// Validates that the pipeline has been created correctly.
  Result Validate() const;

  /// Generates a default color attachment in B8G8R8A8_UNORM.
  std::unique_ptr<Buffer> GenerateDefaultColorAttachmentBuffer();
  /// Generates a default depth/stencil attachment in D32_SFLOAT_S8_UINT format.
  std::unique_ptr<Buffer> GenerateDefaultDepthStencilAttachmentBuffer();

  /// Information on values set for OpenCL-C plain-old-data args.
  struct ArgSetInfo {
    std::string name;
    uint32_t ordinal = 0;
    Format* fmt = nullptr;
    Value value;
  };

  /// Adds value from SET command.
  void SetArg(ArgSetInfo&& info) { set_arg_values_.push_back(std::move(info)); }
  const std::vector<ArgSetInfo>& SetArgValues() const {
    return set_arg_values_;
  }

  /// Generate the buffers necessary for OpenCL PoD arguments populated via SET
  /// command. This should be called after all other buffers are bound.
  Result GenerateOpenCLPodBuffers();

  /// Generate the samplers necessary for OpenCL literal samplers from the
  /// descriptor map. This should be called after all other samplers are bound.
  Result GenerateOpenCLLiteralSamplers();

  /// Generate the push constant buffers necessary for OpenCL kernels.
  Result GenerateOpenCLPushConstants();

  void SetMaxPipelineRayPayloadSize(uint32_t size) {
    max_pipeline_ray_payload_size_ = size;
  }
  uint32_t GetMaxPipelineRayPayloadSize() {
    return max_pipeline_ray_payload_size_;
  }
  void SetMaxPipelineRayHitAttributeSize(uint32_t size) {
    max_pipeline_ray_hit_attribute_size_ = size;
  }
  uint32_t GetMaxPipelineRayHitAttributeSize() {
    return max_pipeline_ray_hit_attribute_size_;
  }
  void SetMaxPipelineRayRecursionDepth(uint32_t depth) {
    max_pipeline_ray_recursion_depth_ = depth;
  }
  uint32_t GetMaxPipelineRayRecursionDepth() {
    return max_pipeline_ray_recursion_depth_;
  }
  void SetCreateFlags(uint32_t flags) {
    create_flags_ = flags;
  }
  uint32_t GetCreateFlags() const {
    return create_flags_;
  }

  void AddPipelineLibrary(Pipeline* pipeline) { libs_.push_back(pipeline); }
  const std::vector<Pipeline*>& GetPipelineLibraries() const { return libs_; }

 private:
  void UpdateFramebufferSizes();

  Result SetShaderRequiredSubgroupSize(
      const Shader* shader,
      const ShaderInfo::RequiredSubgroupSizeSetting setting,
      const uint32_t subgroupSize);

  Result CreatePushConstantBuffer();

  Result ValidateGraphics() const;
  Result ValidateCompute() const;
  Result ValidateRayTracing() const;

  PipelineType pipeline_type_ = PipelineType::kCompute;
  std::string name_;
  std::vector<ShaderInfo> shaders_;
  std::vector<TLASInfo> tlases_;
  std::vector<BufferInfo> color_attachments_;
  std::vector<BufferInfo> resolve_targets_;
  std::vector<BufferInfo> vertex_buffers_;
  std::vector<BufferInfo> buffers_;
  std::vector<std::unique_ptr<type::Type>> types_;
  std::vector<SamplerInfo> samplers_;
  std::vector<std::unique_ptr<Format>> formats_;
  BufferInfo depth_stencil_buffer_;
  BufferInfo push_constant_buffer_;
  Buffer* index_buffer_ = nullptr;
  PipelineData pipeline_data_;
  uint32_t fb_width_ = 250;
  uint32_t fb_height_ = 250;

  std::vector<ArgSetInfo> set_arg_values_;
  std::vector<std::unique_ptr<Buffer>> opencl_pod_buffers_;
  /// Maps (descriptor set, binding) to the buffer for that binding pair.
  std::map<std::pair<uint32_t, uint32_t>, Buffer*> opencl_pod_buffer_map_;
  std::vector<std::unique_ptr<Sampler>> opencl_literal_samplers_;
  std::unique_ptr<Buffer> opencl_push_constants_;

  std::map<std::string, ShaderGroup*> name_to_shader_group_;
  std::vector<std::shared_ptr<ShaderGroup>> shader_groups_;
  std::map<std::string, SBT*> name_to_sbt_;
  std::vector<std::unique_ptr<SBT>> sbts_;
  uint32_t max_pipeline_ray_payload_size_ = 0;
  uint32_t max_pipeline_ray_hit_attribute_size_ = 0;
  uint32_t max_pipeline_ray_recursion_depth_ = 1;
  uint32_t create_flags_ = 0;
  std::vector<Pipeline*> libs_;
};

}  // namespace amber

#endif  // SRC_PIPELINE_H_
