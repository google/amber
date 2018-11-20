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

#include "src/vulkan/graphics_pipeline.h"

#include <cassert>
#include <cmath>

#include "src/command.h"
#include "src/make_unique.h"
#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {
namespace {

const VkAttachmentDescription kDefaultAttachmentDesc = {
    0,                     /* flags */
    VK_FORMAT_UNDEFINED,   /* format */
    VK_SAMPLE_COUNT_1_BIT, /* samples */
    // TODO(jaebaek): Set up proper loadOp, StoreOp.
    VK_ATTACHMENT_LOAD_OP_LOAD,           /* loadOp */
    VK_ATTACHMENT_STORE_OP_STORE,         /* storeOp */
    VK_ATTACHMENT_LOAD_OP_LOAD,           /* stencilLoadOp */
    VK_ATTACHMENT_STORE_OP_STORE,         /* stencilStoreOp */
    VK_IMAGE_LAYOUT_UNDEFINED,            /* initialLayout */
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, /* finalLayout */
};

const VkPipelineRasterizationStateCreateInfo kDefaultRasterizationInfo = {
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, /* sType */
    nullptr,                                                    /* pNext */
    0,                                                          /* flags */
    VK_FALSE,                /* depthClampEnable */
    VK_FALSE,                /* rasterizerDiscardEnable */
    VK_POLYGON_MODE_FILL,    /* polygonMode */
    VK_CULL_MODE_NONE,       /* cullMode */
    VK_FRONT_FACE_CLOCKWISE, /* frontFace */
    VK_FALSE,                /* depthBiasEnable */
    0,                       /* depthBiasConstantFactor */
    0,                       /* depthBiasClamp */
    0,                       /* depthBiasSlopeFactor */
    0,                       /* lineWidth */
};

const VkSampleMask kSampleMask = ~0U;

const VkPipelineMultisampleStateCreateInfo kMultisampleInfo = {
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, /* sType */
    nullptr,                                                  /* pNext */
    0,                                                        /* flags */
    kDefaultAttachmentDesc.samples, /* rasterizationSamples */
    VK_FALSE,                       /* sampleShadingEnable */
    0,                              /* minSampleShading */
    &kSampleMask,                   /* pSampleMask */
    VK_FALSE,                       /* alphaToCoverageEnable */
    VK_FALSE,                       /* alphaToOneEnable */
};

const float kEpsilon = 0.002f;

bool IsFloatPixelEqualInt(float pixel, uint8_t expected) {
  // TODO(jaebaek): Change kEpsilon to tolerance.
  return std::fabs(pixel - static_cast<float>(expected) / 255.0f) < kEpsilon;
}

VkPrimitiveTopology ToVkTopology(Topology topology) {
  switch (topology) {
    case Topology::kPointList:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case Topology::kLineList:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case Topology::kLineStrip:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case Topology::kTriangleList:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case Topology::kTriangleStrip:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case Topology::kTriangleFan:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case Topology::kLineListWithAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case Topology::kLineStripWithAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case Topology::kTriangleListWithAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case Topology::kTriangleStripWithAdjacency:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case Topology::kPatchList:
      return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default:
      break;
  }

  assert(false && "Vulkan::Unknown topology");
  return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
}

}  // namespace

GraphicsPipeline::GraphicsPipeline(
    PipelineType type,
    VkDevice device,
    const VkPhysicalDeviceMemoryProperties& properties,
    VkFormat color_format,
    VkFormat depth_stencil_format,
    uint32_t fence_timeout_ms,
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_info)
    : Pipeline(type, device, properties),
      color_format_(color_format),
      depth_stencil_format_(depth_stencil_format),
      shader_stage_info_(shader_stage_info),
      fence_timeout_ms_(fence_timeout_ms) {}

GraphicsPipeline::~GraphicsPipeline() = default;

