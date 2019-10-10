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

#include "src/type.h"

#include "gtest/gtest.h"

namespace amber {
namespace type {

using TypeTest = testing::Test;

TEST_F(TypeTest, IsArray) {
  Number i(FormatMode::kSInt, 16);

  EXPECT_FALSE(i.IsArray());
  EXPECT_FALSE(i.IsRuntimeArray());
  EXPECT_FALSE(i.IsSizedArray());

  i.SetIsRuntimeArray();
  EXPECT_TRUE(i.IsArray());
  EXPECT_TRUE(i.IsRuntimeArray());
  EXPECT_FALSE(i.IsSizedArray());

  i.SetIsSizedArray(3);
  EXPECT_TRUE(i.IsArray());
  EXPECT_FALSE(i.IsRuntimeArray());
  EXPECT_TRUE(i.IsSizedArray());
  EXPECT_EQ(3, i.ArraySize());
}

TEST_F(TypeTest, IsStruct) {
  EXPECT_FALSE(Number(FormatMode::kSInt).IsStruct());
  EXPECT_TRUE(Struct().IsStruct());
  EXPECT_FALSE(List().IsStruct());
}

TEST_F(TypeTest, IsNumber) {
  EXPECT_TRUE(Number(FormatMode::kSInt).IsNumber());
  EXPECT_FALSE(Struct().IsNumber());
  EXPECT_FALSE(List().IsNumber());
}

TEST_F(TypeTest, IsList) {
  EXPECT_FALSE(Number(FormatMode::kSInt).IsList());
  EXPECT_FALSE(Struct().IsList());
  EXPECT_TRUE(List().IsList());
}

TEST_F(TypeTest, Vectors) {
  Number i(FormatMode::kSInt, 16);
  i.SetRowCount(2);

  EXPECT_EQ(2, i.RowCount());
  EXPECT_TRUE(i.IsVec());
  EXPECT_FALSE(i.IsVec3());
  EXPECT_FALSE(i.IsMatrix());

  i.SetColumnCount(3);
  EXPECT_FALSE(i.IsVec3());
}

TEST_F(TypeTest, Matrix) {
  Number i(FormatMode::kSInt, 16);
  i.SetColumnCount(2);
  i.SetRowCount(2);

  EXPECT_EQ(2, i.ColumnCount());
  EXPECT_EQ(2, i.RowCount());

  EXPECT_FALSE(i.IsVec());
  EXPECT_TRUE(i.IsMatrix());
}

TEST_F(TypeTest, SizeInBytesForVector) {
  Number i(FormatMode::kSInt, 32);
  uint32_t bytes = i.SizeInBytes();

  i.SetRowCount(3);
  EXPECT_EQ(bytes, i.SizeInBytes());
}

TEST_F(TypeTest, SizeInBytesForMatrix) {
  Number i(FormatMode::kSInt, 32);
  uint32_t bytes = i.SizeInBytes();

  i.SetColumnCount(3);
  i.SetRowCount(3);
  EXPECT_EQ(bytes, i.SizeInBytes());
}

TEST_F(TypeTest, SizeInBytesForArray) {
  Number i(FormatMode::kSInt, 32);
  uint32_t bytes = i.SizeInBytes();

  i.SetIsSizedArray(3);
  EXPECT_EQ(bytes, i.SizeInBytes());
}

TEST_F(TypeTest, NumberEqual) {
  Number n1(FormatMode::kSFloat, 32);
  Number n2(FormatMode::kSFloat, 32);
  Number n3(FormatMode::kSFloat, 16);
  Number n4(FormatMode::kSInt, 32);

  List l;
  Struct s;

  EXPECT_TRUE(n1.Equal(&n2));
  EXPECT_FALSE(n1.Equal(&n3));
  EXPECT_FALSE(n1.Equal(&n4));
  EXPECT_FALSE(n1.Equal(&l));
  EXPECT_FALSE(n1.Equal(&s));
}

TEST_F(TypeTest, ListPacked) {
  List l;
  EXPECT_FALSE(l.IsPacked());
  EXPECT_EQ(0U, l.PackSizeInBits());

  l.SetPackSizeInBits(32);
  EXPECT_TRUE(l.IsPacked());
  EXPECT_EQ(32U, l.PackSizeInBits());
}

TEST_F(TypeTest, ListEqual) {
  List l1;
  List l2;

  l1.AddMember(FormatComponentType::kR, FormatMode::kSFloat, 32);
  l2.AddMember(FormatComponentType::kR, FormatMode::kSFloat, 32);
  EXPECT_TRUE(l1.Equal(&l2));

  l2.SetPackSizeInBits(24);
  EXPECT_FALSE(l1.Equal(&l2));

  List l3;
  l3.AddMember(FormatComponentType::kR, FormatMode::kSFloat, 16);
  EXPECT_FALSE(l1.Equal(&l3));

  List l4;
  l4.AddMember(FormatComponentType::kR, FormatMode::kSInt, 16);
  EXPECT_FALSE(l1.Equal(&l4));

  List l5;
  l5.AddMember(FormatComponentType::kG, FormatMode::kSFloat, 32);
  EXPECT_FALSE(l1.Equal(&l5));

  List l6;
  l6.AddMember(FormatComponentType::kR, FormatMode::kSFloat, 32);
  l6.AddMember(FormatComponentType::kG, FormatMode::kSFloat, 32);
  EXPECT_FALSE(l1.Equal(&l6));
}

TEST_F(TypeTest, StructStride) {
  Struct s;
  EXPECT_FALSE(s.HasStride());
  EXPECT_EQ(0U, s.StrideInBytes());

  s.SetStrideInBytes(32);
  EXPECT_TRUE(s.HasStride());
  EXPECT_EQ(32U, s.StrideInBytes());
}

TEST_F(TypeTest, StructEqual) {
  Struct s1;
  Struct s2;

  auto num32 = Number::Float(32);
  auto m1 = s1.AddMember(num32.get());
  s2.AddMember(num32.get());
  EXPECT_TRUE(s1.Equal(&s2));

  s2.SetStrideInBytes(20);
  EXPECT_FALSE(s1.Equal(&s2));

  Struct s3;
  auto num16 = Number::Float(16);
  s3.AddMember(num16.get());
  EXPECT_FALSE(s1.Equal(&s3));

  Struct s4;
  auto m = s4.AddMember(num16.get());
  m->offset_in_bytes = 20;
  EXPECT_FALSE(s1.Equal(&s4));

  m->offset_in_bytes = m1->offset_in_bytes;
  m->array_stride_in_bytes = 20;
  EXPECT_FALSE(s1.Equal(&s4));

  m->array_stride_in_bytes = m1->array_stride_in_bytes;
  m->matrix_stride_in_bytes = 20;
  EXPECT_FALSE(s1.Equal(&s4));
}

TEST_F(TypeTest, NumberDefault32Bits) {
  EXPECT_EQ(4, Number(FormatMode::kUNorm).SizeInBytes());
}

TEST_F(TypeTest, NumberInBytes) {
  EXPECT_EQ(1, Number(FormatMode::kSInt, 8).SizeInBytes());
  EXPECT_EQ(2, Number(FormatMode::kSInt, 16).SizeInBytes());
  EXPECT_EQ(4, Number(FormatMode::kSInt, 32).SizeInBytes());
  EXPECT_EQ(8, Number(FormatMode::kSInt, 64).SizeInBytes());
}

TEST_F(TypeTest, IsInt) {
  EXPECT_TRUE(Type::IsInt(FormatMode::kSInt));
  EXPECT_TRUE(Type::IsInt(FormatMode::kSNorm));
  EXPECT_TRUE(Type::IsInt(FormatMode::kSScaled));
  EXPECT_TRUE(Type::IsInt(FormatMode::kSRGB));
  EXPECT_TRUE(Type::IsInt(FormatMode::kUNorm));
  EXPECT_TRUE(Type::IsInt(FormatMode::kUInt));
  EXPECT_TRUE(Type::IsInt(FormatMode::kUScaled));
  EXPECT_FALSE(Type::IsInt(FormatMode::kSFloat));
  EXPECT_FALSE(Type::IsInt(FormatMode::kUFloat));
}

TEST_F(TypeTest, IsSignedInt) {
  EXPECT_TRUE(Type::IsSignedInt(FormatMode::kSInt));
  EXPECT_TRUE(Type::IsSignedInt(FormatMode::kSNorm));
  EXPECT_TRUE(Type::IsSignedInt(FormatMode::kSScaled));
  EXPECT_FALSE(Type::IsSignedInt(FormatMode::kSRGB));
  EXPECT_FALSE(Type::IsSignedInt(FormatMode::kUNorm));
  EXPECT_FALSE(Type::IsSignedInt(FormatMode::kUInt));
  EXPECT_FALSE(Type::IsSignedInt(FormatMode::kUScaled));
  EXPECT_FALSE(Type::IsSignedInt(FormatMode::kSFloat));
  EXPECT_FALSE(Type::IsSignedInt(FormatMode::kUFloat));
}

TEST_F(TypeTest, IsUnsignedInt) {
  EXPECT_FALSE(Type::IsUnsignedInt(FormatMode::kSInt));
  EXPECT_FALSE(Type::IsUnsignedInt(FormatMode::kSNorm));
  EXPECT_FALSE(Type::IsUnsignedInt(FormatMode::kSScaled));
  EXPECT_TRUE(Type::IsUnsignedInt(FormatMode::kSRGB));
  EXPECT_TRUE(Type::IsUnsignedInt(FormatMode::kUNorm));
  EXPECT_TRUE(Type::IsUnsignedInt(FormatMode::kUInt));
  EXPECT_TRUE(Type::IsUnsignedInt(FormatMode::kUScaled));
  EXPECT_FALSE(Type::IsUnsignedInt(FormatMode::kSFloat));
  EXPECT_FALSE(Type::IsUnsignedInt(FormatMode::kUFloat));
}

TEST_F(TypeTest, IsFloat) {
  EXPECT_FALSE(Type::IsFloat(FormatMode::kSInt));
  EXPECT_FALSE(Type::IsFloat(FormatMode::kSNorm));
  EXPECT_FALSE(Type::IsFloat(FormatMode::kSScaled));
  EXPECT_FALSE(Type::IsFloat(FormatMode::kSRGB));
  EXPECT_FALSE(Type::IsFloat(FormatMode::kUNorm));
  EXPECT_FALSE(Type::IsFloat(FormatMode::kUInt));
  EXPECT_FALSE(Type::IsFloat(FormatMode::kUScaled));
  EXPECT_TRUE(Type::IsFloat(FormatMode::kSFloat));
  EXPECT_TRUE(Type::IsFloat(FormatMode::kUFloat));
}

TEST_F(TypeTest, IsInt8) {
  EXPECT_TRUE(Type::IsInt8(FormatMode::kSInt, 8));
  EXPECT_TRUE(Type::IsInt8(FormatMode::kSNorm, 8));
  EXPECT_TRUE(Type::IsInt8(FormatMode::kSScaled, 8));
  EXPECT_FALSE(Type::IsInt8(FormatMode::kSRGB, 8));
  EXPECT_FALSE(Type::IsInt8(FormatMode::kUNorm, 8));
  EXPECT_FALSE(Type::IsInt8(FormatMode::kUInt, 8));
  EXPECT_FALSE(Type::IsInt8(FormatMode::kUScaled, 8));
  EXPECT_FALSE(Type::IsInt8(FormatMode::kUFloat, 8));
  EXPECT_FALSE(Type::IsInt8(FormatMode::kSFloat, 8));
}

TEST_F(TypeTest, IsInt16) {
  EXPECT_TRUE(Type::IsInt16(FormatMode::kSInt, 16));
  EXPECT_TRUE(Type::IsInt16(FormatMode::kSNorm, 16));
  EXPECT_TRUE(Type::IsInt16(FormatMode::kSScaled, 16));
  EXPECT_FALSE(Type::IsInt16(FormatMode::kSRGB, 16));
  EXPECT_FALSE(Type::IsInt16(FormatMode::kUNorm, 16));
  EXPECT_FALSE(Type::IsInt16(FormatMode::kUInt, 16));
  EXPECT_FALSE(Type::IsInt16(FormatMode::kUScaled, 16));
  EXPECT_FALSE(Type::IsInt16(FormatMode::kUFloat, 16));
  EXPECT_FALSE(Type::IsInt16(FormatMode::kSFloat, 16));
}

TEST_F(TypeTest, IsInt32) {
  EXPECT_TRUE(Type::IsInt32(FormatMode::kSInt, 32));
  EXPECT_TRUE(Type::IsInt32(FormatMode::kSNorm, 32));
  EXPECT_TRUE(Type::IsInt32(FormatMode::kSScaled, 32));
  EXPECT_FALSE(Type::IsInt32(FormatMode::kSRGB, 32));
  EXPECT_FALSE(Type::IsInt32(FormatMode::kUNorm, 32));
  EXPECT_FALSE(Type::IsInt32(FormatMode::kUInt, 32));
  EXPECT_FALSE(Type::IsInt32(FormatMode::kUScaled, 32));
  EXPECT_FALSE(Type::IsInt32(FormatMode::kUFloat, 32));
  EXPECT_FALSE(Type::IsInt32(FormatMode::kSFloat, 32));
}

TEST_F(TypeTest, IsInt64) {
  EXPECT_TRUE(Type::IsInt64(FormatMode::kSInt, 64));
  EXPECT_TRUE(Type::IsInt64(FormatMode::kSNorm, 64));
  EXPECT_TRUE(Type::IsInt64(FormatMode::kSScaled, 64));
  EXPECT_FALSE(Type::IsInt64(FormatMode::kSRGB, 64));
  EXPECT_FALSE(Type::IsInt64(FormatMode::kUNorm, 64));
  EXPECT_FALSE(Type::IsInt64(FormatMode::kUInt, 64));
  EXPECT_FALSE(Type::IsInt64(FormatMode::kUScaled, 64));
  EXPECT_FALSE(Type::IsInt64(FormatMode::kUFloat, 64));
  EXPECT_FALSE(Type::IsInt64(FormatMode::kSFloat, 64));
}

TEST_F(TypeTest, IsUint8) {
  EXPECT_FALSE(Type::IsUint8(FormatMode::kSInt, 8));
  EXPECT_FALSE(Type::IsUint8(FormatMode::kSNorm, 8));
  EXPECT_FALSE(Type::IsUint8(FormatMode::kSScaled, 8));
  EXPECT_TRUE(Type::IsUint8(FormatMode::kSRGB, 8));
  EXPECT_TRUE(Type::IsUint8(FormatMode::kUNorm, 8));
  EXPECT_TRUE(Type::IsUint8(FormatMode::kUInt, 8));
  EXPECT_TRUE(Type::IsUint8(FormatMode::kUScaled, 8));
  EXPECT_FALSE(Type::IsUint8(FormatMode::kUFloat, 8));
  EXPECT_FALSE(Type::IsUint8(FormatMode::kSFloat, 8));
}

TEST_F(TypeTest, IsUint16) {
  EXPECT_FALSE(Type::IsUint16(FormatMode::kSInt, 16));
  EXPECT_FALSE(Type::IsUint16(FormatMode::kSNorm, 16));
  EXPECT_FALSE(Type::IsUint16(FormatMode::kSScaled, 16));
  EXPECT_TRUE(Type::IsUint16(FormatMode::kSRGB, 16));
  EXPECT_TRUE(Type::IsUint16(FormatMode::kUNorm, 16));
  EXPECT_TRUE(Type::IsUint16(FormatMode::kUInt, 16));
  EXPECT_TRUE(Type::IsUint16(FormatMode::kUScaled, 16));
  EXPECT_FALSE(Type::IsUint16(FormatMode::kUFloat, 16));
  EXPECT_FALSE(Type::IsUint16(FormatMode::kSFloat, 16));
}

TEST_F(TypeTest, IsUint32) {
  EXPECT_FALSE(Type::IsUint32(FormatMode::kSInt, 32));
  EXPECT_FALSE(Type::IsUint32(FormatMode::kSNorm, 32));
  EXPECT_FALSE(Type::IsUint32(FormatMode::kSScaled, 32));
  EXPECT_TRUE(Type::IsUint32(FormatMode::kSRGB, 32));
  EXPECT_TRUE(Type::IsUint32(FormatMode::kUNorm, 32));
  EXPECT_TRUE(Type::IsUint32(FormatMode::kUInt, 32));
  EXPECT_TRUE(Type::IsUint32(FormatMode::kUScaled, 32));
  EXPECT_FALSE(Type::IsUint32(FormatMode::kUFloat, 32));
  EXPECT_FALSE(Type::IsUint32(FormatMode::kSFloat, 32));
}

TEST_F(TypeTest, IsUint64) {
  EXPECT_FALSE(Type::IsUint64(FormatMode::kSInt, 64));
  EXPECT_FALSE(Type::IsUint64(FormatMode::kSNorm, 64));
  EXPECT_FALSE(Type::IsUint64(FormatMode::kSScaled, 64));
  EXPECT_TRUE(Type::IsUint64(FormatMode::kSRGB, 64));
  EXPECT_TRUE(Type::IsUint64(FormatMode::kUNorm, 64));
  EXPECT_TRUE(Type::IsUint64(FormatMode::kUInt, 64));
  EXPECT_TRUE(Type::IsUint64(FormatMode::kUScaled, 64));
  EXPECT_FALSE(Type::IsUint64(FormatMode::kUFloat, 64));
  EXPECT_FALSE(Type::IsUint64(FormatMode::kSFloat, 64));
}

TEST_F(TypeTest, IsFloat16) {
  EXPECT_TRUE(Type::IsFloat16(FormatMode::kSFloat, 16));
  EXPECT_TRUE(Type::IsFloat16(FormatMode::kUFloat, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kSInt, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kSNorm, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kSScaled, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kSRGB, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kUNorm, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kUInt, 16));
  EXPECT_FALSE(Type::IsFloat16(FormatMode::kUScaled, 16));
}

TEST_F(TypeTest, IsFloat32) {
  EXPECT_TRUE(Type::IsFloat32(FormatMode::kSFloat, 32));
  EXPECT_TRUE(Type::IsFloat32(FormatMode::kUFloat, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kSInt, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kSNorm, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kSScaled, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kSRGB, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kUNorm, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kUInt, 32));
  EXPECT_FALSE(Type::IsFloat32(FormatMode::kUScaled, 32));
}

TEST_F(TypeTest, IsFloat64) {
  EXPECT_TRUE(Type::IsFloat64(FormatMode::kSFloat, 64));
  EXPECT_TRUE(Type::IsFloat64(FormatMode::kUFloat, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kSInt, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kSNorm, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kSScaled, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kSRGB, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kUNorm, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kUInt, 64));
  EXPECT_FALSE(Type::IsFloat64(FormatMode::kUScaled, 64));
}

}  // namespace type
}  // namespace amber
