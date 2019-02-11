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

#include <memory>
#include <string>
#include <vector>

#include "amber/result.h"
#include "src/buffer.h"
#include "src/shader.h"

namespace amber {

enum class PipelineType { kCompute = 0, kGraphics };

class Pipeline {
 public:
  class ShaderInfo {
   public:
    ShaderInfo(const Shader*, ShaderType type);
    ShaderInfo(const ShaderInfo&);
    ~ShaderInfo();

    void SetShaderOptimizations(const std::vector<std::string>& opts) {
      shader_optimizations_ = opts;
    }
    const std::vector<std::string>& GetShaderOptimizations() const {
      return shader_optimizations_;
    }

    const Shader* GetShader() const { return shader_; }

    void SetEntryPoint(const std::string& ep) { entry_point_ = ep; }
    std::string GetEntryPoint() const { return entry_point_; }

    void SetShaderType(ShaderType type) { shader_type_ = type; }
    ShaderType GetShaderType() const { return shader_type_; }

   private:
    const Shader* shader_ = nullptr;
    ShaderType shader_type_;
    std::vector<std::string> shader_optimizations_;
    std::string entry_point_;
  };

  struct BufferInfo {
    BufferInfo() = default;
    explicit BufferInfo(Buffer* buf) : buffer(buf) {}

    Buffer* buffer = nullptr;
    uint32_t location = 0;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  static const char* kDefaultColorBufferFormat;
  static const char* kDefaultDepthBufferFormat;
  static const char* kGeneratedColorBuffer;
  static const char* kGeneratedDepthBuffer;

  explicit Pipeline(PipelineType type);
  ~Pipeline();

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

  Result AddShader(const Shader*, ShaderType);
  const std::vector<ShaderInfo>& GetShaders() const { return shaders_; }

  Result SetShaderType(const Shader* shader, ShaderType type);
  Result SetShaderEntryPoint(const Shader* shader, const std::string& name);
  Result SetShaderOptimizations(const Shader* shader,
                                const std::vector<std::string>& opts);

  const std::vector<BufferInfo>& GetColorAttachments() const {
    return color_attachments_;
  }
  Result AddColorAttachment(Buffer* buf, uint32_t location);

  Result SetDepthBuffer(Buffer* buf);
  const BufferInfo& GetDepthBuffer() const { return depth_buffer_; }

  const std::vector<BufferInfo>& GetVertexBuffers() const {
    return vertex_buffers_;
  }
  Result AddVertexBuffer(Buffer* buf, uint32_t location);

  Result SetIndexBuffer(Buffer* buf);
  const Buffer* GetIndexBuffer() const { return index_buffer_; }

  // Validates that the pipeline has been created correctly.
  Result Validate() const;

  std::unique_ptr<Buffer> GenerateDefaultColorAttachmentBuffer() const;
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
  BufferInfo depth_buffer_;
  Buffer* index_buffer_ = nullptr;

  uint32_t fb_width_ = 250;
  uint32_t fb_height_ = 250;
};

}  // namespace amber

#endif  // SRC_PIPELINE_H_
