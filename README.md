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

### Assets

The assets used in this game are not bundled in this repository.
If you are attempting to compile and run this repository, then you need to download the files from the creator yourself.
The files you download then need to be placed in an `assets/` folder.

This game uses the [DINOBYTE bitmap font](https://mby.itch.io/dinobyte) by [mby](https://mby.itch.io/).
It is licensed under [Creative Commons Attribution 4.0](http://creativecommons.org/licenses/by/4.0/).
Download the file `dbyte_1x.png` from the creator and save it in this directory.

The expected directory setup is:

```
roguelikedev-2026/            (this repo)
├── assets/
│   ├── dbyte_1x.png          (bitmap font not included in the repo)
├── src/
└── CMakeLists.txt
```

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

### Emscripten

This project can be built for web browsers using emscripten.
Emscripten will transpile the C code into an HTML file (and some other accompanying files).
Typically, emscripten builds are single-config with the Release build type.

You can install the emscripten toolchain using available scripts:

```powershell
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
.\emsdk install latest
.\emsdk activate latest
```

New terminal sessions need the emsdk environment activated to ensure that `emcmake`/`emcc` can be used (i.e., are on `PATH`).
For example, in Windows using Powershell:

```powershell
cd emsdk
.\emsdk_env.bat
```

Note that the configure step must use `emcmake`:

```
emcmake cmake -S . -B build-emscripten -DCMAKE_BUILD_TYPE=Release
cmake --build build-emscripten --target a_roguelike
```

The output is `build-emscripten/src/index.html` (plus its accompanying `.js`/`.wasm`).
However, if you'd like to run the game locally, you should not open the HTML file itself (browsers block `.wasm` loading from `file://` URLs).
Instead, serve the built directory over HTTP.
For example, using Python:

```
python -m http.server --directory build-emscripten/src
```