Result GraphicsPipeline::CreateRenderPass() {
  VkSubpassDescription subpass_desc = {};
  subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  std::vector<VkAttachmentDescription> attachment_desc;

  VkAttachmentReference color_refer = {};
  VkAttachmentReference depth_refer = {};

  if (color_format_ != VK_FORMAT_UNDEFINED) {
    attachment_desc.push_back(kDefaultAttachmentDesc);
    attachment_desc.back().format = color_format_;

    color_refer.attachment = static_cast<uint32_t>(attachment_desc.size() - 1);
    color_refer.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_refer;
  }

  if (depth_stencil_format_ != VK_FORMAT_UNDEFINED) {
    attachment_desc.push_back(kDefaultAttachmentDesc);
    attachment_desc.back().format = depth_stencil_format_;

    depth_refer.attachment = static_cast<uint32_t>(attachment_desc.size() - 1);
    depth_refer.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass_desc.pDepthStencilAttachment = &depth_refer;
  }

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount =
      static_cast<uint32_t>(attachment_desc.size());
  render_pass_info.pAttachments = attachment_desc.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass_desc;

  if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateRenderPass Fail");
  }

  return {};
}

VkPipelineDepthStencilStateCreateInfo
GraphicsPipeline::GetPipelineDepthStencilInfo() {
  VkPipelineDepthStencilStateCreateInfo depthstencil_info = {};
  // TODO(jaebaek): Depth/stencil test setup should be come from the
  // PipelineData.
  depthstencil_info.depthTestEnable = VK_TRUE;
  depthstencil_info.depthWriteEnable = VK_TRUE;
  depthstencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
  depthstencil_info.depthBoundsTestEnable = VK_FALSE;
  depthstencil_info.stencilTestEnable = VK_FALSE;
  return depthstencil_info;
}

VkPipelineColorBlendAttachmentState
GraphicsPipeline::GetPipelineColorBlendAttachmentState() {
  VkPipelineColorBlendAttachmentState colorblend_attachment = {};
  // TODO(jaebaek): Update blend state should be come from the PipelineData.
  colorblend_attachment.blendEnable = VK_FALSE;
  colorblend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorblend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorblend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorblend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorblend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorblend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorblend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  return colorblend_attachment;
}

Result GraphicsPipeline::CreateVkGraphicsPipeline(
    VkPrimitiveTopology topology) {
  if (pipeline_ != VK_NULL_HANDLE)
    return Result("Vulkan::Pipeline already created");

  Result r = CreatePipelineLayout();
  if (!r.IsSuccess())
    return r;

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;

  VkVertexInputBindingDescription vertex_binding_desc = {};
  if (vertex_buffer_) {
    vertex_binding_desc = vertex_buffer_->GetVertexInputBinding();
    const auto& vertex_attr_desc = vertex_buffer_->GetVertexInputAttr();

    vertex_input_info.pVertexBindingDescriptions = &vertex_binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertex_attr_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = vertex_attr_desc.data();
  } else {
    vertex_binding_desc.binding = 0;
    vertex_binding_desc.stride = 0;
    vertex_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertex_input_info.pVertexBindingDescriptions = &vertex_binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;
  }

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
  input_assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // TODO(jaebaek): Handle the given index if exists.
  input_assembly_info.topology = topology;
  input_assembly_info.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {0,
                         0,
                         static_cast<float>(frame_->GetWidth()),
                         static_cast<float>(frame_->GetHeight()),
                         0,
                         1};

  VkRect2D scissor = {{0, 0}, {frame_->GetWidth(), frame_->GetHeight()}};

  VkPipelineViewportStateCreateInfo viewport_info = {};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<uint32_t>(shader_stage_info_.size());
  pipeline_info.pStages = shader_stage_info_.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly_info;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &kDefaultRasterizationInfo;
  pipeline_info.pMultisampleState = &kMultisampleInfo;

  VkPipelineDepthStencilStateCreateInfo depthstencil_info;
  if (depth_stencil_format_ != VK_FORMAT_UNDEFINED) {
    depthstencil_info = GetPipelineDepthStencilInfo();
    pipeline_info.pDepthStencilState = &depthstencil_info;
  }

  VkPipelineColorBlendStateCreateInfo colorblend_info = {};
  VkPipelineColorBlendAttachmentState colorblend_attachment;
  if (color_format_ != VK_FORMAT_UNDEFINED) {
    colorblend_attachment = GetPipelineColorBlendAttachmentState();

    colorblend_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorblend_info.logicOpEnable = VK_FALSE;
    colorblend_info.logicOp = VK_LOGIC_OP_COPY;
    colorblend_info.attachmentCount = 1;
    colorblend_info.pAttachments = &colorblend_attachment;
    pipeline_info.pColorBlendState = &colorblend_info;
  }

  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;

  if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateGraphicsPipelines Fail");
  }

  return {};
}

