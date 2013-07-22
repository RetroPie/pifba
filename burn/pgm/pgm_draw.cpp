#include "pgm.h"

static unsigned short * pgm_sprite_source; 

/* PGM Palette */ 

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x7C00) >> 7;  // Red 
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static int pgmPalUpdate()
{
	int i;
	unsigned short* ps;
	unsigned int* pd;

	for (i = 0, ps = (unsigned short*)RamPal, pd = RamCurPal; i < 0x1200; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
	return 0;
}

static void pgm_drawsprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int boffset, int flip, unsigned int /*xzoom*/, int /*xgrow*/, unsigned int /*yzoom*/, int /*ygrow*/ )
{
	unsigned char * bdata = PGMSPRMaskROM;
	unsigned char * adata = PGMSPRColROM;

	unsigned int aoffset;
	unsigned short msk;
	unsigned short * dst = (unsigned short *)pBurnDraw;

	aoffset = (bdata[boffset+3] << 24) | (bdata[boffset+2] << 16) | (bdata[boffset+1] << 8) | bdata[boffset];
	aoffset >>= 1;
	unsigned int gfx3data = 0;
	unsigned int gfx3datapos = 0;
	boffset += 4; /* because the first dword is the a data offset */
	
	dst += (ypos * 448) + xpos;
	
	int yflip_dir = 448;
	if (flip & 0x02) {
		yflip_dir = -448;
		dst += (high - 1) * 448;
	} 
	
	for (int ycnt = 0 ; ycnt < high ; ycnt++) {
		int newy;
		if (flip & 0x02) {
			newy = ypos + (high - 1) - ycnt;
			if ( newy < 0) break;
		} else {
			newy = ycnt + ypos;
			if ( newy >= 224) break;
		}
		if (newy >= 0 && newy <224) {
			
			if ( flip & 0x01 ) {
				for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
					int newxb = wide*16 - 1;
					msk = (( bdata[boffset+1] << 8) | bdata[boffset]);
					for (int x=0;x<16;x++) {
						int newx = newxb - xcnt*16 - x;
						if (!(msk & 0x0001)) {
							if (gfx3datapos == 0) {
								gfx3data = adata[aoffset] | (adata[aoffset+1] << 8);
								aoffset += 2;
							}
							if (((newx + xpos) >= 0) && ((newx + xpos)<448))
								dst[ newx ] = RamCurPal[ (gfx3data & 0x1f) + palt ];
							if (gfx3datapos == 2) {
								gfx3datapos = 0;
							} else {
								gfx3datapos ++;
								gfx3data >>= 5;
							}
						}
						msk >>=1;
					}
					boffset+=2;
				}
			} else {
				for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
					msk = (( bdata[boffset+1] << 8) | bdata[boffset]);
					for (int x=0;x<16;x++) {
						int newx = xcnt*16 + x;
						if (!(msk & 0x0001)) {
							if (gfx3datapos == 0) {
								gfx3data = adata[aoffset] | (adata[aoffset+1] << 8);
								aoffset += 2;
							}
							if (((newx + xpos) >= 0) && ((newx + xpos)<448))
								dst[ newx ] = RamCurPal[ (gfx3data & 0x1f) + palt ];
							if (gfx3datapos == 2) {
								gfx3datapos = 0;
							} else {
								gfx3datapos ++;
								gfx3data >>= 5;
							}
						}
						msk >>=1;
					}
					boffset+=2;
				}	
			}
		
		} else {
			for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
				msk = (( bdata[boffset+1] << 8) | bdata[boffset]);
				for (int x=0;x<16;x++) {
					if (!(msk & 0x0001)) {
						if (gfx3datapos == 0) {
							gfx3data = adata[aoffset] | (adata[aoffset+1] << 8);
							aoffset += 2;
						}
						if (gfx3datapos == 2) {
							gfx3datapos = 0;
						} else {
							gfx3datapos ++;
							gfx3data >>= 5;
						}
					}
					msk >>=1;
				}
				boffset+=2;
			}
		}
		dst += yflip_dir;
	}
	
}

