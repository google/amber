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

#include "src/vulkan/vertex_buffer.h"

#include <utility>

#include "amber/value.h"
#include "gtest/gtest.h"
#include "src/format.h"
#include "src/make_unique.h"
#include "src/vulkan/transfer_buffer.h"

namespace amber {
namespace vulkan {
namespace {

const VkPhysicalDeviceMemoryProperties kMemoryProperties =
    VkPhysicalDeviceMemoryProperties();

class BufferForTest : public TransferBuffer {
 public:
  BufferForTest(Device* device,
                uint32_t size_in_bytes,
                const VkPhysicalDeviceMemoryProperties& properties)
      : TransferBuffer(device, size_in_bytes, properties) {
    memory_.resize(4096);
    SetMemoryPtr(memory_.data());
  }
  ~BufferForTest() override = default;

  void CopyToDevice(CommandBuffer*) override {}

 private:
  std::vector<uint8_t> memory_;
};

class VertexBufferTest : public testing::Test {
 public:
  VertexBufferTest() {
    vertex_buffer_ = MakeUnique<VertexBuffer>(nullptr);

    std::unique_ptr<TransferBuffer> buffer =
        MakeUnique<BufferForTest>(nullptr, 0U, kMemoryProperties);
    buffer_memory_ = buffer->HostAccessibleMemoryPtr();
    vertex_buffer_->SetBufferForTest(std::move(buffer));
  }

  ~VertexBufferTest() = default;

  Result SetIntData(uint8_t location,
                    std::unique_ptr<Format>&& format,
                    std::vector<Value> values) {
    auto buffer = MakeUnique<FormatBuffer>();
    buffer->SetFormat(std::move(format));
    buffer->SetData(std::move(values));

    vertex_buffer_->SetData(location, buffer.get());
    return vertex_buffer_->SendVertexData(nullptr, kMemoryProperties);
  }

  Result SetDoubleData(uint8_t location,
                       std::unique_ptr<Format>&& format,
                       std::vector<Value> values) {
    auto buffer = MakeUnique<FormatBuffer>();
    buffer->SetFormat(std::move(format));
    buffer->SetData(std::move(values));

    vertex_buffer_->SetData(location, buffer.get());
    return vertex_buffer_->SendVertexData(nullptr, kMemoryProperties);
  }

  const void* GetVkBufferPtr() const { return buffer_memory_; }

 private:
  std::unique_ptr<VertexBuffer> vertex_buffer_;
  const void* buffer_memory_ = nullptr;
};

}  // namespace

void VertexBuffer::SetBufferForTest(std::unique_ptr<TransferBuffer> buffer) {
  transfer_buffer_ = std::move(buffer);
}

TEST_F(VertexBufferTest, R8G8B8A8_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 8);
  format->AddComponent(FormatComponentType::kG, FormatMode::kUInt, 8);
  format->AddComponent(FormatComponentType::kB, FormatMode::kUInt, 8);
  format->AddComponent(FormatComponentType::kA, FormatMode::kUInt, 8);

  Result r = SetIntData(0, std::move(format), values);
  const uint8_t* ptr = static_cast<const uint8_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R16G16B16A16_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 16);
  format->AddComponent(FormatComponentType::kG, FormatMode::kUInt, 16);
  format->AddComponent(FormatComponentType::kB, FormatMode::kUInt, 16);
  format->AddComponent(FormatComponentType::kA, FormatMode::kUInt, 16);

  Result r = SetIntData(0, std::move(format), values);
  const uint16_t* ptr = static_cast<const uint16_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R32G32B32A32_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 32);
  format->AddComponent(FormatComponentType::kG, FormatMode::kUInt, 32);
  format->AddComponent(FormatComponentType::kB, FormatMode::kUInt, 32);
  format->AddComponent(FormatComponentType::kA, FormatMode::kUInt, 32);

  Result r = SetIntData(0, std::move(format), values);
  const uint32_t* ptr = static_cast<const uint32_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R64G64B64A64_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 64);
  format->AddComponent(FormatComponentType::kG, FormatMode::kUInt, 64);
  format->AddComponent(FormatComponentType::kB, FormatMode::kUInt, 64);
  format->AddComponent(FormatComponentType::kA, FormatMode::kUInt, 64);

  Result r = SetIntData(0, std::move(format), values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R8G8B8A8_SNORM) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-128));
  values[3].SetIntValue(127);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSNorm, 8);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSNorm, 8);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSNorm, 8);
  format->AddComponent(FormatComponentType::kA, FormatMode::kSNorm, 8);

  Result r = SetIntData(0, std::move(format), values);
  const int8_t* ptr = static_cast<const int8_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-128, ptr[2]);
  EXPECT_EQ(127, ptr[3]);
}

