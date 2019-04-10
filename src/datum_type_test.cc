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

#include "src/datum_type.h"

#include "gtest/gtest.h"

namespace amber {

using DatumTypeTest = testing::Test;

struct Data {
  DataType type;
  uint32_t row_count;
  FormatType format_type;
};
using DatumTypeTestFormat = testing::TestWithParam<Data>;
TEST_P(DatumTypeTestFormat, ToFormat) {
  auto test_data = GetParam();

  DatumType dt;
  dt.SetType(test_data.type);
  dt.SetRowCount(test_data.row_count);

  auto fmt = dt.AsFormat();
  EXPECT_EQ(test_data.format_type, fmt->GetFormatType());
}
INSTANTIATE_TEST_CASE_P(
    DatumTypeTestFormat,
    DatumTypeTestFormat,
    testing::Values(
        Data{DataType::kInt8, 1, FormatType::kR8_SINT},
        Data{DataType::kInt8, 2, FormatType::kR8G8_SINT},
        Data{DataType::kInt8, 3, FormatType::kR8G8B8_SINT},
        Data{DataType::kInt8, 4, FormatType::kR8G8B8A8_SINT},
        Data{DataType::kInt16, 1, FormatType::kR16_SINT},
        Data{DataType::kInt16, 2, FormatType::kR16G16_SINT},
        Data{DataType::kInt16, 3, FormatType::kR16G16B16_SINT},
        Data{DataType::kInt16, 4, FormatType::kR16G16B16A16_SINT},
        Data{DataType::kInt32, 1, FormatType::kR32_SINT},
        Data{DataType::kInt32, 2, FormatType::kR32G32_SINT},
        Data{DataType::kInt32, 3, FormatType::kR32G32B32_SINT},
        Data{DataType::kInt32, 4, FormatType::kR32G32B32A32_SINT},
        Data{DataType::kInt64, 1, FormatType::kR64_SINT},
        Data{DataType::kInt64, 2, FormatType::kR64G64_SINT},
        Data{DataType::kInt64, 3, FormatType::kR64G64B64_SINT},
        Data{DataType::kInt64, 4, FormatType::kR64G64B64A64_SINT},

        Data{DataType::kUint8, 1, FormatType::kR8_UINT},
        Data{DataType::kUint8, 2, FormatType::kR8G8_UINT},
        Data{DataType::kUint8, 3, FormatType::kR8G8B8_UINT},
        Data{DataType::kUint8, 4, FormatType::kR8G8B8A8_UINT},
        Data{DataType::kUint16, 1, FormatType::kR16_UINT},
        Data{DataType::kUint16, 2, FormatType::kR16G16_UINT},
        Data{DataType::kUint16, 3, FormatType::kR16G16B16_UINT},
        Data{DataType::kUint16, 4, FormatType::kR16G16B16A16_UINT},
        Data{DataType::kUint32, 1, FormatType::kR32_UINT},
        Data{DataType::kUint32, 2, FormatType::kR32G32_UINT},
        Data{DataType::kUint32, 3, FormatType::kR32G32B32_UINT},
        Data{DataType::kUint32, 4, FormatType::kR32G32B32A32_UINT},
        Data{DataType::kUint64, 1, FormatType::kR64_UINT},
        Data{DataType::kUint64, 2, FormatType::kR64G64_UINT},
        Data{DataType::kUint64, 3, FormatType::kR64G64B64_UINT},
        Data{DataType::kUint64, 4, FormatType::kR64G64B64A64_UINT},

        Data{DataType::kFloat, 1, FormatType::kR32_SFLOAT},
        Data{DataType::kFloat, 2, FormatType::kR32G32_SFLOAT},
        Data{DataType::kFloat, 3, FormatType::kR32G32B32_SFLOAT},
        Data{DataType::kFloat, 4, FormatType::kR32G32B32A32_SFLOAT},

        Data{DataType::kDouble, 1, FormatType::kR64_SFLOAT},
        Data{DataType::kDouble, 2, FormatType::kR64G64_SFLOAT},
        Data{DataType::kDouble, 3, FormatType::kR64G64B64_SFLOAT},
        Data{
            DataType::kDouble, 4,
            FormatType::kR64G64B64A64_SFLOAT}), );  // NOLINT(whitespace/parens)

}  // namespace amber
