# SASM – The Smallest Assembler Ever

SASM is an ultra-lightweight x86 assembler written in pure C. At only ~309 lines of code and ~12 KB when compiled with [TCC](https://bellard.org/tcc/)
, it supports ~400 x86 instructions including SSE, AVX, x87, MMX, and even AVX-512.

It’s fast, standalone, and minimal, making it perfect for hobby OS development, learning x86 assembly, or any project where you want a tiny but capable assembler.

## Features

Supports ~400 x86 instructions: MOV, ADD, SUB, PUSH/POP, JMP/CALL, SSE/AVX, x87, MMX, AVX-512.

Handles labels and relative jumps/calls.

Supports registers: 32-bit (EAX..EDI), 16-bit (AX..DI), 8-bit (AL..BH).

Produces flat binaries (.bin) from assembly source.

Compiles with TCC into a single executable (~12 KB).

Extremely lightweight and fast, perfect for small projects.

## Usage

Compile SASM using TCC:

```tcc sasm.c -o sasm.exe```


Assemble a file:
sasm input.asm output.bin

Example ASM file (hello.asm):
MOV EAX, 1
MOV EBX, 0
INT 0x80
Assemble it:

sasm hello.asm hello.bin

## Notes
There is a ; at the start of the file because TCC prolly thinks its an assembler source, without it, you might recieve a error like `sasm.c:1: error: ';' expected (got "#")`

SASM is minimalistic: does not support macros or complex memory expressions yet.

Immediate values are handled for reg→imm instructions.

Memory operands ([EAX]) are not fully implemented.

# License
GPL v3





