#include "burnint.h"

// pgm_run.cpp
extern unsigned char PgmJoy1[];
extern unsigned char PgmJoy2[];
extern unsigned char PgmJoy3[];
extern unsigned char PgmJoy4[];
extern unsigned char PgmBtn1[];
extern unsigned char PgmBtn2[];
extern unsigned char PgmInput[];
extern unsigned char PgmReset;

extern int pgmInit();
extern int pgmExit();
extern int pgmFrame();
extern int pgmDraw();
extern int pgmScan(int /*nAction*/, int */*pnMin*/);

extern int nPGM68KROMLen;
extern unsigned char *USER0, *USER1, *USER2;
extern unsigned char *PGM68KROM, *PGMTileROM, *PGMTileROMExp, *PGMSPRColROM, *PGMSPRMaskROM, *PGMSNDROM;
extern unsigned short *RamRs, *RamPal, *RamVReg, *RamSpr;
extern unsigned int *RamBg, *RamTx, *RamCurPal;
extern unsigned char nPgmPalRecalc;

extern void (*pPgmInitCallback)();
extern int (*pPgmScanCallback)(int, int*);

// pgm_draw
extern int pgmDraw();

// pgm_snd
extern unsigned char ics2115read(unsigned char offset);
extern void ics2115write(unsigned char offset, unsigned char data);

extern int ics2115_init();
extern void ics2115_exit();
extern void ics2115_reset();

extern unsigned short ics2115_soundlatch_r(int i);
extern void ics2115_soundlatch_w(int i, unsigned short d);

extern void ics2115_frame();
extern void ics2115_update(int length);
extern void ics2115_scan(int nAction,int *pnMin);

// pgm_prot.cpp

extern void pgm_asic28_w(unsigned short offset, unsigned short data);
extern unsigned short pgm_asic28_r(unsigned short offset);

extern void pgm_asic3_reg_w(unsigned short offset, unsigned short data);
extern void pgm_asic3_w(unsigned short offset, unsigned short data);
extern unsigned char pgm_asic3_r(unsigned short offset);
extern unsigned short sango_protram_r(unsigned short offset);

extern unsigned short dw2_d80000_r(unsigned int sekAddress);

extern unsigned short *killbld_sharedprotram;
extern void killbld_prot_w(unsigned int sekAddress, unsigned short data);
extern unsigned short killbld_prot_r(unsigned int sekAddress);

extern unsigned short PSTARS_protram_r(unsigned int sekAddress);
extern unsigned short PSTARS_r16(unsigned int sekAddress);
extern void PSTARS_w16(unsigned int sekAddress, unsigned short data);

extern void olds_w16(unsigned int offset, unsigned short data);
extern unsigned short olds_r16(unsigned short offset);

extern void prot_reset();

extern int asic28Scan(int nAction,int */*pnMin*/);
extern int asic3Scan(int nAction,int */*pnMin*/);
extern int killbldtScan(int nAction,int */*pnMin*/);
extern int pstarsScan(int nAction,int */*pnMin*/);
extern int oldsScan(int nAction,int */*pnMin*/);

// pgm_crypt.cpp

extern void pgm_kov_decrypt();
extern void pgm_kovsh_decrypt();
extern void pgm_dw2_decrypt();
extern void pgm_djlzz_decrypt();
extern void pgm_pstar_decrypt();
extern void pgm_dw3_decrypt();
extern void pgm_killbld_decrypt();
extern void pgm_dfront_decrypt();
extern void pgm_ddp2_decrypt();
extern void pgm_mm_decrypt();
extern void pgm_kov2_decrypt();
extern void pgm_puzzli2_decrypt();

