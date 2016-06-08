#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"
#include "bt9.h"
#include "bt9_reader.h"

class PREDICTOR
{
 private:
	int **localPht;
	int *ghr;
	int **bank1;
	int **bank2;
	int **bank3;
	int **bank4;
	int **bank5;
	int **bank6;
	int **bank7;
	int **bank8;
	int **bank9;
	int **bank10;
	int ghr012,ghr1224, ghr2436, ghr3648;
	int ghr4860, ghr6072, ghr7284, ghr8496;
	int ghr96108, ghr108120;
	int bank1Index,bank2Index,bank3Index,bank4Index,bank5Index;
	int bank6Index,bank7Index,bank8Index,bank9Index,bank10Index;
	int bank1tag,bank2tag,bank3tag,bank4tag,bank5tag;
	int bank6tag,bank7tag,bank8tag,bank9tag,bank10tag;
	int phtTableIndex,banktagComp;
 public:
   PREDICTOR(void);
  bool    GetPrediction(UINT64 PC, bool btbANSF, bool btbATSF, bool btbDYN);
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget, bool btbANSF, bool btbATSF, bool btbDYN);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir,UINT64 branchTarget);
  int	  convertToDecimal(int *var,int start,int end); 
  
};





//TOTAL size = 80 + (8192 * 3) + (8192 * 12 * 4) = 1024Kbits
PREDICTOR::PREDICTOR(void)
{
	ghr = (int *)calloc(256,sizeof(int));
	localPht = (int **)calloc(8192,sizeof(int *));
	for(int i=0;i<8192;i++)
	{
		localPht[i] = (int *)calloc(2,sizeof(int));
	}
	bank1 = (int **)calloc(8192,sizeof(int *));
	bank2 = (int **)calloc(8192,sizeof(int *));
	bank3 = (int **)calloc(8192,sizeof(int *));
	bank4 = (int **)calloc(8192,sizeof(int *));
	bank5 = (int **)calloc(8192,sizeof(int *));
	bank6 = (int **)calloc(8192,sizeof(int *));
	bank7 = (int **)calloc(8192,sizeof(int *));
	bank8 = (int **)calloc(8192,sizeof(int *));
	bank9 = (int **)calloc(8192,sizeof(int *));
	bank10 = (int **)calloc(8192,sizeof(int *));
	for(int i=0;i<8192;i++)
	{
		bank1[i]=(int *)calloc(3,sizeof(int));
		bank2[i]=(int *)calloc(3,sizeof(int));
		bank3[i]=(int *)calloc(3,sizeof(int));
		bank4[i]=(int *)calloc(3,sizeof(int));
		bank5[i]=(int *)calloc(3,sizeof(int));
		bank6[i]=(int *)calloc(3,sizeof(int));
		bank7[i]=(int *)calloc(3,sizeof(int));
		bank8[i]=(int *)calloc(3,sizeof(int));
		bank9[i]=(int *)calloc(3,sizeof(int));
		bank10[i]=(int *)calloc(3,sizeof(int));
	}	

}


//binary to decimal converter start
int PREDICTOR::convertToDecimal(int *var,int start,int end)
{
	int ret = 0;
	int j=0;
	for(int i=start;i<=end;i++,j++)	
		ret = ret + (var[i]<<j);	
	return ret;
}
//binary to decimal converter end


