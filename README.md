# Decompiler for Lua 4 (WIP)

Uses compiled Lua 4 bytecode as input and converts it to readable Lua 4 code.

- [Lua 4.0 Code](https://www.lua.org/source/4.0)
- [Lua 4.0 Reference](https://www.lua.org/manual/4.0)


## TODOs

- Implement the rest of the operators
- JMPONF might be used different
- POP command needs more attention
- Complete documentation


## Documentation

Beginner-friendly [Documentation](https://github.com/styinx/lua4dec/blob/master/doc/Doc.md)


## Build

[![Windows MSVC Build @ master](https://github.com/styinx/lua4dec/actions/workflows/build-windows-msvc.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/build-windows-msvc.yml)

[![Ubuntu Clang Build @ master](https://github.com/styinx/lua4dec/actions/workflows/build-ubuntu-clang.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/build-ubuntu-clang.yml)

[![Test @ master](https://github.com/styinx/lua4dec/actions/workflows/test.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/test.yml)


### CMake

```
mkdir build
cmake -B build
cmake --build build --config Release
```


## Run

```
./luadec luac.out
```


## Run test (compiles and decompiles scripts in the tests/scripts folder)

```
./test ./luac ./luadec ./tests/scripts/
```

## Inspect the byte code with a GUI (WIP)

[lua4dec-browser](https://github.com/styinx/lua4dec-browser)