TEST_F(VertexBufferTest, R16G16B16A16_SNORM) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-27));
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSNorm, 16);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSNorm, 16);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSNorm, 16);
  format->AddComponent(FormatComponentType::kA, FormatMode::kSNorm, 16);

  Result r = SetIntData(0, std::move(format), values);
  const int16_t* ptr = static_cast<const int16_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R32G32B32A32_SINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-27));
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSInt, 32);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSInt, 32);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSInt, 32);
  format->AddComponent(FormatComponentType::kA, FormatMode::kSInt, 32);

  Result r = SetIntData(0, std::move(format), values);
  const int32_t* ptr = static_cast<const int32_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R64G64B64A64_SINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-27));
  values[3].SetIntValue(255);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSInt, 64);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSInt, 64);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSInt, 64);
  format->AddComponent(FormatComponentType::kA, FormatMode::kSInt, 64);

  Result r = SetIntData(0, std::move(format), values);
  const int64_t* ptr = static_cast<const int64_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R16G11B10_SFLOAT) {
  std::vector<Value> values(3);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  uint64_t expected = 50688ULL;
  values[0].SetDoubleValue(-6.0);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  expected |= 1200ULL << 16ULL;
  values[1].SetDoubleValue(14.0);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  expected |= 380ULL << (16ULL + 11ULL);
  values[2].SetDoubleValue(0.1171875);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 16);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 11);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 10);

  Result r = SetDoubleData(0, std::move(format), values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(expected, *ptr);
}

TEST_F(VertexBufferTest, R10G16B11_SFLOAT) {
  std::vector<Value> values(3);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  uint64_t expected = 380ULL;
  values[0].SetDoubleValue(0.1171875);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  expected |= 50688ULL << 10ULL;
  values[1].SetDoubleValue(-6.0);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  expected |= 1200ULL << (16ULL + 10ULL);
  values[2].SetDoubleValue(14.0);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 10);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 16);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 11);

  Result r = SetDoubleData(0, std::move(format), values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(expected, *ptr);
}

TEST_F(VertexBufferTest, R11G16B10_SFLOAT) {
  std::vector<Value> values(3);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  uint64_t expected = 1200ULL;
  values[0].SetDoubleValue(14.0);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  expected |= 50688ULL << 11ULL;
  values[1].SetDoubleValue(-6.0);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  expected |= 380ULL << (16ULL + 11ULL);
  values[2].SetDoubleValue(0.1171875);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 11);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 16);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 10);

  Result r = SetDoubleData(0, std::move(format), values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(expected, *ptr);
}

TEST_F(VertexBufferTest, R32G32B32_SFLOAT) {
  std::vector<Value> values(3);
  values[0].SetDoubleValue(-6.0);
  values[1].SetDoubleValue(14.0);
  values[2].SetDoubleValue(0.1171875);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 32);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 32);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 32);

  Result r = SetDoubleData(0, std::move(format), values);
  const float* ptr = static_cast<const float*>(GetVkBufferPtr());
  EXPECT_FLOAT_EQ(-6.0f, ptr[0]);
  EXPECT_FLOAT_EQ(14.0f, ptr[1]);
  EXPECT_FLOAT_EQ(0.1171875f, ptr[2]);
}

TEST_F(VertexBufferTest, R64G64B64_SFLOAT) {
  std::vector<Value> values(3);
  values[0].SetDoubleValue(-6.0);
  values[1].SetDoubleValue(14.0);
  values[2].SetDoubleValue(0.1171875);

  auto format = MakeUnique<Format>();
  format->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 64);
  format->AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 64);
  format->AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 64);

  Result r = SetDoubleData(0, std::move(format), values);
  const double* ptr = static_cast<const double*>(GetVkBufferPtr());
  EXPECT_DOUBLE_EQ(-6.0, ptr[0]);
  EXPECT_DOUBLE_EQ(14.0, ptr[1]);
  EXPECT_DOUBLE_EQ(0.1171875, ptr[2]);
}

}  // namespace vulkan
}  // namespace amber
