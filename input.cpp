#include "burner.h"
#include "gp2xsdk.h"
extern struct usbjoy *joys[4];
extern char joyCount;
extern unsigned char ServiceRequest;
extern unsigned char P1P2Start;


struct GameInp {
	unsigned char *pVal;  // Destination for the Input Value
	unsigned char nType;  // 0=binary (0,1) 1=analog (0x01-0xFF) 2=dip switch
    unsigned char nConst;
	int nBit;   // bit offset of Keypad data
};

struct DIPInfo{
	unsigned char nDIP;
	unsigned short nFirstDIP;
	struct GameInp *DIPData;
} DIPInfo;
// Mapping of PC inputs to game inputs
struct GameInp GameInp[4][12];
unsigned int nGameInpCount = 0;
static bool bInputOk = false;
unsigned char *ServiceDip = 0;
unsigned char *P1Start = 0;
unsigned char *P2Start = 0;

int DoInputBlank(int /*bDipSwitch*/)
{
  int iJoyNum = 0;
  unsigned int i=0; 
  // Reset all inputs to undefined (even dip switches, if bDipSwitch==1)
  char controlName[12];
  
  DIPInfo.nDIP = 0;
  // Get the targets in the library for the Input Values
  for (i=0; i<nGameInpCount; i++)
  {
    struct BurnInputInfo bii;
    gp2x_memset(&bii,0,sizeof(bii));
    BurnDrvGetInputInfo(&bii,i);
    
    //if (bDipSwitch==0 && bii.nType==2) continue; // Don't blank the dip switches
	
	if (bii.nType==BIT_DIPSWITCH)
	{
		if (DIPInfo.nDIP == 0)
		{
			DIPInfo.nFirstDIP = i;
			DIPInfo.nDIP = nGameInpCount - i;
			DIPInfo.DIPData = (struct GameInp *)malloc(DIPInfo.nDIP * sizeof(struct GameInp));
			gp2x_memset(DIPInfo.DIPData,0,DIPInfo.nDIP * sizeof(struct GameInp));
		}
		DIPInfo.DIPData[i-DIPInfo.nFirstDIP].pVal = bii.pVal;
		DIPInfo.DIPData[i-DIPInfo.nFirstDIP].nType = bii.nType;
		DIPInfo.DIPData[i-DIPInfo.nFirstDIP].nConst = 0;
		DIPInfo.DIPData[i-DIPInfo.nFirstDIP].nBit = 0;
	}

	if (bii.szInfo[0]=='p')
		iJoyNum = bii.szInfo[1] - '1';	
	else
	{
		if (strcmp(bii.szInfo, "diag") == 0 || strcmp(bii.szInfo, "test") == 0)
		{
			ServiceDip = bii.pVal;
		}	
		continue;
	}
	sprintf(controlName,"p%i coin",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][0].nBit = 4;
		GameInp[iJoyNum][0].pVal = bii.pVal;
		GameInp[iJoyNum][0].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i start",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][1].nBit = 5;
		GameInp[iJoyNum][1].pVal = bii.pVal;
		GameInp[iJoyNum][1].nType = bii.nType;
		switch (iJoyNum)
		{
			case 0:
				P1Start = bii.pVal;
			break;
			case 1:
				P2Start = bii.pVal;
			break;
		}	
	}
    else {
	sprintf(controlName,"p%i up",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][2].nBit = 0;
		GameInp[iJoyNum][2].pVal = bii.pVal;
		GameInp[iJoyNum][2].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i down",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][3].nBit = 1;
		GameInp[iJoyNum][3].pVal = bii.pVal;
		GameInp[iJoyNum][3].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i left",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][4].nBit = 2;
		GameInp[iJoyNum][4].pVal = bii.pVal;
		GameInp[iJoyNum][4].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i right",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][5].nBit = 3;
		GameInp[iJoyNum][5].pVal = bii.pVal;
		GameInp[iJoyNum][5].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i fire 1",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][6].nBit = 6;
		GameInp[iJoyNum][6].pVal = bii.pVal;
		GameInp[iJoyNum][6].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i fire 2",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][7].nBit = 7;
		GameInp[iJoyNum][7].pVal = bii.pVal;
		GameInp[iJoyNum][7].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i fire 3",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][8].nBit = 8;
		GameInp[iJoyNum][8].pVal = bii.pVal;
		GameInp[iJoyNum][8].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i fire 4",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][9].nBit = 9;
		GameInp[iJoyNum][9].pVal = bii.pVal;
		GameInp[iJoyNum][9].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i fire 5",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][10].nBit = 10;
		GameInp[iJoyNum][10].pVal = bii.pVal;
		GameInp[iJoyNum][10].nType = bii.nType;
    }
    else {
	sprintf(controlName,"p%i fire 6",iJoyNum+1);
    if (strcmp(bii.szInfo, controlName) == 0)
    {
    	GameInp[iJoyNum][11].nBit = 11;
		GameInp[iJoyNum][11].pVal = bii.pVal;
		GameInp[iJoyNum][11].nType = bii.nType;
    }}}}}}}}}}}}

