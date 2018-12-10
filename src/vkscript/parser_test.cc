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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>

#include "gtest/gtest.h"
#include "src/feature.h"
#include "src/format.h"
#include "src/vkscript/parser.h"

namespace amber {
namespace vkscript {

using VkScriptParserTest = testing::Test;

TEST_F(VkScriptParserTest, RequireBlockNoArgumentFeatures) {
  struct {
    const char* name;
    Feature feature;
  } features[] = {
      {"robustBufferAccess", Feature::kRobustBufferAccess},
      {"fullDrawIndexUint32", Feature::kFullDrawIndexUint32},
      {"imageCubeArray", Feature::kImageCubeArray},
      {"independentBlend", Feature::kIndependentBlend},
      {"geometryShader", Feature::kGeometryShader},
      {"tessellationShader", Feature::kTessellationShader},
      {"sampleRateShading", Feature::kSampleRateShading},
      {"dualSrcBlend", Feature::kDualSrcBlend},
      {"logicOp", Feature::kLogicOp},
      {"multiDrawIndirect", Feature::kMultiDrawIndirect},
      {"drawIndirectFirstInstance", Feature::kDrawIndirectFirstInstance},
      {"depthClamp", Feature::kDepthClamp},
      {"depthBiasClamp", Feature::kDepthBiasClamp},
      {"fillModeNonSolid", Feature::kFillModeNonSolid},
      {"depthBounds", Feature::kDepthBounds},
      {"wideLines", Feature::kWideLines},
      {"largePoints", Feature::kLargePoints},
      {"alphaToOne", Feature::kAlphaToOne},
      {"multiViewport", Feature::kMultiViewport},
      {"samplerAnisotropy", Feature::kSamplerAnisotropy},
      {"textureCompressionETC2", Feature::kTextureCompressionETC2},
      {"textureCompressionASTC_LDR", Feature::kTextureCompressionASTC_LDR},
      {"textureCompressionBC", Feature::kTextureCompressionBC},
      {"occlusionQueryPrecise", Feature::kOcclusionQueryPrecise},
      {"pipelineStatisticsQuery", Feature::kPipelineStatisticsQuery},
      {"vertexPipelineStoresAndAtomics",
       Feature::kVertexPipelineStoresAndAtomics},
      {"fragmentStoresAndAtomics", Feature::kFragmentStoresAndAtomics},
      {"shaderTessellationAndGeometryPointSize",
       Feature::kShaderTessellationAndGeometryPointSize},
      {"shaderImageGatherExtended", Feature::kShaderImageGatherExtended},
      {"shaderStorageImageExtendedFormats",
       Feature::kShaderStorageImageExtendedFormats},
      {"shaderStorageImageMultisample",
       Feature::kShaderStorageImageMultisample},
      {"shaderStorageImageReadWithoutFormat",
       Feature::kShaderStorageImageReadWithoutFormat},
      {"shaderStorageImageWriteWithoutFormat",
       Feature::kShaderStorageImageWriteWithoutFormat},
      {"shaderUniformBufferArrayDynamicIndexing",
       Feature::kShaderUniformBufferArrayDynamicIndexing},
      {"shaderSampledImageArrayDynamicIndexing",
       Feature::kShaderSampledImageArrayDynamicIndexing},
      {"shaderStorageBufferArrayDynamicIndexing",
       Feature::kShaderStorageBufferArrayDynamicIndexing},
      {"shaderStorageImageArrayDynamicIndexing",
       Feature::kShaderStorageImageArrayDynamicIndexing},
      {"shaderClipDistance", Feature::kShaderClipDistance},
      {"shaderCullDistance", Feature::kShaderCullDistance},
      {"shaderFloat64", Feature::kShaderFloat64},
      {"shaderInt64", Feature::kShaderInt64},
      {"shaderInt16", Feature::kShaderInt16},
      {"shaderResourceResidency", Feature::kShaderResourceResidency},
      {"shaderResourceMinLod", Feature::kShaderResourceMinLod},
      {"sparseBinding", Feature::kSparseBinding},
      {"sparseResidencyBuffer", Feature::kSparseResidencyBuffer},
      {"sparseResidencyImage2D", Feature::kSparseResidencyImage2D},
      {"sparseResidencyImage3D", Feature::kSparseResidencyImage3D},
      {"sparseResidency2Samples", Feature::kSparseResidency2Samples},
      {"sparseResidency4Samples", Feature::kSparseResidency4Samples},
      {"sparseResidency8Samples", Feature::kSparseResidency8Samples},
      {"sparseResidency16Samples", Feature::kSparseResidency16Samples},
      {"sparseResidencyAliased", Feature::kSparseResidencyAliased},
      {"variableMultisampleRate", Feature::kVariableMultisampleRate},
      {"inheritedQueries", Feature::kInheritedQueries},
  };

  for (const auto& feature : features) {
    Parser parser;
    Result r = parser.ProcessRequireBlockForTesting(feature.name);
    ASSERT_TRUE(r.IsSuccess()) << r.Error();

    auto script = parser.GetScript();
    auto& feats = ToVkScript(script.get())->RequiredFeatures();
    ASSERT_EQ(1U, feats.size());
    EXPECT_EQ(feature.feature, feats[0]);
  }
}

TEST_F(VkScriptParserTest, RequireBlockExtensions) {
  std::string block = R"(VK_KHR_storage_buffer_storage_class
VK_KHR_variable_pointers)";

