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
#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {
namespace {

const VkAttachmentDescription kDefaultAttachmentDesc = {
    0,                                    /* flags */
    VK_FORMAT_UNDEFINED,                  /* format */
    VK_SAMPLE_COUNT_1_BIT,                /* samples */
    VK_ATTACHMENT_LOAD_OP_LOAD,           /* loadOp */
    VK_ATTACHMENT_STORE_OP_STORE,         /* storeOp */
    VK_ATTACHMENT_LOAD_OP_LOAD,           /* stencilLoadOp */
    VK_ATTACHMENT_STORE_OP_STORE,         /* stencilStoreOp */
    VK_IMAGE_LAYOUT_UNDEFINED,            /* initialLayout */
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, /* finalLayout */
};

const VkSampleMask kSampleMask = ~0U;

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

VkStencilOp ToVkStencilOp(StencilOp op) {
  switch (op) {
    case StencilOp::kKeep:
      return VK_STENCIL_OP_KEEP;
    case StencilOp::kZero:
      return VK_STENCIL_OP_ZERO;
    case StencilOp::kReplace:
      return VK_STENCIL_OP_REPLACE;
    case StencilOp::kIncrementAndClamp:
      return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOp::kDecrementAndClamp:
      return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOp::kInvert:
      return VK_STENCIL_OP_INVERT;
    case StencilOp::kIncrementAndWrap:
      return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOp::kDecrementAndWrap:
      return VK_STENCIL_OP_DECREMENT_AND_WRAP;
  }
  assert(false && "Vulkan::Unknown StencilOp");
  return VK_STENCIL_OP_KEEP;
}

VkCompareOp ToVkCompareOp(CompareOp op) {
  switch (op) {
    case CompareOp::kNever:
      return VK_COMPARE_OP_NEVER;
    case CompareOp::kLess:
      return VK_COMPARE_OP_LESS;
    case CompareOp::kEqual:
      return VK_COMPARE_OP_EQUAL;
    case CompareOp::kLessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::kGreater:
      return VK_COMPARE_OP_GREATER;
    case CompareOp::kNotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::kGreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::kAlways:
      return VK_COMPARE_OP_ALWAYS;
  }
  assert(false && "Vulkan::Unknown CompareOp");
  return VK_COMPARE_OP_NEVER;
}

VkPolygonMode ToVkPolygonMode(PolygonMode mode) {
  switch (mode) {
    case PolygonMode::kFill:
      return VK_POLYGON_MODE_FILL;
    case PolygonMode::kLine:
      return VK_POLYGON_MODE_LINE;
    case PolygonMode::kPoint:
      return VK_POLYGON_MODE_POINT;
  }
  assert(false && "Vulkan::Unknown PolygonMode");
  return VK_POLYGON_MODE_FILL;
}

VkCullModeFlags ToVkCullMode(CullMode mode) {
  switch (mode) {
    case CullMode::kNone:
      return VK_CULL_MODE_NONE;
    case CullMode::kFront:
      return VK_CULL_MODE_FRONT_BIT;
    case CullMode::kBack:
      return VK_CULL_MODE_BACK_BIT;
    case CullMode::kFrontAndBack:
      return VK_CULL_MODE_FRONT_AND_BACK;
  }
  assert(false && "Vulkan::Unknown CullMode");
  return VK_CULL_MODE_NONE;
}

VkFrontFace ToVkFrontFace(FrontFace front_face) {
  return front_face == FrontFace::kClockwise ? VK_FRONT_FACE_CLOCKWISE
                                             : VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

VkLogicOp ToVkLogicOp(LogicOp op) {
  switch (op) {
    case LogicOp::kClear:
      return VK_LOGIC_OP_CLEAR;
    case LogicOp::kAnd:
      return VK_LOGIC_OP_AND;
    case LogicOp::kAndReverse:
      return VK_LOGIC_OP_AND_REVERSE;
    case LogicOp::kCopy:
      return VK_LOGIC_OP_COPY;
    case LogicOp::kAndInverted:
      return VK_LOGIC_OP_AND_INVERTED;
    case LogicOp::kNoOp:
      return VK_LOGIC_OP_NO_OP;
    case LogicOp::kXor:
      return VK_LOGIC_OP_XOR;
    case LogicOp::kOr:
      return VK_LOGIC_OP_OR;
    case LogicOp::kNor:
      return VK_LOGIC_OP_NOR;
    case LogicOp::kEquivalent:
      return VK_LOGIC_OP_EQUIVALENT;
    case LogicOp::kInvert:
      return VK_LOGIC_OP_INVERT;
    case LogicOp::kOrReverse:
      return VK_LOGIC_OP_OR_REVERSE;
    case LogicOp::kCopyInverted:
      return VK_LOGIC_OP_COPY_INVERTED;
    case LogicOp::kOrInverted:
      return VK_LOGIC_OP_OR_INVERTED;
    case LogicOp::kNand:
      return VK_LOGIC_OP_NAND;
    case LogicOp::kSet:
      return VK_LOGIC_OP_SET;
  }
  assert(false && "Vulkan::Unknown LogicOp");
  return VK_LOGIC_OP_CLEAR;
}

VkBlendFactor ToVkBlendFactor(BlendFactor factor) {
  switch (factor) {
    case BlendFactor::kZero:
      return VK_BLEND_FACTOR_ZERO;
    case BlendFactor::kOne:
      return VK_BLEND_FACTOR_ONE;
    case BlendFactor::kSrcColor:
      return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFactor::kOneMinusSrcColor:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::kDstColor:
      return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFactor::kOneMinusDstColor:
      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::kSrcAlpha:
      return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFactor::kOneMinusSrcAlpha:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::kDstAlpha:
      return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFactor::kOneMinusDstAlpha:
      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case BlendFactor::kConstantColor:
      return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case BlendFactor::kOneMinusConstantColor:
      return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case BlendFactor::kConstantAlpha:
      return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case BlendFactor::kOneMinusConstantAlpha:
      return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case BlendFactor::kSrcAlphaSaturate:
      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case BlendFactor::kSrc1Color:
      return VK_BLEND_FACTOR_SRC1_COLOR;
    case BlendFactor::kOneMinusSrc1Color:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case BlendFactor::kSrc1Alpha:
      return VK_BLEND_FACTOR_SRC1_ALPHA;
    case BlendFactor::kOneMinusSrc1Alpha:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
  }
  assert(false && "Vulkan::Unknown BlendFactor");
  return VK_BLEND_FACTOR_ZERO;
}

VkBlendOp ToVkBlendOp(BlendOp op) {
  switch (op) {
    case BlendOp::kAdd:
      return VK_BLEND_OP_ADD;
    case BlendOp::kSubtract:
      return VK_BLEND_OP_SUBTRACT;
    case BlendOp::kReverseSubtract:
      return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOp::kMin:
      return VK_BLEND_OP_MIN;
    case BlendOp::kMax:
      return VK_BLEND_OP_MAX;
    case BlendOp::kZero:
      return VK_BLEND_OP_ZERO_EXT;
    case BlendOp::kSrc:
      return VK_BLEND_OP_SRC_EXT;
    case BlendOp::kDst:
      return VK_BLEND_OP_DST_EXT;
    case BlendOp::kSrcOver:
      return VK_BLEND_OP_SRC_OVER_EXT;
    case BlendOp::kDstOver:
      return VK_BLEND_OP_DST_OVER_EXT;
    case BlendOp::kSrcIn:
      return VK_BLEND_OP_SRC_IN_EXT;
    case BlendOp::kDstIn:
      return VK_BLEND_OP_DST_IN_EXT;
    case BlendOp::kSrcOut:
      return VK_BLEND_OP_SRC_OUT_EXT;
    case BlendOp::kDstOut:
      return VK_BLEND_OP_DST_OUT_EXT;
    case BlendOp::kSrcAtop:
      return VK_BLEND_OP_SRC_ATOP_EXT;
    case BlendOp::kDstAtop:
      return VK_BLEND_OP_DST_ATOP_EXT;
    case BlendOp::kXor:
      return VK_BLEND_OP_XOR_EXT;
    case BlendOp::kMultiply:
      return VK_BLEND_OP_MULTIPLY_EXT;
    case BlendOp::kScreen:
      return VK_BLEND_OP_SCREEN_EXT;
    case BlendOp::kOverlay:
      return VK_BLEND_OP_OVERLAY_EXT;
    case BlendOp::kDarken:
      return VK_BLEND_OP_DARKEN_EXT;
    case BlendOp::kLighten:
      return VK_BLEND_OP_LIGHTEN_EXT;
    case BlendOp::kColorDodge:
      return VK_BLEND_OP_COLORDODGE_EXT;
    case BlendOp::kColorBurn:
      return VK_BLEND_OP_COLORBURN_EXT;
    case BlendOp::kHardLight:
      return VK_BLEND_OP_HARDLIGHT_EXT;
    case BlendOp::kSoftLight:
      return VK_BLEND_OP_SOFTLIGHT_EXT;
    case BlendOp::kDifference:
      return VK_BLEND_OP_DIFFERENCE_EXT;
    case BlendOp::kExclusion:
      return VK_BLEND_OP_EXCLUSION_EXT;
    case BlendOp::kInvert:
      return VK_BLEND_OP_INVERT_EXT;
    case BlendOp::kInvertRGB:
      return VK_BLEND_OP_INVERT_RGB_EXT;
    case BlendOp::kLinearDodge:
      return VK_BLEND_OP_LINEARDODGE_EXT;
    case BlendOp::kLinearBurn:
      return VK_BLEND_OP_LINEARBURN_EXT;
    case BlendOp::kVividLight:
      return VK_BLEND_OP_VIVIDLIGHT_EXT;
    case BlendOp::kLinearLight:
      return VK_BLEND_OP_LINEARLIGHT_EXT;
    case BlendOp::kPinLight:
      return VK_BLEND_OP_PINLIGHT_EXT;
    case BlendOp::kHardMix:
      return VK_BLEND_OP_HARDMIX_EXT;
    case BlendOp::kHslHue:
      return VK_BLEND_OP_HSL_HUE_EXT;
    case BlendOp::kHslSaturation:
      return VK_BLEND_OP_HSL_SATURATION_EXT;
    case BlendOp::kHslColor:
      return VK_BLEND_OP_HSL_COLOR_EXT;
    case BlendOp::kHslLuminosity:
      return VK_BLEND_OP_HSL_LUMINOSITY_EXT;
    case BlendOp::kPlus:
      return VK_BLEND_OP_PLUS_EXT;
    case BlendOp::kPlusClamped:
      return VK_BLEND_OP_PLUS_CLAMPED_EXT;
    case BlendOp::kPlusClampedAlpha:
      return VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT;
    case BlendOp::kPlusDarker:
      return VK_BLEND_OP_PLUS_DARKER_EXT;
    case BlendOp::kMinus:
      return VK_BLEND_OP_MINUS_EXT;
    case BlendOp::kMinusClamped:
      return VK_BLEND_OP_MINUS_CLAMPED_EXT;
    case BlendOp::kContrast:
      return VK_BLEND_OP_CONTRAST_EXT;
    case BlendOp::kInvertOvg:
      return VK_BLEND_OP_INVERT_OVG_EXT;
    case BlendOp::kRed:
      return VK_BLEND_OP_RED_EXT;
    case BlendOp::kGreen:
      return VK_BLEND_OP_GREEN_EXT;
    case BlendOp::kBlue:
      return VK_BLEND_OP_BLUE_EXT;
  }
  assert(false && "Vulkan::Unknown BlendOp");
  return VK_BLEND_OP_ADD;
}

class RenderPassGuard {
 public:
  explicit RenderPassGuard(GraphicsPipeline* pipeline) : pipeline_(pipeline) {
    auto* frame = pipeline_->GetFrameBuffer();
    auto* cmd = pipeline_->GetCommandBuffer();
    frame->ChangeFrameToDrawLayout(cmd);

    VkRenderPassBeginInfo render_begin_info = VkRenderPassBeginInfo();
    render_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_begin_info.renderPass = pipeline_->GetVkRenderPass();
    render_begin_info.framebuffer = frame->GetVkFrameBuffer();
    render_begin_info.renderArea = {{0, 0},
                                    {frame->GetWidth(), frame->GetHeight()}};
    pipeline_->GetDevice()->GetPtrs()->vkCmdBeginRenderPass(
        cmd->GetVkCommandBuffer(), &render_begin_info,
        VK_SUBPASS_CONTENTS_INLINE);
  }

  ~RenderPassGuard() {
    auto* cmd = pipeline_->GetCommandBuffer();

    pipeline_->GetDevice()->GetPtrs()->vkCmdEndRenderPass(
        cmd->GetVkCommandBuffer());

    auto* frame = pipeline_->GetFrameBuffer();
    frame->ChangeFrameToProbeLayout(cmd);
  }

 private:
  GraphicsPipeline* pipeline_;
};

}  // namespace

GraphicsPipeline::GraphicsPipeline(
    Device* device,
    const std::vector<amber::Pipeline::BufferInfo>& color_buffers,
    Format* depth_stencil_format,
    uint32_t fence_timeout_ms,
    const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info)
    : Pipeline(PipelineType::kGraphics,
               device,
               fence_timeout_ms,
               shader_stage_info),
      depth_stencil_format_(depth_stencil_format) {
  for (const auto& info : color_buffers)
    color_buffers_.push_back(&info);
}

GraphicsPipeline::~GraphicsPipeline() {
  if (render_pass_) {
    device_->GetPtrs()->vkDestroyRenderPass(device_->GetVkDevice(),
                                            render_pass_, nullptr);
  }
}

Result GraphicsPipeline::CreateRenderPass() {
  VkSubpassDescription subpass_desc = VkSubpassDescription();
  subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  std::vector<VkAttachmentDescription> attachment_desc;

  std::vector<VkAttachmentReference> color_refer;
  VkAttachmentReference depth_refer = VkAttachmentReference();

  for (const auto* info : color_buffers_) {
    attachment_desc.push_back(kDefaultAttachmentDesc);
    attachment_desc.back().format =
        device_->GetVkFormat(*info->buffer->GetFormat());
    attachment_desc.back().initialLayout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_desc.back().finalLayout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ref = VkAttachmentReference();
    ref.attachment = static_cast<uint32_t>(attachment_desc.size() - 1);
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_refer.push_back(ref);
  }
  subpass_desc.colorAttachmentCount = static_cast<uint32_t>(color_refer.size());
  subpass_desc.pColorAttachments = color_refer.data();

  if (depth_stencil_format_ && depth_stencil_format_->IsFormatKnown()) {
    attachment_desc.push_back(kDefaultAttachmentDesc);
    attachment_desc.back().format =
        device_->GetVkFormat(*depth_stencil_format_);
    attachment_desc.back().initialLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment_desc.back().finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depth_refer.attachment = static_cast<uint32_t>(attachment_desc.size() - 1);
    depth_refer.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass_desc.pDepthStencilAttachment = &depth_refer;
  }

  VkRenderPassCreateInfo render_pass_info = VkRenderPassCreateInfo();
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount =
      static_cast<uint32_t>(attachment_desc.size());
  render_pass_info.pAttachments = attachment_desc.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass_desc;

  if (device_->GetPtrs()->vkCreateRenderPass(device_->GetVkDevice(),
                                             &render_pass_info, nullptr,
                                             &render_pass_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateRenderPass Fail");
  }

  return {};
}

VkPipelineDepthStencilStateCreateInfo
GraphicsPipeline::GetVkPipelineDepthStencilInfo(
    const PipelineData* pipeline_data) {
  VkPipelineDepthStencilStateCreateInfo depthstencil_info =
      VkPipelineDepthStencilStateCreateInfo();
  depthstencil_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  depthstencil_info.depthTestEnable = pipeline_data->GetEnableDepthTest();
  depthstencil_info.depthWriteEnable = pipeline_data->GetEnableDepthWrite();
  depthstencil_info.depthCompareOp =
      ToVkCompareOp(pipeline_data->GetDepthCompareOp());
  depthstencil_info.depthBoundsTestEnable =
      pipeline_data->GetEnableDepthBoundsTest();
  depthstencil_info.stencilTestEnable = pipeline_data->GetEnableStencilTest();

  depthstencil_info.front.failOp =
      ToVkStencilOp(pipeline_data->GetFrontFailOp());
  depthstencil_info.front.passOp =
      ToVkStencilOp(pipeline_data->GetFrontPassOp());
  depthstencil_info.front.depthFailOp =
      ToVkStencilOp(pipeline_data->GetFrontDepthFailOp());
  depthstencil_info.front.compareOp =
      ToVkCompareOp(pipeline_data->GetFrontCompareOp());
  depthstencil_info.front.compareMask = pipeline_data->GetFrontCompareMask();
  depthstencil_info.front.writeMask = pipeline_data->GetFrontWriteMask();
  depthstencil_info.front.reference = pipeline_data->GetFrontReference();

  depthstencil_info.back.failOp = ToVkStencilOp(pipeline_data->GetBackFailOp());
  depthstencil_info.back.passOp = ToVkStencilOp(pipeline_data->GetBackPassOp());
  depthstencil_info.back.depthFailOp =
      ToVkStencilOp(pipeline_data->GetBackDepthFailOp());
  depthstencil_info.back.compareOp =
      ToVkCompareOp(pipeline_data->GetBackCompareOp());
  depthstencil_info.back.compareMask = pipeline_data->GetBackCompareMask();
  depthstencil_info.back.writeMask = pipeline_data->GetBackWriteMask();
  depthstencil_info.back.reference = pipeline_data->GetBackReference();

  depthstencil_info.minDepthBounds = pipeline_data->GetMinDepthBounds();
  depthstencil_info.maxDepthBounds = pipeline_data->GetMaxDepthBounds();

  return depthstencil_info;
}

std::vector<VkPipelineColorBlendAttachmentState>
GraphicsPipeline::GetVkPipelineColorBlendAttachmentState(
    const PipelineData* pipeline_data) {
  std::vector<VkPipelineColorBlendAttachmentState> states;

  for (size_t i = 0; i < color_buffers_.size(); ++i) {
    VkPipelineColorBlendAttachmentState colorblend_attachment =
        VkPipelineColorBlendAttachmentState();
    colorblend_attachment.blendEnable = pipeline_data->GetEnableBlend();
    colorblend_attachment.srcColorBlendFactor =
        ToVkBlendFactor(pipeline_data->GetSrcColorBlendFactor());
    colorblend_attachment.dstColorBlendFactor =
        ToVkBlendFactor(pipeline_data->GetDstColorBlendFactor());
    colorblend_attachment.colorBlendOp =
        ToVkBlendOp(pipeline_data->GetColorBlendOp());
    colorblend_attachment.srcAlphaBlendFactor =
        ToVkBlendFactor(pipeline_data->GetSrcAlphaBlendFactor());
    colorblend_attachment.dstAlphaBlendFactor =
        ToVkBlendFactor(pipeline_data->GetDstAlphaBlendFactor());
    colorblend_attachment.alphaBlendOp =
        ToVkBlendOp(pipeline_data->GetAlphaBlendOp());
    colorblend_attachment.colorWriteMask = pipeline_data->GetColorWriteMask();
    states.push_back(colorblend_attachment);
  }
  return states;
}

Result GraphicsPipeline::CreateVkGraphicsPipeline(
    const PipelineData* pipeline_data,
    VkPrimitiveTopology topology,
    const VertexBuffer* vertex_buffer,
    const VkPipelineLayout& pipeline_layout,
    VkPipeline* pipeline) {
  if (!pipeline_data) {
    return Result(
        "Vulkan: GraphicsPipeline::CreateVkGraphicsPipeline PipelineData is "
        "null");
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_info =
      VkPipelineVertexInputStateCreateInfo();
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;

  VkVertexInputBindingDescription vertex_binding_desc =
      VkVertexInputBindingDescription();
  if (vertex_buffer != nullptr) {
    vertex_binding_desc = vertex_buffer->GetVkVertexInputBinding();
    const auto& vertex_attr_desc = vertex_buffer->GetVkVertexInputAttr();

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

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info =
      VkPipelineInputAssemblyStateCreateInfo();
  input_assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // TODO(jaebaek): Handle the given index if exists.
  input_assembly_info.topology = topology;
  input_assembly_info.primitiveRestartEnable =
      pipeline_data->GetEnablePrimitiveRestart();

  VkViewport viewport = {
      0, 0, static_cast<float>(frame_width_), static_cast<float>(frame_height_),
      0, 1};

  VkRect2D scissor = {{0, 0}, {frame_width_, frame_height_}};

  VkPipelineViewportStateCreateInfo viewport_info =
      VkPipelineViewportStateCreateInfo();
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  auto shader_stage_info = GetVkShaderStageInfo();
  bool is_tessellation_needed = false;
  for (auto& info : shader_stage_info) {
    info.pName = GetEntryPointName(info.stage);
    if (info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
        info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
      is_tessellation_needed = true;
    }
  }

  VkPipelineMultisampleStateCreateInfo multisampleInfo = {
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

  VkGraphicsPipelineCreateInfo pipeline_info = VkGraphicsPipelineCreateInfo();
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<uint32_t>(shader_stage_info.size());
  pipeline_info.pStages = shader_stage_info.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly_info;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pMultisampleState = &multisampleInfo;

  VkPipelineRasterizationStateCreateInfo rasterization_info =
      VkPipelineRasterizationStateCreateInfo();
  rasterization_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_info.depthClampEnable = pipeline_data->GetEnableDepthClamp();
  rasterization_info.rasterizerDiscardEnable =
      pipeline_data->GetEnableRasterizerDiscard();
  rasterization_info.polygonMode =
      ToVkPolygonMode(pipeline_data->GetPolygonMode());
  rasterization_info.cullMode = ToVkCullMode(pipeline_data->GetCullMode());
  rasterization_info.frontFace = ToVkFrontFace(pipeline_data->GetFrontFace());
  rasterization_info.depthBiasEnable = pipeline_data->GetEnableDepthBias();
  rasterization_info.depthBiasConstantFactor =
      pipeline_data->GetDepthBiasConstantFactor();
  rasterization_info.depthBiasClamp = pipeline_data->GetDepthBiasClamp();
  rasterization_info.depthBiasSlopeFactor =
      pipeline_data->GetDepthBiasSlopeFactor();
  rasterization_info.lineWidth = pipeline_data->GetLineWidth();
  pipeline_info.pRasterizationState = &rasterization_info;

  VkPipelineTessellationStateCreateInfo tess_info =
      VkPipelineTessellationStateCreateInfo();
  if (is_tessellation_needed) {
    tess_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tess_info.patchControlPoints = patch_control_points_;
    pipeline_info.pTessellationState = &tess_info;
  }

  VkPipelineDepthStencilStateCreateInfo depthstencil_info;
  if (depth_stencil_format_ && depth_stencil_format_->IsFormatKnown()) {
    depthstencil_info = GetVkPipelineDepthStencilInfo(pipeline_data);
    pipeline_info.pDepthStencilState = &depthstencil_info;
  }

  VkPipelineColorBlendStateCreateInfo colorblend_info =
      VkPipelineColorBlendStateCreateInfo();

  auto colorblend_attachment =
      GetVkPipelineColorBlendAttachmentState(pipeline_data);

  colorblend_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorblend_info.logicOpEnable = pipeline_data->GetEnableLogicOp();
  colorblend_info.logicOp = ToVkLogicOp(pipeline_data->GetLogicOp());
  colorblend_info.attachmentCount =
      static_cast<uint32_t>(colorblend_attachment.size());
  colorblend_info.pAttachments = colorblend_attachment.data();
  pipeline_info.pColorBlendState = &colorblend_info;

  pipeline_info.layout = pipeline_layout;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;

  if (device_->GetPtrs()->vkCreateGraphicsPipelines(
          device_->GetVkDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
          pipeline) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateGraphicsPipelines Fail");
  }

  return {};
}

Result GraphicsPipeline::Initialize(uint32_t width,
                                    uint32_t height,
                                    CommandPool* pool) {
  Result r = Pipeline::Initialize(pool);
  if (!r.IsSuccess())
    return r;

  r = CreateRenderPass();
  if (!r.IsSuccess())
    return r;

  frame_ = MakeUnique<FrameBuffer>(device_, color_buffers_, width, height);
  r = frame_->Initialize(render_pass_, depth_stencil_format_);
  if (!r.IsSuccess())
    return r;

  frame_width_ = width;
  frame_height_ = height;

  return {};
}

Result GraphicsPipeline::SendVertexBufferDataIfNeeded(
    VertexBuffer* vertex_buffer) {
  if (!vertex_buffer || vertex_buffer->VertexDataSent())
    return {};
  return vertex_buffer->SendVertexData(command_.get());
}

Result GraphicsPipeline::SetIndexBuffer(Buffer* buffer) {
  if (index_buffer_) {
    return Result(
        "GraphicsPipeline::SetIndexBuffer must be called once when "
        "index_buffer_ is created");
  }

  index_buffer_ = MakeUnique<IndexBuffer>(device_);

  CommandBufferGuard guard(GetCommandBuffer());
  if (!guard.IsRecording())
    return guard.GetResult();

  Result r = index_buffer_->SendIndexData(command_.get(), buffer);
  if (!r.IsSuccess())
    return r;

  return guard.Submit(GetFenceTimeout());
}

Result GraphicsPipeline::SetClearColor(float r, float g, float b, float a) {
  clear_color_r_ = r;
  clear_color_g_ = g;
  clear_color_b_ = b;
  clear_color_a_ = a;
  return {};
}

Result GraphicsPipeline::SetClearStencil(uint32_t stencil) {
  if (!depth_stencil_format_ || !depth_stencil_format_->IsFormatKnown()) {
    return Result(
        "Vulkan::ClearStencilCommand No DepthStencil Buffer for FrameBuffer "
        "Exists");
  }

  clear_stencil_ = stencil;
  return {};
}

Result GraphicsPipeline::SetClearDepth(float depth) {
  if (!depth_stencil_format_ || !depth_stencil_format_->IsFormatKnown()) {
    return Result(
        "Vulkan::ClearStencilCommand No DepthStencil Buffer for FrameBuffer "
        "Exists");
  }

  clear_depth_ = depth;
  return {};
}

Result GraphicsPipeline::Clear() {
  VkClearValue colour_clear;
  colour_clear.color = {
      {clear_color_r_, clear_color_g_, clear_color_b_, clear_color_a_}};

  Result r = ClearBuffer(colour_clear, VK_IMAGE_ASPECT_COLOR_BIT);
  if (!r.IsSuccess())
    return r;

  if (!depth_stencil_format_ || !depth_stencil_format_->IsFormatKnown())
    return {};

  VkClearValue depth_clear;
  depth_clear.depthStencil = {clear_depth_, clear_stencil_};

  return ClearBuffer(
      depth_clear,
      depth_stencil_format_ && depth_stencil_format_->HasStencilComponent()
          ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
          : VK_IMAGE_ASPECT_DEPTH_BIT);
}

Result GraphicsPipeline::ClearBuffer(const VkClearValue& clear_value,
                                     VkImageAspectFlags aspect) {
  CommandBufferGuard cmd_buf_guard(GetCommandBuffer());
  if (!cmd_buf_guard.IsRecording())
    return cmd_buf_guard.GetResult();

  frame_->ChangeFrameToWriteLayout(GetCommandBuffer());
  frame_->CopyBuffersToImages();
  frame_->TransferColorImagesToDevice(GetCommandBuffer());

  {
    RenderPassGuard render_pass_guard(this);

    std::vector<VkClearAttachment> clears;
    for (size_t i = 0; i < color_buffers_.size(); ++i) {
      VkClearAttachment clear_attachment = VkClearAttachment();
      clear_attachment.aspectMask = aspect;
      clear_attachment.colorAttachment = static_cast<uint32_t>(i);
      clear_attachment.clearValue = clear_value;

      clears.push_back(clear_attachment);
    }

    VkClearRect clear_rect;
    clear_rect.rect = {{0, 0}, {frame_width_, frame_height_}};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    device_->GetPtrs()->vkCmdClearAttachments(
        command_->GetVkCommandBuffer(), static_cast<uint32_t>(clears.size()),
        clears.data(), 1, &clear_rect);
  }

  frame_->TransferColorImagesToHost(command_.get());

  Result r = cmd_buf_guard.Submit(GetFenceTimeout());
  if (!r.IsSuccess())
    return r;

  frame_->CopyImagesToBuffers();
  return {};
}

Result GraphicsPipeline::Draw(const DrawArraysCommand* command,
                              VertexBuffer* vertex_buffer) {
  Result r = SendDescriptorDataToDeviceIfNeeded();
  if (!r.IsSuccess())
    return r;

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  r = CreateVkPipelineLayout(&pipeline_layout);
  if (!r.IsSuccess())
    return r;

  VkPipeline pipeline = VK_NULL_HANDLE;
  r = CreateVkGraphicsPipeline(command->GetPipelineData(),
                               ToVkTopology(command->GetTopology()),
                               vertex_buffer, pipeline_layout, &pipeline);
  if (!r.IsSuccess())
    return r;

  // Note that a command updating a descriptor set and a command using
  // it must be submitted separately, because using a descriptor set
  // while updating it is not safe.
  UpdateDescriptorSetsIfNeeded();

  {
    CommandBufferGuard cmd_buf_guard(GetCommandBuffer());
    if (!cmd_buf_guard.IsRecording())
      return cmd_buf_guard.GetResult();

    r = SendVertexBufferDataIfNeeded(vertex_buffer);
    if (!r.IsSuccess())
      return r;

    frame_->ChangeFrameToWriteLayout(GetCommandBuffer());
    frame_->CopyBuffersToImages();
    frame_->TransferColorImagesToDevice(GetCommandBuffer());

    {
      RenderPassGuard render_pass_guard(this);

      BindVkDescriptorSets(pipeline_layout);

      r = RecordPushConstant(pipeline_layout);
      if (!r.IsSuccess())
        return r;

      device_->GetPtrs()->vkCmdBindPipeline(command_->GetVkCommandBuffer(),
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            pipeline);

      if (vertex_buffer != nullptr)
        vertex_buffer->BindToCommandBuffer(command_.get());

      uint32_t instance_count = command->GetInstanceCount();
      if (instance_count == 0 && command->GetVertexCount() != 0)
        instance_count = 1;

      if (command->IsIndexed()) {
        if (!index_buffer_)
          return Result("Vulkan: Draw indexed is used without given indices");

        r = index_buffer_->BindToCommandBuffer(command_.get());
        if (!r.IsSuccess())
          return r;

        // VkRunner spec says
        //   "vertexCount will be used as the index count, firstVertex
        //    becomes the vertex offset and firstIndex will always be zero."
        device_->GetPtrs()->vkCmdDrawIndexed(
            command_->GetVkCommandBuffer(),
            command->GetVertexCount(), /* indexCount */
            instance_count,            /* instanceCount */
            0,                         /* firstIndex */
            static_cast<int32_t>(
                command->GetFirstVertexIndex()), /* vertexOffset */
            0 /* firstInstance */);
      } else {
        device_->GetPtrs()->vkCmdDraw(command_->GetVkCommandBuffer(),
                                      command->GetVertexCount(), instance_count,
                                      command->GetFirstVertexIndex(), 0);
      }
    }

    frame_->TransferColorImagesToHost(command_.get());

    r = cmd_buf_guard.Submit(GetFenceTimeout());
    if (!r.IsSuccess())
      return r;
  }

  r = ReadbackDescriptorsToHostDataQueue();
  if (!r.IsSuccess())
    return r;

  frame_->CopyImagesToBuffers();

  device_->GetPtrs()->vkDestroyPipeline(device_->GetVkDevice(), pipeline,
                                        nullptr);
  device_->GetPtrs()->vkDestroyPipelineLayout(device_->GetVkDevice(),
                                              pipeline_layout, nullptr);
  return {};
}

}  // namespace vulkan
}  // namespace amber
