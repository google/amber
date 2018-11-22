#!/bin/bash
# Copyright 2018 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e  # fail on error
set -x  # display commands

BUILD_ROOT=$PWD
SRC=$PWD/github/amber

cd $SRC
./tools/git-sync-deps

echo $(date): Check copyright...
./tools/copyright.py --check;
echo $(date): check completed.

echo $(date): Linting...
./tools/run-lint.sh;
echo $(date): check complete.

# Check format adds a bunch of stuff to the repo so do that check last.

# Get clang-format-5.0.0.
# Once kokoro upgrades the Ubuntu VMs, we can use 'apt-get install clang-format'
curl -L http://releases.llvm.org/5.0.0/clang+llvm-5.0.0-linux-x86_64-ubuntu14.04.tar.xz -o clang-llvm.tar.xz
tar xf clang-llvm.tar.xz
export PATH=$PWD/clang+llvm-5.0.0-linux-x86_64-ubuntu14.04/bin:$PATH

cd $SRC
curl -L http://llvm.org/svn/llvm-project/cfe/trunk/tools/clang-format/clang-format-diff.py -o tools/clang-format-diff.py;

echo $(date): Check formatting...
./tools/check_code_format.sh;
echo $(date): check completed.