  Parser parser;
  Result r = parser.ProcessRequireBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto& exts = script->RequiredExtensions();
  ASSERT_EQ(2U, exts.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", exts[0]);
  EXPECT_EQ("VK_KHR_variable_pointers", exts[1]);
}

TEST_F(VkScriptParserTest, RequireBlockFramebuffer) {
  std::string block = "framebuffer R32G32B32A32_SFLOAT";

  Parser parser;
  Result r = parser.ProcessRequireBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());
  EXPECT_EQ(BufferType::kColor, buffers[0]->GetBufferType());
  EXPECT_TRUE(buffers[0]->IsFormatBuffer());
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
      buffers[0]->AsFormatBuffer()->GetFormat().GetFormatType());
}

TEST_F(VkScriptParserTest, RequireBlockDepthStencil) {
  std::string block = "depthstencil D24_UNORM_S8_UINT";

  Parser parser;
  Result r = parser.ProcessRequireBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());
  EXPECT_EQ(BufferType::kDepth, buffers[0]->GetBufferType());
  EXPECT_TRUE(buffers[0]->IsFormatBuffer());
  EXPECT_EQ(FormatType::kD24_UNORM_S8_UINT,
      buffers[0]->AsFormatBuffer()->GetFormat().GetFormatType());
}

TEST_F(VkScriptParserTest, RequireBlockMultipleLines) {
  std::string block = R"(
# Requirements block stuff.
depthstencil D24_UNORM_S8_UINT
sparseResidency4Samples
framebuffer R32G32B32A32_SFLOAT
# More comments
inheritedQueries # line comment
)";

  Parser parser;
  Result r = parser.ProcessRequireBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());
  EXPECT_EQ(BufferType::kDepth, buffers[0]->GetBufferType());
  EXPECT_TRUE(buffers[0]->IsFormatBuffer());
  EXPECT_EQ(FormatType::kD24_UNORM_S8_UINT,
      buffers[0]->AsFormatBuffer()->GetFormat().GetFormatType());

  EXPECT_EQ(BufferType::kColor, buffers[1]->GetBufferType());
  EXPECT_TRUE(buffers[1]->IsFormatBuffer());
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
      buffers[1]->AsFormatBuffer()->GetFormat().GetFormatType());

  auto& feats = script->RequiredFeatures();
  EXPECT_EQ(Feature::kSparseResidency4Samples, feats[0]);
  EXPECT_EQ(Feature::kInheritedQueries, feats[1]);
}

TEST_F(VkScriptParserTest, IndicesBlock) {
  std::string block = "1 2 3";

  Parser parser;
  Result r = parser.ProcessIndicesBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());
  ASSERT_EQ(BufferType::kIndex, buffers[0]->GetBufferType());

  auto buffer_ptr = buffers[0].get();
  ASSERT_TRUE(buffer_ptr->IsDataBuffer());

  auto buffer = buffer_ptr->AsDataBuffer();
  EXPECT_TRUE(buffer->GetDatumType().IsUint16());
  EXPECT_EQ(3U, buffer->GetSize());
  auto& data = buffer->GetData();
  ASSERT_EQ(3U, data.size());

  EXPECT_TRUE(data[0].IsInteger());
  EXPECT_EQ(1, data[0].AsUint16());
  EXPECT_TRUE(data[1].IsInteger());
  EXPECT_EQ(2, data[1].AsUint16());
  EXPECT_TRUE(data[2].IsInteger());
  EXPECT_EQ(3, data[2].AsUint16());
}

