# Decompiler for Lua 4 (WIP)

Uses compiled Lua 4 bytecode as input and converts it to readable Lua 4 code.

## Build

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Run

```
./lua4dec luac.out
```
