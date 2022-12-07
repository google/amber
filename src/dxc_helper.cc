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
#include "src/virtual_file_store.h"

#if AMBER_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4003)
#endif  // AMBER_PLATFORM_WINDOWS

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif  // __STDC_LIMIT_MACROS
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif  // __STDC_CONSTANT_MACROS

// clang-format off
// The order here matters, so don't reformat.
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/dxcapi.h"
#include "dxc/Support/microcom.h"
// clang-format on

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

class IncludeHandler : public IDxcIncludeHandler {
 public:
  IncludeHandler(const VirtualFileStore* file_store,
                 IDxcLibrary* dxc_lib,
                 IDxcIncludeHandler* fallback)
      : file_store_(file_store), dxc_lib_(dxc_lib), fallback_(fallback) {}

  HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename,
                                       IDxcBlob** ppIncludeSource) override {
    std::wstring wide_path(pFilename);
    std::string path = std::string(wide_path.begin(), wide_path.end());

    std::string content;
    Result r = file_store_->Get(path, &content);
    if (r.IsSuccess()) {
      IDxcBlobEncoding* source;
      auto res = dxc_lib_->CreateBlobWithEncodingOnHeapCopy(
          content.data(), static_cast<uint32_t>(content.size()), CP_UTF8,
          &source);
      if (res != S_OK) {
        DxcCleanupThreadMalloc();
        return res;
      }
      *ppIncludeSource = source;
      return S_OK;
    }

    return fallback_->LoadSource(pFilename, ppIncludeSource);
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void** ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

 private:
  const VirtualFileStore* const file_store_;
  IDxcLibrary* const dxc_lib_;
  IDxcIncludeHandler* const fallback_;
};

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
#if AMBER_PLATFORM_WINDOWS
#pragma warning(pop)
#endif  // AMBER_PLATFORM_WINDOWS

}  // namespace

Result Compile(const std::string& src,
               const std::string& entry,
               const std::string& profile,
               const std::string& spv_env,
               const std::string& filename,
               const VirtualFileStore* virtual_files,
               std::vector<uint32_t>* generated_binary) {
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

  IDxcIncludeHandler* fallback_include_handler;
  if (dxc_lib->CreateIncludeHandler(&fallback_include_handler) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateIncludeHandler");
  }

  CComPtr<IDxcIncludeHandler> include_handler(
      new IncludeHandler(virtual_files, dxc_lib, fallback_include_handler));

  IDxcCompiler* compiler;
  if (DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler),
                        reinterpret_cast<void**>(&compiler)) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXCCreateInstance for DXCCompiler failed");
  }

  std::string filepath = filename.empty() ? ("amber." + profile) : filename;

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

  IDxcOperationResult* result;
  if (compiler->Compile(
          source, /* source text */
          std::wstring(filepath.begin(), filepath.end())
              .c_str(), /* original file source */
          std::wstring(entry.begin(), entry.end())
              .c_str(), /* entry point name */
          std::wstring(profile.begin(), profile.end())
              .c_str(),     /* shader profile to compile */
          dxc_flags.data(), /* arguments */
          static_cast<uint32_t>(dxc_flags.size()), /* argument count */
          nullptr,                                 /* defines */
          0,                                       /* define count */
          include_handler,                         /* handler for #include */
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
  if (static_cast<HRESULT>(result_status) >= 0) {
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
