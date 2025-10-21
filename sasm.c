;
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
    unsigned char opcode_reg_reg;    // <-- add this
    unsigned char opcode_reg_mem;    // optional, if your code needs it
    unsigned char opcode_mem_reg;    // optional
    unsigned char opcode_reg_imm;
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

    // Stack / push-pop
    {"PUSH", 0x50, 0}, {"POP", 0x58, 0}, {"PUSHA", 0x60, 0}, {"POPA", 0x61, 0},
    {"PUSHAD", 0x60, 0}, {"POPAD", 0x61, 0}, {"PUSHF", 0x9C, 0}, {"POPF", 0x9D, 0}, {"LAHF", 0x9F, 0}, {"SAHF", 0x9E, 0},

    // Control flow / jumps
    {"JMP", 0xE9, 0}, {"JE", 0x0F, 0x84}, {"JZ", 0x0F, 0x84}, {"JNE", 0x0F, 0x85}, {"JNZ", 0x0F, 0x85},
    {"JG", 0x0F, 0x8F}, {"JGE", 0x0F, 0x8D}, {"JL", 0x0F, 0x8C}, {"JLE", 0x0F, 0x8E},
    {"JB", 0x0F, 0x82}, {"JNB", 0x0F, 0x83}, {"JO", 0x0F, 0x80}, {"JNO", 0x0F, 0x81}, {"JS", 0x0F, 0x88}, {"JNS", 0x0F, 0x89},
    {"JCXZ", 0xE3, 0}, {"LOOP", 0xE2, 0}, {"LOOPE", 0xE1, 0}, {"LOOPNE", 0xE0, 0}, {"CALL", 0xE8, 0}, {"RET", 0xC3, 0}, {"IRET", 0xCF, 0}, {"INT", 0xCD, 0},

    // Flags
    {"CLC", 0xF8, 0}, {"STC", 0xF9, 0}, {"CMC", 0xF5, 0}, {"CLD", 0xFC, 0}, {"STD", 0xFD, 0}, {"STI", 0xFB, 0}, {"CLI", 0xFA, 0},

    // NOP / Misc
    {"NOP", 0x90, 0}, {"HLT", 0xF4, 0}, {"WAIT", 0x9B, 0}, {"ESC", 0xD8, 0}, {"INVLPG", 0x0F, 0x01}, {"CPUID", 0x0F, 0xA2},
    {"RDTSC", 0x0F, 0x31}, {"RDMSR", 0x0F, 0x32}, {"WRMSR", 0x0F, 0x30}, {"UD2", 0x0F, 0x0B},

    // x87 FPU
    {"FADD", 0xD8, 0}, {"FMUL", 0xD8, 1}, {"FSUB", 0xD8, 4}, {"FDIV", 0xD8, 6},
    {"FLD", 0xD9, 0}, {"FST", 0xD9, 2}, {"FSTP", 0xD9, 3}, {"FILD", 0xDB, 0}, {"FISTP", 0xDB, 3},
    {"FCLEX", 0xDB, 0xE2}, {"FSTCW", 0xD9, 5}, {"FLDCW", 0xD9, 6}, {"FNSTSW", 0xDF, 0xE0},
    
    // SSE / SSE2 / SSE3 / SSE4
    {"MOVAPS", 0x0F, 0x28}, {"MOVUPS", 0x0F, 0x10}, {"MOVAPD", 0x0F, 0x28}, {"MOVUPD", 0x0F, 0x10},
    {"ADDPS", 0x0F, 0x58}, {"ADDPD", 0x0F, 0x58}, {"SUBPS", 0x0F, 0x5C}, {"SUBPD", 0x0F, 0x5C},
    {"MULPS", 0x0F, 0x59}, {"MULPD", 0x0F, 0x59}, {"DIVPS", 0x0F, 0x5E}, {"DIVPD", 0x0F, 0x5E},
    {"ANDPS", 0x0F, 0x54}, {"ANDPD", 0x0F, 0x54}, {"ORPS", 0x0F, 0x56}, {"ORPD", 0x0F, 0x56},
    {"XORPS", 0x0F, 0x57}, {"XORPD", 0x0F, 0x57},

    // MMX
    {"MOVD", 0x0F, 0x6E}, {"MOVQ", 0x0F, 0x7E}, {"PADDW", 0x0F, 0xFD}, {"PADDD", 0x0F, 0xFE},
    {"PSUBW", 0x0F, 0xF9}, {"PSUBD", 0x0F, 0xFA}, {"PMULLW", 0x0F, 0xD5}, {"PMULUDQ", 0x0F, 0xF4},
    {"PAND", 0x0F, 0xDB}, {"POR", 0x0F, 0xEB}, {"PXOR", 0x0F, 0xEF}, {"PSLLW", 0x0F, 0xF1},
    {"PSLLD", 0x0F, 0xF2}, {"PSRLW", 0x0F, 0xD1}, {"PSRLD", 0x0F, 0xD2}, {"PSRAW", 0x0F, 0xE1}, {"PSRAD", 0x0F, 0xE2},

    // AVX / AVX2 (VEX-prefixed)
    {"VMOVAPS", 0xC5, 0xF8}, {"VMOVUPS", 0xC5, 0xF0}, {"VADDPS", 0xC5, 0xF8}, {"VSUBPS", 0xC5, 0xFA},
    {"VMULPS", 0xC5, 0xFB}, {"VDIVPS", 0xC5, 0xFD}, {"VANDPS", 0xC5, 0xFC}, {"VORPS", 0xC5, 0xFE}, {"VXORPS", 0xC5, 0xFF},
    {"VADDPD", 0xC5, 0x58}, {"VSUBPD", 0xC5, 0x5C}, {"VMULPD", 0xC5, 0x59}, {"VDIVPD", 0xC5, 0x5E},

    // AVX integer / MMX extensions
    {"VPADDW", 0xC5, 0xFD}, {"VPADDD", 0xC5, 0xFE}, {"VPSUBW", 0xC5, 0xF9}, {"VPSUBD", 0xC5, 0xFA},
    {"VPMULLW", 0xC5, 0xD5}, {"VPMULUDQ", 0xC5, 0xF4}, {"VPAND", 0xC5, 0xDB}, {"VPOR", 0xC5, 0xEB}, {"VPXOR", 0xC5, 0xEF},

    // Segment overrides
    {"CS:", 0x2E, 0}, {"SS:", 0x36, 0}, {"DS:", 0x3E, 0}, {"ES:", 0x26, 0}, {"FS:", 0x64, 0}, {"GS:", 0x65, 0},

    // Extra / rarely used
    {"BOUND", 0x62, 0}, {"ARPL", 0x63, 0}, {"SALC", 0xD6, 0}, {"XLATB", 0xD7, 0},
    {"ENTER", 0xC8, 0}, {"LEAVE", 0xC9, 0}, {"INSB", 0x6C, 0}, {"INSW", 0x6D, 0}, {"OUTSB", 0x6E, 0}, {"OUTSW", 0x6F, 0},

    // AVX-512 (VEX + EVEX prefixes)
    {"VADDPS512", 0x62, 0xF1}, {"VSUBPS512", 0x62, 0xF5}, {"VMULPS512", 0x62, 0xF7}, {"VDIVPS512", 0x62, 0xFB},
    {"VADDPD512", 0x62, 0x58}, {"VSUBPD512", 0x62, 0x5C}, {"VMULPD512", 0x62, 0x59}, {"VDIVPD512", 0x62, 0x5E},
    {"VPADDW512", 0x62, 0xFD}, {"VPADDD512", 0x62, 0xFE}, {"VPSUBW512", 0x62, 0xF9}, {"VPSUBD512", 0x62, 0xFA},
    {"VPMULLW512", 0x62, 0xD5}, {"VPMULUDQ512", 0x62, 0xF4}, {"VPAND512", 0x62, 0xDB}, {"VPOR512", 0x62, 0xEB}, {"VPXOR512", 0x62, 0xEF},

    // SSE4.2
    {"PCMPEQQ", 0x66, 0x0F}, {"PCMPISTRI", 0x66, 0x3A}, {"PCMPISTRM", 0x66, 0x3A},
    {"CRC32", 0xF2, 0x0F}, {"POPCNT", 0xF3, 0x0F},

    // Rare x87 instructions
    {"FCOMPP", 0xDE, 0xF7}, {"FCOMP", 0xD8, 0xD8}, {"FCOM", 0xD8, 0xD0}, {"FDECSTP", 0xD9, 0xF6}, {"FINCSTP", 0xD9, 0xF7},
    {"FSIN", 0xD9, 0xFE}, {"FCOS", 0xD9, 0xFF}, {"FPTAN", 0xD9, 0xF2}, {"FPATAN", 0xD9, 0xF3}, {"FYL2X", 0xD9, 0xF1},    
    {"FYL2XP1", 0xD9, 0xF9}, {"FSCALE", 0xD9, 0xFD}, {"FSQRT", 0xD9, 0xFA}, {"FCHS", 0xD9, 0xE0}, {"FABS", 0xD9, 0xE1},
    {"FTST", 0xD9, 0xE4}, {"FXAM", 0xD9, 0xE5}, {"FLD1", 0xD9, 0xE8}, {"FLDL2T", 0xD9, 0xEA}, {"FLDL2E", 0xD9, 0xEB},
    {"FLDPI", 0xD9, 0xE9}, {"FLDLG2", 0xD9, 0xEC}, {"FLDLN2", 0xD9, 0xED}, {"FLDZ", 0xD9, 0xEE},

    {"", 0, 0} // End marker
};


