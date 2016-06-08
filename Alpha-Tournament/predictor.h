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


#define PHT_CTR_INIT 3 		//initial prediction - strongly taken
#define LOCAL_PATTERN_INIT 63

class PREDICTOR
{
 private:
  UINT32  ghr;    
  int *local_history_register;
  int *local_pattern_table;
  int *global_pattern_table;
  int *choice_pattern_table;
  bool temp_local,temp_global,tempghr;
  int local_history_index;
  int local_history_register_value;
  int local_final_value;
 public:
  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC,bool& btbANSF, bool& btbATSF, bool& btbDYN);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget,bool& btbANSF, bool& btbATSF,bool&  btbDYN);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir,UINT64 branchTarget);
};



//Total hardware budget = 1024Kbits
//4096*10 + 4096*6 + 4096*4*4




PREDICTOR::PREDICTOR(void)
{ 

  //initialisation start
  local_history_register = new int[4096];
  local_pattern_table = new int[4096];
  global_pattern_table = new int[4096];
  choice_pattern_table	= new int[4096];
  
  ghr = 0;
  for(int i=0; i < 4096; i++)
  {
	local_history_register[i] = 0; 
	local_pattern_table[i] = LOCAL_PATTERN_INIT; 
	global_pattern_table[i] = PHT_CTR_INIT;	
	choice_pattern_table[i] = PHT_CTR_INIT;
  }
  //initiasation end
}





void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
  return;
}


bool PREDICTOR::GetPrediction(UINT64 PC,bool& btbANSF, bool& btbATSF, bool& btbDYN)
{  
  local_history_index = (PC & 0xFFF) ^ ((PC & 0xFFF000)>>12) ^ ((PC & 0xFFF000000)>>24);
  local_history_register_value = (local_history_register[local_history_index] & 0xFFF);  
  local_final_value = (local_pattern_table[local_history_register_value]);
  
  if(local_final_value<=31)
  	temp_local=NOT_TAKEN;
  else
  	temp_local=TAKEN;
  
  
  tempghr = ghr & 0xFFF;
  int global_pattern_entry = (global_pattern_table[tempghr]);
  
  if(global_pattern_entry<=1)
  	temp_global=NOT_TAKEN;
  else
  	temp_global=TAKEN;
  
  
  
  if(temp_local == temp_global)
  	return temp_local;
  
  int choice_pattern_entry = (choice_pattern_table[tempghr]);  	
  if(choice_pattern_entry <= 1)
  	return temp_local;
  else
  	return temp_global;    
}


void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget,bool& btbANSF, bool& btbATSF,bool&  btbDYN)
{   
  
  //update choice_pattern_entry start  
  tempghr = ghr & 0xFFF;  
  if(choice_pattern_table[tempghr] == 0 && predDir == NOT_TAKEN && resolveDir == TAKEN)  
  	choice_pattern_table[tempghr] = 1;
  else if (choice_pattern_table[tempghr] == 1 && predDir == TAKEN && resolveDir == NOT_TAKEN)	
  	choice_pattern_table[tempghr] = 0;
  else if (choice_pattern_table[tempghr] == 1 && predDir == NOT_TAKEN && resolveDir == TAKEN)
  	choice_pattern_table[tempghr] = 2;
  else if(choice_pattern_table[tempghr] == 2 && predDir == TAKEN && resolveDir == NOT_TAKEN) 		
  	choice_pattern_table[tempghr] = 1;
  else if(choice_pattern_table[tempghr] == 2 && predDir == NOT_TAKEN && resolveDir == TAKEN)	
  	choice_pattern_table[tempghr] = 3;
  else if(choice_pattern_table[tempghr] == 3 && predDir == TAKEN && resolveDir == NOT_TAKEN)
  	choice_pattern_table[tempghr] = 2;
  else
    	asm volatile("nop");  //just a dummy instruction	
  //update choice_pattern_entry end
  
  
  

  
  //update local_pattern_table start  
  if(resolveDir == TAKEN)
  	local_pattern_table[local_history_register_value] = SatIncrement(local_pattern_table[local_history_register_value], (LOCAL_PATTERN_INIT));
  else
  	local_pattern_table[local_history_register_value] = SatDecrement(local_pattern_table[local_history_register_value]);
  //update local_pattern_table end
  
  
  	
  
  //update global_pattern_entry start
  tempghr = ghr & 0xFFF;
  if(resolveDir == TAKEN)
  	global_pattern_table[tempghr] = SatIncrement(global_pattern_table[tempghr], PHT_CTR_INIT);
  else
  	global_pattern_table[tempghr] = SatDecrement(global_pattern_table[tempghr]);  
  //update global_pattern_entry end 
  
  
  
  //update local_history_register start
  local_history_register[local_history_index] = (local_history_register[local_history_index] & 0xFFF) << 1;
  if(resolveDir == TAKEN)
  	local_history_register[local_history_index] = local_history_register[local_history_index] + 1;	
  //update local_history_register end
  
  
  
  //update ghr start
  ghr = (ghr&(0xFFF)) << 1;
  if(resolveDir == TAKEN)
	ghr=ghr+1;
  //update ghr end  
}
#endif

