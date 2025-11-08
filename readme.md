# SASM – The Smallest Assembler Ever

SASM is an ultra-lightweight x86 assembler written in pure C. At only 500+ lines of code and ~30 KB when compiled with [TCC](https://bellard.org/tcc/)
, it supports 400+ x86 instructions including SSE, AVX, x87, MMX, and even AVX-512.

It’s fast, standalone, and minimal, making it perfect for hobby OS development, learning x86 assembly, or any project where you want a tiny but capable assembler.

## Features

Supports 400+ x86 instructions: MOV, ADD, SUB, PUSH/POP, JMP/CALL, SSE/AVX, x87, MMX, AVX-512.

Handles labels and relative jumps/calls.

Supports registers: 32-bit (EAX..EDI), 16-bit (AX..DI), 8-bit (AL..BH).

Produces flat binaries (.bin) from assembly source.

Compiles with TCC into a single executable (~30 KB).

Extremely lightweight and fast, perfect for small projects.

## Usage

Compile SASM using TCC:

```tcc sasm.c -o sasm.exe```


Assemble a file:
`sasm input.asm output.bin`

Example ASM file (hello.asm):
`MOV EAX, 1
MOV EBX, 0
INT 0x80`
Assemble it:

`sasm hello.asm hello.bin`

## Notes
There is a ; at the start of the file because TCC prolly thinks its an assembler source, without it, you might recieve a error like `sasm.c:1: error: ';' expected (got "#")`

SASM is minimalistic: does not support macros or complex memory expressions yet.

Immediate values are handled for reg→imm instructions.

Memory operands ([EAX]) are not fully implemented.

## GAS vs NASM vs FASM vs Small Assembler
| Feature                        | GAS (GNU Assembler)                        | FASM                          | NASM                          | SASM (This Project)                                 |
| ------------------------------ | ------------------------------------------ | ----------------------------- | ----------------------------- | --------------------------------------------------- |
| **Binary Size**                | Part of binutils, hundreds of MB installed | ~1–2 MB                       | ~1.9 MB                       | ~30 KB                                              |
| **Lines of Code**              | Very large (binutils)                      | Tens of thousands             | Tens of thousands             | 500 LOC                                                 |
| **Instruction Coverage**       | Full x86/x86-64, ARM, others               | Full x86/x86-64               | Full x86/x86-64               | 400+ x86 instructions (SSE, AVX, x87, MMX, AVX-512) |
| **Macros / Advanced Features** | Yes                                        | Yes                           | Yes                           | No (minimalistic)                                   |
| **Labels / Jumps**             | Yes                                        | Yes                           | Yes                           | Yes                                                 |
| **Memory Operand Support**     | Full                                       | Full                          | Full                          | Partial (mostly reg→reg / reg→imm)                  |
| **Platform**                   | Linux, Unix, Windows (via Cygwin/MSYS2)    | Windows / Linux               | Cross-platform                | Windows / Linux (via TCC)                           |
| **Use Case**                   | Professional assembly, OS dev, multi-arch  | Professional assembly, OS dev | Professional assembly, OS dev | Lightweight projects, learning, hobby OS dev        |
| **Compilation**                | Part of GNU binutils                       | Standard C / custom           | Standard C                    | Single-file, compiled with TCC (~30 KB)             |

## License
GPL v3