TEST_F(VkScriptParserTest, IndicesBlockMultipleLines) {
  std::string block = R"(
# comment line
1 2 3   4 5 6
# another comment
7 8 9  10 11 12
)";

  std::vector<uint16_t> results = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  Parser parser;
  Result r = parser.ProcessIndicesBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());
  ASSERT_EQ(buffers[0]->GetBufferType(), BufferType::kIndex);

  auto& data = buffers[0]->GetData();
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_TRUE(data[i].IsInteger());
    EXPECT_EQ(results[i], data[i].AsUint16());
  }
}

TEST_F(VkScriptParserTest, IndicesBlockBadValue) {
  std::string block = "1 a 3";

  Parser parser;
  Result r = parser.ProcessIndicesBlockForTesting(block);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid value in indices block", r.Error());
}

TEST_F(VkScriptParserTest, IndicesBlockValueTooLarge) {
  std::string block = "100000000000 3";

  Parser parser;
  Result r = parser.ProcessIndicesBlockForTesting(block);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Value too large in indices block", r.Error());
}

TEST_F(VkScriptParserTest, VertexDataEmpty) {
  std::string block = "\n#comment\n";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  EXPECT_TRUE(script->GetBuffers().empty());
}

TEST_F(VkScriptParserTest, VertexDataHeaderFormatString) {
  std::string block = "0/R32G32_SFLOAT 1/A8B8G8R8_UNORM_PACK32";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());

  ASSERT_EQ(BufferType::kVertex, buffers[0]->GetBufferType());
  EXPECT_EQ(static_cast<uint8_t>(0U), buffers[0]->GetLocation());
  EXPECT_EQ(FormatType::kR32G32_SFLOAT,
            buffers[0]->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_TRUE(buffers[0]->GetData().empty());

  ASSERT_EQ(BufferType::kVertex, buffers[1]->GetBufferType());
  EXPECT_EQ(1U, buffers[1]->GetLocation());
  EXPECT_EQ(FormatType::kA8B8G8R8_UNORM_PACK32,
            buffers[1]->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_TRUE(buffers[1]->GetData().empty());
}

TEST_F(VkScriptParserTest, VertexDataHeaderGlslString) {
  std::string block = "0/float/vec2 1/int/vec3";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());

  ASSERT_EQ(BufferType::kVertex, buffers[0]->GetBufferType());
  EXPECT_EQ(static_cast<uint8_t>(0U), buffers[0]->GetLocation());
  EXPECT_EQ(FormatType::kR32G32_SFLOAT,
            buffers[0]->AsFormatBuffer()->GetFormat().GetFormatType());
  auto& comps1 = buffers[0]->AsFormatBuffer()->GetFormat().GetComponents();
  ASSERT_EQ(2U, comps1.size());
  EXPECT_EQ(FormatMode::kSFloat, comps1[0].mode);
  EXPECT_EQ(FormatMode::kSFloat, comps1[1].mode);
  EXPECT_TRUE(buffers[0]->GetData().empty());

  ASSERT_EQ(BufferType::kVertex, buffers[1]->GetBufferType());
  EXPECT_EQ(1U, buffers[1]->GetLocation());
  EXPECT_EQ(FormatType::kR32G32B32_SINT,
            buffers[1]->AsFormatBuffer()->GetFormat().GetFormatType());
  auto& comps2 = buffers[1]->AsFormatBuffer()->GetFormat().GetComponents();
  ASSERT_EQ(3U, comps2.size());
  EXPECT_EQ(FormatMode::kSInt, comps2[0].mode);
  EXPECT_EQ(FormatMode::kSInt, comps2[1].mode);
  EXPECT_EQ(FormatMode::kSInt, comps2[2].mode);
  EXPECT_TRUE(buffers[1]->GetData().empty());
}

