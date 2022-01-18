:: Copyright The Amber Authors.
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

@echo on

set BUILD_ROOT=%cd%
set SRC=%cd%\github\amber
set BUILD_TYPE=%1

choco install cmake --yes --no-progress --limit-output
choco upgrade cmake --yes --no-progress --limit-output

:: Force usage of python 3.6 and add cmake to the path.
set PATH=C:\python36;"C:\Program Files\CMake\bin";%PATH%

cd %SRC%
python tools\git-sync-deps

:: #########################################
:: set up msvc build env
:: #########################################
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo "Using VS 2017..."

cmake --version

cd %SRC%
mkdir build
cd build

:: #########################################
:: Install prerequisites for local Vulkan build.
:: #########################################
echo "Build and install SPIR-V headers. %DATE% %TIME%"
mkdir -p spirv-headers
cd spirv-headers
cmake -GNinja -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe ..\..\third_party\spirv-headers\
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
cd ..
echo "Build and install SPIR-V tools. %DATE% %TIME%"
mkdir -p spirv-tools
cd spirv-tools
cmake -GNinja -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe -DSPIRV-Headers_SOURCE_DIR=%SRC%\third_party\spirv-headers -DSPIRV_SKIP_TESTS=ON ..\..\third_party\spirv-tools
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
cd ..

:: #########################################
:: Start building.
:: #########################################
echo "Starting build... %DATE% %TIME%"
if "%KOKORO_GITHUB_COMMIT%." == "." (
  set BUILD_SHA=%KOKORO_GITHUB_PULL_REQUEST_COMMIT%
) else (
  set BUILD_SHA=%KOKORO_GITHUB_COMMIT%
)

cmake -GNinja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe -DAMBER_USE_LOCAL_VULKAN=1 ..
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%

echo "Build everything... %DATE% %TIME%"
ninja
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
echo "Build Completed %DATE% %TIME%"

:: ################################################
:: Run the tests (We no longer run tests on VS2013)
:: ################################################
echo "Running Tests... %DATE% %TIME%"
amber_unittests
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
echo "Tests Completed %DATE% %TIME%"

exit /b %ERRORLEVEL%