Reg32 regs[] = {
    {"EAX",0}, {"ECX",1}, {"EDX",2}, {"EBX",3},
    {"ESP",4}, {"EBP",5}, {"ESI",6}, {"EDI",7},
    {"AX",0}, {"CX",1}, {"DX",2}, {"BX",3},
    {"SP",4}, {"BP",5}, {"SI",6}, {"DI",7},
    {"AL",0}, {"CL",1}, {"DL",2}, {"BL",3},
    {"AH",4}, {"CH",5}, {"DH",6}, {"BH",7},
    {"",0}
};

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

OpCode32* find_opcode(char *name){
    for(int i=0; opcodes[i].name[0]; i++)
        if(strcmp(opcodes[i].name,name)==0) return &opcodes[i];
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

    // Handle label
    if (is_label(tokens[0])) {
        tokens[0][strlen(tokens[0])-1] = 0; // remove colon
        add_label(tokens[0], pos);
        return 0;
    }

    OpCode32 *op = find_opcode(tokens[0]);
    if (!op) { 
        printf("Unknown instruction: %s\n", tokens[0]);
        exit(1);
    }

    int len = 0;

    // Single-byte instructions
    if (count == 1) {
        out[0] = op->opcode1;
        return 1;
    }

    int r1 = find_reg(tokens[1]);

    // Two-operand instructions
    if (count == 3) {
        int r2 = find_reg(tokens[2]);

        // reg -> reg
        if (r1 >= 0 && r2 >= 0) {
            out[0] = op->opcode_reg_reg;
            out[1] = (r2 << 3) | r1; // ModRM
            len = 2;
        }
        // reg -> imm
        else if (r1 >= 0) {
            int imm = atoi(tokens[2]);
            out[0] = op->opcode_reg_imm + r1;
            // Determine immediate size (default 4 bytes)
            if (op->opcode_reg_imm >= 0xB0 && op->opcode_reg_imm <= 0xB7) {
                out[1] = imm & 0xFF; // 8-bit
                len = 2;
            } else if (op->opcode_reg_imm == 0xB8) {
                out[1] = imm & 0xFF;
                out[2] = (imm >> 8) & 0xFF;
                out[3] = (imm >> 16) & 0xFF;
                out[4] = (imm >> 24) & 0xFF; // 32-bit
                len = 5;
            } else {
                out[1] = imm & 0xFF;
                out[2] = (imm >> 8) & 0xFF;
                out[3] = (imm >> 16) & 0xFF;
                out[4] = (imm >> 24) & 0xFF;
                len = 5;
            }
        } else {
            printf("Cannot encode operands: %s %s %s\n", tokens[0], tokens[1], tokens[2]);
            exit(1);
        }
    }
    // One-operand instructions (like JMP label)
    else if (count == 2) {
        if (strcmp(op->name,"JMP")==0 || strcmp(op->name,"CALL")==0) {
            int addr = find_label(tokens[1]);
            out[0] = op->opcode1;
            if (addr >= 0) {
                int rel = addr - (pos + 5); // 32-bit relative
                out[1] = rel & 0xFF;
                out[2] = (rel >> 8) & 0xFF;
                out[3] = (rel >> 16) & 0xFF;
                out[4] = (rel >> 24) & 0xFF;
            } else {
                out[1] = out[2] = out[3] = out[4] = 0; // unresolved
            }
            len = 5;
        } else if (r1 >= 0) { // e.g., INC r32
            out[0] = op->opcode_reg_reg + r1;
            len = 1;
        } else {
            printf("Cannot encode line: %s %s\n", tokens[0], tokens[1]);
            exit(1);
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
