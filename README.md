# Zyrex Menu

Roblox external menu built with C++20, ImGui, and Direct3D 11.

## Pre-built Binary

`bin/menu.exe` is ready to use. Rebuild only if you modify the source code.

## Building (only if needed)

**Requirements:** Visual Studio 2022 with "Desktop development with C++" workload.

1. Open `ZyrexMenu\ZyrexMenu.sln` in Visual Studio 2022
2. Set **Release | x64**
3. Build Solution (Ctrl+Shift+B)
4. Output: `bin\menu.exe`

Or run `.\build.ps1` from PowerShell.

## Project Structure

```
ZyrexMenu/
├── README.md
├── build.ps1                          # Build script
├── gen_header.ps1                     # Converts binary files to C++ headers
├── bin/
│   └── menu.exe                       # Compiled output
│
└── ZyrexMenu/                         # Visual Studio solution
    ├── ZyrexMenu.sln
    ├── projects/
    │   ├── menu/                      # Main menu project
    │   │   ├── Main.cpp               # Entry point
    │   │   ├── Menu.cpp/.h            # ImGui menu rendering
    │   │   ├── src/                   # Core source
    │   │   │   ├── settings.h         # All settings/globals
    │   │   │   ├── cache/             # Entity caching
    │   │   │   ├── features/          # Feature code (config, football)
    │   │   │   ├── game/              # Game state & rescanning
    │   │   │   ├── memory/            # Memory R/W (luck.asm)
    │   │   │   ├── menu/              # Keybinds, radar
    │   │   │   ├── Offsets/           # Roblox memory offsets
    │   │   │   └── sdk/               # Roblox SDK + math
    │   │   ├── ext/                   # Third-party libs (ImGui, JSON, FreeType, etc.)
    │   │   └── render/                # 3D rendering
    │   └── shared/                    # Framework (Application, Console, DX11, Window)
    └── vendors/                       # SDKs (Boost, DirectX, FreeType)
```

## Technical Details

- **C++20**, Windows subsystem (no console)
- Static CRT (`/MT`) — no DLL dependencies
- `luck.asm` for low-level memory operations (MASM x64)
- All settings are in-memory only — no files written to disk

## Dependencies

All included in the repo: ImGui, FreeType, nlohmann/json, BLAKE3, Zstandard, xxHash, libcurl, DirectX SDK, Boost.

## Notes

- Memory offsets in `src/Offsets/Offsets.hpp` are version-specific — update when Roblox patches.
