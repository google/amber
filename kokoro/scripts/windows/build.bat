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

set PATH=C:\python36;%PATH%

cd %SRC%
python tools\git-sync-deps

choco install cmake --pre --yes --no-progress
choco upgrade cmake --pre --yes --no-progress

:: Force usage of python 3.6 and add cmake to the path.
set PATH=C:\python36;"C:\Program Files\CMake\bin";%PATH%

:: #########################################
:: set up msvc build env
:: #########################################
::call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
::echo "Using VS 2017..."
set GENERATOR="Visual Studio 15 2017 Win64"
echo "Using VS 2017..."

cd %SRC%
mkdir build
cd build

:: Need WDK at least 1803 for Loader build
::wget -o wdksetup.exe https://go.microsoft.com/fwlink/?linkid=2085767
::.\wdksetup.exe /quiet /norestart

:: #########################################
:: Start building.
:: #########################################
echo "Starting build... %DATE% %TIME%"
if "%KOKORO_GITHUB_COMMIT%." == "." (
  set BUILD_SHA=%KOKORO_GITHUB_PULL_REQUEST_COMMIT%
) else (
  set BUILD_SHA=%KOKORO_GITHUB_COMMIT%
)

cmake -A x64 -G%GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DAMBER_USE_LOCAL_VULKAN=1 ..
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%

echo "Build everything... %DATE% %TIME%"
cmake --build . --config %BUILD_TYPE%
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
echo "Build Completed %DATE% %TIME%"

:: ################################################
:: Run the tests
:: ################################################
echo "Running Tests... %DATE% %TIME%"
amber_unittests
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
echo "Tests Completed %DATE% %TIME%"

exit /b %ERRORLEVEL%
