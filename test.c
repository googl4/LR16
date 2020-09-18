#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "isa.h"

#define INSTR( op, a, b, c ) ( ( (op) << 12 ) | ( (a) << 8 ) | ( (b) << 4 ) | (c) )
#define INSTR_IMM( op, c, ab ) ( ( (op) << 12 ) | ( (ab) << 4 ) | (c) )
#define INSTR_EXT( op, c, b ) ( 0xF000 | ( ( (op) - 16 ) << 8 ) | ( (b) << 4 ) | (c) )

int main( int argc, char* argv[] ) {
	u16 bin[8192];
	u16 blen = 0;
	
	/*
	bin[0]  = INSTR_IMM( ORI,  R0,  0x80     );
	bin[1]  = INSTR_EXT( LDS,  R0,  S0       );
	bin[2]  = INSTR_IMM( ORI,  R1,  0xA0     );
	bin[3]  = INSTR_EXT( LDS,  R1,  S1       );
	bin[4]  = INSTR_IMM( LDH,  R2,  0x07     );
	bin[5]  = INSTR_IMM( ORI,  R2,  0x80     );
	bin[6]  = INSTR_IMM( ORI,  R3,  0x55     );
	bin[7]  = INSTR_IMM( ORI,  R6,  0x08     );
	bin[8]  = INSTR(     STR,  R4,  S1,  R3  );
	bin[9]  = INSTR_IMM( XORI, R3,  0x01     );
	bin[10] = INSTR_IMM( ADDI, R4,  0x01     );
	bin[11] = INSTR(     SLT,  R4,  R2,  R5  );
	bin[12] = INSTR(     CMOV, R6,  R5,  R15 );
	
	bin[13] = INSTR_IMM( SUBI, R15, 0x01     );
	blen = 14;
	*/
	
	/*
	bin[0]  = INSTR_IMM( ORI,  R0,  0xA0     );
	bin[1]  = INSTR_EXT( LDS,  R0,  S0       );
	bin[2]  = INSTR_IMM( LDH,  R1,  0xE0     );
	bin[3]  = INSTR_IMM( ORI,  R2,  0x04     );
	bin[4]  = INSTR(     STR,  R3,  S0,  R3  );
	bin[5]  = INSTR_IMM( ADDI, R3,  0x01     );
	bin[6]  = INSTR(     SLT,  R3,  R1,  R4  );
	bin[7]  = INSTR(     CMOV, R2,  R4,  R15 );
	bin[8]  = INSTR_IMM( SUBI, R15, 0x01     );
	blen = 9;
	*/
	
	/*
	bin[0]  = INSTR_IMM( ORI,  R0,  0xA0     );
	bin[1]  = INSTR_EXT( LDS,  R0,  S0       );
	bin[2]  = INSTR_IMM( ORI,  R1,  0xC0     );
	bin[3]  = INSTR_EXT( LDS,  R1,  S1       );
	bin[4]  = INSTR_IMM( LDH,  R2,  0xE0     );
	bin[5]  = INSTR_IMM( ORI,  R3,  0x00     );
	bin[6]  = INSTR_IMM( ORI,  R4,  0x07     );
	
	bin[7]  = INSTR(     LDR,  R3,  S1,  R5  );
	bin[8]  = INSTR_IMM( SHLI, R5,  0x09     );
	
	bin[9]  = INSTR(     STR,  R6,  S0,  R5  );
	bin[10] = INSTR_IMM( ADDI, R6,  0x01     );
	
	bin[11] = INSTR(     SLT,  R6,  R2,  R7  );
	bin[12] = INSTR(     CMOV, R4,  R7,  R15 );
	
	bin[13] = INSTR_IMM( SUBI, R15, 0x01     );
	blen = 14;
	*/
	
	
	bin[0]  = INSTR_IMM( ORI,  R0,  0xA0     ); // vram segment
	bin[1]  = INSTR_EXT( LDS,  R0,  S0       );
	bin[2]  = INSTR_IMM( LDH,  R1,  0x55     ); // fill value
	bin[3]  = INSTR_IMM( ORI,  R1,  0x55     );
	bin[4]  = INSTR_IMM( LDH,  R2,  0xFA     ); // num pixels
	bin[5]  = INSTR_IMM( ORI,  R5,  0x06     ); // loop start
	bin[6]  = INSTR_IMM( ORI,  R6,  0xC0     ); // control segment
	bin[7]  = INSTR_EXT( LDS,  R6,  S1       );
	
	bin[8]  = INSTR(     STR,  R3,  S0,  R1  );
	bin[9]  = INSTR_IMM( ADDI, R3,  0x01     );
	bin[10] = INSTR_IMM( MULI, R1,  0x03     );
	bin[11] = INSTR(     SLT,  R3,  R2,  R4  );
	bin[12] = INSTR(     CMOV, R5,  R4,  R15 );
	
	bin[13] = INSTR_IMM( ANDI, R3,  0x00     ); // clear counter
	bin[14] = INSTR(     LDR,  R7,  S1,  R8  ); // check vblank
	bin[15] = INSTR_IMM( ANDI, R8,  0x01     );
	bin[16] = INSTR_IMM( XORI, R8,  0x01     );
	bin[17] = INSTR(     CMOV, R5,  R8,  R15 ); // restart loop
	bin[18] = INSTR_IMM( SUBI, R15, 0x05     );
	blen = 19;
	
	
	/*
	const char* testStr[] = {
		">a=4",
		">b=7",
		">c=5",
		"]d=a-b",
		"]e=c*d"
	};
	int testLen = 6;
	
	const int VAR_IMM = 0;
	const int VAR_REG = 1;
	
	typedef struct {
		char name;
		int type;
		int val;
		int reg;
	} var_t;
	
	var_t vars[64];
	int regVars[15];
	
	memset( vars, 0, sizeof( vars ) );
	for( int i = 0; i < 15; i++ ) {
		regVars[i] = -1;
	}
	
	int allocReg( void ) {
		for( int i = 0; i < 15; i++ ) {
			if( regVars[i] == -1 ) {
				return i;
			}
		}
		
		return -1;
	}
	
	int getVar( char name ) {
		for( int v = 0; v < 64; v++ ) {
			if( vars[v].name == var ) {
				return v;
			}
		}
		
		return -1;
	}
	
	for( int i = 0; i < testLen; i++ ) {
		const char* p = testStr[i];
		
		if( *p == '>' ) {
			p++;
			char var = *p;
			p += 2;
			int val = *p - '0';
			for( int v = 0; v < 64; v++ ) {
				if( vars[v].name == var || vars[v].name == 0 ) {
					vars[v].name = var;
					vars[v].type = VAR_IMM;
					vars[v].val = val;
				}
			}
			
		} else if( *p == ']' ) {
			p++;
			char varR = *p;
			p += 2;
			char varA = *p;
			p++;
			int op = *p;
			p++;
			char varB = *p;
			for( int v = 0; v < 64; v++ ) {
				if( vars[v].name == var || vars[v].name == 0 ) {
					vars[v].name = var;
					vars[v].type = VAR_REG;
					vars[v].reg = allocReg();
					
					int v1 = getVar( varA );
					int v2 = getVar( varB );
				}
			}
		}
	}
	*/
	FILE* f = fopen( "test.bin", "wb" );
	fwrite( bin, 2, blen, f );
	fclose( f );
	
	return 0;
}
