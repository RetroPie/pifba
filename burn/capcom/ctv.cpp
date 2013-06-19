#include "cps.h"

// CPS Tile Variants
// horizontal/vertical clip rolls
unsigned int nCtvRollX=0,nCtvRollY=0;
// Add 0x7fff after each pixel/line
// If nRollX/Y&0x20004000 both == 0, you can draw the pixel

unsigned char *pCtvTile=NULL; // Pointer to tile data
int nCtvTileAdd=0; // Amount to add after each tile line
unsigned char *pCtvLine=NULL; // Pointer to output bitmap

// Include all tile variants:
#include "ctv.h"

static int nLastBpp=0;
int CtvReady()
{
  // Set up the CtvDoX functions to point to the correct bpp functions.
  // Must be called before calling CpstOne
  if (nBurnBpp!=nLastBpp)
  {
	  if (nBurnBpp==2) {
		gp2x_memcpy(CtvDoX,CtvDo2,sizeof(CtvDoX));
		gp2x_memcpy(CtvDoXM,CtvDo2m,sizeof(CtvDoXM));
		gp2x_memcpy(CtvDoXB,CtvDo2b,sizeof(CtvDoXB));
	  }
	  else if (nBurnBpp==3) {
		gp2x_memcpy(CtvDoX,CtvDo3,sizeof(CtvDoX));
		gp2x_memcpy(CtvDoXM,CtvDo3m,sizeof(CtvDoXM));
		gp2x_memcpy(CtvDoXB,CtvDo3b,sizeof(CtvDoXB));
	  }
	  else if (nBurnBpp==4) {
		gp2x_memcpy(CtvDoX,CtvDo4,sizeof(CtvDoX));
		gp2x_memcpy(CtvDoXM,CtvDo4m,sizeof(CtvDoXM));
		gp2x_memcpy(CtvDoXB,CtvDo4b,sizeof(CtvDoXB));
	  }
  }
  nLastBpp=nBurnBpp;
  return 0;
}
