;
/* SASM, version 1.8
theres an actual semicolon at the start because error on without it*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 128
#define MAX_TOKENS 8
#define MAX_LABELS 256
#define MAX_OUT 1024

typedef struct {
    const char* name;
    unsigned char opcode1;
    unsigned char opcode2;
    unsigned char opcode3;
    unsigned char opcode4;
    unsigned char opcode5;
    unsigned char opcode6;
    unsigned char opcode7;
    unsigned char opcode8;
    unsigned char opcode9;
    unsigned char opcode_reg_reg; // for external use
    unsigned char opcode_reg_mem; // optional
    unsigned char opcode_mem_reg; // optional
    unsigned char opcode_reg_imm; // optional
} OpCode32;

typedef struct {
    char name[8];
    unsigned char code;
} Reg32;

typedef struct {
    char name[32];
    int addr;
} Label;

OpCode32 opcodes[] = {
    // Data movement
    {"MOV", 0x89, 0xB8}, {"MOVSX", 0x0F, 0xBE}, {"MOVZX", 0x0F, 0xB6}, {"XCHG", 0x87, 0x90}, {"LEA", 0x8D, 0},
    {"LDS", 0xC5, 0}, {"LES", 0xC4, 0}, {"LFS", 0x0F, 0xB4}, {"LGS", 0x0F, 0xB5}, {"LSS", 0x0F, 0xB2},
    {"MOVSB", 0xA4, 0}, {"MOVSW", 0xA5, 0}, {"MOVSD", 0xA5, 0}, {"LODSB", 0xAC, 0}, {"LODSW", 0xAD, 0},
    {"LODSD", 0xAD, 0}, {"STOSB", 0xAA, 0}, {"STOSW", 0xAB, 0}, {"STOSD", 0xAB, 0}, {"SCASB", 0xAE, 0},
    {"SCASW", 0xAF, 0}, {"SCASD", 0xAF, 0}, {"CMPSB", 0xA6, 0}, {"CMPSW", 0xA7, 0}, {"CMPSD", 0xA7, 0},
    {"XLAT", 0xD7, 0}, {"XCHG_AX_AX", 0x90, 0}, {"MOV_AL_imm8", 0xB0, 0}, {"MOV_AX_imm16", 0xB8, 0},

    // Arithmetic
    {"ADD", 0x01, 0x05}, {"ADC", 0x11, 0x15}, {"SUB", 0x29, 0x2D}, {"SBB", 0x19, 0x1D},
    {"INC", 0x40, 0}, {"DEC", 0x48, 0}, {"NEG", 0xF7, 0xD8}, {"MUL", 0xF7, 0x04}, {"IMUL", 0xF7, 0x05},
    {"DIV", 0xF7, 0x06}, {"IDIV", 0xF7, 0x07}, {"CMP", 0x39, 0x3D}, {"TEST", 0x85, 0xA8}, {"CMPXCHG", 0x0F, 0xB1},
    {"AAA", 0x37, 0}, {"AAS", 0x3F, 0}, {"DAA", 0x27, 0}, {"DAS", 0x2F, 0}, {"CBW", 0x98, 0}, {"CWD", 0x99, 0},
    {"CDQ", 0x99, 0}, {"AAD", 0xD5, 0x0A}, {"AAM", 0xD4, 0x0A},

    // Bitwise / shifts / rotates
    {"AND", 0x21, 0x25}, {"OR", 0x09, 0x0D}, {"XOR", 0x31, 0x35}, {"NOT", 0xF7, 0xD0},
    {"SHL", 0xD1, 0}, {"SAL", 0xD1, 0}, {"SHR", 0xD1, 5}, {"SAR", 0xD1, 7}, {"ROL", 0xD1, 0}, {"ROR", 0xD1, 1},
    {"RCL", 0xD1, 2}, {"RCR", 0xD1, 3}, {"BT", 0x0F, 0xA3}, {"BTS", 0x0F, 0xAB}, {"BTR", 0x0F, 0xB3}, {"BTC", 0x0F, 0xBB},
    {"BSF", 0x0F, 0xBC}, {"BSR", 0x0F, 0xBD}, {"SETB", 0x0F, 0x92}, {"SETBE", 0x0F, 0x96}, {"SETE", 0x0F, 0x94},
    {"SETNE", 0x0F, 0x95}, {"SETG", 0x0F, 0x9F}, {"SETGE", 0x0F, 0x9D}, {"SETL", 0x0F, 0x9C}, {"SETLE", 0x0F, 0x9E},
    {"SETS", 0x0F, 0x98}, {"SETNS", 0x0F, 0x99}, {"SETO", 0x0F, 0x90}, {"SETNO", 0x0F, 0x91}, {"SETA", 0x0F, 0x97},
    {"SETAE", 0x0F, 0x93}, {"SETNA", 0x0F, 0x96}, {"SETNBE", 0x0F, 0x97},

    // Stack
    {"PUSH", 0x50, 0}, {"POP", 0x58, 0}, {"PUSHA", 0x60, 0}, {"POPA", 0x61, 0}, {"PUSHAD", 0x60, 0}, {"POPAD", 0x61, 0},
    {"PUSHF", 0x9C, 0}, {"POPF", 0x9D, 0}, {"LAHF", 0x9F, 0}, {"SAHF", 0x9E, 0},

    // Control flow
    {"JMP", 0xE9, 0}, {"JE", 0x0F, 0x84}, {"JZ", 0x0F, 0x84}, {"JNE", 0x0F, 0x85}, {"JNZ", 0x0F, 0x85},
    {"JG", 0x0F, 0x8F}, {"JGE", 0x0F, 0x8D}, {"JL", 0x0F, 0x8C}, {"JLE", 0x0F, 0x8E}, {"JB", 0x0F, 0x82},
    {"JNB", 0x0F, 0x83}, {"JO", 0x0F, 0x80}, {"JNO", 0x0F, 0x81}, {"JS", 0x0F, 0x88}, {"JNS", 0x0F, 0x89},
    {"JCXZ", 0xE3, 0}, {"LOOP", 0xE2, 0}, {"LOOPE", 0xE1, 0}, {"LOOPNE", 0xE0, 0}, {"CALL", 0xE8, 0},
    {"RET", 0xC3, 0}, {"IRET", 0xCF, 0}, {"INT", 0xCD, 0},

    // Flags
    {"CLC", 0xF8, 0}, {"STC", 0xF9, 0}, {"CMC", 0xF5, 0}, {"CLD", 0xFC, 0}, {"STD", 0xFD, 0}, {"STI", 0xFB, 0}, {"CLI", 0xFA, 0},

    // Misc / NOP
    {"NOP", 0x90, 0}, {"HLT", 0xF4, 0}, {"WAIT", 0x9B, 0}, {"ESC", 0xD8, 0}, {"INVLPG", 0x0F, 0x01}, {"CPUID", 0x0F, 0xA2},
    {"RDTSC", 0x0F, 0x31}, {"RDMSR", 0x0F, 0x32}, {"WRMSR", 0x0F, 0x30}, {"UD2", 0x0F, 0x0B},

    // x87 FPU
    {"FADD", 0xD8, 0}, {"FMUL", 0xD8, 1}, {"FSUB", 0xD8, 4}, {"FDIV", 0xD8, 6},
    {"FLD", 0xD9, 0}, {"FST", 0xD9, 2}, {"FSTP", 0xD9, 3}, {"FILD", 0xDB, 0}, {"FISTP", 0xDB, 3},
    {"FCLEX", 0xDB, 0xE2}, {"FSTCW", 0xD9, 5}, {"FLDCW", 0xD9, 6}, {"FNSTSW", 0xDF, 0xE0},

    // SSE / SIMD basics
    {"MOVAPS", 0x0F, 0x28}, {"MOVUPS", 0x0F, 0x10}, {"MOVAPD", 0x0F, 0x28}, {"MOVUPD", 0x0F, 0x10},
    {"ADDPS", 0x0F, 0x58}, {"ADDPD", 0x0F, 0x58}, {"SUBPS", 0x0F, 0x5C}, {"SUBPD", 0x0F, 0x5C},
    {"MULPS", 0x0F, 0x59}, {"MULPD", 0x0F, 0x59}, {"DIVPS", 0x0F, 0x5E}, {"DIVPD", 0x0F, 0x5E},
    {"ANDPS", 0x0F, 0x54}, {"ANDPD", 0x0F, 0x54}, {"ORPS", 0x0F, 0x56}, {"ORPD", 0x0F, 0x56},
    {"XORPS", 0x0F, 0x57}, {"XORPD", 0x0F, 0x57},

    // MMX basics
    {"MOVD", 0x0F, 0x6E}, {"MOVQ", 0x0F, 0x7E}, {"PADDW", 0x0F, 0xFD}, {"PADDD", 0x0F, 0xFE},
    {"PSUBW", 0x0F, 0xF9}, {"PSUBD", 0x0F, 0xFA}, {"PMULLW", 0x0F, 0xD5}, {"PMULUDQ", 0x0F, 0xF4},
    {"PAND", 0x0F, 0xDB}, {"POR", 0x0F, 0xEB}, {"PXOR", 0x0F, 0xEF}, {"PSLLW", 0x0F, 0xF1},
    {"PSLLD", 0x0F, 0xF2}, {"PSRLW", 0x0F, 0xD1}, {"PSRLD", 0x0F, 0xD2}, {"PSRAW", 0x0F, 0xE1}, {"PSRAD", 0x0F, 0xE2},
    
    // AVX / VEX-prefixed
    {"VMOVAPS", 0xC5, 0xF8}, {"VMOVUPS", 0xC5, 0xF0}, {"VADDPS", 0xC5, 0xF8}, {"VSUBPS", 0xC5, 0xFA},
    {"VMULPS", 0xC5, 0xFB}, {"VDIVPS", 0xC5, 0xFD}, {"VANDPS", 0xC5, 0xFC}, {"VORPS", 0xC5, 0xFE}, {"VXORPS", 0xC5, 0xFF},
    {"VADDPD", 0xC5, 0x58}, {"VSUBPD", 0xC5, 0x5C}, {"VMULPD", 0xC5, 0x59}, {"VDIVPD", 0xC5, 0x5E},

    // AVX integer / MMX extensions
    {"VPADDW", 0xC5, 0xFD}, {"VPADDD", 0xC5, 0xFE}, {"VPSUBW", 0xC5, 0xF9}, {"VPSUBD", 0xC5, 0xFA},
    {"VPMULLW", 0xC5, 0xD5}, {"VPMULUDQ", 0xC5, 0xF4}, {"VPAND", 0xC5, 0xDB}, {"VPOR", 0xC5, 0xEB}, {"VPXOR", 0xC5, 0xEF},

    // AVX-512 (EVEX prefix)
    {"VADDPS512", 0x62, 0xF1}, {"VSUBPS512", 0x62, 0xF5}, {"VMULPS512", 0x62, 0xF7}, {"VDIVPS512", 0x62, 0xFB},
    {"VADDPD512", 0x62, 0x58}, {"VSUBPD512", 0x62, 0x5C}, {"VMULPD512", 0x62, 0x59}, {"VDIVPD512", 0x62, 0x5E},
    {"VPADDW512", 0x62, 0xFD}, {"VPADDD512", 0x62, 0xFE}, {"VPSUBW512", 0x62, 0xF9}, {"VPSUBD512", 0x62, 0xFA},
    {"VPMULLW512", 0x62, 0xD5}, {"VPMULUDQ512", 0x62, 0xF4}, {"VPAND512", 0x62, 0xDB}, {"VPOR512", 0x62, 0xEB}, {"VPXOR512", 0x62, 0xEF},

    // SSE4.1 / SSE4.2 extras
    {"PCMPEQQ", 0x66, 0x0F}, {"PCMPISTRI", 0x66, 0x3A}, {"PCMPISTRM", 0x66, 0x3A},
    {"CRC32", 0xF2, 0x0F}, {"POPCNT", 0xF3, 0x0F}, {"LZCNT", 0xF3, 0x0F}, {"BLENDPS", 0x0F, 0x3A}, {"BLENDPD", 0x0F, 0x3A},

    // More rare x87 instructions
    {"FCOMPP", 0xDE, 0xF7}, {"FCOMP", 0xD8, 0xD8}, {"FCOM", 0xD8, 0xD0}, {"FDECSTP", 0xD9, 0xF6}, {"FINCSTP", 0xD9, 0xF7},
    {"FSIN", 0xD9, 0xFE}, {"FCOS", 0xD9, 0xFF}, {"FPTAN", 0xD9, 0xF2}, {"FPATAN", 0xD9, 0xF3}, {"FYL2X", 0xD9, 0xF1},    
    {"FYL2XP1", 0xD9, 0xF9}, {"FSCALE", 0xD9, 0xFD}, {"FSQRT", 0xD9, 0xFA}, {"FCHS", 0xD9, 0xE0}, {"FABS", 0xD9, 0xE1},
    {"FTST", 0xD9, 0xE4}, {"FXAM", 0xD9, 0xE5}, {"FLD1", 0xD9, 0xE8}, {"FLDL2T", 0xD9, 0xEA}, {"FLDL2E", 0xD9, 0xEB},
    {"FLDPI", 0xD9, 0xE9}, {"FLDLG2", 0xD9, 0xEC}, {"FLDLN2", 0xD9, 0xED}, {"FLDZ", 0xD9, 0xEE},

    // AVX-512 masking / blend ops
    {"VPBLENDD", 0xC4, 0xE1}, {"VPMASKMOVD", 0xC4, 0xE7}, {"VPMASKMOVQ", 0xC4, 0xEF}, {"VPBROADCASTD", 0xC4, 0xF9},
    {"VPBROADCASTQ", 0xC4, 0xFB}, {"VPERMD", 0xC4, 0xE2}, {"VPERMPD", 0xC4, 0xE3}, {"VPERMQ", 0xC4, 0xE4},

    // Miscellaneous SIMD ops
    {"PTEST", 0x0F, 0x38}, {"PCLMULQDQ", 0x0F, 0x3A}, {"AESENC", 0x0F, 0x38}, {"AESDEC", 0x0F, 0x38},
    {"AESENCLAST", 0x0F, 0x38}, {"AESDECLAST", 0x0F, 0x38}, {"AESIMC", 0x0F, 0x38}, {"AESKEYGENASSIST", 0x0F, 0x3A},

    // Segment overrides & special prefixes
    {"CS:", 0x2E, 0}, {"SS:", 0x36, 0}, {"DS:", 0x3E, 0}, {"ES:", 0x26, 0}, {"FS:", 0x64, 0}, {"GS:", 0x65, 0},

    // Extra / rare instructions
    {"BOUND", 0x62, 0}, {"ARPL", 0x63, 0}, {"SALC", 0xD6, 0}, {"XLATB", 0xD7, 0},
    {"ENTER", 0xC8, 0}, {"LEAVE", 0xC9, 0}, {"INSB", 0x6C, 0}, {"INSW", 0x6D, 0}, {"OUTSB", 0x6E, 0}, {"OUTSW", 0x6F, 0},

    // MMX extended
    {"MOVQ", 0x0F, 0x7E}, {"MOVD", 0x0F, 0x6E}, {"PADDW", 0x0F, 0xFD}, {"PADDD", 0x0F, 0xFE},
    {"PSUBW", 0x0F, 0xF9}, {"PSUBD", 0x0F, 0xFA}, {"PMULLW", 0x0F, 0xD5}, {"PMULUDQ", 0x0F, 0xF4},
    {"PAND", 0x0F, 0xDB}, {"POR", 0x0F, 0xEB}, {"PXOR", 0x0F, 0xEF}, {"PSLLW", 0x0F, 0xF1}, {"PSLLD", 0x0F, 0xF2},
    {"PSRLW", 0x0F, 0xD1}, {"PSRLD", 0x0F, 0xD2}, {"PSRAW", 0x0F, 0xE1}, {"PSRAD", 0x0F, 0xE2},

    // FMA (Fused Multiply-Add)
    {"VFMADD132PS", 0xC4, 0xE1}, {"VFMADD213PS", 0xC4, 0xE2}, {"VFMADD231PS", 0xC4, 0xE3},
    {"VFMADD132PD", 0xC4, 0xE4}, {"VFMADD213PD", 0xC4, 0xE5}, {"VFMADD231PD", 0xC4, 0xE6},
    {"VFMADDSUB132PS", 0xC4, 0xE7}, {"VFMADDSUB213PS", 0xC4, 0xE8}, {"VFMADDSUB231PS", 0xC4, 0xE9},
    {"VFMADDSUB132PD", 0xC4, 0xEA}, {"VFMADDSUB213PD", 0xC4, 0xEB}, {"VFMADDSUB231PD", 0xC4, 0xEC},

    // BMI1 / BMI2 (Bit Manipulation)
    {"ANDN", 0x0F, 0x38}, {"BEXTR", 0x0F, 0x38}, {"BLSI", 0x0F, 0x38}, {"BLSMSK", 0x0F, 0x38}, {"BLSR", 0x0F, 0x38},
    {"TZCNT", 0xF3, 0x0F}, {"LZCNT", 0xF3, 0x0F}, {"MULX", 0x0F, 0x38}, {"PDEP", 0x0F, 0x38}, {"PEXT", 0x0F, 0x38},

    // MPX (Memory Protection Extensions)
    {"BNDMK", 0x0F, 0x1E}, {"BNDCL", 0x0F, 0x1D}, {"BNDCU", 0x0F, 0x1D}, {"BNDMOV", 0x0F, 0x1E},

    // XOP (AMD)
    {"VPERMILPS", 0x8F, 0x00}, {"VPERMILPD", 0x8F, 0x01}, {"VPERMPD", 0x8F, 0x02}, {"VPERMQ", 0x8F, 0x03},
    {"VPPERM", 0x8F, 0x04}, {"VPBLENDMB", 0x8F, 0x05}, {"VPBLENDMW", 0x8F, 0x06},

    // AES / crypto
    {"AESENC", 0x0F, 0x38}, {"AESDEC", 0x0F, 0x38}, {"AESENCLAST", 0x0F, 0x38}, {"AESDECLAST", 0x0F, 0x38},
    {"AESIMC", 0x0F, 0x38}, {"AESKEYGENASSIST", 0x0F, 0x3A}, {"SHA1MSG1", 0x0F, 0x38}, {"SHA1MSG2", 0x0F, 0x38},
    {"SHA1NEXTE", 0x0F, 0x38}, {"SHA1RNDS4", 0x0F, 0x38}, {"SHA256MSG1", 0x0F, 0x38}, {"SHA256MSG2", 0x0F, 0x38},
    {"SHA256RNDS2", 0x0F, 0x38},

    // AVX-512 more masking / scatter / gather
    {"VPGATHERDD", 0x62, 0xF3}, {"VPGATHERDQ", 0x62, 0xF2}, {"VPGATHERQD", 0x62, 0xF4}, {"VPGATHERQQ", 0x62, 0xF5},
    {"VSCATTERDD", 0x62, 0xF6}, {"VSCATTERDQ", 0x62, 0xF7}, {"VSCATTERQD", 0x62, 0xF8}, {"VSCATTERQQ", 0x62, 0xF9},

    // Floating point extras
    {"FNCLEX", 0xDB, 0xE2}, {"FNSTCW", 0xD9, 0x07}, {"FNSTSW", 0xDF, 0xE0}, {"FRSTOR", 0x0F, 0xAE}, {"FSAVE", 0x0F, 0xAE},
    {"FSTSW_AX", 0x9F, 0x00}, {"FUCOMPP", 0xDE, 0xF7}, {"FYL2X", 0xD9, 0xF1}, {"FYL2XP1", 0xD9, 0xF9}, {"FSQRT", 0xD9, 0xFA},

    // Misc / rarely used instructions
    {"INVLPG", 0x0F, 0x01}, {"WBINVD", 0x0F, 0x09}, {"MOVBE", 0x0F, 0x38}, {"RDRAND", 0x0F, 0xC7}, {"RDSEED", 0x0F, 0xC7},
    {"MONITOR", 0x0F, 0x01}, {"MWAIT", 0x0F, 0x01}, {"CLFLUSH", 0x0F, 0xAE}, {"CLFLUSHOPT", 0x66, 0x0F},

    // Legacy / rarely used x86
    {"XLATB", 0xD7, 0}, {"AAA", 0x37, 0}, {"AAS", 0x3F, 0}, {"DAA", 0x27, 0}, {"DAS", 0x2F, 0},
    {"CBW", 0x98, 0}, {"CWD", 0x99, 0}, {"CDQ", 0x99, 0}, {"LOOPNZ", 0xE0, 0}, {"LOOPZ", 0xE1, 0},
    {"JECXZ", 0xE3, 0}, {"ARPL", 0x63, 0}, {"BOUND", 0x62, 0}, {"SALC", 0xD6, 0}, {"ENTER", 0xC8, 0},
    {"LEAVE", 0xC9, 0}, {"INSB", 0x6C, 0}, {"INSW", 0x6D, 0}, {"OUTSB", 0x6E, 0}, {"OUTSW", 0x6F, 0},

    // System / control instructions
    {"LGDT", 0x0F, 0x01}, {"SGDT", 0x0F, 0x01}, {"LIDT", 0x0F, 0x01}, {"SIDT", 0x0F, 0x01},
    {"LLDT", 0x0F, 0x00}, {"SLDT", 0x0F, 0x00}, {"LTR", 0x0F, 0x00}, {"STR", 0x0F, 0x00},
    {"VERR", 0x0F, 0x00}, {"VERW", 0x0F, 0x00}, {"CLTS", 0x0F, 0x06}, {"INVD", 0x0F, 0x08},
    {"WBINVD", 0x0F, 0x09}, {"UD2", 0x0F, 0x0B}, {"MOVCR0", 0x0F, 0x20}, {"MOVCR2", 0x0F, 0x22},
    {"MOVCR3", 0x0F, 0x23}, {"MOVCR4", 0x0F, 0x24}, {"MOVDR0", 0x0F, 0x21}, {"MOVDR1", 0x0F, 0x21},
    {"MOVDR2", 0x0F, 0x21}, {"MOVDR3", 0x0F, 0x21}, {"MOVDR6", 0x0F, 0x21}, {"MOVDR7", 0x0F, 0x21},

    // Virtualization (VMX / Intel VT-x)
    {"VMFUNC", 0x0F, 0x01}, {"VMLAUNCH", 0x0F, 0x01}, {"VMRESUME", 0x0F, 0x01}, {"VMXOFF", 0x0F, 0x01},
    {"VMREAD", 0x0F, 0x01}, {"VMWRITE", 0x0F, 0x01},

    // Intel TSX
    {"XBEGIN", 0xC7, 0xF8}, {"XEND", 0x0F, 0x01}, {"XTEST", 0x0F, 0x01},

    // More FPU / x87 rare instructions
    {"F2XM1", 0xD9, 0xF0}, {"FABS", 0xD9, 0xE1}, {"FCHS", 0xD9, 0xE0}, {"FCOS", 0xD9, 0xFF}, {"FSIN", 0xD9, 0xFE},
    {"FSCALE", 0xD9, 0xFD}, {"FSQRT", 0xD9, 0xFA}, {"FPTAN", 0xD9, 0xF2}, {"FPATAN", 0xD9, 0xF3},
    {"FXTRACT", 0xD9, 0xF4}, {"FPREM1", 0xD9, 0xF5}, {"FDECSTP", 0xD9, 0xF6}, {"FINCSTP", 0xD9, 0xF7},
    {"FUCOMPP", 0xDE, 0xF7}, {"FYL2X", 0xD9, 0xF1}, {"FYL2XP1", 0xD9, 0xF9},

    // AVX-512 ZMM ops
    {"VADDPS512_MASK", 0x62, 0xF1}, {"VSUBPS512_MASK", 0x62, 0xF5}, {"VMULPS512_MASK", 0x62, 0xF7}, {"VDIVPS512_MASK", 0x62, 0xFB},
    {"VADDPD512_MASK", 0x62, 0x58}, {"VSUBPD512_MASK", 0x62, 0x5C}, {"VMULPD512_MASK", 0x62, 0x59}, {"VDIVPD512_MASK", 0x62, 0x5E},

    // AVX-512 integer ops
    {"VPADDW512_MASK", 0x62, 0xFD}, {"VPADDD512_MASK", 0x62, 0xFE}, {"VPSUBW512_MASK", 0x62, 0xF9}, {"VPSUBD512_MASK", 0x62, 0xFA},
    {"VPMULLW512_MASK", 0x62, 0xD5}, {"VPMULUDQ512_MASK", 0x62, 0xF4}, {"VPAND512_MASK", 0x62, 0xDB}, {"VPOR512_MASK", 0x62, 0xEB},
    {"VPXOR512_MASK", 0x62, 0xEF},

    // AES / crypto instructions
    {"AESENC", 0x0F, 0x38}, {"AESENCLAST", 0x0F, 0x38}, {"AESDEC", 0x0F, 0x38}, {"AESDECLAST", 0x0F, 0x38},
    {"VAESENC", 0xC4, 0xE3}, {"VAESENCLAST", 0xC4, 0xE3}, {"VAESDEC", 0xC4, 0xE3}, {"VAESDECLAST", 0xC4, 0xE3},
    {"VPGATHERDD", 0xC4, 0xE3}, {"VPGATHERQD", 0xC4, 0xE3}, {"VPGATHERDQ", 0xC4, 0xE3}, {"VPGATHERQQ", 0xC4, 0xE3},

    // SHA / hash instructions
    {"SHA1MSG1", 0x0F, 0x38}, {"SHA1MSG2", 0x0F, 0x38}, {"SHA1NEXTE", 0x0F, 0x38}, {"SHA1RNDS4", 0x0F, 0x38},
    {"SHA256MSG1", 0x0F, 0x38}, {"SHA256MSG2", 0x0F, 0x38}, {"SHA256RNDS2", 0x0F, 0x38},

    // FPU rare / system instructions
    {"FISTTP", 0xDB, 0x07}, {"FNCLEX", 0xDB, 0xE2}, {"FNINIT", 0xDB, 0xE3}, {"FNSTCW", 0xD9, 0x07}, {"FNSTSW", 0xDF, 0xE0},
    {"FUCOMPP", 0xDE, 0xF7}, {"F2XM1", 0xD9, 0xF0}, {"FABS", 0xD9, 0xE1}, {"FSCALE", 0xD9, 0xFD},

    // System / privileged instructions
    {"SWAPGS", 0x0F, 0x01}, {"RDGSBASE", 0x0F, 0xAE}, {"WRGSBASE", 0x0F, 0xAE}, {"RDPID", 0x0F, 0x38}, {"RDTSCP", 0x0F, 0x01},
    {"XGETBV", 0x0F, 0x01}, {"XSAVE", 0x0F, 0xAE}, {"XSAVE64", 0x0F, 0xAE}, {"XRSTOR", 0x0F, 0xAE}, {"XRSTOR64", 0x0F, 0xAE},

    // Misc / other exotic ops
    {"MONITOR", 0x0F, 0x01}, {"MWAIT", 0x0F, 0x01}, {"CLWB", 0x0F, 0xAE}, {"CLFLUSHOPT", 0x0F, 0xAE}, {"PREFETCHWT1", 0x0F, 0x18},
    {"PTWRITE", 0x0F, 0x38}, {"RDPKRU", 0x0F, 0x01}, {"WRPKRU", 0x0F, 0x01},

    // FMA / AVX fused multiply-add
    {"VFMADD132PS", 0xC4, 0xE1}, {"VFMADD213PS", 0xC4, 0xE2}, {"VFMADD231PS", 0xC4, 0xE3},
    {"VFMADD132PD", 0xC4, 0xE9}, {"VFMADD213PD", 0xC4, 0xEA}, {"VFMADD231PD", 0xC4, 0xEB},
    {"VFMADD132SS", 0xC4, 0xF1}, {"VFMADD213SS", 0xC4, 0xF2}, {"VFMADD231SS", 0xC4, 0xF3},
    {"VFMADD132SD", 0xC4, 0xF9}, {"VFMADD213SD", 0xC4, 0xFA}, {"VFMADD231SD", 0xC4, 0xFB},

    // BMI1 / BMI2
    {"ANDN", 0x0F, 0x38}, {"BEXTR", 0x0F, 0xF3}, {"BLSI", 0x0F, 0x38}, {"BLSMSK", 0x0F, 0x38}, {"BLSR", 0x0F, 0x38},
    {"TZCNT", 0x0F, 0xBC}, {"LZCNT", 0x0F, 0xBD}, {"PDEP", 0x0F, 0x38}, {"PEXT", 0x0F, 0x38}, {"MULX", 0x0F, 0x38},

    // ADX (multi-precision add)
    {"ADCX", 0xF3, 0x0F}, {"ADOX", 0x66, 0x0F},

    // MPX (Memory Protection Extensions)
    {"BNDMOV", 0x0F, 0x01}, {"BNDLDX", 0x0F, 0x01}, {"BNDSTX", 0x0F, 0x01},

    // AES / crypto extensions
    {"AESENC", 0x0F, 0x38}, {"AESDEC", 0x0F, 0x38}, {"AESENCLAST", 0x0F, 0x38}, {"AESDECLAST", 0x0F, 0x38},
    {"AESKEYGENASSIST", 0x0F, 0x3A}, {"PCLMULQDQ", 0x0F, 0x3A},

    // AVX-512 extended
    {"VADDPS512_MASK", 0x62, 0xF1}, {"VMULPD512_MASK", 0x62, 0x59}, {"VSUBPS512_MASK", 0x62, 0xF5},
    {"VPAND512_MASK", 0x62, 0xDB}, {"VPOR512_MASK", 0x62, 0xEB}, {"VPXOR512_MASK", 0x62, 0xEF},

    // Masked move / blend
    {"VMOVDQA32_MASK", 0x62, 0xF2}, {"VMOVDQA64_MASK", 0x62, 0xF3}, {"VPBLENDMB", 0x62, 0xF4}, {"VPBLENDMW", 0x62, 0xF5},

    // Scatter / gather
    {"VPGATHERDD", 0x62, 0xF7}, {"VPGATHERDQ", 0x62, 0xF6}, {"VPGATHERQD", 0x62, 0xF5}, {"VPGATHERQQ", 0x62, 0xF4},
    {"VPSCATTERDD", 0x62, 0xFB}, {"VPSCATTERDQ", 0x62, 0xFA}, {"VPSCATTERQD", 0x62, 0xF9}, {"VPSCATTERQQ", 0x62, 0xF8},

    // More miscellaneous AVX / AVX2
    {"VBROADCASTSS", 0xC4, 0x78}, {"VBROADCASTSD", 0xC4, 0x79}, {"VBROADCASTF128", 0xC4, 0x7A}, {"VBROADCASTI32X4", 0xC4, 0x7B},
    {"VBROADCASTI64X4", 0xC4, 0x7C}, {"VPERMPS", 0xC4, 0x79}, {"VPERMPD", 0xC4, 0x7A}, {"VPERMQ", 0xC4, 0x7B}, {"VPERMT2PS", 0xC4, 0x7C},
    // AVX-512 masked ops / op variants
    {"VADDPD512_MASKZ", 0x62, 0x58}, {"VSUBPD512_MASKZ", 0x62, 0x5C}, {"VMULPD512_MASKZ", 0x62, 0x59},
    {"VDIVPD512_MASKZ", 0x62, 0x5E}, {"VPADDQ512_MASKZ", 0x62, 0xFD}, {"VPSUBQ512_MASKZ", 0x62, 0xF9},
    {"VPANDQ512_MASKZ", 0x62, 0xDB}, {"VPORQ512_MASKZ", 0x62, 0xEB}, {"VPXORQ512_MASKZ", 0x62, 0xEF},

    // FMA variants
    {"VFMADD132PD_MASK", 0xC4, 0xE9}, {"VFMADD213PD_MASK", 0xC4, 0xEA}, {"VFMADD231PD_MASK", 0xC4, 0xEB},
    {"VFMADD132PS_MASK", 0xC4, 0xE1}, {"VFMADD213PS_MASK", 0xC4, 0xE2}, {"VFMADD231PS_MASK", 0xC4, 0xE3},
    {"VFMADD132SD_MASK", 0xC4, 0xF9}, {"VFMADD213SD_MASK", 0xC4, 0xFA}, {"VFMADD231SD_MASK", 0xC4, 0xFB},
    {"VFMADD132SS_MASK", 0xC4, 0xF1}, {"VFMADD213SS_MASK", 0xC4, 0xF2}, {"VFMADD231SS_MASK", 0xC4, 0xF3},

    // Bit-manipulation / BMI2
    {"MULX_MASK", 0x0F, 0x38}, {"PDEP_MASK", 0x0F, 0x38}, {"PEXT_MASK", 0x0F, 0x38},
    {"BEXTR_MASK", 0x0F, 0xF3}, {"BLSI_MASK", 0x0F, 0x38}, {"BLSMSK_MASK", 0x0F, 0x38}, {"BLSR_MASK", 0x0F, 0x38},

    // ADX
    {"ADCX_MASK", 0xF3, 0x0F}, {"ADOX_MASK", 0x66, 0x0F},

    // AES / crypto
    {"AESENC_MASK", 0x0F, 0x38}, {"AESDEC_MASK", 0x0F, 0x38}, {"AESENCLAST_MASK", 0x0F, 0x38}, {"AESDECLAST_MASK", 0x0F, 0x38},
    {"AESKEYGENASSIST_MASK", 0x0F, 0x3A}, {"PCLMULQDQ_MASK", 0x0F, 0x3A},

    // Rare x87 / FPU
    {"FNCLEX", 0xDB, 0xE2}, {"FNINIT", 0xDB, 0xE3}, {"FNSAVE", 0xDD, 0xE0}, {"FNSTCW", 0xD9, 6},
    {"FNSTSW", 0xDF, 0xE0}, {"FWAIT", 0x9B, 0},

    // System instructions
    {"RMPUPDATE", 0x0F, 0x01}, {"ENCLS", 0x0F, 0x01}, {"ENCLU", 0x0F, 0x01}, {"XGETBV", 0x0F, 0x01},
    {"XSAVE", 0x0F, 0xAE}, {"XSAVE64", 0x0F, 0xAE}, {"XSAVEOPT", 0x0F, 0xAE}, {"XSAVES", 0x0F, 0xAE},
    {"XRSTOR", 0x0F, 0xAE}, {"XRSTORS", 0x0F, 0xAE},

    // Scatter / gather AVX-512 variants
    {"VGATHERDPD_MASK", 0x62, 0xF7}, {"VGATHERQPD_MASK", 0x62, 0xF6},
    {"VGATHERDPS_MASK", 0x62, 0xF5}, {"VGATHERQPS_MASK", 0x62, 0xF4},
    {"VSCATTERDPD_MASK", 0x62, 0xFB}, {"VSCATTERQPD_MASK", 0x62, 0xFA},
    {"VSCATTERDPS_MASK", 0x62, 0xF9}, {"VSCATTERQPS_MASK", 0x62, 0xF8},

    // Mask moves / blends / permutes
    {"VPMASKMOVD", 0x0F, 0x38}, {"VPMASKMOVQ", 0x0F, 0x38}, {"VPBLENDMD", 0x0F, 0x3A}, {"VPBLENDMQ", 0x0F, 0x3A},
    {"VPERMT2PD", 0xC4, 0x7C}, {"VPERMT2PS", 0xC4, 0x7C}, {"VPERMT2DQ", 0xC4, 0x7C},
    // AVX-512 FP conversions / moves
    {"VCVTDQ2PD512", 0x62, 0xF2}, {"VCVTPD2DQ512", 0x62, 0xF3}, {"VCVTDQ2PS512", 0x62, 0xF4},
    {"VCVTPS2DQ512", 0x62, 0xF5}, {"VCVTPS2PD512", 0x62, 0xF6}, {"VCVTPD2PS512", 0x62, 0xF7},

    // AVX-512 masked / zeroing moves
    {"VMOVDQA32_MASKZ", 0x62, 0x7C}, {"VMOVDQA64_MASKZ", 0x62, 0x7D},
    {"VMOVDQU32_MASKZ", 0x62, 0x7E}, {"VMOVDQU64_MASKZ", 0x62, 0x7F},

    // Vector shifts / rotates AVX-512
    {"VPSLLVD", 0x62, 0xF1}, {"VPSLLVQ", 0x62, 0xF2}, {"VPSRLVD", 0x62, 0xF3}, {"VPSRLVQ", 0x62, 0xF4},
    {"VPSRAVD", 0x62, 0xF5}, {"VPSLLVW", 0x62, 0xF6}, {"VPSRLVW", 0x62, 0xF7}, {"VPSRAVW", 0x62, 0xF8},

    // Vector compress / expand
    {"VPCOMPRESSD", 0x62, 0xF9}, {"VPCOMPRESSQ", 0x62, 0xFA},
    {"VPEXPANDD", 0x62, 0xFB}, {"VPEXPANDQ", 0x62, 0xFC},

    // FMA / FMS / fused ops
    {"VFMADD132PS512", 0x62, 0xF1}, {"VFMADD213PS512", 0x62, 0xF2}, {"VFMADD231PS512", 0x62, 0xF3},
    {"VFMADD132PD512", 0x62, 0xF4}, {"VFMADD213PD512", 0x62, 0xF5}, {"VFMADD231PD512", 0x62, 0xF6},

    {"VFMSUB132PS512", 0x62, 0xF7}, {"VFMSUB213PS512", 0x62, 0xF8}, {"VFMSUB231PS512", 0x62, 0xF9},
    {"VFMSUB132PD512", 0x62, 0xFA}, {"VFMSUB213PD512", 0x62, 0xFB}, {"VFMSUB231PD512", 0x62, 0xFC},

    // Vector permutes / shuffles
    {"VPERMD512", 0x62, 0xF1}, {"VPERMQ512", 0x62, 0xF2}, {"VPERMPD512", 0x62, 0xF3}, {"VPERMPS512", 0x62, 0xF4},

    // Vector compress / gather/scatter misc
    {"VGATHERQPD512_MASK", 0x62, 0xF5}, {"VGATHERDPD512_MASK", 0x62, 0xF6},
    {"VSCATTERQPD512_MASK", 0x62, 0xF7}, {"VSCATTERDPD512_MASK", 0x62, 0xF8},

    // Rare system instructions / privileged
    {"MONITOR", 0x0F, 0x01}, {"MWAIT", 0x0F, 0x01}, {"CLDEMOTE", 0x0F, 0x01}, {"CLWB", 0x0F, 0xAE},
    {"CLFLUSHOPT", 0x0F, 0xAE}, {"CLFLUSH", 0x0F, 0xAE}, {"PREFETCHNTA", 0x0F, 0x18}, {"PREFETCHT0", 0x0F, 0x18},
    {"PREFETCHT1", 0x0F, 0x18}, {"PREFETCHT2", 0x0F, 0x18},

    // Misc / NOP-like padding for multi-byte instructions
    {"NOPW", 0x66, 0x0F}, {"NOPD", 0x0F, 0x1F}, {"NOPQ", 0x0F, 0x1F},

    // AVX-512 masked / scatter-gather / conversions
    {"VPCMPD512_MASK", 0x62, 0xF9}, {"VPCMPQ512_MASK", 0x62, 0xFA},
    {"VPCMPEQD512", 0x62, 0xFB}, {"VPCMPEQQ512", 0x62, 0xFC}, {"VPCMPGTD512", 0x62, 0xFD},
    {"VPCMPGTQ512", 0x62, 0xFE}, {"VPCMPB512", 0x62, 0xFF},

    // AVX-512 floating point ops
    {"VREDUCEPD512", 0x62, 0xF1}, {"VREDUCESD512", 0x62, 0xF2}, {"VREDUCESS512", 0x62, 0xF3},
    {"VGETEXPPD512", 0x62, 0xF4}, {"VGETEXPS512", 0x62, 0xF5}, {"VGETEXPSD512", 0x62, 0xF6},

    // AVX / AVX2 vector ops
    {"VPMOVZXBD", 0xC5, 0x20}, {"VPMOVZXBW", 0xC5, 0x21}, {"VPMOVZXDQ", 0xC5, 0x22},
    {"VPMOVSXBD", 0xC5, 0x23}, {"VPMOVSXBW", 0xC5, 0x24}, {"VPMOVSXDQ", 0xC5, 0x25},
    {"VPERM2F128", 0xC5, 0x06}, {"VINSERTF128", 0xC5, 0x18}, {"VEXTRACTF128", 0xC5, 0x19},

    // SSE4.2 string / compare ops
    {"PCMPISTRI", 0x66, 0x3A}, {"PCMPISTRM", 0x66, 0x3A}, {"PCMPESTRI", 0x66, 0x3A},
    {"PCMPESTRM", 0x66, 0x3A}, {"CRC32", 0xF2, 0x0F}, {"POPCNT", 0xF3, 0x0F},

    // Crypto / AES / SHA
    {"AESENC", 0x0F, 0x38}, {"AESENCLAST", 0x0F, 0x38}, {"AESDEC", 0x0F, 0x38}, {"AESDECLAST", 0x0F, 0x38},
    {"AESIMC", 0x0F, 0x38}, {"AESKEYGENASSIST", 0x0F, 0x3A},
    {"SHA1MSG1", 0x0F, 0x38}, {"SHA1MSG2", 0x0F, 0x38}, {"SHA1NEXTE", 0x0F, 0x38}, {"SHA1RNDS4", 0x0F, 0x38},
    {"SHA256MSG1", 0x0F, 0x38}, {"SHA256MSG2", 0x0F, 0x38}, {"SHA256RNDS2", 0x0F, 0x38},

    // Rare x87 / floating-point
    {"FBLD", 0xDB, 0xC0}, {"FBSTP", 0xDF, 0xD0}, {"FNCLEX", 0xDB, 0xE2}, {"FNINIT", 0xDB, 0xE3},
    {"FCMOVB", 0xDA, 0xC0}, {"FCMOVE", 0xDA, 0xC8}, {"FCMOVBE", 0xDA, 0xD0}, {"FCMOVU", 0xDA, 0xD8},

    // Misc / padding / multi-byte NOPs
    {"NOPW", 0x66, 0x0F}, {"NOPD", 0x0F, 0x1F}, {"NOPQ", 0x0F, 0x1F}, {"PAUSE", 0xF3, 0x90},
    {"MFENCE", 0x0F, 0xAE}, {"LFENCE", 0x0F, 0xAE}, {"SFENCE", 0x0F, 0xAE},

    // System / privileged
    {"SGDT", 0x0F, 0x01}, {"SIDT", 0x0F, 0x01}, {"LGDT", 0x0F, 0x01}, {"LIDT", 0x0F, 0x01},
    {"SMSW", 0x0F, 0x01}, {"LMSW", 0x0F, 0x01}, {"INVD", 0x0F, 0x08}, {"WBINVD", 0x0F, 0x09},
    {"XRSTOR", 0x0F, 0xAE}, {"XSAVE", 0x0F, 0xAE}, {"XSAVEOPT", 0x0F, 0xAE}, {"XRSTORS", 0x0F, 0xAE},

    // Misc advanced SIMD / vector
    {"VPMOVDB", 0xC5, 0x38}, {"VPMOVDW", 0xC5, 0x39}, {"VPMOVQB", 0xC5, 0x3A}, {"VPMOVQD", 0xC5, 0x3B},
    {"VPMOVSWB", 0xC5, 0x3C}, {"VPMOVSDW", 0xC5, 0x3D}, {"VPMOVUSDB", 0xC5, 0x3E}, {"VPMOVUSDW", 0xC5, 0x3F},

    // More AVX-512 masked ops
    {"VPMASKMOVD", 0x62, 0xF0}, {"VPMASKMOVQ", 0x62, 0xF1}, {"VPMASKMOVPD", 0x62, 0xF2}, {"VPMASKMOVPS", 0x62, 0xF3},
    {"VPBLENDMD", 0x62, 0xF4}, {"VPBLENDMQ", 0x62, 0xF5}, {"VPBLENDMPD", 0x62, 0xF6}, {"VPBLENDMPS", 0x62, 0xF7},

    // AVX-512 scatter / gather / compress / expand
    {"VPGATHERDD", 0x62, 0xF8}, {"VPGATHERDQ", 0x62, 0xF9},
    {"VPGATHERQD", 0x62, 0xFA}, {"VPGATHERQQ", 0x62, 0xFB},
    {"VSCATTERDD", 0x62, 0xFC}, {"VSCATTERDQ", 0x62, 0xFD},
    {"VSCATTERQD", 0x62, 0xFE}, {"VSCATTERQQ", 0x62, 0xFF},

    // AVX-512 reduction / minmax ops
    {"VREDUCEMINPS", 0x62, 0xF0}, {"VREDUCEMAXPS", 0x62, 0xF1},
    {"VREDUCEMINPD", 0x62, 0xF2}, {"VREDUCEMAXPD", 0x62, 0xF3},
    {"VREDUCESUMPS", 0x62, 0xF4}, {"VREDUCESUMPD", 0x62, 0xF5},

    // AVX-512 masked gather/scatter ops
    {"VGATHERDPD", 0x62, 0xF6}, {"VGATHERDPS", 0x62, 0xF7},
    {"VGATHERQPD", 0x62, 0xF8}, {"VGATHERQPS", 0x62, 0xF9},
    {"VSCATTERDPD", 0x62, 0xFA}, {"VSCATTERDPS", 0x62, 0xFB},
    {"VSCATTERQPD", 0x62, 0xFC}, {"VSCATTERQPS", 0x62, 0xFD},

    // x87 FPU – transcendental & comparison ops
    {"F2XM1", 0xD9, 0xF0}, {"FYL2X", 0xD9, 0xF1}, {"FPTAN", 0xD9, 0xF2}, {"FPATAN", 0xD9, 0xF3},
    {"FXTRACT", 0xD9, 0xF4}, {"FPREM1", 0xD9, 0xF5}, {"FDECSTP", 0xD9, 0xF6}, {"FINCSTP", 0xD9, 0xF7},
    {"FPREM", 0xD9, 0xF8}, {"FYL2XP1", 0xD9, 0xF9}, {"FSCALE", 0xD9, 0xFD}, {"FSQRT", 0xD9, 0xFA},
    {"FSTENV", 0xD9, 0xC0}, {"FLDENV", 0xD9, 0xC0},

    // Misc / rarely used instructions
    {"MOVBE", 0x0F, 0x38}, {"PREFETCHNTA", 0x0F, 0x18}, {"PREFETCHT0", 0x0F, 0x18},
    {"PREFETCHT1", 0x0F, 0x18}, {"PREFETCHT2", 0x0F, 0x18}, {"MONITOR", 0x0F, 0x01},
    {"MWAIT", 0x0F, 0x01}, {"CLFLUSH", 0x0F, 0xAE}, {"CLFLUSHOPT", 0x0F, 0xAE},

    // XSAVE / XRSTOR variants
    {"XSAVEC", 0x0F, 0xAE}, {"XSAVES", 0x0F, 0xAE}, {"XRSTORS", 0x0F, 0xAE}, {"XRSTOR64", 0x0F, 0xAE},

    // System / control registers
    {"MOV_CR0", 0x0F, 0x20}, {"MOV_CR2", 0x0F, 0x20}, {"MOV_CR3", 0x0F, 0x20}, {"MOV_CR4", 0x0F, 0x20},
    {"MOV_DR0", 0x0F, 0x21}, {"MOV_DR1", 0x0F, 0x21}, {"MOV_DR2", 0x0F, 0x21}, {"MOV_DR3", 0x0F, 0x21},
    {"MOV_DR6", 0x0F, 0x21}, {"MOV_DR7", 0x0F, 0x21},

    // Privileged instructions
    {"SGDT", 0x0F, 0x01}, {"SIDT", 0x0F, 0x01}, {"LGDT", 0x0F, 0x01}, {"LIDT", 0x0F, 0x01},
    {"SMSW", 0x0F, 0x01}, {"LMSW", 0x0F, 0x01}, {"INVLPG", 0x0F, 0x01},

    // Multi-byte / padding / NOP variants
    {"NOP1", 0x90, 0x00}, {"NOP2", 0x66, 0x90}, {"NOP3", 0x0F, 0x1F}, {"NOP4", 0x0F, 0x1F, 0x00},
    {"NOP5", 0x0F, 0x1F, 0x40, 0x00}, {"NOP6", 0x0F, 0x1F, 0x44, 0x00, 0x00}, {"NOP7", 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00},
    {"NOP8", 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00}, {"NOP9", 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00},

    {"", 0, 0} 
};



Reg32 regs[] = {
    {"EAX",0}, {"ECX",1}, {"EDX",2}, {"EBX",3},
    {"ESP",4}, {"EBP",5}, {"ESI",6}, {"EDI",7},
    {"AX",0}, {"CX",1}, {"DX",2}, {"BX",3},
    {"SP",4}, {"BP",5}, {"SI",6}, {"DI",7},
    {"AL",0}, {"CL",1}, {"DL",2}, {"BL",3},
    {"AH",4}, {"CH",5}, {"DH",6}, {"BH",7},

    {"XMM0",0}, {"XMM1",1}, {"XMM2",2}, {"XMM3",3},
    {"XMM4",4}, {"XMM5",5}, {"XMM6",6}, {"XMM7",7},

    {"",0} // terminator
};

const char* directives[] = {
    "SECTION",
    "DATA",
    "TEXT",
    "GLOBAL",
    "EXTERN"
};

#define NUM_DIRECTIVES (sizeof(directives)/sizeof(directives[0]))


#define NUM_OPCODES (sizeof(opcodes) / sizeof(opcodes[0]))

Label labels[MAX_LABELS];
int label_count=0;

void strtoupper(char *s){
    for(int i=0;s[i];i++) s[i]=toupper(s[i]);
}

int tokenize(char *line, char tokens[MAX_TOKENS][32]){
    int count=0;
    char *p = strtok(line," ,\t\n");
    while(p && count<MAX_TOKENS){
        strtoupper(p);
        strcpy(tokens[count++],p);
        p=strtok(NULL," ,\t\n");
    }
    return count;
}

OpCode32* find_opcode(const char *mnemonic, int operand_count) {
    for (int i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(opcodes[i].name, mnemonic) == 0) {
            return &opcodes[i]; // return first match, ignore operand_count
        }
    }
    return NULL;
}


int find_reg(char *name){
    for(int i=0; regs[i].name[0]; i++)
        if(strcmp(regs[i].name,name)==0) return regs[i].code;
    return -1;
}

int find_label(char *name){
    for(int i=0;i<label_count;i++)
        if(strcmp(labels[i].name,name)==0) return labels[i].addr;
    return -1;
}

void add_label(char *name, int addr){
    strcpy(labels[label_count].name,name);
    labels[label_count].addr = addr;
    label_count++;
}

int is_label(char *token){
    int len = strlen(token);
    return token[len-1]==':';
}

int assemble_line(char tokens[MAX_TOKENS][32], int count, unsigned char *out, int pos) {
    if (count == 0) return 0;

    // Skip comments
    if (tokens[0][0] == ';') return 0;

    // Skip directives
    for (int i = 0; i < NUM_DIRECTIVES; i++) {
        if (strcmp(tokens[0], directives[i]) == 0) {
            return 0; // optionally handle directives later
        }
    }

    // Handle label
    if (is_label(tokens[0])) {
        tokens[0][strlen(tokens[0])-1] = 0; // remove colon
        add_label(tokens[0], pos);
        return 0;
    }

    // Find opcode
    OpCode32 *op = find_opcode(tokens[0], count);
    if (!op) {
        fprintf(stderr, "Unknown instruction or unsupported operand combination: %s\n", tokens[0]);
        return 0; // skip instead of exit
    }

    int len = 0;
    int r1 = (count > 1) ? find_reg(tokens[1]) : -1;
    int r2 = (count > 2) ? find_reg(tokens[2]) : -1;

    // Single-byte instructions
    if (count == 1) {
        if (out) out[0] = op->opcode1;
        return 1;
    }

    // Two-operand instructions
    if (count == 3) {
        if (r1 >= 0 && r2 >= 0) { // reg -> reg or xmm -> xmm
            if (out) {
                out[0] = op->opcode_reg_reg != 0 ? op->opcode_reg_reg : op->opcode1;
                out[1] = 0xC0 | (r2 << 3) | r1; // ModRM
            }
            len = 2;
        } else if (r1 >= 0) { // reg -> imm
            int imm = atoi(tokens[2]);
            if (out) {
                out[0] = op->opcode_reg_imm + r1;
                out[1] = imm & 0xFF;
                out[2] = (imm >> 8) & 0xFF;
                out[3] = (imm >> 16) & 0xFF;
                out[4] = (imm >> 24) & 0xFF;
            }
            len = 5;
        } else {
            fprintf(stderr, "Cannot encode operands: %s %s %s\n", tokens[0], tokens[1], tokens[2]);
            return 0;
        }
    }
    // One-operand instructions
    else if (count == 2) {
        if ((strcmp(op->name, "JMP") == 0 || strcmp(op->name, "CALL") == 0)) {
            int addr = find_label(tokens[1]);
            if(out) out[0] = op->opcode1;
            if(addr >= 0) {
                int rel = addr - (pos + 5);
                if(out) {
                    out[1] = rel & 0xFF;
                    out[2] = (rel >> 8) & 0xFF;
                    out[3] = (rel >> 16) & 0xFF;
                    out[4] = (rel >> 24) & 0xFF;
                }
            } else if(out) {
                out[1] = out[2] = out[3] = out[4] = 0;
            }
            len = 5;
        } else if(r1 >= 0) { // INC r32 or similar
            if(out) out[0] = op->opcode_reg_reg + r1;
            len = 1;
        } else {
            fprintf(stderr, "Cannot encode line: %s %s\n", tokens[0], tokens[1]);
            return 0;
        }
    }

    return len;
}


void assemble_file(const char *infile, const char *outfile){
    FILE *f=fopen(infile,"r");
    if(!f){ perror("fopen"); exit(1);}
    FILE *o=fopen(outfile,"wb");
    if(!o){ perror("fopen"); exit(1);}
    
    char line[MAX_LINE];
    unsigned char out[MAX_OUT];
    int pos=0;
    // first pass: detect labels
    while(fgets(line,sizeof(line),f)){
        char tokens[MAX_TOKENS][32];
        int count = tokenize(line,tokens);
        assemble_line(tokens,count,NULL,pos);
        pos+=5; // rough estimate
    }
    rewind(f);
    pos=0;
    while(fgets(line,sizeof(line),f)){
        char tokens[MAX_TOKENS][32];
        int count = tokenize(line,tokens);
        int len = assemble_line(tokens,count,out,pos);
        fwrite(out,1,len,o);
        pos+=len;
    }
    fclose(f); fclose(o);
}

int main(int argc, char **argv){
    if(argc<3){ printf("Usage: sasm input.asm output.bin\n"); return 1;}
    assemble_file(argv[1],argv[2]);
    printf("Assembled %s -> %s\n",argv[1],argv[2]);
    return 0;
}
