// Copyright 2024 The Amber Authors.
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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"

namespace amber {
namespace amberscript {

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, RayTracingBlasName) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Bottom level acceleration structure requires a name",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasNameDup) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
END
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "4: Bottom level acceleration structure with this name already defined",
      r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasNameNoEOL) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: New line expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasNoEND) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: END command missing", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasNoId) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Identifier expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasUnexpId) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  UNEXPECTED)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Unexpected identifier", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasUnexpGeomId) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY 1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Identifier expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasUnexpGeom) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY UNEXPECTED)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Unexpected geometry type", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasGeomSingleType) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
    0 0 0  0 1 0  1 0 0
  END
  GEOMETRY AABBS
    0 0 0  1 1 1
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Only one type of geometry is allowed within a BLAS", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasTriangleEmpty) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: No triangles have been specified.", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasTriangleThreeVertices) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
    0.0 0.0 0.0  0.0 0.0 0.0
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: Each triangle should include three vertices.", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasTriangleThreeFloats) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
    0.0 0.0 0.0  0.0 0.0 0.0  0.0
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: Each vertex consists of three float coordinates.", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasTriangleNoEND) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: END expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasTriangleUnexpDataType) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES "unexpected_string"
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Unexpected data type", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasTriangleGeometryFlags) {
  {
    std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
    FLAGS OPAQUE NO_DUPLICATE_ANY_HIT NO_SUCH_FLAG
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("4: Unknown flag: NO_SUCH_FLAG", r.Error());
  }
  {
    std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY TRIANGLES
    FLAGS 1
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("4: Identifier expected", r.Error());
  }
}

TEST_F(AmberScriptParserTest, RayTracingBlasAABBEmpty) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: No AABBs have been specified.", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasAABBInvalidData) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  0.0 0.0 0.0  0.0
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "5: Each vertex consists of three float coordinates. Each AABB should "
      "include two vertices.",
      r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasAABBNoEND) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: END expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasAABBUnexpDataType) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS "unexpected_string"
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Unexpected data type", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingBlasAABBGeometryFlags) {
  {
    std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    FLAGS OPAQUE NO_DUPLICATE_ANY_HIT NO_SUCH_FLAG
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("4: Unknown flag: NO_SUCH_FLAG", r.Error());
  }
  {
    std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    FLAGS 1
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("4: Identifier expected", r.Error());
  }
}

TEST_F(AmberScriptParserTest, RayTracingTlasName) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid TLAS name provided", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasNameDup) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas_name
END
ACCELERATION_STRUCTURE TOP_LEVEL tlas_name
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: duplicate TLAS name provided", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasNameNoEOL) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas_name END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: New line expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasNoEND) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas_name
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: END command missing", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasNoId) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas_name
1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: expected identifier", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasUnexpId) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas_name
  UNEXPECTED)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown token: UNEXPECTED", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstNoName) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Bottom level acceleration structure name expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstNoBlas) {
  std::string in = R"(
ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Bottom level acceleration structure with given name not found",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstUnexpEnd) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Unexpected end", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstExpId) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name 1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: expected identifier", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstInvalidToken) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name TOKEN)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Unknown token in BOTTOM_LEVEL_INSTANCE block: TOKEN",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstMask) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name MASK no_mask)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Integer or hex value expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstOffset) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name OFFSET no_offset)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Integer or hex value expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstIndex) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name INDEX no_index)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Integer or hex value expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstFlagsEmpty) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name FLAGS)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: END command missing", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstFlagsUnkFlag) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name FLAGS 16 0x0F NO_SUCH_FLAG)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Unknown flag: NO_SUCH_FLAG", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstFlagsIdExp) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name FLAGS "no_id")";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Identifier expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstTransformNoEnd) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
    TRANSFORM
      1 0 0 0  0 1 0 0  0 0 1 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: END command missing", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstTransformUnknownToken) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name TRANSFORM
    INVALID_TOKEN
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: Unknown token: INVALID_TOKEN", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingTlasBlasInstTransformIncomplete) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name TRANSFORM
    1 2
  END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: Transform matrix expected to have 12 numbers", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBind) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND 0 tlas1 DESCRIPTOR_SET 0 BINDING 0
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "14: missing BUFFER, BUFFER_ARRAY, SAMPLER, SAMPLER_ARRAY, or "
      "ACCELERATION_STRUCTURE in BIND command",
      r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindNothing) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND ACCELERATION_STRUCTURE 0
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: missing top level acceleration structure name in BIND command",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindNoTlas) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND ACCELERATION_STRUCTURE no_tlas
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: unknown top level acceleration structure: no_tlas", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindNoSetOrBinding) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND ACCELERATION_STRUCTURE tlas1 NO_TOKEN
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: missing DESCRIPTOR_SET or BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindBadSet) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND ACCELERATION_STRUCTURE tlas1 DESCRIPTOR_SET 0.0
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindBadBindingKeyword) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND ACCELERATION_STRUCTURE tlas1 DESCRIPTOR_SET 0 NOT_BINDING
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindBadBindingValue) {
  std::string in = R"(
ACCELERATION_STRUCTURE BOTTOM_LEVEL blas_name
  GEOMETRY AABBS
    0.0 0.0 0.0  1.0 1.0 1.0
  END
END

ACCELERATION_STRUCTURE TOP_LEVEL tlas1
  BOTTOM_LEVEL_INSTANCE blas_name
  END
END

PIPELINE raytracing my_rtpipeline
  BIND ACCELERATION_STRUCTURE tlas1 DESCRIPTOR_SET 0 BINDING 0.0
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupNoName) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_GROUP 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Group name expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupNoNameDup) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP group raygen1
  SHADER_GROUP group raygen1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: Group name already exists", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupEmpty) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_GROUP group
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupNoShaderName) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_GROUP group 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Shader name expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupNoShader) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_GROUP group no_shader
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Shader not found: no_shader", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupInvalidShader) {
  std::string in = R"(
