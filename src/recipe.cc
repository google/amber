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

#include "amber/recipe.h"

namespace amber {

RecipeImpl::RecipeImpl() = default;

RecipeImpl::~RecipeImpl() = default;

Recipe::Recipe() : impl_(nullptr) {}

Recipe::~Recipe() {
  delete impl_;
}

std::vector<ShaderInfo> Recipe::GetShaderInfo() const {
  if (!impl_)
    return {};
  return impl_->GetShaderInfo();
}

std::vector<std::string> Recipe::GetRequiredFeatures() const {
  return impl_ ? impl_->GetRequiredFeatures() : std::vector<std::string>();
}

std::vector<std::string> Recipe::GetRequiredDeviceExtensions() const {
  return impl_ ? impl_->GetRequiredDeviceExtensions()
               : std::vector<std::string>();
}

std::vector<std::string> Recipe::GetRequiredInstanceExtensions() const {
  return impl_ ? impl_->GetRequiredInstanceExtensions()
               : std::vector<std::string>();
}

void Recipe::SetFenceTimeout(uint32_t timeout_ms) {
  if (impl_)
    impl_->SetFenceTimeout(timeout_ms);
}

}  // namespace amber
