// op a b c
//  4 4 4 4

// 16 x 16 bit registers
// R0~14
// PC

// 8 x 8 bit segment registers
// S0~6
// CS

//  0    : CMOV : if( b != 0 ) c = a        : A0 = c, A1 = r[a], A2 = r[b]
//  1    : ADDI : c += ab                   : A0 = c, A1 = r[c], A2 = ab
//  2    : SUBI : c -= ab                   : A0 = c, A1 = r[c], A2 = ab
//  3    : MULI : c *= ab                   : A0 = c, A1 = r[c], A2 = ab
//  4    : SHLI : c <<= ab                  : A0 = c, A1 = r[c], A2 = ab
//  5    : SHRI : c >>= ab                  : A0 = c, A1 = r[c], A2 = ab
//  6    : ASRI : c >>= ab                  : A0 = c, A1 = r[c], A2 = ab
//  7    : ANDI : c &= ab                   : A0 = c, A1 = r[c], A2 = ab
//  8    : ORI  : c |= ab                   : A0 = c, A1 = r[c], A2 = ab
//  9    : XORI : c ^= ab                   : A0 = c, A1 = r[c], A2 = ab
// 10    : LDH  : c = ab << 8               : A0 = c, A1 =  ?  , A2 = ab
// 11    : LDR  : c = mem[s[b]:a]           : A0 = c, A1 = r[a], A2 = s[b]
// 12    : STR  : mem[s[b]:a] = c           : A0 = c, A1 = r[a], A2 = s[b], A3 = r[c]
// 13    : SLT  : c = a < b                 : A0 = c, A1 = r[a], A2 = r[b]
// 14    : SLTS : c = a < b                 : A0 = c, A1 = r[a], A2 = r[b]
// 15;0  : MOV  : c = b                     : A0 = c, A1 =  ?  , A2 = r[b]
// 15;1  : ADD  : c += b                    : A0 = c, A1 = r[c], A2 = r[b]
// 15;2  : SUB  : c -= b                    : A0 = c, A1 = r[c], A2 = r[b]
// 15;3  : MUL  : c *= b                    : A0 = c, A1 = r[c], A2 = r[b]
// 15;4  : SHL  : c <<= b                   : A0 = c, A1 = r[c], A2 = r[b]
// 15;5  : SHR  : c >>= b                   : A0 = c, A1 = r[c], A2 = r[b]
// 15;6  : ASR  : c >>= b                   : A0 = c, A1 = r[c], A2 = r[b]
// 15;7  : AND  : c &= b                    : A0 = c, A1 = r[c], A2 = r[b]
// 15;8  : OR   : c |= b                    : A0 = c, A1 = r[c], A2 = r[b]
// 15;9  : XOR  : c ^= b                    : A0 = c, A1 = r[c], A2 = r[b]
// 15:10 : NOT  : c = ~b                    : A0 = c, A1 =  ?  , A2 = r[b]
// 15;11 : STS  : c = s[b]                  : A0 = c, A1 =  ?  , A2 = s[b]
// 15;12 : LDS  : s[b] = c                  : A0 = c, A1 = r[c], A2 = b
// 15;13 : LJMP : s[7] = s[b], r[15] = r[c] : A0 = ?, A1 = r[c], A2 = s[b]
// 15;14 : ?    : ?                         : A0 = ?, A1 =  ?  , A2 = ?
// 15;15 : ?    : ?                         : A0 = ?, A1 =  ?  , A2 = ?

// LDR/STR only, segment index bit 3 = auto-increment address register

// pipeline registers
//   FN [5]
//   A0 [4]
//   A1 [16]
//   A2 [16]
//   A3 [16]
//   IP [24]
//   WBS
//   WBR
//   LDR
//   STR

// fetch stage
//   FT_IR = mem[FT_IP]
//   FT_IP = FT_IP + 1

// decode stage, needs forwarding muxes
//   DC_OP = FT_IR[12:15]
//   DC_A = FT_IR[8:11]
//   DC_B = FT_IR[4:7]
//   DC_C = FT_IR[0:3]
//   DC_EXOP = ( DC_OP == 15 )
//   DC_FN[5] = DC_EXOP
//   DC_FN[0:4] = DC_EXOP ? DC_A : DC_OP
//   DC_A0 = DC_C
//   DC_RDA = ( DC_FN[0:4] == 0 || DC_FN[0:4] == 11 || DC_FN[0:4] == 12 || DC_FN[0:4] == 13 || DC_FN[0:5] == 14 )
//   DC_A1 = DC_RDA ? r[DC_A] : r[DC_C]
//   DC_RDB = ( DC_FN[0:5] == 27 || ( DC_FN[0:5] > 0 && DC_FN[0:5] < 11 ) )
//   DC_SGB = ( DC_FN[0:4] == 11 || DC_FN[0:4] == 12 || DC_FN[0:5] == 30 )
//   DC_A2 = DC_RDB ? ( DC_SGB ? s[DC_B] : r[DC_B] ) : { DC_A, DC_B }
//   DC_STB = ( DC_FN[[0:5] == 12 )
//   DC_A3 = r[c] // ? DC_STB
//   DC_IP = FT_IP

