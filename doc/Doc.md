# Lua 4 Bytecode

## Intro

### Bytecode Explanation

Run the Lua 4 decompiler in debug mode to print the bytecode to the terminal.
The printed bytecode will look similar to the Lua 4 compilers `-l` option.
The following sections show an example.


#### First part of the output:

```
Name: @.\tests\scripts\map.lua
Line: 0
Params: 0
Variadic: false
Stack: 7
Globals: 5
        0: "simple"
        1: "a"
        2: "b"
        3: "c"
        4: "nested"
Locals: 0
```

It shows the general information about the chunk/function like its name and the local and global variables.
This data is stored in the bytecode header of the function.


#### Second part of the output:

```
Instructions: 18
I:         209 (0x000000d1) | OP: 17 (0x11) (CREATETABLE) | A:     0 (0x0000000) | B:   3 (0x003) | U:          3 (0x00000003) | S: -33554428 (0xfe000004)
I:          71 (0x00000047) | OP:  7 (0x07) ( PUSHSTRING) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002) | a
I: -2147483642 (0x80000006) | OP:  6 (0x06) (    PUSHINT) | A: 65536 (0x0010000) | B:   0 (0x000) | U:   33554432 (0x02000000) | S:         1 (0x00000001) | 1
I:         135 (0x00000087) | OP:  7 (0x07) ( PUSHSTRING) | A:     0 (0x0000000) | B:   2 (0x002) | U:          2 (0x00000002) | S: -33554429 (0xfe000003) | b
I:           8 (0x00000008) | OP:  8 (0x08) (    PUSHNUM) | A:     0 (0x0000000) | B:   0 (0x000) | U:          0 (0x00000000) | S: -33554431 (0xfe000001) | 1.123000
I:         199 (0x000000c7) | OP:  7 (0x07) ( PUSHSTRING) | A:     0 (0x0000000) | B:   3 (0x003) | U:          3 (0x00000003) | S: -33554428 (0xfe000004) | c
I:          71 (0x00000047) | OP:  7 (0x07) ( PUSHSTRING) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002) | a
I:         214 (0x000000d6) | OP: 22 (0x16) (     SETMAP) | A:     0 (0x0000000) | B:   3 (0x003) | U:          3 (0x00000003) | S: -33554428 (0xfe000004)
I:          19 (0x00000013) | OP: 19 (0x13) (  SETGLOBAL) | A:     0 (0x0000000) | B:   0 (0x000) | U:          0 (0x00000000) | S: -33554431 (0xfe000001) | simple
I:          81 (0x00000051) | OP: 17 (0x11) (CREATETABLE) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002)
I:          71 (0x00000047) | OP:  7 (0x07) ( PUSHSTRING) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002) | a
I:          81 (0x00000051) | OP: 17 (0x11) (CREATETABLE) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002)
I:         135 (0x00000087) | OP:  7 (0x07) ( PUSHSTRING) | A:     0 (0x0000000) | B:   2 (0x002) | U:          2 (0x00000002) | S: -33554429 (0xfe000003) | b
I: -2147483642 (0x80000006) | OP:  6 (0x06) (    PUSHINT) | A: 65536 (0x0010000) | B:   0 (0x000) | U:   33554432 (0x02000000) | S:         1 (0x00000001) | 1
I:          86 (0x00000056) | OP: 22 (0x16) (     SETMAP) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002)
I:          86 (0x00000056) | OP: 22 (0x16) (     SETMAP) | A:     0 (0x0000000) | B:   1 (0x001) | U:          1 (0x00000001) | S: -33554430 (0xfe000002)
I:         275 (0x00000113) | OP: 19 (0x13) (  SETGLOBAL) | A:     0 (0x0000000) | B:   4 (0x004) | U:          4 (0x00000004) | S: -33554427 (0xfe000005) | nested
I:           0 (0x00000000) | OP:  0 (0x00) (        END) | A:     0 (0x0000000) | B:   0 (0x000) | U:          0 (0x00000000) | S: -33554431 (0xfe000001)
```
It shows the instructions of that chunk/function.
`I` shows the full instruction in decimal and hex (32 bits).
`OP` shows the OP-code in decimal, hex, and as string (bits 0-5, lower 6 bits).
`A` shows the A-registers unsigned value in decimal and hex (bits 15-31, upper 17 bits).
`B` shows the B-registers unsigned value in decimal and hex (bits 6-14, middle 9 bits).
`U` shows the U-registers unsigned value in decimal and hex (bits 6-31, upper 26 bits).
`S` shows the S-registers signed value in decimal and hex (bits 6-31, upper 26 bits).
The last part of the line shows a special value that is encoded into the instruction (meaning that the OP-code is using one or multiple register values).


#### Third part of the output:

```
simple = {a = 1, b = 1.123000, c = "a"}
nested = {a = {b = 1}}
```
It shows the reconstructed Lua 4 source code from the Lua 4 bytecode.


### Expressions

#### Simple Primitives

Primitives like integers and nil are encoded directly into the instruction (PUSHINT uses the S register, PUSHNIL just pushed `nil` onto the stack).


#### Complex Primitives

Primitives like numbers, locals, globals are stored in a list within the chunk/function.
The instruction contains the index of the value within that list.
Usually `B` or `U` is used for the index.
In the example above, the second instruction (0x00000047) contains the PUSHSTRING OP-code which uses the `B` register as index to the global list.
Therefore, the string `a` is pushed onto the stack.


#### Operators




### Statements

#### Call

```
```
