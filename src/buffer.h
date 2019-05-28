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

#ifndef SRC_BUFFER_H_
#define SRC_BUFFER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "src/datum_type.h"
#include "src/format.h"

namespace amber {

/// Types of buffers which can be created.
enum class BufferType : int8_t {
  /// Unknown buffer type
  kUnknown = -1,
  /// A color buffer.
  kColor = 0,
  /// A depth/stencil buffer.
  kDepth,
  /// An index buffer.
  kIndex,
  /// A sampled buffer.
  kSampled,
  /// A storage buffer.
  kStorage,
  /// A uniform buffer.
  kUniform,
  /// A push constant buffer.
  kPushConstant,
  /// A vertex buffer.
  kVertex
};

/// A buffer stores data. The buffer maybe provided from the input script, or
/// maybe created as needed. A buffer must have a unique name.
class Buffer {
 public:
  /// Create a buffer of unknown type.
  Buffer();
  /// Create a buffer of |type_|.
  explicit Buffer(BufferType type);

  ~Buffer();

  /// Returns the BufferType of this buffer.
  BufferType GetBufferType() const { return buffer_type_; }
  /// Sets the BufferType for this buffer.
  void SetBufferType(BufferType type) { buffer_type_ = type; }

  /// Sets the Format of the buffer to |format|.
  void SetFormat(std::unique_ptr<Format> format) {
    format_is_default_ = false;
    format_ = std::move(format);
  }
  /// Returns the Format describing the buffer data.
  Format* GetFormat() const { return format_.get(); }

  void SetFormatIsDefault(bool val) { format_is_default_ = val; }
  bool FormatIsDefault() const { return format_is_default_; }

  /// Sets the buffer |name|.
  void SetName(const std::string& name) { name_ = name; }
  /// Returns the name of the buffer.
  std::string GetName() const { return name_; }

  /// Gets the number of elements this buffer is wide.
  uint32_t GetWidth() const { return width_; }
  /// Set the number of elements wide for the buffer.
  void SetWidth(uint32_t width) { width_ = width; }
  /// Get the number of elements this buffer is high.
  uint32_t GetHeight() const { return height_; }
  /// Set the number of elements high for the buffer.
  void SetHeight(uint32_t height) { height_ = height; }

  // | ---------- Element ---------- | ElementCount == 1
  // | Value | Value | Value | Value |   ValueCount == 4
  // | | | | | | | | | | | | | | | | |  SizeInBytes == 16
  // Note, the SizeInBytes maybe be greater then the size of the values. If
  // the format IsStd140() and there are 3 rows, the SizeInBytes will be
  // inflated to 4 values per row, instead of 3.

  /// Sets the number of elements in the buffer.
  void SetElementCount(uint32_t count) { element_count_ = count; }
  /// Returns the number of elements in the buffer.
  uint32_t ElementCount() const { return element_count_; }

  /// Sets the number of values in the buffer.
  void SetValueCount(uint32_t count) {
    if (!format_) {
      element_count_ = 0;
      return;
    }
    if (format_->GetPackSize() > 0) {
      element_count_ = count;
    } else {
      // This divides by the needed input values, not the values per element.
      // The assumption being the values coming in are read from the input,
      // where components are specified. The needed values maybe less then the
      // values per element.
      element_count_ = count / format_->InputNeededPerElement();
    }
  }
  /// Returns the number of values in the buffer.
  uint32_t ValueCount() const {
    if (!format_)
      return 0;
    // Packed formats are single values.
    if (format_->GetPackSize() > 0)
      return element_count_;
    return element_count_ * format_->ValuesPerElement();
  }

  /// Returns the number of bytes needed for the data in the buffer.
  uint32_t GetSizeInBytes() const {
    if (!format_)
      return 0;
    return ElementCount() * format_->SizeInBytes();
  }

  /// Returns the number of bytes for one element in the buffer.
  uint32_t GetTexelStride() { return format_->SizeInBytes(); }

  /// Returns the number of bytes for one row of elements in the buffer.
  uint32_t GetRowStride() { return GetTexelStride() * GetWidth(); }

  /// Sets the data into the buffer.
  Result SetData(const std::vector<Value>& data);

  /// Resizes the buffer to hold |element_count| elements. This is separate
  /// from SetElementCount() because we may not know the format when we set the
  /// initial count. This requires the format to have been set.
  void ResizeTo(uint32_t element_count);

  /// Write |data| into the buffer |offset| bytes from the start. Write
  /// |size_in_bytes| of data.
  Result SetDataWithOffset(const std::vector<Value>& data, uint32_t offset);

  /// Writes |src| data into buffer at |offset|.
  Result SetDataFromBuffer(const Buffer* src, uint32_t offset);

  /// Returns a pointer to the internal storage of the buffer.
  std::vector<uint8_t>* ValuePtr() { return &bytes_; }
  /// Returns a pointer to the internal storage of the buffer.
  const std::vector<uint8_t>* ValuePtr() const { return &bytes_; }

  /// Returns a casted pointer to the internal storage of the buffer.
  template <typename T>
  const T* GetValues() const {
    return reinterpret_cast<const T*>(bytes_.data());
  }

  /// Copies the buffer values to an other one
  Result CopyTo(Buffer* buffer) const;

  /// Succeeds only if both buffer contents are equal
  Result IsEqual(Buffer* buffer) const;

 private:
  uint32_t WriteValueFromComponent(const Value& value,
                                   const Format::Component& comp,
                                   uint8_t* ptr);

  BufferType buffer_type_ = BufferType::kUnknown;
  std::string name_;
  uint32_t element_count_ = 0;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  bool format_is_default_ = false;
  std::vector<uint8_t> bytes_;
  std::unique_ptr<Format> format_;
};

}  // namespace amber

#endif  // SRC_BUFFER_H_
