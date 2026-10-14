# *******************************************************************************
# Copyright (c) 2026 ZF Friedrichshafen AG
#
# See the NOTICE file(s) distributed with this work for additional
# information regarding copyright ownership.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# 
# Contributors:
#   Thomas Pfleiderer - initial function designer
# *******************************************************************************
#
# AutoAPI Function Designer

wxWidgets desktop application for reading, inspecting and writing Eclipse
autoapiframework function specifications (`*.afs`, YAML content).

## What it does

* **Read .afs file** – opens a function specification, parses it and builds
  the in-memory structures (`FunctionSpecification`, order preserving `YamlNode`
  tree).
* Displays the content on four tabs:
  * **Attributes** – `name`, `version`, `metaModelRef.*`, collection sizes and
    the full `description`.
  * **Signal collection** – entries of `dataInterfaces`, columns taken from the
    `Data` interface type of the meta model.
  * **Parameter collection** – entries of `parameters`, columns from the
    `Parameter` interface type.
  * **Scheduling collection** – entries of `scheduling`, columns from the
    `Scheduling` interface type.
  Keys that are present in the file but not in the meta model are appended as
  extra columns, so nothing is hidden.
* **Write .afs file as...** – writes the parsed content back under a new
  name in the same block style layout (key order, folded `>-` descriptions and
  quoting are preserved).

`../autoapiframework_meta_model.yaml` is located automatically on startup
(relative to the executable and the working directory); it can also be selected
manually via *Load meta model...*.

## Build

Requires CMake >= 3.16 (>= 3.21 for the presets), a C++17 compiler,
wxWidgets, and Python. The build requires `vss-tools` version 6.1.0 because
the application uses its `vspec` executable to convert VSS files to JSON.

Install the Python package in a virtual environment before configuring CMake:

```powershell
python -m venv venv
.\venv\Scripts\Activate.ps1
python -m pip install vss-tools==6.1.0
```

On Linux, activate the environment with `source venv/bin/activate` instead.
Keep the environment activated while configuring and building so CMake can
find `vspec`.

### VS Code (CMake Tools extension)

`cmake.sourceDirectory` in `.vscode/settings.json` already points to this
folder, so *CMake: Select Configure Preset* -> *Configure* -> *Build* -> *Run*
works from the repository window. Available configure presets are
`windows-gcc-debug`, `windows-gcc-release`, `msvc_x64_x64_ninja_debug`,
`msvc_x64_x64_ninja_release`, `linux-debug` and `linux-release`.

### Windows

#### GCC

Install CMake, MSYS2 UCRT64 GCC and `mingw32-make`. Ensure
`C:\msys64\ucrt64\bin` is on `PATH`, then configure and build with GCC:

```powershell
cmake --preset windows-gcc-debug
cmake --build --preset windows-gcc-debug
.\build\windows-gcc-debug\bin\AutoAPIFunctionDesigner.exe
```

#### MSVC

Install Visual Studio with the C++ desktop workload and Ninja. Run the
commands from an x64 Native Tools Command Prompt or a Developer PowerShell:

```powershell
cmake --preset msvc_x64_x64_ninja_debug
cmake --build --preset "msvc_x64_x64 (Ninja - Debug)"
.\build\msvc_x64_x64_debug\bin\AutoAPIFunctionDesigner.exe
```

Use `msvc_x64_x64_ninja_release` and
`"msvc_x64_x64 (Ninja - Release)"` for a release build.

If wxWidgets is not already installed, CMake downloads and builds wxWidgets
3.2.6 automatically. This is slow only on the first configure and is cached
afterwards. Disable it with `-DACD_FETCH_WXWIDGETS=OFF`.

### Linux

```bash
sudo apt-get install cmake libwxgtk3.2-dev
cd function_designer
cmake --preset linux-debug
cmake --build --preset linux-debug
./build/linux-debug/bin/AutoAPIFunctionDesigner
```

The meta model and the sample specification are copied next to the executable
after each build.

## Sample

`../examples/vehicle_speed_fusion_multirate` and
`../examples/speed_hazard_detection.afs` are ready to use function
specifications for trying out read, display and write.

`../examples/autoapiframework_metadata_V04.yaml` is the current meta model specification file.

## Sources

| File | Purpose |
| --- | --- |
| `src/main.cpp` | wxApp entry point |
| `src/main_frame.*` | Window, buttons and the four collection views |
| `src/yaml_node.*` | Order preserving YAML document tree |
| `src/yaml_parser.*` | Block style YAML subset parser |
| `src/yaml_writer.*` | Emitter reproducing the `.afs` layout |
| `src/validate_function.*` | Methods for syntax checks |
| `src/meta_model.*` | `autoapiframework_meta_model.yaml` (enums, interface types) |
| `src/function_specification.*` | Function specification access and load/save |

## vspec files

The official VSS repository is recommended when working with `.vspec` files:

```powershell
git clone https://github.com/COVESA/vehicle_signal_specification.git
cd vehicle_signal_specification
```

With the Python virtual environment activated, convert a `.vspec` file to
JSON with `vss-tools`:

```powershell
vspec export json --vspec spec/Vehicle/Vehicle.vspec --output VehicleSignalSpecification.json
```

The application also runs this conversion from *Add via vss* and lets you
select the input `.vspec` file and output `.json` file.
