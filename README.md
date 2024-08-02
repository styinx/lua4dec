# Decompiler for Lua 4 (WIP)

Uses compiled Lua 4 bytecode as input and converts it to readable Lua 4 code.

- [Lua 4.0 Code](https://www.lua.org/source/4.0)
- [Lua 4.0 Reference](https://www.lua.org/manual/4.0)


## TODOs

- JMPONF might be used different
- JMPONT
- Complete documentation


## Documentation

Beginner-friendly [Documentation](https://github.com/styinx/lua4dec/blob/master/doc/Doc.md)


## Checkout

```
git clone --recursive https://github.com/styinx/lua4dec.git
```

## Build

[![Windows MSVC Build @ master](https://github.com/styinx/lua4dec/actions/workflows/build-windows-msvc.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/build-windows-msvc.yml)

[![Ubuntu Clang Build @ master](https://github.com/styinx/lua4dec/actions/workflows/build-ubuntu-clang.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/build-ubuntu-clang.yml)

[![Test @ master](https://github.com/styinx/lua4dec/actions/workflows/test.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/test.yml)


### CMake

```
cmake -S . -B build -DTARGET_ARCH=64  # 32 for 32bit lua bytecode
cmake --build build --config Release
```

### Pre-built binaries

- Pre-build binaries of the pipeline: [https://github.com/styinx/lua4dec/actions](https://github.com/styinx/lua4dec/actions)
- Binaries of Releases: [https://github.com/styinx/lua4dec/releases](https://github.com/styinx/lua4dec/releases)


## Run

```
./luadec_64 luac.out
.\luadec_64.exe luac.out
```


## Run test (compiles and decompiles scripts in the tests/scripts folder)

```
test.exe lua4\luac_64.exe luadec_64.exe differ.exe tests\scripts\
```

## Inspect the byte code with a GUI (WIP)

[lua4dec-browser](https://github.com/styinx/lua4dec-browser)

