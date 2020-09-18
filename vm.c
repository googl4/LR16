#include <stdio.h>
#include <string.h>

#include "tigr.h"

#include "types.h"
#include "isa.h"

u16 r[16];
u8 s[8];

u16 rom[8192];
u16 ram[65536];

u16 FB1[65536];
u16 FB2[65536];

u16 ctrl[256];
// 0  : video status [ vblank : 1, hblank : 1, FB select : 1 ]
// 1  : scanline

u16 load( u8 segment, u16 addr ) {
	switch( segment ) {
		case 0x00:
			return rom[addr];
			
		case 0x80:
			return ram[addr];
			
		case 0xA0:
			return FB1[addr];
			
		case 0xA1:
			return FB2[addr];
			
		case 0xC0:
			return ctrl[addr];
	}
	
	return 0;
}

void store( u8 segment, u16 addr, u16 val ) {
	//printf( "%.2X:%.4X = %.4X\n", segment, addr, val );
	
	if( segment == 0x80 ) {
		ram[addr] = val;
	}
	
	if( segment == 0xA0 ) {
		FB1[addr] = val;
	}
	
	if( segment == 0xA1 ) {
		FB2[addr] = val;
	}
	
	if( segment == 0xC0 ) {
		ctrl[addr] = val;
	}
}

/*
	execute:
		; setup : r0 = cycles, r1 = segment registers, r2 = data registers, r3 = rom data
		mov r1, s
		mov r2, r
		mov r3, rom
	execute_mainLoop:
		; fetch : r4 = pc, r5 = ir
		; NB cs(s[7]) ignored
		mov r4, [r+15*2]
		mov r5, [rom+r4*2]
		inc r4
		mov [r+15*2], r4
		; decode : r6 = op, r7 = a
		shr r6, r5, 12
		shr r7, r5, 8
		and r7, r7, 0Fh
		add r8, r6, 16
		cmp r6, OP_EXT
		cmov.eq r6, r8
		;
*/

void execute( int cycles ) {
	while( cycles > 0 ) {
		u16 ir = load( s[7], r[15] );
		
		u32 pc = ( (u32)s[7] << 16 ) | r[15];
		
		//printf( "%X : %X\n", pc, ir );
		
		pc += 1;
		s[7] = pc >> 16;
		r[15] = pc & 0xFFFF;
		
		u8 op = ir >> 12;
		u8 a = ( ir >> 8 ) & 0x0F;
		u8 b = ( ir >> 4 ) & 0x0F;
		u8 c = ir & 0x0F;
		u8 ab = ( ir >> 4 ) & 0xFF;
		
		if( op == OP_EXT ) {
			op = 16 + a;
		}
		
		switch( op ) {
			case CMOV:
				if( r[b] != 0 ) {
					r[c] = r[a];
				}
				break;
				
			case ADDI:
				r[c] += ab;
				break;
				
			case SUBI:
				r[c] -= ab;
				break;
				
			case MULI:
				r[c] *= ab;
				break;
				
			case SHLI:
				r[c] <<= ab;
				break;
				
			case SHRI:
				r[c] >>= ab;
				break;
				
			case ASRI:
				r[c] = (s16)r[c] >> ab;
				break;
				
			case ANDI:
				r[c] &= ab;
				break;
				
			case ORI:
				r[c] |= ab;
				break;
				
			case XORI:
				r[c] ^= ab;
				break;
				
			case LDH:
				r[c] = (u16)ab << 8;
				break;
				
			case LDR:
				r[c] = load( s[b & 0x7], r[a] );
				if( b & 0x8 ) {
					r[a]++;
				}
				break;
				
			case STR:
				store( s[b & 0x7], r[a], r[c] );
				if( b & 0x8 ) {
					r[a]++;
				}
				break;
				
			case SLT:
				r[c] = r[a] < r[b];
				break;
				
			case SLTS:
				r[c] = (s16)r[a] < (s16)r[b];
				break;
				
			case MOV:
				r[c] = r[b];
				break;
				
			case ADD:
				r[c] += r[b];
				break;
				
			case SUB:
				r[c] -= r[b];
				break;
				
			case MUL:
				r[c] *= r[b];
				break;
				
			case SHL:
				r[c] <<= r[b];
				break;
				
			case SHR:
				r[c] >>= r[b];
				break;
				
			case ASR:
				r[c] = (s16)r[c] >> r[b];
				break;
				
			case AND:
				r[c] &= r[b];
				break;
				
			case OR:
				r[c] |= r[b];
				break;
				
			case XOR:
				r[c] ^= r[b];
				break;
				
			case NOT:
				r[c] = ~r[b];
				break;
				
			case LNOT:
				r[c] = !r[b];
				break;
				
			case LDS:
				s[b] = r[c];
				break;
				
			case STS:
				r[c] = s[b];
				break;
				
			case LJMP:
				s[7] = s[b];
				r[15] = r[c];
				break;
				
			default:
				__builtin_unreachable();
		}
		
		cycles--;
	}
}

void renderScanline( Tigr* win, int y ) {
	u16* fb = ctrl[0] & 0x04 ? FB2 : FB1;
	for( int x = 0; x < 320; x++ ) {
		u8 r = fb[y*320+x] >> 11;
		u8 g = ( fb[y*320+x] >> 5 ) & 0x3F;
		u8 b = fb[y*320+x] & 0x1F;
		win->pix[y*320+x].r = ( r << 3 ) | ( r >> 2 );
		win->pix[y*320+x].g = ( g << 2 ) | ( g >> 3 );
		win->pix[y*320+x].b = ( b << 3 ) | ( b >> 2 );
		win->pix[y*320+x].a = 255;
	}
}

int main( int argc, char* argv[] ) {
	FILE* f = fopen( argv[1], "rb" );
	fread( rom, 8192, 1, f );
	fclose( f );
	
	memset( r, 0, 16 * 2 );
	memset( s, 0, 8 );
	
	//execute( 4096 * 4096 * 16 );
	
	Tigr* window = tigrWindow( 320, 200, "LR16", 0 );
	
	while( !tigrClosed( window ) ) {
		for( int y = 0; y < 256; y++ ) {
			ctrl[0] = ( ctrl[0] & ~0x03 ) | ( y >= 200 );
			ctrl[1] = y;
			
			execute( 4096 );
			
			if( y < 200 ) {
				renderScanline( window, y );
			}
			
			ctrl[0] |= 0x02;
			
			execute( 256 );
		}
		tigrUpdate( window );
	}
	
	return 0;
}