static void pgm_drawsprites(int priority)
{
	/* 10 byte per sprite, 256 sprites
	   ZZZZ Zxxx xxxx xxxx
       zzzz z-yy yyyy yyyy
       -ffp pppp Pvvv vvvv
       vvvv vvvv vvvv vvvv
       wwww wwwh hhhh hhhh */

    const unsigned short * finish = RamSpr + (0xA00 / 2);
    while( pgm_sprite_source < finish ) {
    	int xpos = pgm_sprite_source[0] & 0x07ff;
		int ypos = pgm_sprite_source[1] & 0x03ff;
		int xzom = (pgm_sprite_source[0] & 0x7800) >> 11;
		int xgrow = (pgm_sprite_source[0] & 0x8000) >> 15;
		int yzom = (pgm_sprite_source[1] & 0x7800) >> 11;
		int ygrow = (pgm_sprite_source[1] & 0x8000) >> 15;
		int palt = (pgm_sprite_source[2] & 0x1f00) >> 3;
		int flip = (pgm_sprite_source[2] & 0x6000) >> 13;
		int boff = ((pgm_sprite_source[2] & 0x007f) << 16) | (pgm_sprite_source[3] & 0xffff);
		int wide = (pgm_sprite_source[4] & 0x7e00) >> 9;
		int high = pgm_sprite_source[4] & 0x01ff;
		int pri = (pgm_sprite_source[2] & 0x0080) >>  7;
		unsigned int xzoom, yzoom;
		unsigned short * pgm_sprite_zoomtable = & RamVReg[0x1000 / 2];

		if (xgrow) {
			xzom = 0x10-xzom; // this way it doesn't but there is a bad line when zooming after the level select?
		}
		if (ygrow) {
			yzom = 0x10-yzom;
		}
		xzoom = (pgm_sprite_zoomtable[xzom*2]<<16)|pgm_sprite_zoomtable[xzom*2+1];
		yzoom = (pgm_sprite_zoomtable[yzom*2]<<16)|pgm_sprite_zoomtable[yzom*2+1];
		
		boff *= 2;
		if (xpos > 0x3ff) xpos -=0x800;
		if (ypos > 0x1ff) ypos -=0x400;

		if (high == 0) break; /* is this right? */
		if ((priority == 1) && (pri == 0)) break;
		pgm_drawsprite_new_zoomed(wide, high, xpos, ypos, palt, boff, flip, xzoom,xgrow, yzoom,ygrow);
		pgm_sprite_source += 5;
    }
    
}

static void pgm_tile_tx()
{
/* 0x904000 - 0x905fff is the Text Overlay Ram ( RamTx )
    each tile uses 4 bytes, the tilemap is 64x32?

   the layer uses 4bpp 8x8 tiles from the 'T' roms
   colours from 0xA01000 - 0xA017FF

   scroll registers are at 0xB05000 (Y) and 0xB06000 (X)

    ---- ---- ffpp ppp- nnnn nnnn nnnn nnnn

    n = tile number
    p = palette
    f = flip
*/
	int tileno, colour, flipyx;
	int mx=-1, my=0, x, y;
	unsigned int *pal = & RamCurPal[0x800];
	
	const unsigned int * finish = RamTx + 0x800;
	for (unsigned int * tiledata = RamTx; tiledata < finish; tiledata++) {
		tileno = (*tiledata >>  0) & 0xFFFF;
		colour = (*tiledata >> 13) & 0x1F0;
		flipyx = (*tiledata >> 22) & 0x03;
		
		if (tileno > 0xbfff) { tileno -= 0xc000 ; tileno += 0x20000; } // not sure about this
		
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}
		
		// is this right ? but it will save many cpu cycles ..
		if (tileno == 0) continue;

		x = mx * 8 - (signed short)RamVReg[0x6000 / 2];
		y = my * 8 - (signed short)RamVReg[0x5000 / 2];

		if ( x<=-8 || x>=448 || y<=-8 || y>= 224 ) continue;

		unsigned char *d = PGMTileROM + tileno * 32;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 448 + x;
		unsigned int v;

		if ( x >=0 && x < 440 && y >= 0 && y < 216) {
			for (int k=0;k<8;k++) {
				v = d[0] & 0xf;	if (v != 15) p[0] = pal[ v | colour ];
 				v = d[0] >> 4;	if (v != 15) p[1] = pal[ v | colour ];
 				v = d[1] & 0xf;	if (v != 15) p[2] = pal[ v | colour ];
 				v = d[1] >> 4;	if (v != 15) p[3] = pal[ v | colour ];
 				v = d[2] & 0xf;	if (v != 15) p[4] = pal[ v | colour ];
 				v = d[2] >> 4;	if (v != 15) p[5] = pal[ v | colour ];
 				v = d[3] & 0xf;	if (v != 15) p[6] = pal[ v | colour ];
 				v = d[3] >> 4;	if (v != 15) p[7] = pal[ v | colour ];
 				d += 4;
 				p += 448;
 			}
		} else {
			for (int k=0;k<8;k++) {
				if ((y+k) >= 224) break;
				if ((y+k) >= 0) {
	 				v = d[0] & 0xf;	if (v != 15 && (x + 0) >= 0 && (x + 0)<448) p[0] = pal[ v | colour ];
	 				v = d[0] >> 4;	if (v != 15 && (x + 1) >= 0 && (x + 1)<448) p[1] = pal[ v | colour ];
	 				v = d[1] & 0xf;	if (v != 15 && (x + 2) >= 0 && (x + 2)<448) p[2] = pal[ v | colour ];
	 				v = d[1] >> 4;	if (v != 15 && (x + 3) >= 0 && (x + 3)<448) p[3] = pal[ v | colour ];
	 				v = d[2] & 0xf;	if (v != 15 && (x + 4) >= 0 && (x + 4)<448) p[4] = pal[ v | colour ];
	 				v = d[2] >> 4;	if (v != 15 && (x + 5) >= 0 && (x + 5)<448) p[5] = pal[ v | colour ];
	 				v = d[3] & 0xf;	if (v != 15 && (x + 6) >= 0 && (x + 6)<448) p[6] = pal[ v | colour ];
	 				v = d[3] >> 4;	if (v != 15 && (x + 7) >= 0 && (x + 7)<448) p[7] = pal[ v | colour ];
 				}
 				d += 4;
 				p += 448;
 			}
		}
	}
}

