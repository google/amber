// Copyright 2018 The Amber Authors.
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
#include <utility>
#include <vector>

#include "amber/result.h"
#include "src/buffer.h"
#include "src/shader.h"

namespace amber {

enum class PipelineType { kCompute = 0, kGraphics };

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

    void SetShaderOptimizations(const std::vector<std::string>& opts) {
      shader_optimizations_ = opts;
    }
    const std::vector<std::string>& GetShaderOptimizations() const {
      return shader_optimizations_;
    }

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

   private:
    Shader* shader_ = nullptr;
    ShaderType shader_type_;
    std::vector<std::string> shader_optimizations_;
    std::string entry_point_;
    std::vector<uint32_t> data_;
    std::map<uint32_t, uint32_t> specialization_;
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
  };

  static const char* kGeneratedColorBuffer;
  static const char* kGeneratedDepthBuffer;

  explicit Pipeline(PipelineType type);
  ~Pipeline();

  std::unique_ptr<Pipeline> Clone() const;

  bool IsGraphics() const { return pipeline_type_ == PipelineType::kGraphics; }
  bool IsCompute() const { return pipeline_type_ == PipelineType::kCompute; }

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

  /// Sets the |type| of |shader| in the pipeline.
  Result SetShaderType(const Shader* shader, ShaderType type);
  /// Sets the entry point |name| for |shader| in this pipeline.
  Result SetShaderEntryPoint(const Shader* shader, const std::string& name);
  /// Sets the optimizations (|opts|) for |shader| in this pipeline.
  Result SetShaderOptimizations(const Shader* shader,
                                const std::vector<std::string>& opts);

  /// Returns a list of all colour attachments in this pipeline.
  const std::vector<BufferInfo>& GetColorAttachments() const {
    return color_attachments_;
  }
  /// Adds |buf| as a colour attachment at |location| in the pipeline.
  Result AddColorAttachment(Buffer* buf, uint32_t location);
  /// Retrieves the location that |buf| is bound to in the pipeline. The
  /// location will be written to |loc|. An error result will be return if
  /// something goes wrong.
  Result GetLocationForColorAttachment(Buffer* buf, uint32_t* loc) const;

  /// Sets |buf| as the depth buffer for this pipeline.
  Result SetDepthBuffer(Buffer* buf);
  /// Returns information on the depth buffer bound to the pipeline. If no
  /// depth buffer is bound the |BufferInfo::buffer| parameter will be nullptr.
  const BufferInfo& GetDepthBuffer() const { return depth_buffer_; }

  /// Returns information on all vertex buffers bound to the pipeline.
  const std::vector<BufferInfo>& GetVertexBuffers() const {
    return vertex_buffers_;
  }
  /// Adds |buf| as a vertex buffer at |location| in the pipeline.
  Result AddVertexBuffer(Buffer* buf, uint32_t location);

  /// Binds |buf| as the index buffer for this pipeline.
  Result SetIndexBuffer(Buffer* buf);
  /// Returns the index buffer bound to this pipeline or nullptr if no index
  /// buffer bound.
  Buffer* GetIndexBuffer() const { return index_buffer_; }

  /// Adds |buf| to the pipeline at the given |descriptor_set| and |binding|.
  void AddBuffer(Buffer* buf, uint32_t descriptor_set, uint32_t binding);
  /// Returns information on all buffers in this pipeline.
  const std::vector<BufferInfo>& GetBuffers() const { return buffers_; }

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
  std::unique_ptr<Buffer> GenerateDefaultColorAttachmentBuffer() const;
  /// Generates a default depth attachment in D32_SFLOAT_S8_UINT format.
  std::unique_ptr<Buffer> GenerateDefaultDepthAttachmentBuffer() const;

 private:
  void UpdateFramebufferSizes();

  Result ValidateGraphics() const;
  Result ValidateCompute() const;

  PipelineType pipeline_type_ = PipelineType::kCompute;
  std::string name_;
  std::vector<ShaderInfo> shaders_;
  std::vector<BufferInfo> color_attachments_;
  std::vector<BufferInfo> vertex_buffers_;
  std::vector<BufferInfo> buffers_;
  BufferInfo depth_buffer_;
  BufferInfo push_constant_buffer_;
  Buffer* index_buffer_ = nullptr;

  uint32_t fb_width_ = 250;
  uint32_t fb_height_ = 250;
};

}  // namespace amber

#endif  // SRC_PIPELINE_H_
