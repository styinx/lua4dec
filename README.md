# Decompiler for Lua 4 (WIP)

Uses compiled Lua 4 bytecode as input and converts it to readable Lua 4 code.

## TODOs

- Conditions: create "else if" blocks
- For In
- AstOperation
- Call

## Build
[![Build && Test @ master](https://github.com/styinx/lua4dec/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/styinx/lua4dec/actions/workflows/main.yml)

### CMake

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

### Make
```
make lua4dec
```

## Run

```
./lua4dec luac.out
```