Result GraphicsPipeline::Initialize(uint32_t width,
                                    uint32_t height,
                                    VkCommandPool pool,
                                    VkQueue queue) {
  Result r = Pipeline::InitializeCommandBuffer(pool, queue);
  if (!r.IsSuccess())
    return r;

  r = CreateRenderPass();
  if (!r.IsSuccess())
    return r;

  frame_ = MakeUnique<FrameBuffer>(device_, width, height);
  r = frame_->Initialize(render_pass_, color_format_, depth_stencil_format_,
                         memory_properties_);
  if (!r.IsSuccess())
    return r;

  frame_width_ = width;
  frame_height_ = height;

  return {};
}

void GraphicsPipeline::SetBuffer(BufferType type,
                                 uint8_t location,
                                 const Format& format,
                                 const std::vector<Value>& values) {
  // TODO(jaebaek): Handle indices data.
  if (type != BufferType::kVertex)
    return;

  if (!vertex_buffer_)
    vertex_buffer_ = MakeUnique<VertexBuffer>(device_);

  vertex_buffer_->SetData(location, format, values);
}

Result GraphicsPipeline::SendBufferDataIfNeeded() {
  if (!vertex_buffer_)
    return {};

  if (vertex_buffer_->VertexDataSent())
    return {};

  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  DeactivateRenderPassIfNeeded();

  // TODO(jaebaek): Send indices data too.
  return vertex_buffer_->SendVertexData(command_->GetCommandBuffer(),
                                        memory_properties_);
}

void GraphicsPipeline::ActivateRenderPassIfNeeded() {
  if (render_pass_state_ == RenderPassState::kActive)
    return;

  VkRenderPassBeginInfo render_begin_info = {};
  render_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_begin_info.renderPass = render_pass_;
  render_begin_info.framebuffer = frame_->GetFrameBuffer();
  render_begin_info.renderArea = {{0, 0}, {frame_width_, frame_height_}};
  vkCmdBeginRenderPass(command_->GetCommandBuffer(), &render_begin_info,
                       VK_SUBPASS_CONTENTS_INLINE);
  render_pass_state_ = RenderPassState::kActive;
}

void GraphicsPipeline::DeactivateRenderPassIfNeeded() {
  if (render_pass_state_ == RenderPassState::kInactive)
    return;

  vkCmdEndRenderPass(command_->GetCommandBuffer());
  render_pass_state_ = RenderPassState::kInactive;
}

Result GraphicsPipeline::SetClearColor(float r, float g, float b, float a) {
  if (color_format_ == VK_FORMAT_UNDEFINED) {
    return Result(
        "Vulkan::ClearColorCommand No Color Buffer for FrameBuffer Exists");
  }

  clear_color_r_ = r;
  clear_color_g_ = g;
  clear_color_b_ = b;
  clear_color_a_ = a;
  return {};
}

Result GraphicsPipeline::SetClearStencil(uint32_t stencil) {
  if (depth_stencil_format_ == VK_FORMAT_UNDEFINED) {
    return Result(
        "Vulkan::ClearStencilCommand No DepthStencil Buffer for FrameBuffer "
        "Exists");
  }

  clear_stencil_ = stencil;
  return {};
}

Result GraphicsPipeline::SetClearDepth(float depth) {
  if (depth_stencil_format_ == VK_FORMAT_UNDEFINED) {
    return Result(
        "Vulkan::ClearStencilCommand No DepthStencil Buffer for FrameBuffer "
        "Exists");
  }

  clear_depth_ = depth;
  return {};
}