// execute stage
//   EX_RES = fn( DC_FN, DC_A1, DC_A2 )
//   EX_FN = DC_FN

// memory stage
//   MEM_RES = EX_RES

// writeback stage
//   r[c] = MEM_RES

// comparisons
// A <  B  ==  A < B
// A >  B  ==  B < A
// A <= B  ==  !( B < A )
// A >= B  ==  !( A < B )
// A == B  ==  !( A ^ B )
// A != B  ==  A ^ B

// ROM
// 00:0000

// RAM
// 80:0000

// VRAM
// A0:0000

// CONTROL REGISTERS
// C0:0000

// psuedo-ops
// LDI = LDH, ORI
// JMP = ADDI/SUBI r15, offset
// JNZ = CMOV r15, dest, cond
// ALU(I)
// PUSH = SUBI r14, 1 ; STR reg, [s6:r14]
// POP = LDR reg, [s6:r14+]
// RET = MOV r15, r13
// CALL = MOV r13, r15 ; ADDI r13, 2 ; JMP label

// r0-7  args / caller saved
// r8-12 callee saved
// r13   lr
// r14   sp
// r15   pc
// s0-2  args / caller saved
// s4-5  callee saved
// s6    ss
// s7    cs

// pipeline
//   fetch
//     IR, IP
//   decode
//     IR, IP, A0, A1, A2
//   execute
//     IR, IP, R0, R1
//   memory
//     IR, IP, R0, R1
//   writeback
//     IR, IP

// pipelined multiply
// 5 * 300
// 5 * 44 + 5 * 256 + 0 * 44 + 0 * 256
// r = l1 * l2 +
//     l1 * h2 +
//     h1 * l2 +
//     h1 * h2

#define OP_CMOV 0
#define OP_ADDI 1
#define OP_SUBI 2
#define OP_MULI 3
#define OP_SHLI 4
#define OP_SHRI 5
#define OP_ASRI 6
#define OP_ANDI 7
#define OP_ORI  8
#define OP_XORI 9
#define OP_LDH  10
#define OP_LDR  11
#define OP_STR  12
#define OP_SLT  13
#define OP_SLTS 14
#define OP_EXT  15
#define OP_MOV  0
#define OP_ADD  1
#define OP_SUB  2
#define OP_MUL  3
#define OP_SHL  4
#define OP_SHR  5
#define OP_ASR  6
#define OP_AND  7
#define OP_OR   8
#define OP_XOR  9
#define OP_NOT  10
#define OP_LNOT 11
#define OP_LDS  12
#define OP_STS  13
#define OP_LJMP 14

#define CMOV 0
#define ADDI 1
#define SUBI 2
#define MULI 3
#define SHLI 4
#define SHRI 5
#define ASRI 6
#define ANDI 7
#define ORI  8
#define XORI 9
#define LDH  10
#define LDR  11
#define STR  12
#define SLT  13
#define SLTS 14
#define MOV  16
#define ADD  17
#define SUB  18
#define MUL  19
#define SHL  20
#define SHR  21
#define ASR  22
#define AND  23
#define OR   24
#define XOR  25
#define NOT  26
#define LNOT 27
#define LDS  28
#define STS  29
#define LJMP 30

#define R0  0
#define R1  1
#define R2  2
#define R3  3
#define R4  4
#define R5  5
#define R6  6
#define R7  7
#define R8  8
#define R9  9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15

#define LR 13
#define SP 14
#define PC 15

#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define S5 5
#define S6 6
#define S7 7

#define SS 6
#define CS 7

const char* opNames[] = {
	"CMOV",
	"ADDI",
	"SUBI",
	"MULI",
	"SHLI",
	"SHRI",
	"ASRI",
	"ANDI",
	"ORI",
	"XORI",
	"LDH",
	"LDR",
	"STR",
	"SLT",
	"SLTS",
	"EXT_OP",
	"MOV",
	"ADD",
	"SUB",
	"MUL",
	"SHL",
	"SHR",
	"ASR",
	"AND",
	"OR",
	"XOR",
	"NOT",
	"LNOT",
	"LDS",
	"STS",
	"LJMP"
};
