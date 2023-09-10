# Decompiler for Lua 4 (WIP)

Uses compiled Lua 4 bytecode as input and converts it to readable Lua 4 code.

## Build
![build master](https://github.com/styinx/lua4dec/actions/workflows/build.yml/badge.svg?branch=master)
![test master](https://github.com/styinx/lua4dec/actions/workflows/test.yml/badge.svg?branch=master)

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
