# SyncMySSD

Modern C++ file synchronization tool for Windows with a stylish Dear ImGui interface.

## Features
- **Bidirectional sync** between any two folders (PC ↔ external drive)
- **Diff preview** with color-coded status (new / modified / deleted)
- **Selective sync** – check/uncheck individual files before syncing
- **Async scanning** – non-blocking UI during directory analysis
- **Progress tracking** with real-time file progress overlay

## Requirements
- **Windows 10/11** with DirectX 11
- **CMake 3.20+**
- **Visual Studio 2022** (MSVC C++20)
- Git (for FetchContent to download Dear ImGui)

## Build
```bash
# Configure (downloads Dear ImGui automatically)
cmake -B build -S . -G "Visual Studio 17 2022"

# Build
cmake --build build --config Release

# Run
build\Release\SyncMySSD.exe
```

## Project Structure
```
src/
├── main.cpp              # Win32 + DX11 bootstrap
├── ui/
│   ├── theme.h/.cpp      # Dark theme with teal accents
│   └── app_ui.h/.cpp     # Main application UI
├── sync/
│   ├── sync_types.h      # Data structures
│   ├── file_scanner.h/.cpp  # Recursive directory scanner
│   └── sync_engine.h/.cpp   # Diff computation & sync execution
└── resources/
    ├── app.manifest      # DPI awareness
    └── app.rc            # Resource script
```