static void pgm_tile_bg()
{
	int tileno, colour, flipx, flipy;
	int mx=-1, my=0, x, y;
	unsigned int *pal = & RamCurPal[0x400];
	
	const unsigned int * finish = RamBg + 0x1000;
	for (unsigned int * tiledata = RamBg; tiledata < finish; tiledata ++) {
		tileno = (*tiledata) & 0xFFFF;
		
		if (tileno > 0x7ff)
			tileno += 0x1000;	 // Tiles 0x800+ come from the GAME Roms
	
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}
		
		if (tileno == 0) continue;

		colour = (*tiledata >> 12) & 0x3E0;
		flipx = (*tiledata >> 22) & 0x01;
		flipy = (*tiledata >> 23) & 0x01;

		x = mx * 32 - (signed short)RamVReg[0x3000 / 2];
		if (x <= (448 - 64 * 32)) x += (64 * 32);
		
		y = my * 32 - (signed short)RamVReg[0x2000 / 2];
		if (y <= (224 - 64 * 32)) y += (64 * 32);
		
		if ( x<=-32 || x>=448 || y<=-32 || y>= 224 ) 
			continue;
		
		if ( x >=0 && x < (448-32) && y >= 0 && y < (224-32)) {
			unsigned char * d = PGMTileROMExp + tileno * 1024;
 			unsigned short * p = (unsigned short *) pBurnDraw + y * 448 + x;
 			
 			if ( flipy ) {
 				
 				p += 31 * 448;
 				
 				if ( flipx ) {
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[31] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[30] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[29] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[28] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[27] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[26] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[25] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[24] = pal[ d[ 7] | colour ];
			
		 				if (d[ 8] != 31) p[23] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[22] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[21] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[20] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[19] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[18] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[17] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[16] = pal[ d[15] | colour ];
			
		 				if (d[16] != 31) p[15] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[14] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[13] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[12] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[11] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[10] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[ 9] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[ 8] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[ 7] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[ 6] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[ 5] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[ 4] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[ 3] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[ 2] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[ 1] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[ 0] = pal[ d[31] | colour ];
		 				d += 32;
		 				p -= 448;
		 			}
 					
 				} else {
 					// not flip x , flip y
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[ 0] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[ 1] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[ 2] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[ 3] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[ 4] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[ 5] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[ 6] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[ 7] = pal[ d[ 7] | colour ];
		
		 				if (d[ 8] != 31) p[ 8] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[ 9] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[10] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[11] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[12] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[13] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[14] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[15] = pal[ d[15] | colour ];
		
		 				if (d[16] != 31) p[16] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[17] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[18] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[19] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[20] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[21] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[22] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[23] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[24] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[25] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[26] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[27] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[28] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[29] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[30] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[31] = pal[ d[31] | colour ];
		 				d += 32;
		 				p -= 448;
		 			}
 				}			
 			
 			} else {
 				
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[31] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[30] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[29] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[28] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[27] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[26] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[25] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[24] = pal[ d[ 7] | colour ];
		
		 				if (d[ 8] != 31) p[23] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[22] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[21] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[20] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[19] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[18] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[17] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[16] = pal[ d[15] | colour ];
		
		 				if (d[16] != 31) p[15] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[14] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[13] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[12] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[11] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[10] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[ 9] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[ 8] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[ 7] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[ 6] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[ 5] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[ 4] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[ 3] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[ 2] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[ 1] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[ 0] = pal[ d[31] | colour ];
		 				d += 32;
		 				p += 448;
		 			}
 				} else {
 					// not flip x , not flip y
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[ 0] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[ 1] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[ 2] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[ 3] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[ 4] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[ 5] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[ 6] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[ 7] = pal[ d[ 7] | colour ];
		
		 				if (d[ 8] != 31) p[ 8] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[ 9] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[10] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[11] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[12] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[13] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[14] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[15] = pal[ d[15] | colour ];
		
		 				if (d[16] != 31) p[16] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[17] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[18] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[19] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[20] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[21] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[22] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[23] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[24] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[25] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[26] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[27] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[28] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[29] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[30] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[31] = pal[ d[31] | colour ];
		 				d += 32;
		 				p += 448;
		 			}
 				}
  			}
 		} else {
			
			unsigned char * d = PGMTileROMExp + tileno * 1024;
 			unsigned short * p = (unsigned short *) pBurnDraw + y * 448 + x;
 			
 			if ( flipy ) {
 				
 				p += 31 * 448;
 				
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=31;k>=0;k--) {
						if ((y+k) < 0) break;
						if ((y+k) < 224) {
			 				if (d[31] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[31] | colour ];
			 				if (d[30] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[30] | colour ];
			 				if (d[29] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[29] | colour ];
			 				if (d[28] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[28] | colour ];
			 				if (d[27] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[27] | colour ];
			 				if (d[26] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[26] | colour ];
			 				if (d[25] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[25] | colour ];
			 				if (d[24] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[24] | colour ];
			
			 				if (d[23] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[23] | colour ];
			 				if (d[22] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[22] | colour ];
			 				if (d[21] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[21] | colour ];
			 				if (d[20] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[20] | colour ];
			 				if (d[19] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[19] | colour ];
			 				if (d[18] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[18] | colour ];
			 				if (d[17] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[17] | colour ];
			 				if (d[16] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[16] | colour ];
			
			 				if (d[15] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[15] | colour ];
			 				if (d[14] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[14] | colour ];
			 				if (d[13] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[13] | colour ];
			 				if (d[12] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[12] | colour ];
			 				if (d[11] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[11] | colour ];
			 				if (d[10] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[10] | colour ];
			 				if (d[ 9] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[ 9] | colour ];
			 				if (d[ 8] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[ 8] | colour ];
			
			 				if (d[ 7] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[ 7] | colour ];
			 				if (d[ 6] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[ 6] | colour ];
			 				if (d[ 5] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[ 5] | colour ];
			 				if (d[ 4] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[ 4] | colour ];
			 				if (d[ 3] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[ 3] | colour ];
			 				if (d[ 2] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[ 2] | colour ];
			 				if (d[ 1] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[ 1] | colour ];
			 				if (d[ 0] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[ 0] | colour ];
						}
		 				d += 32;
		 				p -= 448;
		 			}
 				} else {
 					for (int k=31;k>=0;k--) {
						if ((y+k) < 0) break;
						if ((y+k) < 224) {
			 				if (d[ 0] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[ 0] | colour ];
			 				if (d[ 1] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[ 1] | colour ];
			 				if (d[ 2] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[ 2] | colour ];
			 				if (d[ 3] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[ 3] | colour ];
			 				if (d[ 4] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[ 4] | colour ];
			 				if (d[ 5] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[ 5] | colour ];
			 				if (d[ 6] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[ 6] | colour ];
			 				if (d[ 7] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[ 7] | colour ];
			
			 				if (d[ 8] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[ 8] | colour ];
			 				if (d[ 9] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[ 9] | colour ];
			 				if (d[10] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[10] | colour ];
			 				if (d[11] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[11] | colour ];
			 				if (d[12] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[12] | colour ];
			 				if (d[13] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[13] | colour ];
			 				if (d[14] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[14] | colour ];
			 				if (d[15] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[15] | colour ];
			
			 				if (d[16] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[16] | colour ];
			 				if (d[17] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[17] | colour ];
			 				if (d[18] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[18] | colour ];
			 				if (d[19] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[19] | colour ];
			 				if (d[20] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[20] | colour ];
			 				if (d[21] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[21] | colour ];
			 				if (d[22] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[22] | colour ];
			 				if (d[23] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[23] | colour ];
			
			 				if (d[24] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[24] | colour ];
			 				if (d[25] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[25] | colour ];
			 				if (d[26] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[26] | colour ];
			 				if (d[27] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[27] | colour ];
			 				if (d[28] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[28] | colour ];
			 				if (d[29] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[29] | colour ];
			 				if (d[30] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[30] | colour ];
			 				if (d[31] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[31] | colour ];
						}
		 				d += 32;
		 				p -= 448;
		 			}
 				}			
 			} else {
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=0;k<32;k++) {
						if ((y+k) >= 224) break;
						if ((y+k) >= 0) {
			 				if (d[31] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[31] | colour ];
			 				if (d[30] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[30] | colour ];
			 				if (d[29] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[29] | colour ];
			 				if (d[28] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[28] | colour ];
			 				if (d[27] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[27] | colour ];
			 				if (d[26] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[26] | colour ];
			 				if (d[25] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[25] | colour ];
			 				if (d[24] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[24] | colour ];
			
			 				if (d[23] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[23] | colour ];
			 				if (d[22] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[22] | colour ];
			 				if (d[21] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[21] | colour ];
			 				if (d[20] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[20] | colour ];
			 				if (d[19] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[19] | colour ];
			 				if (d[18] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[18] | colour ];
			 				if (d[17] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[17] | colour ];
			 				if (d[16] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[16] | colour ];
			
			 				if (d[15] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[15] | colour ];
			 				if (d[14] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[14] | colour ];
			 				if (d[13] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[13] | colour ];
			 				if (d[12] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[12] | colour ];
			 				if (d[11] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[11] | colour ];
			 				if (d[10] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[10] | colour ];
			 				if (d[ 9] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[ 9] | colour ];
			 				if (d[ 8] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[ 8] | colour ];
			
			 				if (d[ 7] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[ 7] | colour ];
			 				if (d[ 6] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[ 6] | colour ];
			 				if (d[ 5] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[ 5] | colour ];
			 				if (d[ 4] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[ 4] | colour ];
			 				if (d[ 3] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[ 3] | colour ];
			 				if (d[ 2] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[ 2] | colour ];
			 				if (d[ 1] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[ 1] | colour ];
			 				if (d[ 0] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[ 0] | colour ];
						}
		 				d += 32;
		 				p += 448;
		 			}
 				} else {
 					for (int k=0;k<32;k++) {
						if ((y+k) >= 224) break;
						if ((y+k) >= 0) {
			 				if (d[ 0] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[ 0] | colour ];
			 				if (d[ 1] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[ 1] | colour ];
			 				if (d[ 2] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[ 2] | colour ];
			 				if (d[ 3] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[ 3] | colour ];
			 				if (d[ 4] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[ 4] | colour ];
			 				if (d[ 5] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[ 5] | colour ];
			 				if (d[ 6] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[ 6] | colour ];
			 				if (d[ 7] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[ 7] | colour ];
			
			 				if (d[ 8] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[ 8] | colour ];
			 				if (d[ 9] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[ 9] | colour ];
			 				if (d[10] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[10] | colour ];
			 				if (d[11] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[11] | colour ];
			 				if (d[12] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[12] | colour ];
			 				if (d[13] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[13] | colour ];
			 				if (d[14] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[14] | colour ];
			 				if (d[15] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[15] | colour ];
			
			 				if (d[16] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[16] | colour ];
			 				if (d[17] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[17] | colour ];
			 				if (d[18] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[18] | colour ];
			 				if (d[19] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[19] | colour ];
			 				if (d[20] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[20] | colour ];
			 				if (d[21] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[21] | colour ];
			 				if (d[22] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[22] | colour ];
			 				if (d[23] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[23] | colour ];
			
			 				if (d[24] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[24] | colour ];
			 				if (d[25] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[25] | colour ];
			 				if (d[26] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[26] | colour ];
			 				if (d[27] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[27] | colour ];
			 				if (d[28] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[28] | colour ];
			 				if (d[29] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[29] | colour ];
			 				if (d[30] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[30] | colour ];
			 				if (d[31] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[31] | colour ];
						}
		 				d += 32;
		 				p += 448;
		 			}
 				}
 			}
		}
	}
}

int pgmDraw()
{
	memset(pBurnDraw, 0, 448*224*2);

	pgmPalUpdate();
	pgm_sprite_source = RamSpr;	
	pgm_drawsprites(1);
	pgm_tile_bg();
	pgm_drawsprites(0);
	pgm_tile_tx();
	
	return 0;
}
