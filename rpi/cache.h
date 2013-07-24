#ifndef BURN_CACHE_H
#define BURN_CACHE_H

extern int bBurnUseRomCache;

int BurnCacheInit(const char * cfname, char *rom_name);

int BurnCacheBlockSize(int blockid);
int BurnCacheRead(unsigned char * dst, int blockid);
void * BurnCacheMap(int blockid);

void BurnCacheExit();

#endif	// BURN_CACHE_H