//prediction start
bool PREDICTOR::GetPrediction(UINT64 PC, bool btbANSF, bool btbATSF, bool btbDYN)
{
	
	//pht start
	phtTableIndex = PC & 0x1FFF;	
	//pht end
	
	//tag value start
	banktagComp = (PC&0xff)^(convertToDecimal(ghr,0,7))^(convertToDecimal(ghr,0,6)<<1)^(convertToDecimal(ghr,0,5)<<2);
	//tag value end

	//hashing start
	ghr012 = convertToDecimal(ghr,0,12);
	ghr1224 = convertToDecimal(ghr,12,24);
	ghr2436 = convertToDecimal(ghr,24,36);
	ghr3648 = convertToDecimal(ghr,36,48);
	ghr4860 = convertToDecimal(ghr,48,60);
	ghr6072 = convertToDecimal(ghr,60,72);
	ghr7284 = convertToDecimal(ghr,72,84);
	ghr8496 = convertToDecimal(ghr,84,96);
	ghr96108 = convertToDecimal(ghr,96,108);
	ghr108120 = convertToDecimal(ghr,108,120);
	//hashing end
	
	
	//compute tags for each bank start
	bank1Index = (PC & 0x1FFF)^ghr012;
	bank1tag = bank1[bank1Index][1];	
	
	bank2Index = bank1Index^(ghr1224);
	bank2tag = bank2[bank2Index][1];	
	
	bank3Index = bank2Index^(ghr2436)^(ghr3648);
	bank3tag = bank3[bank3Index][1];	
	
	bank4Index = bank3Index^(ghr4860)^(ghr6072);
	bank4tag = bank4[bank4Index][1];	
	
	bank5Index = bank4Index^(ghr7284)^(ghr8496);
	bank5tag = bank5[bank5Index][1];
	
	bank6Index = bank5Index^(ghr96108)^(ghr108120);
	bank6tag = bank6[bank6Index][1];
	
	bank7Index = bank6Index^(convertToDecimal(ghr,120,132))^(convertToDecimal(ghr,132,144));
	bank7tag = bank7[bank7Index][1];
	
	bank8Index = bank7Index^(convertToDecimal(ghr,144,156))^(convertToDecimal(ghr,156,168));
	bank8tag = bank8[bank8Index][1];
	
	bank9Index = bank8Index^(convertToDecimal(ghr,168,180))^(convertToDecimal(ghr,180,192));
	bank9tag = bank9[bank9Index][1];
	
	bank10Index = bank9Index^(convertToDecimal(ghr,192,204))^(convertToDecimal(ghr,204,216));
	bank10tag = bank10[bank10Index][1];	
	
	//compute tags for each branch end
	
	
	
	//make prediction start
	if(bank10tag == banktagComp)
	{
		if(bank10[bank10Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	
	
	
	if(bank9tag == banktagComp)
	{
		if(bank9[bank9Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}	
	
	
	if(bank8tag == banktagComp)
	{
		if(bank8[bank8Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	
	
	if(bank7tag == banktagComp)
	{
		if(bank7[bank7Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	
	
	
	if(bank6tag == banktagComp)
	{
		if(bank6[bank6Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	
	
	
	
	if(bank5tag == banktagComp)
	{
		if(bank5[bank5Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	
	
	if(bank4tag == banktagComp)
	{
		if(bank4[bank4Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	
	if(bank3tag == banktagComp)
	{
		if(bank3[bank3Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	if(bank2tag == banktagComp)
	{
		if(bank2[bank2Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	if(bank1tag == banktagComp)
	{
		if(bank1[bank1Index][2]>3)
			return TAKEN;
		else
			return NOT_TAKEN;
	}
	if(localPht[phtTableIndex][1]>3)
		return TAKEN;
	else
		return NOT_TAKEN;
	//make prediction end	
}
//prediction end



//update predictor start
void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget, bool btbANSF, bool btbATSF, bool btbDYN)
{	


	//update the tag for bank start
	int banknumber;
	if(bank10tag==banktagComp)
	{
		banknumber=10;
		if(resolveDir==TAKEN)
			bank10[bank10Index][2] = SatIncrement(bank10[bank10Index][2],7);
		else
			bank10[bank10Index][2] = SatDecrement(bank10[bank10Index][2]);
		goto label;
	}
	if(bank9tag==banktagComp)
	{
		banknumber=9;
		if(resolveDir==TAKEN)
			bank9[bank9Index][2] = SatIncrement(bank9[bank9Index][2],7);
		else
			bank9[bank9Index][2] = SatDecrement(bank9[bank9Index][2]);
		goto label;
	}
	if(bank8tag==banktagComp)
	{
		banknumber=8;
		if(resolveDir==TAKEN)
			bank8[bank8Index][2] = SatIncrement(bank8[bank8Index][2],7);
		else
			bank8[bank8Index][2] = SatDecrement(bank8[bank8Index][2]);
		goto label;
	}
	if(bank7tag==banktagComp)
	{
		banknumber=7;
		if(resolveDir==TAKEN)
			bank7[bank7Index][2] = SatIncrement(bank7[bank7Index][2],7);
		else
			bank7[bank7Index][2] = SatDecrement(bank7[bank7Index][2]);
		goto label;
	}
	if(bank6tag==banktagComp)
	{
		banknumber=6;
		if(resolveDir==TAKEN)
			bank6[bank6Index][2] = SatIncrement(bank6[bank6Index][2],7);
		else
			bank6[bank6Index][2] = SatDecrement(bank6[bank6Index][2]);
		goto label;
	}
	if(bank5tag==banktagComp)
	{
		banknumber=5;
		if(resolveDir==TAKEN)
			bank5[bank5Index][2] = SatIncrement(bank5[bank5Index][2],7);
		else
			bank5[bank5Index][2] = SatDecrement(bank5[bank5Index][2]);
		goto label;
	}
	if(bank4tag==banktagComp)
	{
		banknumber=4;
		if(resolveDir==TAKEN)
			bank4[bank4Index][2] = SatIncrement(bank4[bank4Index][2],7);
		else
			bank4[bank4Index][2] = SatDecrement(bank4[bank4Index][2]);
		goto label;
	}
	if(bank3tag==banktagComp)
	{
		banknumber=3;
		if(resolveDir==TAKEN)
			bank3[bank3Index][2] = SatIncrement(bank3[bank3Index][2],7);
		else
			bank3[bank3Index][2] = SatDecrement(bank3[bank3Index][2]);
		goto label;
	}
	if(bank2tag==banktagComp)
	{
		banknumber=2;
		if(resolveDir==TAKEN)
			bank2[bank2Index][2] = SatIncrement(bank2[bank2Index][2],7);
		else
			bank2[bank2Index][2] = SatDecrement(bank2[bank2Index][2]);
		goto label;
	}
	if(bank1tag==banktagComp)
	{
		banknumber=1;
		if(resolveDir==TAKEN)
			bank1[bank1Index][2] = SatIncrement(bank1[bank1Index][2],7);
		else
			bank1[bank1Index][2] = SatDecrement(bank1[bank1Index][2]);
		goto label;
	}
	
	banknumber=0;
	if(resolveDir==TAKEN)
		localPht[phtTableIndex][1] = SatIncrement(localPht[phtTableIndex][1],7);	
	else
		localPht[phtTableIndex][1] = SatDecrement(localPht[phtTableIndex][1]);	
	////update the tag for bank end
	
	
	
	
	 
  label:if(banknumber!=10 && predDir!=resolveDir)
	{
		int u1 = bank1[bank1Index][0];
		int u2 = bank2[bank2Index][0];
		int u3 = bank3[bank3Index][0];
		int u4 = bank4[bank4Index][0];
		int u5 = bank4[bank5Index][0];
		int u6 = bank4[bank6Index][0];
		int u7 = bank4[bank7Index][0];
		int u8 = bank4[bank8Index][0];
		int u9 = bank4[bank9Index][0];
		int u10 = bank4[bank10Index][0];
		if(banknumber==0)
		{			
			if(u10==0 || (u1==1 && u2==1 && u3==1 && u4==1&& u5==1&& u6==1&& u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}	
			}
			else
				goto label1;			
		}
		
		
		///////////////////////////////////
		if(banknumber==1)
		{
			
			if(u10==0 || (u2==1 && u3==1 && u4==1&& u5==1&& u6==1&& u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}	
			}
			else
				goto label2;
		}
		////////////////////////////////////////////////
		
		
		
		////////////////////////////////////////////////
		if(banknumber==2)
		{
			if(u10==0 || (u3==1 && u4==1&& u5==1&& u6==1&& u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label3;
		}
		////////////////////////////////////////////////
		
		////////////////////////////////////////////////
		if(banknumber==3)
		{
			if(u10==0 || (u4==1&& u5==1&& u6==1&& u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label4;
		}
		////////////////////////////////////////////////
		
		
		////////////////////////////////////////////////
		if(banknumber==4)
		{
			if(u10==0 || (u5==1&& u6==1&& u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label5;
		}
		////////////////////////////////////////////////
		
		////////////////////////////////////////////////
		if(banknumber==5)
		{
			if(u10==0 || (u6==1&& u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label6;
		}
		////////////////////////////////////////////////
		
		////////////////////////////////////////////////
		if(banknumber==6)
		{
			if(u10==0 || (u7==1&& u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label7;
		}
		////////////////////////////////////////////////
		
		////////////////////////////////////////////////
		if(banknumber==7)
		{
			if(u10==0 || (u8==1&& u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label8;
		}
		////////////////////////////////////////////////
		
		////////////////////////////////////////////////
		if(banknumber==8)
		{
			if(u10==0 || (u9==1))
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
			else
				goto label9;
		}
		////////////////////////////////////////////////
		
		////////////////////////////////////////////////
		if(banknumber==9)
		{
			if(u10==0)
			{
				bank10[bank10Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank10[bank10Index][2] = 4;
 					
 					else
 						bank10[bank10Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank10[bank10Index][2] = 4;
 					else
 						bank10[bank10Index][2] = 3;
				}
			}
		}
		/////////////////////////////////////////////////
		
		
		
		label1:	if(u1==0)
			{
				bank1[bank1Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank1[bank1Index][2] = 4;
 					
 					else
 						bank1[bank1Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank1[bank1Index][2] = 4;
 					else
 						bank1[bank1Index][2] = 3;
				}	
			}
		label2:	if(u2==0)
			{
				bank2[bank2Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank2[bank2Index][2] = 4;
 					
 					else
 						bank2[bank2Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank2[bank2Index][2] = 4;
 					else
 						bank2[bank2Index][2] = 3;
				}	
			}
		label3:	if(u3==0)
			{
				bank3[bank3Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank3[bank3Index][2] = 4;
 					
 					else
 						bank3[bank3Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank3[bank3Index][2] = 4;
 					else
 						bank3[bank3Index][2] = 3;
				}	
			}
			
		label4:	if(u4==0)
			{
				bank4[bank4Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank4[bank4Index][2] = 4;
 					
 					else
 						bank4[bank4Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank4[bank4Index][2] = 4;
 					else
 						bank4[bank4Index][2] = 3;
				}	
			}
		label5:	if(u5==0)
			{
				bank5[bank5Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank5[bank5Index][2] = 4;
 					
 					else
 						bank5[bank5Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank5[bank5Index][2] = 4;
 					else
 						bank5[bank5Index][2] = 3;
				}	
			}
		label6:	if(u6==0)
			{
				bank6[bank6Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank6[bank6Index][2] = 4;
 					
 					else
 						bank6[bank6Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank6[bank6Index][2] = 4;
 					else
 						bank6[bank6Index][2] = 3;
				}	
			}
		label7:	if(u7==0)
			{
				bank7[bank7Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank7[bank7Index][2] = 4;
 					
 					else
 						bank7[bank7Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank7[bank7Index][2] = 4;
 					else
 						bank7[bank7Index][2] = 3;
				}	
			}
		label8:	if(u8==0)
			{
				bank8[bank8Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank8[bank8Index][2] = 4;
 					
 					else
 						bank8[bank8Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank8[bank8Index][2] = 4;
 					else
 						bank8[bank8Index][2] = 3;
				}	
			}
		label9:	if(u9==0)
			{
				bank9[bank9Index][1] = banktagComp;
				if(localPht[phtTableIndex][0]==1)
				{
					if(resolveDir == TAKEN)					
						bank9[bank9Index][2] = 4;
 					
 					else
 						bank9[bank9Index][2] = 3;
 					
				}
				else
				{
					if(localPht[phtTableIndex][1] > 3)
						bank9[bank9Index][2] = 4;
 					else
 						bank9[bank9Index][2] = 3;
				}	
			}		
		
	}	
	
	//update m and u start
	if(resolveDir == predDir)
	{		
		localPht[phtTableIndex][0] = 1;		
		if(banknumber==1)
		{			
			bank1[bank1Index][0]  = 1;
		}
		if(banknumber==2)
		{			
			bank2[bank2Index][0]  = 1;
		}
		if(banknumber==3)
		{			
			bank3[bank3Index][0]  = 1;
		}
		if(banknumber==4)
		{			
			bank4[bank4Index][0]  = 1;
		}
		if(banknumber==5)
		{			
			bank5[bank5Index][0]  = 1;
		}
		if(banknumber==6)
		{			
			bank6[bank6Index][0]  = 1;
		}
		if(banknumber==7)
		{			
			bank7[bank7Index][0]  = 1;
		}
		if(banknumber==8)
		{			
			bank8[bank8Index][0]  = 1;
		}
		if(banknumber==9)
		{			
			bank9[bank9Index][0]  = 1;
		}
		if(banknumber==10)
		{			
			bank10[bank10Index][0]  = 1;
		}
	}	
	
	
	else
	{		
		localPht[phtTableIndex][0] = 0;		
		if(banknumber==1)
		{			
			bank1[bank1Index][0]  = 0;
		}
		if(banknumber==2)
		{			
			bank2[bank2Index][0]  = 0;
		}
		if(banknumber==3)
		{			
			bank3[bank3Index][0]  = 0;
		}
		if(banknumber==4)
		{			
			bank4[bank4Index][0]  = 0;
		}
		if(banknumber==5)
		{			
			bank5[bank5Index][0]  = 0;
		}
		if(banknumber==6)
		{			
			bank6[bank6Index][0]  = 0;
		}
		if(banknumber==7)
		{			
			bank7[bank7Index][0]  = 0;
		}
		if(banknumber==8)
		{			
			bank8[bank8Index][0]  = 0;
		}
		if(banknumber==9)
		{			
			bank9[bank9Index][0]  = 0;
		}
		if(banknumber==10)
		{			
			bank10[bank10Index][0]  = 0;
		}
	}
	//update m and u end
	
	
	
	//update ghr start
	for(int i=255;i>0;i--)
	{
		ghr[i]=ghr[i-1];
	}
	if(resolveDir == TAKEN)
		ghr[0]=1;
	else
		ghr[0]=0;
	//update ghr end	
}


void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
	return;	
}




#endif

