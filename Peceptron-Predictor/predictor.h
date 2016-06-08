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


#define theta 512.76f   // training constant

class PREDICTOR{


 private:
  int *ghr,**weight;
  float final_value;
  UINT64 tempPC;
  
  
 public:
  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC,bool& btbANSF, bool& btbATSF, bool& btbDYN);
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget,bool& btbANSF, bool& btbATSF,bool&  btbDYN);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir,UINT64 branchTarget);
  int 	convertToDecimal(int *var,int start,int end);
  UINT64 hash(UINT64 PC, int *ghr);
  
  
};




/* Hardware budget
GHR bits + weights vectors
= 32 + (4096 * 33 * 9)
= 1024 kbits
*/


int PREDICTOR::convertToDecimal(int *var,int start,int end)
{
	int ret = 0;
	int j=0;
	for(int i=start;i<=end;i++,j++)	
		ret = ret + (var[i]<<j);	
	return ret;
}

UINT64 PREDICTOR::hash(UINT64 PC, int *ghr)
{
	
	int *tempghr = (int *)calloc(32,sizeof(int));
	for(int i=0;i<32;i++)
	{
		if(ghr[i]==-1)
			tempghr[i]=0;
		else
			tempghr[i]=1;
	}
	
	int ghr011 = convertToDecimal(tempghr,0,11);
	int ghr1223 = convertToDecimal(tempghr,12,23);
	int ghr2031 = convertToDecimal(tempghr,20,31);
	free(tempghr);	
	return ((PC&0xFFF)^ghr011^ghr1223^ghr2031);	
}

PREDICTOR::PREDICTOR(void)
{
	//initialise start
	ghr=(int *)calloc(32,sizeof(int));
	weight=(int **)calloc(4096,sizeof(int *));
	for(int i=0;i<4096;i++)
	{
		weight[i]=(int *)calloc(33,sizeof(int));
	}
	for(int i=0;i<32;i++)
	{
		ghr[i]=1;
	}
	//initialise end
}

void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
  return;
}



bool PREDICTOR::GetPrediction(UINT64 PC,bool& btbANSF, bool& btbATSF, bool& btbDYN)
{
	//Hash PC start
	tempPC = hash(PC,ghr);
	//Hash PC end
	
	
	
	//final accumulated weight calculation start	
	final_value=weight[tempPC][0];	
	for(int i=1;i<33;i++)
	{
		if(ghr[i-1]==1)
			final_value = final_value + weight[tempPC][i];
		else
			final_value = final_value - weight[tempPC][i];
	}
	//final accumulated weight calculation start	
	
	
	
	
	//make prediction start
	if(final_value >= 0.0f)	
		return TAKEN;
	else
		return NOT_TAKEN;
	//make prediction end
	
}


void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget,bool& btbANSF, bool& btbATSF,bool&  btbDYN)
{	
	//update weights start
	if((resolveDir!=predDir) || (abs(final_value)<=theta))
	{
		if(resolveDir==TAKEN)
		{
			if(weight[tempPC][0]<=511)
			{
				weight[tempPC][0] = weight[tempPC][0] + 1;
			}
		}
			
		else
		{
			if(weight[tempPC][0]>=-512)
			{
				weight[tempPC][0] = weight[tempPC][0] - 1;
			}
		}
					
			
		for(int i=1;i<33;i++)
		{
			if((resolveDir==TAKEN && ghr[i-1]==1) || (resolveDir==NOT_TAKEN && ghr[i-1]==-1))
			{
				if(weight[tempPC][i]<=511)
				{
					weight[tempPC][i] = weight[tempPC][i] + 1;
				}
			}
			else
			{
				if(weight[tempPC][i]>=-512)
				{
					weight[tempPC][i] = weight[tempPC][i] - 1;
				}
			}
		}
			
	}
	//update weights end
	
	
	
	
	
	//update ghr start
	for(int i=31;i>0;i--)
	{
		ghr[i]=ghr[i-1];
	}
	
	if(resolveDir==TAKEN)
		ghr[0]=1;
	else
		ghr[0]=-1;
	//update ghr end
}



/***********************************************************/
#endif
