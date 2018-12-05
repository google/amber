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

#ifndef AMBER_RECIPE_H_
#define AMBER_RECIPE_H_

#include <memory>
#include <utility>
#include <vector>

#include "amber/shader_info.h"

namespace amber {

/// Internal recipe implementation.
class RecipeImpl {
 public:
  virtual ~RecipeImpl();

  /// Retrieves information on all the shaders in the given recipe.
  virtual std::vector<ShaderInfo> GetShaderInfo() const = 0;

 protected:
  RecipeImpl();
};

/// A recipe is the parsed representation of the input script.
class Recipe {
 public:
  Recipe();
  ~Recipe();

  /// Retrieves information on all the shaders in the recipe.
  std::vector<ShaderInfo> GetShaderInfo() const;

  RecipeImpl* GetImpl() const { return impl_.get(); }
  void SetImpl(std::unique_ptr<RecipeImpl> impl) { impl_ = std::move(impl); }

 private:
  std::unique_ptr<RecipeImpl> impl_;
};

}  // namespace amber

#endif  // AMBER_RECIPE_H_
