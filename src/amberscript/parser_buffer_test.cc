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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"

namespace amber {
namespace amberscript {

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, BufferData) {
  std::string in = R"(
BUFFER my_buffer DATA_TYPE uint32 DATA
1 2 3 4
55 99 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("my_buffer", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(7U, buffer->ElementCount());
  EXPECT_EQ(7U, buffer->ValueCount());
  EXPECT_EQ(7U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {1, 2, 3, 4, 55, 99, 1234};
  const auto* data = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferDataOneLine) {
  std::string in = "BUFFER my_buffer DATA_TYPE uint32 DATA 1 2 3 4 END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("my_buffer", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(4U, buffer->ElementCount());
  EXPECT_EQ(4U, buffer->ValueCount());
  EXPECT_EQ(4U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {1, 2, 3, 4};
  const auto* data = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferDataFloat) {
  std::string in = "BUFFER my_buffer DATA_TYPE float DATA 1 2 3 4 END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("my_buffer", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsFloat());
  EXPECT_EQ(4U, buffer->ElementCount());
  EXPECT_EQ(4U, buffer->ValueCount());
  EXPECT_EQ(4U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {1, 2, 3, 4};
  const auto* data = buffer->GetValues<float>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFill) {
  std::string in = "BUFFER my_buffer DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsUint8());
  EXPECT_EQ(5U, buffer->ElementCount());
  EXPECT_EQ(5U, buffer->ValueCount());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {5, 5, 5, 5, 5};
  const auto* data = buffer->GetValues<uint8_t>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFillFloat) {
  std::string in = "BUFFER my_buffer DATA_TYPE float SIZE 5 FILL 5.2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsFloat());
  EXPECT_EQ(5U, buffer->ElementCount());
  EXPECT_EQ(5U, buffer->ValueCount());
  EXPECT_EQ(5U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {5.2f, 5.2f, 5.2f, 5.2f, 5.2f};
  const auto* data = buffer->GetValues<float>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferSeries) {
  std::string in =
      "BUFFER my_buffer DATA_TYPE uint8 SIZE 5 SERIES_FROM 2 INC_BY 1";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsUint8());
  EXPECT_EQ(5U, buffer->ElementCount());
  EXPECT_EQ(5U, buffer->ValueCount());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint8_t> results = {2, 3, 4, 5, 6};
  const auto* data = buffer->GetValues<uint8_t>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferSeriesFloat) {
  std::string in =
      "BUFFER my_buffer DATA_TYPE float SIZE 5 SERIES_FROM 2.2 INC_BY "
      "1.1";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsFloat());
  EXPECT_EQ(5U, buffer->ElementCount());
  EXPECT_EQ(5U, buffer->ValueCount());
  EXPECT_EQ(5U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {2.2f, 3.3f, 4.4f, 5.5f, 6.6f};
  const auto* data = buffer->GetValues<float>();
  ASSERT_EQ(results.size(), buffer->ValueCount());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferMultipleBuffers) {
  std::string in = R"(
BUFFER color_buffer DATA_TYPE uint8 SIZE 5 FILL 5
BUFFER storage_buffer DATA_TYPE uint32 DATA
1 2 3 4
55 99 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("color_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsUint8());
  EXPECT_EQ(5U, buffer->ElementCount());
  EXPECT_EQ(5U, buffer->ValueCount());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results0 = {5, 5, 5, 5, 5};
  const auto* data0 = buffer->GetValues<uint8_t>();
  ASSERT_EQ(results0.size(), buffer->ValueCount());
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }

  ASSERT_TRUE(buffers[1] != nullptr);

  buffer = buffers[1].get();
  EXPECT_EQ("storage_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(7U, buffer->ElementCount());
  EXPECT_EQ(7U, buffer->ValueCount());
  EXPECT_EQ(7U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results1 = {1, 2, 3, 4, 55, 99, 1234};
  const auto* data1 = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results1.size(), buffer->ValueCount());
  for (size_t i = 0; i < results1.size(); ++i) {
    EXPECT_EQ(results1[i], data1[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFillMultiRow) {
  std::string in = R"(
BUFFER my_index_buffer DATA_TYPE vec2<int32> SIZE 5 FILL 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsInt32());
  EXPECT_EQ(5U, buffer->ElementCount());
  EXPECT_EQ(10U, buffer->ValueCount());
  EXPECT_EQ(10U * sizeof(int32_t), buffer->GetSizeInBytes());

  std::vector<int32_t> results0 = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  const auto* data0 = buffer->GetValues<int32_t>();
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferDataMultiRow) {
  std::string in = R"(
BUFFER my_index_buffer DATA_TYPE vec2<int32> DATA
2 3
4 5
6 7
8 9
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsInt32());
  EXPECT_EQ(4U, buffer->ElementCount());
  EXPECT_EQ(8U, buffer->ValueCount());
  EXPECT_EQ(8U * sizeof(int32_t), buffer->GetSizeInBytes());

  std::vector<int32_t> results0 = {2, 3, 4, 5, 6, 7, 8, 9};
  const auto* data0 = buffer->GetValues<int32_t>();
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferDataHex) {
  std::string in = R"(
BUFFER my_index_buffer DATA_TYPE uint32 DATA
0xff000000
0x00ff0000
0x0000ff00
0x000000ff
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(4U, buffer->ElementCount());
  EXPECT_EQ(4U, buffer->ValueCount());
  EXPECT_EQ(4U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results0 = {4278190080, 16711680, 65280, 255};
  const auto* data0 = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results0.size(), buffer->ValueCount());
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFormat) {
  std::string in = "BUFFER my_buf FORMAT R32G32B32A32_SINT";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  auto* buffer = buffers[0].get();
  EXPECT_EQ("my_buf", buffer->GetName());

  auto fmt = buffer->GetFormat();
  auto& comps = fmt->GetComponents();
  ASSERT_EQ(4U, comps.size());

  EXPECT_EQ(FormatComponentType::kR, comps[0].type);
  EXPECT_EQ(FormatMode::kSInt, comps[0].mode);
  EXPECT_EQ(32U, comps[0].num_bits);
  EXPECT_EQ(FormatComponentType::kG, comps[1].type);
  EXPECT_EQ(FormatMode::kSInt, comps[1].mode);
  EXPECT_EQ(32U, comps[1].num_bits);
  EXPECT_EQ(FormatComponentType::kB, comps[2].type);
  EXPECT_EQ(FormatMode::kSInt, comps[2].mode);
  EXPECT_EQ(32U, comps[2].num_bits);
  EXPECT_EQ(FormatComponentType::kA, comps[3].type);
  EXPECT_EQ(FormatMode::kSInt, comps[3].mode);
  EXPECT_EQ(32U, comps[3].num_bits);
}

struct BufferParseError {
  const char* in;
  const char* err;
};
using AmberScriptParserBufferParseErrorTest =
    testing::TestWithParam<BufferParseError>;
TEST_P(AmberScriptParserBufferParseErrorTest, Test) {
  auto test_data = GetParam();

  Parser parser;
  Result r = parser.Parse(test_data.in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.in;
  EXPECT_EQ(std::string(test_data.err), r.Error()) << test_data.in;
}

INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserBufferParseErrorTest,
    AmberScriptParserBufferParseErrorTest,
    testing::Values(
        BufferParseError{"BUFFER my_buf FORMAT 123",
                         "1: BUFFER FORMAT must be a string"},
        BufferParseError{"BUFFER my_buf FORMAT A23A32",
                         "1: invalid BUFFER FORMAT"},
        BufferParseError{"BUFFER my_buf FORMAT",
                         "1: BUFFER FORMAT must be a string"},
        BufferParseError{"BUFFER my_buffer FORMAT R32G32B32A32_SFLOAT EXTRA",
                         "1: unknown token: EXTRA"},
        BufferParseError{"BUFFER 1234 DATA_TYPE uint8 SIZE 5 FILL 5",
                         "1: invalid BUFFER name provided"},
        BufferParseError{"BUFFER DATA_TYPE uint8 SIZE 5 FILL 5",
                         "1: missing BUFFER name"},

        BufferParseError{"BUFFER my_buf 1234",
                         "1: invalid BUFFER command provided"},
        BufferParseError{"BUFFER my_buf INVALID",
                         "1: unknown BUFFER command provided: INVALID"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE INVALID FILL 5",
                         "1: BUFFER size invalid"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE FILL 5",
                         "1: BUFFER size invalid"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL",
                         "1: missing BUFFER fill value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL INVALID",
                         "1: invalid BUFFER fill value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 INVALID 5",
                         "1: invalid BUFFER initializer provided"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL INVALID",
                         "1: invalid BUFFER fill value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL",
                         "1: missing BUFFER fill value"},
        BufferParseError{
            "BUFFER my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM INC_BY 2",
            "1: invalid BUFFER series_from value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 2",
                         "1: missing BUFFER series_from inc_by"},
        BufferParseError{
            "BUFFER my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 2 INC_BY",
            "1: missing BUFFER series_from inc_by value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 "
                         "SERIES_FROM INVALID INC_BY 2",
                         "1: invalid BUFFER series_from value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 "
                         "SERIES_FROM 1 INC_BY INVALID",
                         "1: invalid BUFFER series_from inc_by value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 "
                         "SERIES_FROM 1 INVALID 2",
                         "1: BUFFER series_from invalid command"},
        BufferParseError{
            "BUFFER my_index_buffer DATA_TYPE int32 DATA\n1.234\nEND",
            "2: invalid BUFFER data value: 1.234"},
        BufferParseError{
            "BUFFER my_index_buffer DATA_TYPE int32 DATA\nINVALID\nEND",
            "2: invalid BUFFER data value: INVALID"},
        BufferParseError{
            "BUFFER my_index_buffer DATA_TYPE int32 DATA INVALID\n123\nEND",
            "1: invalid BUFFER data value: INVALID"},
        BufferParseError{"BUFFER my_index_buffer DATA_TYPE int32 SIZE 256 FILL "
                         "5 INVALID\n123\nEND",
                         "1: extra parameters after BUFFER fill command"},
        BufferParseError{
            "BUFFER my_buffer DATA_TYPE int32 SIZE 256 SERIES_FROM 2 "
            "INC_BY 5 "
            "INVALID",
            "1: extra parameters after BUFFER series_from command"},
        BufferParseError{"BUFFER my_buf DATA_TYPE int32 SIZE 5 FILL 5\nBUFFER "
                         "my_buf DATA_TYPE int16 SIZE 5 FILL 2",
                         // NOLINTNEXTLINE(whitespace/parens)
                         "2: duplicate buffer name provided"}));

struct BufferData {
  const char* name;
  FormatMode type;
  size_t num_bits;
  size_t row_count;
  size_t column_count;
};

using AmberScriptParserBufferDataTypeTest = testing::TestWithParam<BufferData>;
TEST_P(AmberScriptParserBufferDataTypeTest, BufferTypes) {
  auto test_data = GetParam();

  std::string in = std::string("BUFFER my_buf DATA_TYPE ") + test_data.name +
                   " SIZE 2 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << test_data.name << " :" << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);

  auto* buffer = buffers[0].get();
  auto fmt = buffer->GetFormat();
  EXPECT_EQ(test_data.row_count, fmt->RowCount());
  EXPECT_EQ(test_data.column_count, fmt->ColumnCount());
  EXPECT_EQ(test_data.type, fmt->GetComponents()[0].mode);
  EXPECT_EQ(test_data.num_bits, fmt->GetComponents()[0].num_bits);
}
INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserTestsDataType,
    AmberScriptParserBufferDataTypeTest,
    testing::Values(BufferData{"int8", FormatMode::kSInt, 8, 1, 1},
                    BufferData{"int16", FormatMode::kSInt, 16, 1, 1},
                    BufferData{"int32", FormatMode::kSInt, 32, 1, 1},
                    BufferData{"int64", FormatMode::kSInt, 64, 1, 1},
                    BufferData{"uint8", FormatMode::kUInt, 8, 1, 1},
                    BufferData{"uint16", FormatMode::kUInt, 16, 1, 1},
                    BufferData{"uint32", FormatMode::kUInt, 32, 1, 1},
                    BufferData{"uint64", FormatMode::kUInt, 64, 1, 1},
                    BufferData{"float", FormatMode::kSFloat, 32, 1, 1},
                    BufferData{"double", FormatMode::kSFloat, 64, 1, 1},
                    BufferData{"vec2<int8>", FormatMode::kSInt, 8, 2, 1},
                    BufferData{"vec3<float>", FormatMode::kSFloat, 32, 3, 1},
                    BufferData{"vec4<uint32>", FormatMode::kUInt, 32, 4, 1},
                    BufferData{"mat2x4<int32>", FormatMode::kSInt, 32, 2, 4},
                    BufferData{"mat3x3<float>", FormatMode::kSFloat, 32, 3, 3},
                    BufferData{"mat4x2<uint16>", FormatMode::kUInt, 16, 4, 2},
                    BufferData{"B8G8R8_UNORM", FormatMode::kUNorm, 8, 3,
                               1}));  // NOLINT(whitespace/parens)

struct NameData {
  const char* name;
};

using AmberScriptParserBufferDataTypeInvalidTest =
    testing::TestWithParam<NameData>;
TEST_P(AmberScriptParserBufferDataTypeInvalidTest, BufferTypes) {
  auto test_data = GetParam();

  std::string in = std::string("BUFFER my_buf DATA_TYPE ") + test_data.name +
                   " SIZE 4 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.name;
  EXPECT_EQ("1: invalid data_type provided", r.Error()) << test_data.name;
}
INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserBufferDataTypeInvalidTestSamples,
    AmberScriptParserBufferDataTypeInvalidTest,
    testing::Values(NameData{"int17"},
                    NameData{"uintt0"},
                    NameData{"vec7<uint8>"},
                    NameData{"vec27<uint8>"},
                    NameData{"vec2<vec2<float>>"},
                    NameData{"vec2<mat2x2<float>>"},
                    NameData{"vec2float>"},
                    NameData{"vec2<uint32"},
                    NameData{"vec2<uint4>"},
                    NameData{"vec2<>"},
                    NameData{"vec2"},
                    NameData{"mat1x1<double>"},
                    NameData{"mat5x2<double>"},
                    NameData{"mat2x5<double>"},
                    NameData{"mat22x22<double>"},
                    NameData{"matx5<double>"},
                    NameData{"mat2<double>"},
                    NameData{"mat2x<double>"},
                    NameData{"mat2x2<vec4<float>>"},
                    NameData{"mat2x2<mat3x3<double>>"},
                    NameData{"mat2x2<unit7>"},
                    NameData{"mat2x2"},
                    NameData{"mat2x2<>"}));  // NOLINT(whitespace/parens)

}  // namespace amberscript
}  // namespace amber
