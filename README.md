# Decompiler for Lua 4 (WIP)

Uses compiled Lua 4 bytecode as input and converts it to readable Lua 4 code.

[Lua 4.0 Code](https://www.lua.org/source/4.0)
[Lua 4.0 Reference](https://www.lua.org/manual/4.0)

## TODOs

- Conditions: create "else if" blocks
- For In
- Table
- closure parameters

## Build
[![Build && Test @ master](https://github.com/styinx/lua4dec/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/main.yml)

### CMake

```
mkdir build
cmake -B build
cmake --build build --config Release
```

### Make
```
make lua4dec
```

## Run

```
./lua4dec luac.out
```