TEST_F(VkScriptParserTest, TestBlock) {
  std::string block = R"(clear color 255 255 255 0
clear depth 10
clear stencil 2
clear)";

  Parser parser;
  Result r = parser.ProcessTestBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& cmds = script->GetCommands();
  ASSERT_EQ(4U, cmds.size());

  ASSERT_TRUE(cmds[0]->IsClearColor());
  auto* color_cmd = cmds[0]->AsClearColor();
  EXPECT_FLOAT_EQ(255.f, color_cmd->GetR());
  EXPECT_FLOAT_EQ(255.f, color_cmd->GetG());
  EXPECT_FLOAT_EQ(255.f, color_cmd->GetB());
  EXPECT_FLOAT_EQ(0.0f, color_cmd->GetA());

  ASSERT_TRUE(cmds[1]->IsClearDepth());
  EXPECT_EQ(10U, cmds[1]->AsClearDepth()->GetValue());

  ASSERT_TRUE(cmds[2]->IsClearStencil());
  EXPECT_EQ(2U, cmds[2]->AsClearStencil()->GetValue());

  EXPECT_TRUE(cmds[3]->IsClear());
}

TEST_F(VkScriptParserTest, VertexDataRows) {
  std::string block = R"(
# Vertex data
0/R32G32B32_SFLOAT  1/R8G8B8_UNORM
-1    -1 0.25       255 0 0  # ending comment
# Another Row
0.25  -1 0.25       255 0 255
)";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());

  ASSERT_EQ(BufferType::kVertex, buffers[0]->GetBufferType());

  std::vector<float> seg_0 = {-1.f, -1.f, 0.25f, 0.25f, -1.f, 0.25f};
  const auto& values_0 = buffers[0]->GetData();
  ASSERT_EQ(seg_0.size(), values_0.size());
  for (size_t i = 0; i < seg_0.size(); ++i) {
    ASSERT_TRUE(values_0[i].IsFloat());
    EXPECT_FLOAT_EQ(seg_0[i], values_0[i].AsFloat());
  }

  ASSERT_EQ(BufferType::kVertex, buffers[1]->GetBufferType());

  std::vector<uint8_t> seg_1 = {255, 0, 0, 255, 0, 255};
  const auto& values_1 = buffers[1]->GetData();
  ASSERT_EQ(seg_1.size(), values_1.size());
  for (size_t i = 0; i < seg_1.size(); ++i) {
    ASSERT_TRUE(values_1[i].IsInteger());
    EXPECT_EQ(seg_1[i], values_1[i].AsUint8());
  }
}

TEST_F(VkScriptParserTest, VertexDataShortRow) {
  std::string block = R"(
0/R32G32B32_SFLOAT  1/R8G8B8_UNORM
-1    -1 0.25       255 0 0
0.25  -1 0.25       255 0
)";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Too few cells in given vertex data row", r.Error());
}

TEST_F(VkScriptParserTest, VertexDataIncorrectValue) {
  std::string block = R"(
0/R32G32B32_SFLOAT  1/R8G8B8_UNORM
-1    -1 0.25       255 StringValue 0
0.25  -1 0.25       255 0 0
)";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid vertex data value", r.Error());
}

TEST_F(VkScriptParserTest, VertexDataRowsWithHex) {
  std::string block = R"(
0/A8B8G8R8_UNORM_PACK32
0xff0000ff
0xffff0000
)";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());
  ASSERT_EQ(BufferType::kVertex, buffers[0]->GetBufferType());

  std::vector<uint32_t> seg_0 = {0xff0000ff, 0xffff0000};
  const auto& values_0 = buffers[0]->GetData();
  ASSERT_EQ(seg_0.size(), values_0.size());

  for (size_t i = 0; i < seg_0.size(); ++i) {
    ASSERT_TRUE(values_0[i].IsInteger());
    EXPECT_EQ(seg_0[i], values_0[i].AsUint32());
  }
}

TEST_F(VkScriptParserTest, VertexDataRowsWithHexWrongColumn) {
  std::string block = R"(
0/R32G32B32_SFLOAT  1/R8G8B8_UNORM
-1    -1 0.25       0xffff0000
0.25  -1 0.25       255 0
)";

  Parser parser;
  Result r = parser.ProcessVertexDataBlockForTesting(block);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid vertex data value", r.Error());
}

}  // namespace vkscript
}  // namespace amber