#if 0
if (pgi->pVal != NULL)
	printf("GI(%02d): %-12s 0x%02x 0x%02x %-12s, [%d]\n", i, bii.szName, bii.nType, *(pgi->pVal), bii.szInfo, pgi->nBit );
else
	printf("GI(%02d): %-12s 0x%02x N/A  %-12s, [%d]\n", i, bii.szName, bii.nType, bii.szInfo, pgi->nBit );
#endif
   
  }
  return 0;
}

int InpInit()
{
  unsigned int i=0; 
  int nRet=0;
  bInputOk = false;
  // Count the number of inputs
  nGameInpCount=0;
  for (i=0;i<0x1000;i++) {
    nRet = BurnDrvGetInputInfo(NULL,i);
    if (nRet!=0) {   // end of input list
    	nGameInpCount=i; 
    	break; 
    }
  }
  
  gp2x_memset(GameInp,0,12*4*sizeof(struct GameInp));
  DoInputBlank(1);

  bInputOk = true;

  return 0;
}

int InpExit()
{
  bInputOk = false;
  nGameInpCount = 0;
  if (DIPInfo.nDIP)
	free (DIPInfo.DIPData);
  return 0;
}

int InpMake(unsigned int key[])
{
	if (!bInputOk) return 1;
	
	static int skip = 0;
	skip ++;
	if (skip > 1) skip = 0;
	if (skip != 1) return 1;
	
	unsigned int i=0; 
	unsigned int down = 0;
	short numJoy = joyCount?joyCount:1;
	if (ServiceDip)
	{
		*(ServiceDip)=ServiceRequest;
	}
	for (short joyNum=0;joyNum<numJoy;joyNum++)
	{
		for (i=0; i<12; i++)
		{
			if (GameInp[joyNum][i].pVal == NULL) continue;
			
			if ( GameInp[joyNum][i].nBit >= 0 )
			{
				down = key[joyNum] & (1U << GameInp[joyNum][i].nBit);
				
				if (GameInp[joyNum][i].nType!=1) {
					// Set analog controls to full
					if (down) *(GameInp[joyNum][i].pVal)=0xff; else *(GameInp[joyNum][i].pVal)=0x01;
				}
				else
				{
					// Binary controls
					if (down) *(GameInp[joyNum][i].pVal)=1;    else *(GameInp[joyNum][i].pVal)=0;
				}
			}
		}
	}
	for (i=0; i<(int)DIPInfo.nDIP; i++) {
		if (DIPInfo.DIPData[i].pVal == NULL)
			continue;
		*(DIPInfo.DIPData[i].pVal) = DIPInfo.DIPData[i].nConst;
	}
	if (P1P2Start)
	{
		*(P1Start) = *(P2Start) = 1;
	}
	return 0;
}

extern int GameScreenMode;

void InpDIP()
{
	struct BurnDIPInfo bdi;
	struct GameInp* pgi;
	int i, j;
	int nDIPOffset = 0;

	// get dip switch offset 
	for (i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
		if (bdi.nFlags == 0xF0) {
			nDIPOffset = bdi.nInput;
			break;
		}

	// set DIP to default
	i = 0;
	bool bDifficultyFound = false;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		
		//printf("%2d. %02x '%s'\n", bdi.nInput, bdi.nFlags, bdi.szText);
		
		if (bdi.nFlags == 0xFF) {
			pgi = DIPInfo.DIPData + (bdi.nInput + nDIPOffset - DIPInfo.nFirstDIP);
			pgi->nConst = (pgi->nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
		} else 
		if (bdi.nFlags == 0xFE) {
			if ( bdi.szText )
				if ( ( strcmp(bdi.szText, "Difficulty") == 0  ) ||
				     ( strcmp(bdi.szText, "Game Level") == 0  )
				
				   ) bDifficultyFound = true;
		} else {
			if (bDifficultyFound) {
				if ( bdi.nFlags == 0x01 ) {
					
					// use GameScreenMode store 
					pgi = DIPInfo.DIPData + (bdi.nInput + nDIPOffset - DIPInfo.nFirstDIP);
					for (j=0; j<8; j++)
						if ((1U << j) & bdi.nMask) 
							break;
					pgi->nConst = (pgi->nConst & ~bdi.nMask) | ((GameScreenMode << j) & bdi.nMask);
					
					printf("Set DIP Difficulty [%d] = 0x%02x\n", bdi.nInput, (GameScreenMode << j) & bdi.nMask);
				}
				bDifficultyFound = false;
			}
		}
		i++;
	}
	for (i=0,pgi=DIPInfo.DIPData; i<(int)DIPInfo.nDIP; i++,pgi++) {
		if (pgi->pVal == NULL)
			continue;
		*(pgi->pVal) = pgi->nConst;
	}
}
