#!/bin/bash
# Copyright 2026 The Amber Authors.
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

# Fail on any error.
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SCRIPT_DIR}/../.." >/dev/null 2>&1 && pwd )"

BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}

set +e
# Allow build failures

# "--privileged" is required to run ptrace in the asan builds.
docker run --rm -i \
  --privileged \
  --volume "${ROOT_DIR}:${ROOT_DIR}" \
  --volume "${KOKORO_ARTIFACTS_DIR}:${KOKORO_ARTIFACTS_DIR}" \
  --workdir "${ROOT_DIR}" \
  --env SCRIPT_DIR=${SCRIPT_DIR} \
  --env ROOT_DIR=${ROOT_DIR} \
  --env KOKORO_ARTIFACTS_DIR="${KOKORO_ARTIFACTS_DIR}" \
  --env BUILD_SHA="${BUILD_SHA}" \
  --entrypoint "${SCRIPT_DIR}/build-docker.sh" \
  us-east4-docker.pkg.dev/shaderc-build/radial-docker/ubuntu-24.04-amd64/cpp-builder
RESULT=$?

exit $RESULT