SHADER vertex vertex1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP group vertex1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: Shader must be of raytracing type", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupTwoGeneral) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

SHADER ray_generation raygen2 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP group raygen1 raygen2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: Two general shaders cannot be in one group", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupAddGenToHit) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

SHADER intersection intersection1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP hit_group intersection1 raygen1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: Hit group cannot contain general shaders", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupAddAHitToGen) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

SHADER any_hit ahit1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group raygen1 ahit1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: General group cannot contain any hit shaders", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupAddCHitToGen) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

SHADER closest_hit chit1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group raygen1 chit1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: General group cannot contain closest hit shaders", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupAddSectToGen) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

SHADER intersection sect1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group raygen1 sect1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: General group cannot contain intersection shaders", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupAHitDouble) {
  std::string in = R"(
SHADER any_hit ahit1 GLSL
  #version 460 core
  void main() {}
END

SHADER any_hit ahit2 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group ahit1 ahit2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: Two any hit shaders cannot be in one group", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupCHitDouble) {
  std::string in = R"(
SHADER closest_hit chit1 GLSL
  #version 460 core
  void main() {}
END

SHADER closest_hit chit2 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group chit1 chit2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: Two closest hit shaders cannot be in one group", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineBindShaderGroupSectDouble) {
  std::string in = R"(
SHADER intersection sect1 GLSL
  #version 460 core
  void main() {}
END

SHADER intersection sect2 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group sect1 sect2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: Two intersection shaders cannot be in one group", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineSBTNoName) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_BINDING_TABLE
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: SHADER_BINDINGS_TABLE requires a name", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineSBTDup) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP gen_group raygen1
  SHADER_BINDING_TABLE sbt1
  END
  SHADER_BINDING_TABLE sbt1
  END
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: SHADER_BINDINGS_TABLE with this name already defined",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineSBTExtraToken) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_BINDING_TABLE sbt1 extra_token
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: New line expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineSBTNoEnd) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_BINDING_TABLE sbt1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: END command missing", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineSBTNoId) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
  SHADER_BINDING_TABLE sbt1
    0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: Identifier expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRun) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 raygen1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline RAYGEN sbt1 1 1 z
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: invalid parameter for RUN command: z", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunIncomplete) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 raygen1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: Incomplete RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunExpectsSBTType) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 raygen1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline 0.0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: Shader binding table type is expected", r.Error());
}


TEST_F(AmberScriptParserTest, RayTracingRunExpectsSBTName) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 raygen1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline RAYGEN 0.0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: Shader binding table name expected", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunExpectsSBTUndefined) {
  std::string in = R"(
