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

#include "src/dxc_helper.h"

#include <algorithm>
#include <sstream>

#include "src/platform.h"

#if AMBER_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4003)
#endif  // AMBER_PLATFORM_WINDOWS

// clang-format off
// The order here matters, so don't reformat.
#include "dxc/Support/WinAdapter.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/dxcapi.h"
// clang-format on

#if AMBER_PLATFORM_WINDOWS
#pragma warning(pop)
#endif  // AMBER_PLATFORM_WINDOWS

namespace amber {
namespace dxchelper {
namespace {

const wchar_t* kDxcFlags[] = {
    L"-spirv",               // SPIR-V compilation
    L"-fcgl",                // No SPIR-V Optimization
    L"-enable-16bit-types",  // Enabling 16bit types
};
const size_t kDxcFlagsCount = sizeof(kDxcFlags) / sizeof(const wchar_t*);

// Converts an IDxcBlob into a vector of 32-bit unsigned integers which
// is returned via the 'binaryWords' argument.
void ConvertIDxcBlobToUint32(IDxcBlob* blob,
                             std::vector<uint32_t>* binaryWords) {
  size_t num32BitWords = (blob->GetBufferSize() + 3) / 4;
  std::string binaryStr(static_cast<char*>(blob->GetBufferPointer()),
                        blob->GetBufferSize());
  binaryStr.resize(num32BitWords * 4, 0);
  binaryWords->resize(num32BitWords, 0);
  memcpy(binaryWords->data(), binaryStr.data(), binaryStr.size());
}

}  // namespace

Result Compile(const std::string& src,
               const std::string& entry,
               const std::string& profile,
               const std::string& spv_env,
               std::vector<uint32_t>* generated_binary) {
  DxcInitThreadMalloc();

  if (hlsl::options::initHlslOptTable()) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: initHlslOptTable");
  }

  IDxcLibrary* dxc_lib;
  if (DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary),
                        reinterpret_cast<void**>(&dxc_lib)) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXCCreateInstance for DXCLibrary failed");
  }

  IDxcBlobEncoding* source;
  if (dxc_lib->CreateBlobWithEncodingOnHeapCopy(
          src.data(), static_cast<uint32_t>(src.size()), CP_UTF8, &source) <
      0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateBlobFromFile");
  }

  IDxcIncludeHandler* include_handler;
  if (dxc_lib->CreateIncludeHandler(&include_handler) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateIncludeHandler");
  }

  IDxcCompiler* compiler;
  if (DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler),
                        reinterpret_cast<void**>(&compiler)) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXCCreateInstance for DXCCompiler failed");
  }

  IDxcOperationResult* result;
  std::wstring src_filename =
      L"amber." + std::wstring(profile.begin(), profile.end());

  std::vector<const wchar_t*> dxc_flags(kDxcFlags, &kDxcFlags[kDxcFlagsCount]);
  const wchar_t* target_env = nullptr;
  if (!spv_env.compare("spv1.3") || !spv_env.compare("vulkan1.1")) {
    target_env = L"-fspv-target-env=vulkan1.1";
  } else if (!spv_env.compare("spv1.0") || !spv_env.compare("vulkan1.0")) {
    target_env = L"-fspv-target-env=vulkan1.0";
  } else if (!spv_env.empty()) {
    return Result(
        "Invalid target environment. Choose spv1.3 or vulkan1.1 for vulkan1.1 "
        "and spv1.0 or vulkan1.0 for vulkan1.0.");
  }
  if (target_env)
    dxc_flags.push_back(target_env);

  if (compiler->Compile(source,               /* source text */
                        src_filename.c_str(), /* original file source */
                        std::wstring(entry.begin(), entry.end())
                            .c_str(), /* entry point name */
                        std::wstring(profile.begin(), profile.end())
                            .c_str(),     /* shader profile to compile */
                        dxc_flags.data(), /* arguments */
                        dxc_flags.size(), /* argument count */
                        nullptr,          /* defines */
                        0,                /* define count */
                        include_handler,  /* handler for #include */
                        &result /* output status */) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: Compile");
  }

  // Compilation is done. We can clean up the HlslOptTable.
  hlsl::options::cleanupHlslOptTable();

  // Get compilation results.
  HRESULT result_status;
  if (result->GetStatus(&result_status) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: GetStatus");
  }

  // Get diagnostics string.
  IDxcBlobEncoding* error_buffer;
  if (result->GetErrorBuffer(&error_buffer)) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: GetErrorBuffer");
  }

  const std::string diagnostics(
      static_cast<char*>(error_buffer->GetBufferPointer()),
      error_buffer->GetBufferSize());

  bool success = true;
  if (SUCCEEDED(result_status)) {
    IDxcBlob* compiled_blob;
    if (result->GetResult(&compiled_blob) < 0) {
      DxcCleanupThreadMalloc();
      return Result("DXC compile failure: GetResult");
    }
    ConvertIDxcBlobToUint32(compiled_blob, generated_binary);
  } else {
    success = false;
  }

  DxcCleanupThreadMalloc();
  return success ? Result() : Result("DXC compile failure: " + diagnostics);
}

}  // namespace dxchelper
}  // namespace amber
