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

#include "src/dxcutil/dxc_util.h"

#include <algorithm>
#include <sstream>

// clang-format off
// Do not change the order of header file inclusion here, which might
// result in compiler errors.
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/dxcapi.use.h"
// clang-format on

namespace amber {
namespace {

// Converts an IDxcBlob into a vector of 32-bit unsigned integers which
// is returned via the 'binaryWords' argument.
void ConvertIDxcBlobToUint32(const CComPtr<IDxcBlob> &blob,
                             std::vector<uint32_t> *binaryWords) {
  size_t num32BitWords = (blob->GetBufferSize() + 3) / 4;
  std::string binaryStr((char *)blob->GetBufferPointer(),
                        blob->GetBufferSize());
  binaryStr.resize(num32BitWords * 4, 0);
  binaryWords->resize(num32BitWords, 0);
  memcpy(binaryWords->data(), binaryStr.data(), binaryStr.size());
}

}  // namespace

// Parses the target profile and entry point from the run command
// returns the target profile, entry point, and the rest via arguments.
// Returns true on success, and false otherwise.
Result ProcessRunCommandArgs(const std::string& cmd,
                           std::string *target, std::string *entry,
                           std::vector<std::string> *rest_args) {
  std::istringstream buf(cmd);
  std::istream_iterator<std::string> start(buf), end;
  std::vector<std::string> tokens(start, end);

  for (uint32_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == "-T" && (++i) < tokens.size())
      *target = tokens[i];
    else if (tokens[i] == "-E" && (++i) < tokens.size())
      *entry = tokens[i];
    else
      rest_args->push_back(tokens[i]);
  }

  if (target->empty()) {
    return Result("ProcessRunCommandArgs: Missing target profile argument (-T)");
  }
  // lib_6_* profile doesn't need an entryPoint
  if (target->c_str()[0] != 'l' && entry->empty()) {
    return Result("ProcessRunCommandArgs: Missing entry point argument (-E)");
  }
  return {};
}

Result RunDXC(const std::string& src,
                                    const std::string& entry_str,
                                    const std::string& profile_str,
                                    const std::vector<std::string> &rest_args,
                                    std::vector<uint32_t> *generated_binary) {
  std::wstring entry(entry_str.begin(), entry_str.end());
  std::wstring profile(profile_str.begin(), profile_str.end());
  std::vector<std::wstring> rest;
  for (const auto &arg : rest_args)
    rest.emplace_back(arg.begin(), arg.end());

  bool success = true;

  dxc::DxcDllSupport dll_support;
  auto result = dll_support.Initialize();
  if (result < 0)
    return Result("DXC compile failure: dll_support.Initialize() " + std::to_string(result));

  DxcInitThreadMalloc();

  if (hlsl::options::initHlslOptTable()) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: initHlslOptTable");
  }

  CComPtr<IDxcLibrary> p_library;
  CComPtr<IDxcCompiler> p_compiler;
  CComPtr<IDxcOperationResult> p_result;
  CComPtr<IDxcBlobEncoding> p_source;
  CComPtr<IDxcBlobEncoding> p_error_buffer;
  CComPtr<IDxcBlob> pCompiledBlob;
  CComPtr<IDxcIncludeHandler> pIncludeHandler;
  HRESULT resultStatus;

  std::vector<LPCWSTR> flags;
  // lib_6_* profile doesn't need an entryPoint
  if (profile.c_str()[0] != 'l') {
    flags.push_back(L"-E");
    flags.push_back(entry.c_str());
  }
  flags.push_back(L"-T");
  flags.push_back(profile.c_str());
  flags.push_back(L"-spirv");
  // Disable validation. We'll run it manually.
  flags.push_back(L"-Vd");
  for (const auto &arg : rest)
    flags.push_back(arg.c_str());

  if (dll_support.CreateInstance(CLSID_DxcLibrary, &p_library) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateInstance");
  }

  if (p_library->CreateBlobWithEncodingOnHeapCopy(src.data(),
              static_cast<uint32_t>(src.size()), CP_UTF8, &p_source) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateBlobFromFile");
  }

  if (p_library->CreateIncludeHandler(&pIncludeHandler) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateIncludeHandler");
  }

  if (dll_support.CreateInstance(CLSID_DxcCompiler, &p_compiler) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: CreateInstance");
  }

  if (p_compiler->Compile(p_source, (L"amber." + profile).c_str(), entry.c_str(),
                         profile.c_str(), flags.data(), static_cast<uint32_t>(flags.size()), nullptr,
                         0, pIncludeHandler, &p_result) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: Compile");
  }

  // Compilation is done. We can clean up the HlslOptTable.
  hlsl::options::cleanupHlslOptTable();

  // Get compilation results.
  if (p_result->GetStatus(&resultStatus) < 0) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: GetStatus");
  }

  // Get diagnostics string.
  if (p_result->GetErrorBuffer(&p_error_buffer)) {
    DxcCleanupThreadMalloc();
    return Result("DXC compile failure: GetErrorBuffer");
  }

  const std::string diagnostics((char *)p_error_buffer->GetBufferPointer(),
                                p_error_buffer->GetBufferSize());

  if (SUCCEEDED(resultStatus)) {
    CComPtr<IDxcBlobEncoding> pStdErr;
    if (p_result->GetResult(&pCompiledBlob) < 0) {
      DxcCleanupThreadMalloc();
      return Result("DXC compile failure: GetResult");
    }
    ConvertIDxcBlobToUint32(pCompiledBlob, generated_binary);
    success = true;
  } else {
    success = false;
  }

  DxcCleanupThreadMalloc();
  return success ? Result() : Result("DXC compile failure: " + diagnostics);
}

}  // namespace amber