PIPELINE raytracing my_rtpipeline
END
RUN my_rtpipeline RAYGEN sbt3
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: Shader binding table with this name was not defined",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunExpectsSBTUnknownType) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 raygen1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline RAYGEN2 sbt1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: Unknown shader binding table type", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunSBTRGenDup) {
  std::string in = R"(
SHADER ray_generation raygen1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 raygen1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline RAYGEN sbt1 RAYGEN sbt1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: RAYGEN shader binding table can specified only once",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunSBTMissDup) {
  std::string in = R"(
SHADER miss miss1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 miss1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline MISS sbt1 MISS sbt1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: MISS shader binding table can specified only once",
            r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunSBTHitDup) {
  std::string in = R"(
SHADER any_hit ahit1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 ahit1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline HIT sbt1 HIT sbt1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: HIT shader binding table can specified only once", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingRunSBTCallDup) {
  std::string in = R"(
SHADER callable call1 GLSL
  #version 460 core
  void main() {}
END

PIPELINE raytracing my_rtpipeline
  SHADER_GROUP g1 call1
  SHADER_BINDING_TABLE sbt1
    g1
  END
END

RUN my_rtpipeline CALL sbt1 CALL sbt1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: CALL shader binding table can specified only once", r.Error());
}

TEST_F(AmberScriptParserTest, RayTracingPipelineMaxRaypayloadSize) {
  {
    std::string in = R"(
PIPELINE compute my_pipeline
  MAX_RAY_PAYLOAD_SIZE 16
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ(
        "3: Ray payload size parameter is allowed only for ray tracing "
        "pipeline",
        r.Error());
  }
  {
    std::string in = R"(
PIPELINE graphics my_pipeline
  MAX_RAY_PAYLOAD_SIZE 16
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ(
        "3: Ray payload size parameter is allowed only for ray tracing "
        "pipeline",
        r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  MAX_RAY_PAYLOAD_SIZE a
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Ray payload size expects an integer", r.Error());
  }
}

TEST_F(AmberScriptParserTest, RayTracingPipelineMaxRayHitAttributeSize) {
  {
    std::string in = R"(
PIPELINE compute my_pipeline
  MAX_RAY_HIT_ATTRIBUTE_SIZE 16
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ(
        "3: Ray hit attribute size is allowed only for ray tracing pipeline",
        r.Error());
  }
  {
    std::string in = R"(
PIPELINE graphics my_pipeline
  MAX_RAY_HIT_ATTRIBUTE_SIZE 16
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ(
        "3: Ray hit attribute size is allowed only for ray tracing pipeline",
        r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  MAX_RAY_HIT_ATTRIBUTE_SIZE a
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Ray hit attribute size expects an integer", r.Error());
  }
}

TEST_F(AmberScriptParserTest, RayTracingPipelineMaxRecursionDepthSize) {
  {
    std::string in = R"(
PIPELINE compute my_pipeline
  MAX_RAY_RECURSION_DEPTH 1
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Ray recursion depth is allowed only for ray tracing pipeline",
              r.Error());
  }
  {
    std::string in = R"(
PIPELINE graphics my_pipeline
  MAX_RAY_RECURSION_DEPTH 1
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Ray recursion depth is allowed only for ray tracing pipeline",
              r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  MAX_RAY_RECURSION_DEPTH a
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Ray recursion depth expects an integer", r.Error());
  }
}

TEST_F(AmberScriptParserTest, RayTracingPipelineFlags) {
  {
    std::string in = R"(
PIPELINE compute my_pipeline
  FLAGS LIBRARY
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Flags are allowed only for ray tracing pipeline", r.Error());
  }
  {
    std::string in = R"(
PIPELINE graphics my_pipeline
  FLAGS LIBRARY
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Flags are allowed only for ray tracing pipeline", r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  FLAGS
    LIBRARY
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("5: END command missing", r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  FLAGS UNKNOWN_FLAG
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Unknown flag: UNKNOWN_FLAG", r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  FLAGS 1.0
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Identifier expected", r.Error());
  }
}

TEST_F(AmberScriptParserTest, RayTracingPipelineUseLibrary) {
  {
    std::string in = R"(
PIPELINE raytracing base_pipeline_lib
  FLAGS LIBRARY
END

PIPELINE compute my_pipeline
  USE_LIBRARY base_pipeline_lib
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("7: Use library is allowed only for ray tracing pipeline",
              r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing base_pipeline_lib
  FLAGS LIBRARY
END

PIPELINE graphics my_pipeline
  USE_LIBRARY base_pipeline_lib
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("7: Use library is allowed only for ray tracing pipeline",
              r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  USE_LIBRARY base_pipeline_lib
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Pipeline not found: base_pipeline_lib", r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  USE_LIBRARY)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: EOL expected", r.Error());
  }
  {
    std::string in = R"(
PIPELINE raytracing my_pipeline
  USE_LIBRARY 1
)";

    Parser parser;
    Result r = parser.Parse(in);
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("3: Unexpected data type", r.Error());
  }
}

}  // namespace amberscript
}  // namespace amber
