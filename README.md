# SweatBoxTrainer

SweatBoxTrainer is a Win32/C++20 training utility for SweatBox scenarios.

## Layout

- `SweatBoxTrainer/include/SweatBoxTrainer`: public project headers
- `SweatBoxTrainer/src`: implementation files
- `SweatBoxTrainer/resources`: Win32 resources and icons
- `data`: sample aircraft, airport, and scenario data
- `bin`: Visual Studio build output
- `build`: intermediate and CMake build output

## Build With VS2022

Open `SweatBoxTrainer.sln` in Visual Studio 2022 and build `Release | x64`.

From a Visual Studio Developer Command Prompt:

```powershell
msbuild SweatBoxTrainer.sln /p:Configuration=Release /p:Platform=x64
```

The executable is written to `bin/x64/Release/SweatBoxTrainer.exe`.

## Package With CMake

```powershell
cmake --preset vs2022-x64
cmake --build --preset vs2022-x64-release
cmake --build --preset vs2022-x64-release --target package
```

The package target creates a ZIP archive under `build/cmake/vs2022-x64`.
