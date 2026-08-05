# Games in C: A Roguelike 

This repository was created for the [RoguelikeDev Does The Complete Roguelike Tutorial (2026)](https://www.reddit.com/r/roguelikedev/comments/1vd9noj/roguelikedev_does_the_complete_roguelike_tutorial/).

## Dependencies

### SDL3

SDL3 is built from source as an ordinary part of this project.
CMake expects it to be available in a sibling `thirdparty` directory, so that it can pull it in via `add_subdirectory` (not `find_package`).
The expected directory setup is:

```
<parent folder>/
├── thirdparty/
│   ├── SDL/                  (plain clone of libsdl-org/SDL, pinned to a release tag)
└── roguelikedev-2026/        (this repo)
    ├── CMakeLists.txt
    ├── src/
    │   └── CMakeLists.txt
```

`CMakeLists.txt` resolves the location of thirdparty via a `GINC_THIRDPARTY_DIR` cache variable, defaulting to `../thirdparty` relative to the repo.
You can override it with `-DGINC_THIRDPARTY_DIR=<path>` if it lives somewhere else on your machine.

## Building the game

CMake is used to manage the build, which relies on other tools called "generators".
Building itself is a two-step process: configure then compile.
But the steps for building, and runninvg, the game varies by platform.

### Windows

On Windows, the build produces an executable.
Depending on which generator you are using, the "build type" (one of: Debug, Release, RelWithDebInfo, MinSizeRel) may need to be specified at configure time (single-config) or build time (multi-config).

With a single-config generator, the build directory contains a single build type.
Starting from the repository root and using Ninja as the generator:

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target a_roguelike
```

With a multi-config generator, the build directory contains multiple build types.

```
cmake -S . -B build -G "Ninja Multi-Config"
cmake --build build --target a_roguelike --config Debug
```

The executable is named `a_roguelike.exe`.
Where it ends up depends on the generator:

- Single-config: `build/src/a_roguelike.exe`
- Multi-config: `build/src/<Config>/a_roguelike.exe`

On Windows, the SDL3 DLL is copied next to the executable automatically as part of the build.