Result GraphicsPipeline::Clear() {
  if (color_format_ == VK_FORMAT_UNDEFINED &&
      depth_stencil_format_ == VK_FORMAT_UNDEFINED) {
    return Result(
        "Vulkan::ClearColorCommand No Color nor DepthStencil Buffer for "
        "FrameBuffer Exists");
  }

  if (color_format_ != VK_FORMAT_UNDEFINED) {
    VkClearValue clear_value;
    clear_value.color = {
        {clear_color_r_, clear_color_g_, clear_color_b_, clear_color_a_}};
    Result r = ClearBuffer(clear_value, VK_IMAGE_ASPECT_COLOR_BIT);
    if (!r.IsSuccess())
      return r;
  }

  if (depth_stencil_format_ == VK_FORMAT_UNDEFINED)
    return {};

  VkClearValue clear_value;
  clear_value.depthStencil = {clear_depth_, clear_stencil_};
  return ClearBuffer(clear_value,
                     VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
}

Result GraphicsPipeline::ClearBuffer(const VkClearValue& clear_value,
                                     VkImageAspectFlags aspect) {
  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  frame_->ChangeFrameImageLayout(command_->GetCommandBuffer(),
                                 FrameImageState::kClearOrDraw);

  // TODO(jaebaek): When multiple clear and draw commands exist, handle
  //                begin/end render pass properly.
  ActivateRenderPassIfNeeded();

  VkClearAttachment clear_attachment = {};
  clear_attachment.aspectMask = aspect;
  clear_attachment.colorAttachment = 0;
  clear_attachment.clearValue = clear_value;

  VkClearRect clear_rect;
  clear_rect.rect = {{0, 0}, {frame_width_, frame_height_}};
  clear_rect.baseArrayLayer = 0;
  clear_rect.layerCount = 1;

  vkCmdClearAttachments(command_->GetCommandBuffer(), 1, &clear_attachment, 1,
                        &clear_rect);

  return {};
}

Result GraphicsPipeline::Draw(const DrawArraysCommand* command) {
  // TODO(jaebaek): Handle primitive topology.
  if (pipeline_ == VK_NULL_HANDLE) {
    Result r = CreateVkGraphicsPipeline(ToVkTopology(command->GetTopology()));
    if (!r.IsSuccess())
      return r;
  }

  Result r = SendBufferDataIfNeeded();
  if (!r.IsSuccess())
    return r;

  r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  frame_->ChangeFrameImageLayout(command_->GetCommandBuffer(),
                                 FrameImageState::kClearOrDraw);

  ActivateRenderPassIfNeeded();

  vkCmdBindPipeline(command_->GetCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

  if (vertex_buffer_)
    vertex_buffer_->BindToCommandBuffer(command_->GetCommandBuffer());

  uint32_t instance_count = command->GetInstanceCount();
  if (instance_count == 0 && command->GetVertexCount() != 0)
    instance_count = 1;

  vkCmdDraw(command_->GetCommandBuffer(), command->GetVertexCount(),
            instance_count, command->GetFirstVertexIndex(), 0);

  return {};
}

Result GraphicsPipeline::SubmitProbeCommand() {
  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  ActivateRenderPassIfNeeded();
  DeactivateRenderPassIfNeeded();

  r = frame_->CopyColorImageToHost(command_->GetCommandBuffer());
  if (!r.IsSuccess())
    return r;

  r = command_->End();
  if (!r.IsSuccess())
    return r;

  return command_->SubmitAndReset(fence_timeout_ms_);
}

Result GraphicsPipeline::VerifyPixels(const uint32_t x,
                                      const uint32_t y,
                                      const uint32_t width,
                                      const uint32_t height,
                                      const ProbeCommand* command) {
  const uint32_t stride = VkFormatToByteSize(color_format_);

  // TODO(jaebaek): Support all VkFormat
  const uint8_t* ptr = static_cast<const uint8_t*>(frame_->GetColorBufferPtr());
  uint32_t count_of_invalid_pixels = 0;
  uint32_t first_invalid_i = 0;
  uint32_t first_invalid_j = 0;
  for (uint32_t j = 0; j < height; ++j) {
    const uint8_t* p = ptr + stride * frame_->GetWidth() * (j + y) + stride * x;
    for (uint32_t i = 0; i < width; ++i) {
      // TODO(jaebaek): Get actual pixel values based on frame buffer formats.
      if (!IsFloatPixelEqualInt(command->GetR(), p[stride * i]) ||
          !IsFloatPixelEqualInt(command->GetG(), p[stride * i + 1]) ||
          !IsFloatPixelEqualInt(command->GetB(), p[stride * i + 2]) ||
          (command->IsRGBA() &&
           !IsFloatPixelEqualInt(command->GetA(), p[stride * i + 3]))) {
        if (!count_of_invalid_pixels) {
          first_invalid_i = i;
          first_invalid_j = j;
        }
        ++count_of_invalid_pixels;
      }
    }
  }

  if (count_of_invalid_pixels) {
    const uint8_t* p =
        ptr + stride * frame_->GetWidth() * (first_invalid_j + y) + stride * x;
    return Result(
        "Probe failed at: " + std::to_string(first_invalid_i + x) + ", " +
        std::to_string(first_invalid_j + y) + "\n" +
        "  Expected RGBA: " + std::to_string(command->GetR() * 255) + ", " +
        std::to_string(command->GetG() * 255) + ", " +
        std::to_string(command->GetB() * 255) +
        (command->IsRGBA() ? ", " + std::to_string(command->GetA() * 255) +
                                 "\n  Actual RGBA: "
                           : "\n  Actual RGB: ") +
        std::to_string(static_cast<int>(p[stride * first_invalid_i])) + ", " +
        std::to_string(static_cast<int>(p[stride * first_invalid_i + 1])) +
        ", " +
        std::to_string(static_cast<int>(p[stride * first_invalid_i + 2])) +
        (command->IsRGBA() ? ", " + std::to_string(static_cast<int>(
                                        p[stride * first_invalid_i + 3]))
                           : "") +
        "\n" + "Probe failed in " + std::to_string(count_of_invalid_pixels) +
        " pixels");
  }

  return {};
}

Result GraphicsPipeline::Probe(const ProbeCommand* command) {
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  const uint32_t frame_width = frame_->GetWidth();
  const uint32_t frame_height = frame_->GetHeight();

  if (command->IsWholeWindow()) {
    width = frame_width;
    height = frame_height;
  } else if (command->IsRelative()) {
    x = static_cast<uint32_t>(frame_width * command->GetX());
    y = static_cast<uint32_t>(frame_height * command->GetY());
    width = static_cast<uint32_t>(frame_width * command->GetWidth());
    height = static_cast<uint32_t>(frame_height * command->GetHeight());
  } else {
    x = static_cast<uint32_t>(command->GetX());
    y = static_cast<uint32_t>(command->GetY());
    width = static_cast<uint32_t>(command->GetWidth());
    height = static_cast<uint32_t>(command->GetHeight());
  }

  if (x + width > frame_width || y + height > frame_height) {
    return Result(
        "Vulkan::Probe Position(" + std::to_string(x + width - 1) + ", " +
        std::to_string(y + height - 1) + ") is out of framebuffer scope (" +
        std::to_string(frame_width) + "," + std::to_string(frame_height) + ")");
  }

  Result r = SubmitProbeCommand();
  if (!r.IsSuccess())
    return r;

  return VerifyPixels(x, y, width, height, command);
}

void GraphicsPipeline::Shutdown() {
  DeactivateRenderPassIfNeeded();

  Result r = command_->End();
  if (r.IsSuccess())
    command_->SubmitAndReset(fence_timeout_ms_);

  Pipeline::Shutdown();
  frame_->Shutdown();
  vkDestroyRenderPass(device_, render_pass_, nullptr);
}

}  // namespace vulkan
}  // namespace amber
