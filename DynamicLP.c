/*******************************************************************************
*
******************************************************************************/
#include "std.h"
#include "AMPYEmailCommon.h"
#include "platform.h"
#include "stdio.h"
#include "string.h"
#include "kernel.h"
#include "intrinsics.h"
#include "LoadProf.h"
#include "Tables.h"
#include "metrology.h"
#include "RTC.h"
#include <stdlib.h>
#include <string.h>
#include "ANSIComms.h"
#include "Eventlog.h"
#include "metercomms.h"
#include "MetrologyDriver.h"
#include "SysMonitor.h"
#include "ATComms.h"

//--------------------------- #defs ----------------------------
//this value used to be 264 in the LG AX but those extra 8 bytes should be for CRC + seq No etc...
#define EE_PAGE_SIZE    DF_DATA_SIZE

//#define LPDEBUG

#define ROLL_OVER_CHK_VALUE   (ENERGY_ROLLOVER)
#define ROLL_OVER_ADJ         (ENERGY_ROLLOVER)
#define ZEROFILL_INT_PER_SET  50
#define LP_SET_FLG_ZFILL      0x01
#define LP_SET_FLG_ZTIMEFWD   0x02
#define LP_SET_FLG_2          0x04
#define LP_SET_FLG_1          0x08
#define LP_FLG_ENABLED        0x01
#define LP_FLG_DATAMOD        0x02
#define LP_FLG_CFGOK          0x04
#define LP_FLG_AUTOSTART      0x08
#define LP_FLG_CRCERR         0x10
#define LP_FLG_3              0x20
#define LP_FLG_2              0x40
#define LP_FLG_1              0x80
#define LP_CHK_MEMLEN         (MAXLPSIZE)
#define LP_CHK_LPFLAGS        0x0400
#define LP_CHK_LPFMAT         0x02
#define LP_CHK_NBRBLKS        (ACT_LP_TBL_DOT_NBR_BLKS_SET1)
#define LP_CHK_BLKINTS        (ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1)
#define LP_CHK_NBRCHAN        (NUM_CH_PER_DATA_SET)
#define LP_CHK_CHNFLG         (0x00)
#define LP_CHK_SRCSEL         (eT16_ATChannel4)
#define LP_CHK_INTFMTCDE      (LP_CTRL_TBL_DOT_INT_FMT_CDE1)

//LP QOS configuration (MT36)
#define LP_QOSNORMAL    0
#define LP_QOSMIN       1
#define LP_QOSMAX       2
#define LP_QOSAVG       3

// ds = dataset, cnl = channel.
#define LP_IsQOSMinMaxAvg(ds, cnl) ((lp_qos_cfg[ds]>>(cnl*2))&0x3)
#define LP_IsQOSMin(ds, cnl) (LP_IsQOSMinMaxAvg(ds, cnl)==LP_QOSMIN)
#define LP_IsQOSMax(ds, cnl) (LP_IsQOSMinMaxAvg(ds, cnl)==LP_QOSMAX)
#define LP_IsQOSAvg(ds, cnl) (LP_IsQOSMinMaxAvg(ds, cnl)==LP_QOSAVG)

//-------------------------- variables --------------------------
_mfg_table_56_t gLPState;//LP state machine variables
//TODO: ijd- lp_qos_cfg could be removed and ValSelect used instead
unsigned char lp_qos_cfg[NUM_LP_DATA_SETS]; // RAM version of nonvolatile: mfg_table_36
extern unsigned char gFirmwareUpdated;
extern unsigned char gFirmwareUpgrade;
unsigned char gLPConfigWritten = 0;
unsigned char gLPResetFlag = 0;

//------------------------ function prototypes -------------------
unsigned char LP_ChkIfIntervalEnd(_lp_data_set_state_t *pLPState);
void LP_CalcIntervalData(char index,unsigned char *Flags, unsigned short *Deltas, unsigned char BlockValUpdate, unsigned char BlockTimeUpdate);
void LP_WriteIntervalData(_lp_data_set_state_t *pLPState,unsigned char *Flags, unsigned short *Deltas,unsigned char index);
unsigned long LP_OffsetOfCurrBlock(_lp_data_set_state_t *pLPState);
unsigned long LP_OffsetOfCurrIntStatus(_lp_data_set_state_t *pLPState);
unsigned long LP_OffsetOfCurrIntValues(_lp_data_set_state_t *pLPState);
void LP_MoveToNextInterval(_lp_data_set_state_t *pLPState,unsigned char index);
unsigned long LP_CalcSizeOfBlock(_lp_data_set_state_t *pLPState);
unsigned long LP_CalcSizeOfInterval(_lp_data_set_state_t *pLPState);
unsigned short LP_TimeTillNextInterval(_lp_data_set_state_t *pLPState);
void LP_ZFIntervalData(unsigned char *Flags, unsigned short *Deltas, unsigned char ZFillType);
void LP_ChkEndZFill(_lp_data_set_state_t *pLPState);
void LP_CalcIntervalTime(_lp_data_set_state_t *pLPState);
unsigned char LP_CheckIfInvalidTables();
void LP_ChangeActiveModeFlags(unsigned char SetNum);
void LPZeroFillingTimeChgn(signed long TimeShift, unsigned char Source);
void LP_mirror_init(void);
void LP_mirror_shutdown(unsigned char LPSets);
void LP_MinMaxAvg(_lp_data_set_state_t *pLPState, char index, unsigned char *Flags);
void LP_ResetMinMaxAvg(_lp_data_set_state_t *pLPState,unsigned char index);
void LP_InitMinMaxAvg(void);

#ifdef Simulate
void SimLPWriteTimeStamp(void);
void SimLPWriteChannelData(unsigned short *ChData, unsigned short NumChs);
#endif

void LP_PrintLPState( void );

#define LP_SET1    0x01
#define LP_SET2    0x02
#define LP_SET3    0x04
#define LP_SET4    0x08
#define LP_SETALL  0x0F

#define LP_DIS_ACT_FLG  0x00
#define LP_ENB_ACT_FLG  0x01


enum{eLPSet1 = 0,
eLPSet2 = 1,
eLPSet3 = 2,
eLPSet4 = 3,};

enum{
  eLPNoChng    =1,
  eLPErrCnfg   =2,
  eLPErrAdd    =3,
  eLPErrDel    =4,
  eLPErrMod    =5,
  eLPHigDel    =6,
  eLPSucAdd    =7,
  eLPSucDel    =8,
  eLPSucMod    =9,
}LPrtn;

enum{
  eLPPwerUp = 0,
  eLPProg    = 1,
};

#pragma pack(1)
typedef struct{
  unsigned char  LP_Sets_Add;
  unsigned char  LP_Sets_Del;
  unsigned char  LP_Sets_Unchg;
  unsigned char  LP_Sets_Mod;
  unsigned char  LP_Sets_Reset;
  unsigned long  LP_Set_SizeN[NUM_LP_DATA_SETS];
  unsigned long  LP_Set_SizeE[NUM_LP_DATA_SETS];
  unsigned char  LP_RqstFlag;
  unsigned char  LP_ExstFlag;
  unsigned short LP_ReturnCode;
}LPConfigScratchPad;
#pragma pack()

unsigned char gLPZeroFilling = 0;

unsigned char LP_CheckifSetFits(unsigned char LPSet,LPConfigScratchPad *pLPScratchPad, _std_table_60_t *pLPSetConfigN);
unsigned char LP_CheckIfInvalidTables_Rev(unsigned char rqst,_std_table_60_t *LPConfiguration,_std_table_62_t *LPChnlData ,LPConfigScratchPad *pLPScratchPad);
void LP_SizeofExistingSets(_std_table_60_t *pLPSetConfigE, LPConfigScratchPad *pLPScratchPad);
void LP_DebugPrint(_std_table_60_t *pLPSetConfigDe, _std_table_60_t *pLPSetConfigEx,_std_table_60_t *pLPSetConfigNe,LPConfigScratchPad *pLPScratchPad);
void LP_ComputeReturnCode(unsigned char rqst,_std_table_60_t *pLPSetConfigN,LPConfigScratchPad *pLPScratchPad);
void LP_AutoStart();
void PowerFactorMinMax(unsigned long *NewPF, signed long *OldPF, unsigned char Type);
unsigned char gSecCheckDone=0;
unsigned char gMissingIntFlag[NUM_LP_DATA_SETS] = {0};
unsigned char gMeterTimeChanged = 0;
//--------------------------- functions ----------------------------
/*-------------------------------------------------------------------------------
* Function:    LP_SecTick
* Outputs:     Updated global variables in gLPState, write data to LP NV mem.
* Description: Updated global variables in gLPState, write data to LP NV mem.
*              Must be called for EVERY second while power is applied.
*------------------------------------------------------------------------------*/
void LP_SecTick()
{
  unsigned char i;
  //unsigned short Deltas[NUM_CH_PER_DATA_SET];
  //unsigned char Flags[(NUM_CH_PER_DATA_SET/2)+1];
  
  if (gSecCheckDone)
	  return;
  
  gSecCheckDone = 1;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(!(gLPState.LPFlags[i]&LP_FLG_ENABLED))
      continue;
	 
    if(LP_ChkIfIntervalEnd(&gLPState.sets[i]))
    {
      if(gLPState.sets[i].NumZeroInt)
      {//holey cat crap we are doing zero fill, we better wait until it's finished
        //calculate the values we would have written and save them
         memset(gLPState.sets[i].BkpFlags,0x00,(NUM_CH_PER_DATA_SET/2)+1);
         LP_MinMaxAvg(&gLPState.sets[i],i,gLPState.sets[i].BkpFlags);
         LP_CalcIntervalData(i,gLPState.sets[i].BkpFlags,gLPState.sets[i].BkpDeltas,1,1);
         TableRead(eStdT52_Clock,offsetof(_std_table_52_t,clock_calendar),sizeof(_ltime_date_t),&gLPState.sets[i].BkpLastTime);
		   //we end an interval in the middle of zero filling. Except all the delta, we should save the common INT flag as well
		   if(gLPState.sets[i].SFlags & LP_SET_FLG_ZTIMEFWD)
		   {
			   gLPState.sets[i].BkpFlags[0]|=LP_INT_FLAG_CLK_FWD;//mark with time shifted forwards flag
		   }
		   else
		   {
    	      if(!gFirmwareUpdated)
            {
               gLPState.sets[i].BkpFlags[0]|= LP_INT_FLAG_PF;//flag to indicate a power failure
            }
         }
		
         if(gLPState.sets[i].SFlags&LP_SET_FLG_ZFILL)
         {//now what? we have already filled our backup!
             gLPState.sets[i].NumZeroInt++;//do a nasty hack. if we are unlucky
             //enough to have multiple intervals during a single zero fill session
             //then all but the last of those intervals will appear as zero fill.
         }
         gLPState.sets[i].SFlags|=LP_SET_FLG_ZFILL;//set flag to show there was a conflict
      }
      else
      {///situation normal
          LP_CalcIntervalData(i,gLPState.sets[i].BkpFlags,gLPState.sets[i].BkpDeltas,0,0);
          LP_WriteIntervalData(&gLPState.sets[i],gLPState.sets[i].BkpFlags,gLPState.sets[i].BkpDeltas,i);
          LP_MoveToNextInterval(&gLPState.sets[i],i);
      }
    }
    else if((gLPState.sets[i].SFlags&LP_SET_FLG_ZFILL) && (!gLPState.sets[i].NumZeroInt))
    {//we have finished zero fill and we had an interval during that time
       gLPState.sets[i].LastTime=gLPState.sets[i].BkpLastTime;
       LP_WriteIntervalData(&gLPState.sets[i],gLPState.sets[i].BkpFlags,gLPState.sets[i].BkpDeltas,i);
       gLPState.sets[i].SFlags&=~(LP_SET_FLG_ZFILL |LP_SET_FLG_ZTIMEFWD) ;//clear flag to show there was a conflict
       LP_MoveToNextInterval(&gLPState.sets[i],i);
    }

	 
	 if(gMissingIntFlag[i])
	 {
      unsigned short Deltas[NUM_CH_PER_DATA_SET];
      unsigned char Flags[(NUM_CH_PER_DATA_SET/2)+1];
      unsigned short IntLenSec;
      IntLenSec = gLPState.sets[i].Max_Int_Time*60;
      memset(Flags,0x00,(NUM_CH_PER_DATA_SET/2)+1);
      LP_CalcIntervalData(i,Flags,Deltas,0,1);
      LP_CalcIntervalTime(&gLPState.sets[i]);
      LP_WriteIntervalData(&gLPState.sets[i],Flags,Deltas,i);
      LP_MoveToNextInterval(&gLPState.sets[i],i);
      gLPState.sets[i].TimeShiftDelta=IntLenSec-LP_TimeTillNextInterval(&gLPState.sets[i]);
      gLPState.sets[i].TimeShiftDelta*=-1;//make it negative
      gMissingIntFlag[i]=0;
	 }
  }

}

/*-------------------------------------------------------------------------------
* Function:    LP_CalcIntervalData
* Inputs:      index-index into gLPState.sets[]
*              BlockUpdate-if non-zero blocks modification of pLPState
* Outputs:     Flags-array to interval flags that will be modified
*              Deltas-array of interval values to be written later
* Description: Calculates interval values and and flags and places the results
*              in the provided arrays. If the interval is an energy quantity a
*              delta is calculated otherwise its a direct snapshot. Intervals
*              are determined based on value in table 28.
*------------------------------------------------------------------------------*/
void LP_CalcIntervalData(char index,unsigned char *Flags, unsigned short *Deltas,unsigned char BlockValUpdate, unsigned char BlockTimeUpdate)
{
  unsigned char i;
  unsigned long val,temp;
  _lp_data_set_state_t * pLPState = &gLPState.sets[index];
  unsigned char overflow;
  signed short wTemp=0;
  
  //memset(Flags,0x00,(NUM_CH_PER_DATA_SET/2)+1);//clear common flags and set LP_INT_STAT_NONE for each channel
  Flags[0]&=0x0F; // clear common flags

  
  Flags[0]|= (pLPState->SFlags&0xF0);//set up common flags
  //TODO: IJD DST flag for LP data. Flags[0]|= LP_INT_FLAG_DST;
  
  for(i=0;i<pLPState->Nbr_Chns;i++)
  {//loop through all channels in this data set
     unsigned char atError=FALSE;
     // Clear flags if we are not using AT fields
     if (!(pLPState->ValSelect[i]>=eT16_ATChannel1  && pLPState->ValSelect[i]<=eT16_ATChannel4) )
     {
        if ( (i%2)==0)
        {
           Flags[(i+1)/2]&=0xF0;
        }
        else
        {
           Flags[(i+1)/2]&=0x0F;
        }
     }

    overflow=0;
    //get the selected quantity for channel
    if(SourceIsEnergy(pLPState->ValSelect[i]))
    {
      TableRead(eStdT28_PresRegData,pLPState->ValSelect[i]*4 , sizeof(val), &val);
      temp=((unsigned long)val) - pLPState->LastVals[i];//channel is an energy quantity so do a delta calc
      if(!BlockValUpdate)
        pLPState->LastVals[i]=val;//update last value ready for next time
    }
    else if (SourceIsQOS(pLPState->ValSelect[i]))
    {
      // no need for diffs on QOS items.
      if (!LP_IsQOSMinMaxAvg(index, i))
      {
        TableRead(eStdT28_PresRegData,pLPState->ValSelect[i]*4 , sizeof(temp), &temp);
        if((pLPState->ValSelect[i] >= eT16_ActPwrPhA) && (pLPState->ValSelect[i] <= eT16_ReactPwrTot))
          temp /= 10;
      }
      else
      {
        if(LP_IsQOSAvg(index, i))
          temp = pLPState->LastVals[i] / gLPState.QOSAvgN[index][i];
        else
          temp = pLPState->LastVals[i];
      }
    }
    else if (pLPState->ValSelect[i]>=eT16_ATChannel1  && pLPState->ValSelect[i]<=eT16_ATChannel4 )
    {
   	 // ATProfile
       atError=!GetProfileValueValid((unsigned char)(pLPState->ValSelect[i])-eT16_ATChannel1);
       if (!LP_IsQOSMinMaxAvg(index, i))
       {
          temp=GetProfileValue((unsigned char)(pLPState->ValSelect[i])-eT16_ATChannel1);
          if (atError==TRUE)
          {
             temp = 0xffff;//7fff
          }
       }
       else
       {
          if(LP_IsQOSAvg(index, i))
             temp = pLPState->LastVals[i] / gLPState.QOSAvgN[index][i];
          else
             temp = pLPState->LastVals[i];
       }
    }
    else if (SysMon_SourceIsSpecial(pLPState->ValSelect[i]))
    {
      if (!LP_IsQOSMinMaxAvg(index, i))
      {
        TableRead(eMfgT24_TempAppStatus,18+(pLPState->ValSelect[i]-eT16_TempPhA)*50,sizeof(wTemp),&wTemp);
        temp = wTemp+1000;  //offset 100 degree to avoid negative value
      }
      else
      {
        if(LP_IsQOSAvg(index, i))
          temp = pLPState->LastVals[i] / gLPState.QOSAvgN[index][i];
        else
          temp = pLPState->LastVals[i];
      }
    }
    
    if(temp>ROLL_OVER_CHK_VALUE)
      temp+=ROLL_OVER_ADJ;//assume energy roll over
    if(temp>0xFFFF)
    {//overflow has occured, let it be known
      temp-=0xFFFF;
      overflow=1;
    }
    
    if (pLPState->ValSelect[i]>=eT16_ATChannel1  && pLPState->ValSelect[i]<=eT16_ATChannel4 )
    {
       overflow=0; // no overflows in AT
    }

    Deltas[i]=temp;//we can't write the value yet, flags must be written first
    
    if (atError==TRUE)
    {
       Flags[(i+1)/2]|=LP_INT_STAT_ATCOMMERROR<<((i%2)*4);
    }
    else if(overflow)//overflow has priority flag
      Flags[(i+1)/2]|=LP_INT_STAT_OVF<<((i%2)*4);
    else if((pLPState->TimeShiftDelta<0) && ((!gFirmwareUpdated)||gMeterTimeChanged))//partial interval flag
    {
      Flags[(i+1)/2]|=LP_INT_STAT_PART<<((i%2)*4);
    }
    else if(pLPState->TimeShiftDelta>0)//long interval flag
      Flags[(i+1)/2]|=LP_INT_STAT_LONG<<((i%2)*4);
  }
  if(!BlockTimeUpdate)
    TableRead(eStdT52_Clock,offsetof(_std_table_52_t,clock_calendar),sizeof(_ltime_date_t),&pLPState->LastTime);
  
}

/*-------------------------------------------------------------------------------
* Function:    LP_WriteIntervalData
* Inputs:      pLPState-pointer to LP data set status
*              Flags-array to interval flags that will be written
*              Deltas-array of interval values to be written
*              index-selects the desired LP data set
* Outputs:     Modifies global NV data in the external Flash chip
* Description: Writes an entire interval (flags and values) to the Load Profile
*              NV memory. This is done via TableWrite which has a special
*              buffered ram page for each LP data set. This function is designed
*              to be called in a predicable sequencial manner as LP is written
*              over time. This avoids burn out of the flash.
*------------------------------------------------------------------------------*/
void LP_WriteIntervalData(_lp_data_set_state_t *pLPState,unsigned char *Flags, unsigned short *Deltas,unsigned char index)
{
  unsigned long offset;
  unsigned char size;
#ifdef LPDEBUG
  // debugging
  unsigned char a;
  unsigned short int *DeltaDebug = Deltas;
#endif
  
  if(index>3)
    return;
#ifdef LPDEBUG
  // hack for debugging lp, use interval number instead of deltas
  printf("lp%u\n",pLPState->ActiveInterval + (pLPState->Nbr_blk_ints*pLPState->ActiveBlock));
  for(a=0;a<pLPState->Nbr_Chns;a++){ *DeltaDebug = pLPState->ActiveInterval + (pLPState->Nbr_blk_ints*pLPState->ActiveBlock); DeltaDebug++; }
#endif

  offset=LP_OffsetOfCurrBlock(pLPState)+LP_OffsetOfCurrIntValues(pLPState);
  size=pLPState->Nbr_Chns * sizeof(_int_fmt1_rcd_t);
  //write data, we must write data in sequencial order to allow for flash page write optimisation
  TableWrite(eStdT64_LPDataSet1+index,offset, size, Deltas);

  offset=LP_OffsetOfCurrBlock(pLPState)+LP_OffsetOfCurrIntStatus(pLPState);
  size=1+(pLPState->Nbr_Chns/2);
  //write data, we must write data in sequencial order to allow for flash page write optimisation
  TableWrite(eStdT64_LPDataSet1+index,offset, size, Flags);
}

/*-------------------------------------------------------------------------------
* Function:    LP_OffsetOfCurrBlock
* Inputs:      pLPState-pointer to LP data set status
* Outputs:     Returns the offset of the currenly active data block for the
given LP data set.
* Description: Returns the zero based offset from the start of the LP data
section. Note that all LP data sets share the same memory area.
i.e. for the first data block this function would return 0 for
data set 1 and return max_size_of(LP data set 1) for data set 2.
*------------------------------------------------------------------------------*/
unsigned long LP_OffsetOfCurrBlock(_lp_data_set_state_t *pLPState)
{
  unsigned long temp;
  
  temp=LP_CalcSizeOfBlock(pLPState);
  temp*=((unsigned long)pLPState->Nbr_Blks-1UL-(unsigned long)pLPState->ActiveBlock);
  return temp;
}

/*-------------------------------------------------------------------------------
* Function:    LP_OffsetOfCurrIntStatus
* Inputs:      pLPState-pointer to LP data set status
* Outputs:     Returns the offset of the currenly interval status array.
* Description: Returns the zero based offset from the start of currently active
data block. Returned values are relative.
*------------------------------------------------------------------------------*/
unsigned long LP_OffsetOfCurrIntStatus(_lp_data_set_state_t *pLPState)
{
  unsigned long temp;
  
  temp=LP_CalcSizeOfInterval(pLPState);
  temp*=((unsigned long)pLPState->Nbr_blk_ints-1UL-(unsigned long)pLPState->ActiveInterval);
  temp+=sizeof(_stime_date_t);//add offset for timestamp at start of block
  return temp;
}

/*-------------------------------------------------------------------------------
* Function:    LP_OffsetOfCurrIntValues
* Inputs:      pLPState-pointer to LP data set status
* Outputs:     Returns the offset of the currenly interval value array.
* Description: Returns the zero based offset from the start of currently active
data block. Returned values ar relative.
*------------------------------------------------------------------------------*/
unsigned long LP_OffsetOfCurrIntValues(_lp_data_set_state_t *pLPState)
{
  unsigned long temp;
  
  temp=LP_CalcSizeOfInterval(pLPState);
  temp*=((unsigned long)pLPState->Nbr_blk_ints-1UL-(unsigned long)pLPState->ActiveInterval);
  temp+=sizeof(_stime_date_t);//add offset for timestamp at start of block
  temp+=(pLPState->Nbr_Chns/2)+1;//add offset for status flags
  return temp;
}

/*-------------------------------------------------------------------------------
* Function:    LP_CalcSizeOfBlock
* Inputs:      pLPState-pointer to LP data set status
* Outputs:     Returns the maximum size of a data block.
*------------------------------------------------------------------------------*/
unsigned long LP_CalcSizeOfBlock(_lp_data_set_state_t *pLPState)
{
  unsigned long temp;
  
  //calculate size of interval array then add time stamp size
  temp=LP_CalcSizeOfInterval(pLPState);
  temp*=(unsigned long)pLPState->Nbr_blk_ints;
  temp+=sizeof(_stime_date_t);
  return temp;
}

/*-------------------------------------------------------------------------------
* Function:    LP_CalcSizeOfInterval
* Inputs:      pLPState-pointer to LP data set status
* Outputs:     Returns the size of a interval.
*------------------------------------------------------------------------------*/
unsigned long LP_CalcSizeOfInterval(_lp_data_set_state_t *pLPState)
{
  unsigned long interval;
  
  //calculate size of int_data array
  interval=sizeof(_int_fmt1_rcd_t);
  interval*=(unsigned long)pLPState->Nbr_Chns;
  
  //calculate size of extended_int_status array and add
  interval+=(pLPState->Nbr_Chns/2)+1;
  
  return interval;
}

/*-------------------------------------------------------------------------------
* Function:    LP_MoveToNextInterval
* Inputs:      pLPState-pointer to LP data set status
index-specifies the slected LP data set, 0,1,2 or 3.
* Outputs:     Updates status table and currenly active indexes in pLPState.
*------------------------------------------------------------------------------*/
void LP_MoveToNextInterval(_lp_data_set_state_t *pLPState,unsigned char index)
{
  _LP_Set_Status_RCD s;
  
  if(index>3)
    return;

  //don't reset the flag because we saved the flag for the current ending interval into bkgflags when ending interval during zero filling
  if (!gLPState.sets[index].SFlags&LP_SET_FLG_ZFILL)
  		memset(gLPState.sets[index].BkpFlags,0x00,(NUM_CH_PER_DATA_SET/2)+1);//clear common flags and set LP_INT_STAT_NONE for each channel

  TableRead(eStdT63_LPStatus,sizeof(_LP_Set_Status_RCD)*index,sizeof(_LP_Set_Status_RCD),&s);
  
  s.Nbr_Valid_Int = pLPState->ActiveInterval+1;
  
  //a block is considered valid if at least one interval has been written to it
  if(pLPState->ActiveInterval==0)
  {//first interval has just ended so the block will now be valid
    if(s.Nbr_Valid_Blocks < pLPState->Nbr_Blks)
    {
      s.Nbr_Valid_Blocks++;
      s.Nbr_Unread_Blocks++;
    }
    s.Last_Block_Element=pLPState->ActiveBlock;
    s.Last_Block_Seq_Nbr++;
  }
  pLPState->ActiveInterval++;
  
  if(pLPState->ActiveInterval >= pLPState->Nbr_blk_ints)
  {//block is now full
    pLPState->ActiveInterval=0;
    
    TableWrite(eStdT64_LPDataSet1+index,LP_OffsetOfCurrBlock(pLPState), sizeof(_stime_date_t), &pLPState->LastTime);
    
    pLPState->ActiveBlock++;
    if(pLPState->ActiveBlock >= pLPState->Nbr_Blks)
    {//we have filled the LP data set so wrap around to the start
      pLPState->ActiveBlock=0;
    }
  }
  TableWrite(eStdT63_LPStatus,sizeof(_LP_Set_Status_RCD)*index,sizeof(_LP_Set_Status_RCD),&s);
  
  //now that data is written, clear flags ready for next interval
  pLPState->SFlags&=0x0F;
  pLPState->TimeShiftDelta=0;
  LP_ResetMinMaxAvg(pLPState,index);
}

/*-------------------------------------------------------------------------------
* Function:    LP_SetUpLPStateGlobal
* Outputs:     modifies global gLPState
* Description: Initialises values in gLPState based on the contents of std 61 &
*              62. This function assumes that values in these tables are valid.
*------------------------------------------------------------------------------*/
void LP_SetUpLPStateGlobal()
{
  unsigned char i,j;
  unsigned long LastSetEnd=0;
  unsigned short remainder;
  //   unsigned long LP_PosixTime;
  //   _LP_Set_Status_Bfld S63StatusBfld;
  
  gLPState.NumSets=0;
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {//check how many LP set have been enabled 
    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*i),sizeof(_lp_config_t),&gLPState.sets[i].Nbr_Blks);
    if(gLPState.sets[i].Nbr_Blks)
    {
      gLPState.NumSets |= (1<<i);
      
      if(gLPState.LPFlags[i]&LP_FLG_CRCERR) //There was data corruption in this set earlier
        gLPState.LPFlags[i] &= ~(LP_FLG_CRCERR);
      
      gLPState.LPFlags[i] |= (LP_FLG_ENABLED|LP_FLG_CFGOK);
    }
    else
    {
      gLPState.LPFlags[i] = 0;
      memset(&gLPState.sets[i], 0, sizeof(gLPState.sets[i]));
      continue;
    }
    
    //set up ValSelect
    for(j=0;j<NUM_CH_PER_DATA_SET;j++) 
    {
      TableRead(eStdT62_LPControl,(sizeof(_Data_Selection_RCD)*i)+(sizeof(_LP_Source_Sel_RCD_t)*j)
                + offsetof(_LP_Source_Sel_RCD_t,LP_Source_Select),
                1,&gLPState.sets[i].ValSelect[j]);
    }
    TableRead(eMfgT36_LP_QOS_Cfg, 0, NUM_LP_DATA_SETS, lp_qos_cfg);
    LP_InitMinMaxAvg();
    //set up DataSetOffset
    gLPState.sets[i].DataSetOffset=LastSetEnd;
    LastSetEnd=LP_CalcSizeOfBlock(&gLPState.sets[i]);
    LastSetEnd*=gLPState.sets[i].Nbr_Blks;
    LastSetEnd+=gLPState.sets[i].DataSetOffset;
    remainder=LastSetEnd%EE_PAGE_SIZE;
    LastSetEnd+=(EE_PAGE_SIZE-remainder);//jump to next flash page
  }
}


/*-------------------------------------------------------------------------------
* Function:    LP_Init
* Outputs:     ?
* Description: Called when AMPY task is initialised (NOT every time the meter is
*              power up). This function is different to LP_PowerUp(). It's
*              possible that LP_PowerUp() and LP_PowerDown() could be called
*              multiple times but LP_Init() is called only once if there is a
*              series of short power interruptions.
*------------------------------------------------------------------------------*/
void LP_Init(unsigned long DownTimeDuration)
{
  unsigned long CRCRef,crc32,crcTemp;
  unsigned char temp;
  
  TableRead(eMfgT56_LPState,0,sizeof(gLPState),&gLPState);
  TableRead(eMfgT36_LP_QOS_Cfg, 0, NUM_LP_DATA_SETS, lp_qos_cfg);
  TableRead(eMfgT00_Hash, eHashLP*sizeof(_hash_entry_t), sizeof(CRCRef), (void *)&CRCRef);
  crc32=0;
  CalcTableHash(eStdT61_ActualLPLimit,&crc32);
  CalcTableHash(eStdT62_LPControl,&crc32);
  crcTemp = crc32;
  CalcTableHash(eMfgT36_LP_QOS_Cfg,&crc32);

#if defined(AMPY_METER_NO_RF)
  //In interruption test we found the fw upgrade event was falsely trigger which cause this piece of code had been excuted twice. 
  //To cover the falsely trigger fw upgrade issue, use the gLPState.sets[0].Nbr_blk_ints as the indicator. Now that Nbr_blk_ints is fixed to 96,
  //we don't need to shift the setting again if the 96 has already been in the correct position.  The 3 bytes later field cannot be 96.
  unsigned char *pDat;
  unsigned short pos;
  if((gFirmwareUpdated)&&(gLPState.sets[0].Nbr_blk_ints!=96))
  { 
     pDat = (unsigned char*)&gLPState; 
     for(pos=(sizeof(gLPState)-1); pos>=offsetof(_mfg_table_56_t,QOSAvgN[0][0]); pos--)
       pDat[pos] = pDat[pos-11];
     pos=offsetof(_mfg_table_56_t,QOSAvgN[0][0]);
     pos--;
     for(;pos>=offsetof(_mfg_table_56_t,sets[0].Nbr_Blks);pos--)
       pDat[pos] = pDat[pos-3];
     gLPState.LPFlags[1]=gLPState.LPFlags[2]=gLPState.LPFlags[3]=gLPState.sets[0].LastUnreadBlk=0;     
  }
#endif
  
  if((crc32 != CRCRef) )//&& (!gFirmwareUpdatedLP))
  {
    temp=0;
    if(gFirmwareUpdated)
    {
       if (crcTemp != CRCRef)
         temp=eHashLP;
    }        
    TableWrite(eMfgT00_Hash, eHashLP*sizeof(_hash_entry_t), sizeof(crc32), (void *)&crc32);
    UpdateHashCheck();
    if (temp==eHashLP)
    {
       EventLogWrite(eEvtNVFail,0,1,&temp);      
       memset(&gLPState.NumSets,0,sizeof(gLPState.NumSets));
       memset(&gLPState.LPFlags,LP_FLG_CRCERR,sizeof(gLPState.LPFlags));
       LP_ChangeActiveModeFlags(LP_SETALL);
    }
  }

#if defined(LIMITED_PRODUCTION_VERSION)
  memset(&gLPState.NumSets,0,sizeof(gLPState.NumSets));
  memset(&gLPState.LPFlags,0,sizeof(gLPState.LPFlags));
  LP_ChangeActiveModeFlags(LP_SETALL);
#endif
       
  /*if(gFirmwareUpdatedLP)
    memset(&gLPState.LPFlags[1],0,sizeof(unsigned char)*3);*/
  
  LP_PowerUp(DownTimeDuration);  
  LP_AutoStart();
}

/*-------------------------------------------------------------------------------
* Function:    LP_FinishZFill
* Inputs:      pLPState-pointer to LP data set status.
* Outputs:     pLPState-pointer to LP data set status.
* Description: Checks to see if the zero fill process has finished. If it has
*              then clear the flags for the zero filling.
*------------------------------------------------------------------------------*/
void LP_FinishZFill(_lp_data_set_state_t *pLPState)
{
   pLPState->SFlags = pLPState->SFlags&(~LP_SET_FLG_ZTIMEFWD);
}

/*-------------------------------------------------------------------------------
* Function:    LP_MainLoopTick
* Outputs:     ?
* Description: Called every main loop tick ~30ms
*------------------------------------------------------------------------------*/
void LP_MainLoopTick()
{
  unsigned char i,j;
  unsigned short Deltas[NUM_CH_PER_DATA_SET];
  unsigned char Flags[(NUM_CH_PER_DATA_SET/2)+1];
  signed short Ttemp;
  
  if(gLPConfigWritten)
  {
    gLPConfigWritten = 0;
    LP_SetUpLPStateGlobal();
    LP_Reset(gLPResetFlag);
    //LP_PrintLPState();
  }
  
  if(gZCLoss==1)
    return;//meter is experiencing a mains loss, halt LP zerofilling.
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(gZCLoss==1)
      return;//meter is experiencing a mains loss, halt LP zerofilling.
    
    if(!(gLPState.LPFlags[i]&LP_FLG_ENABLED))
      continue;

    if(gLPState.sets[i].NumZeroInt)
    {//there is some zero fill required
		 
		 if(gLPState.sets[i].SFlags & LP_SET_FLG_ZTIMEFWD)
        LP_ZFIntervalData(Flags,Deltas,eTimeChange);
      else
        LP_ZFIntervalData(Flags,Deltas,eLPPowerUp);
		
      j=ZEROFILL_INT_PER_SET;
		gLPZeroFilling = 1;
      while((gLPState.sets[i].NumZeroInt)&&(j))
      {
		  LP_WriteIntervalData(&gLPState.sets[i],Flags,Deltas,i);
        LP_CalcIntervalTime(&gLPState.sets[i]);
        Ttemp=gLPState.sets[i].TimeShiftDelta;		  
        LP_MoveToNextInterval(&gLPState.sets[i],i);
        gLPState.sets[i].TimeShiftDelta=Ttemp;
        gLPState.sets[i].NumZeroInt--;
        LP_ChkEndZFill(&gLPState.sets[i]);
        j--;
		  if(gZCLoss==1)
		  {
				break;
		  }
      }
		if (!gLPState.sets[i].NumZeroInt)
			LP_FinishZFill(&gLPState.sets[i]);
		gLPZeroFilling = 0;
    }
	 
    if( (g_MetrologyTableUpdate == TRUE) && (gZCLoss!=1) )//meter is not experiencing a mains loss, and new meterology data is halt QOS.
      LP_MinMaxAvg(&gLPState.sets[i],i,gLPState.sets[i].BkpFlags);
  }
}

/*-------------------------------------------------------------------------------
* Function:    LP_TableUpdate
* Outputs:     Updates status table and currenly active indexes in pLPState.
* Description: Called when LP config tables are written.
*------------------------------------------------------------------------------*/
void LP_TableUpdate()
{
  unsigned long crc32 = 0;
  //gLPState.LPFlags&=~(LP_FLG_CFGOK);//signal the configuration is bad
  //LP_Disable();
  //gLPState.LPFlags|=LP_FLG_DATAMOD;
  CalcTableHash(eStdT61_ActualLPLimit,&crc32);
  CalcTableHash(eStdT62_LPControl,&crc32);
  CalcTableHash(eMfgT36_LP_QOS_Cfg,&crc32);
  TableWrite(eMfgT00_Hash, eHashLP*sizeof(_hash_entry_t), sizeof(crc32), (void *)&crc32);
  UpdateHashCheck();
}

/*-------------------------------------------------------------------------------
* Function:    LP_PowerDown
* Outputs:     ?
* Description: Called when meter power is lost.
*------------------------------------------------------------------------------*/
void LP_PowerDown()
{
  unsigned char i;
  signed short LostTime;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(!(gLPState.LPFlags[i]&LP_FLG_ENABLED))
      continue;
	 //we power up at interval boundary and power down at interval boundary, LP_Sec hasn't got a channce to run
	 //thus one inerval has been missed.  Add 1 to the num zero int to compensate.   It's urgly but we cannot do 
	 //a complete solution because it is in the power down process
    //the RTC init takes 2 seconds. When power comes back, we shouldn't have the same issue.  Double intervals should not happen
	 
    LostTime=LP_TimeTillNextInterval(&gLPState.sets[i]);
    if(!gFirmwareUpgrade)
    {
      //printf("LP POWER DWN\n");
      gLPState.sets[i].SFlags|=LP_INT_FLAG_PF;//set power fail flag
    }
    gLPState.sets[i].TimeShiftDelta-=LostTime;//subtract the lost time from the interval
    //don't write any partial intervals here, writing will be dealt with on power up
  }
#if  (1)
    LP_mirror_shutdown(LP_SETALL);
#endif
  
  TableWrite(eMfgT56_LPState,0,sizeof(gLPState),&gLPState);
}

/*-------------------------------------------------------------------------------
* Function:    LP_PowerUp
* Inputs:      DownTimeDuration- duration of power outage in seconds.
* Outputs:     updates global gLPState and may write LP data.
* Description: Called when power returns to the meter.
*------------------------------------------------------------------------------*/
void LP_PowerUp(unsigned long DownTimeDuration)
{
  unsigned char i;
  unsigned char resetflag = 0;
  
  gSecCheckDone = 0;
  gMeterTimeChanged = 0;
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(!(gLPState.LPFlags[i]&LP_FLG_ENABLED))
      continue;
    if((gLPState.sets[i].Nbr_blk_ints!=LP_CHK_BLKINTS) || (gLPState.sets[i].Nbr_Chns>LP_CHK_NBRCHAN) )
    {
      gLPState.NumSets &= ~(1<<i);
      gLPState.LPFlags[i]&=~LP_FLG_ENABLED;  // disable lp if sets out of range
      gLPState.LPFlags[i]&=~(LP_FLG_CFGOK);  //signal the configuration is bad
      resetflag |= (1<<i);
    }
	 gMissingIntFlag[i] = 0;
  }
  LP_ChangeActiveModeFlags(resetflag);
  
#if  (1)
  LP_mirror_init();
#endif
  LPZeroFillingTimeChgn(DownTimeDuration, eLPPowerUp);
}

/*-------------------------------------------------------------------------------
* Function:    LP_ChkIfIntervalEnd
* Inputs:      pLPState-pointer to LP data set status.
* Outputs:     Returns non-zero if an interval has just expired.
* Description: Check the meter time and determine if an LP interval has just
*              expired.
*------------------------------------------------------------------------------*/
unsigned char LP_ChkIfIntervalEnd(_lp_data_set_state_t *pLPState)
{
  unsigned char temp;
  TableRead(eStdT52_Clock,offsetof(_std_table_52_t,clock_calendar.second),sizeof(temp),&temp);
  if(temp==0)
  {//we are on an 1 minute boundry, do more checking
    if(LP_TimeTillNextInterval(pLPState)==0)
      return 1;
  }
  /*
  else if (!gLPState.sets[i].NumZeroInt)
  {
	   lastIntTime = pLPState->LastTime.hour;
		lastIntTime *= 60;
		lastIntTime += pLPState->LastTime.minute;
		lastIntTime *= 60;
		lastIntTime += pLPState->LastTime.second;
		if ((lastIntTime - time)>a) //missing one LP interval
			return 1;
  }*/
  return 0;
}

/*-------------------------------------------------------------------------------
* Function:    LP_TimeTillNextInterval
* Inputs:      pLPState-pointer to LP data set status.
* Outputs:     Returns number of seconds remaining until the end of the interval
* Description: Returns number of seconds remaining until the end of the interval
*------------------------------------------------------------------------------*/
unsigned short LP_TimeTillNextInterval(_lp_data_set_state_t *pLPState)
{
  unsigned long time;
  unsigned char temp;
  unsigned short a;
  
  TableRead(eStdT52_Clock,offsetof(_std_table_52_t,clock_calendar.hour),sizeof(temp),&temp);
  time=temp;//get hour
  time*=60;//convert to minutes
  TableRead(eStdT52_Clock,offsetof(_std_table_52_t,clock_calendar.minute),sizeof(temp),&temp);
  time+=temp;//add minutes
  time*=60;//convert to seconds
  TableRead(eStdT52_Clock,offsetof(_std_table_52_t,clock_calendar.second),sizeof(temp),&temp);
  time+=temp;//add seconds
  
  a=pLPState->Max_Int_Time;
  a*=60;//convert to seconds
  
  return (a - (time%a))%a;
}

/*-------------------------------------------------------------------------------
* Function:    LP_ZFIntervalData
* Outputs:     Flags-array to interval flags that will be written later
*              Deltas-array of interval values to be written later
* Description: Calculates Zero Fill interval values and flags. This data will
*              then be written for each interval.
*------------------------------------------------------------------------------*/
void LP_ZFIntervalData(unsigned char *Flags, unsigned short *Deltas, unsigned char ZFillType)
{
  unsigned char i;
  
  memset(Flags,0x00,(NUM_CH_PER_DATA_SET/2)+1);//clear common flags and set LP_INT_STAT_NONE for each channel
  memset(Deltas,0x00,NUM_CH_PER_DATA_SET*2);//clear deltas for each channel
  
  if(ZFillType==eLPPowerUp) 
  {
    if(!gFirmwareUpdated)
    {
      //printf("LP ZFIntervalData 1\n");
      Flags[0]|= LP_INT_FLAG_PF;//flag to indicate a power failure
    }
  }
  else
    Flags[0]|=LP_INT_FLAG_CLK_FWD;//mark with time shifted forwards flag
  //TODO: IJD DST flag for LP fill. Flags[0]|= LP_INT_FLAG_DST;//set up common flags
  
  for(i=0;i<NUM_CH_PER_DATA_SET;i++)
  {//loop through all channels in this data set
    Flags[(i+1)/2]|=LP_INT_STAT_SKIP<<((i%2)*4);
  }
}

/*-------------------------------------------------------------------------------
* Function:    LP_ChkEndZFill
* Inputs:      pLPState-pointer to LP data set status.
* Outputs:     pLPState-pointer to LP data set status.
* Description: Checks to see if the zero fill process has finished. If it has
*              then the next interval may have to be set up to indicate a
*              partial interval due to power failure.
*------------------------------------------------------------------------------*/
void LP_ChkEndZFill(_lp_data_set_state_t *pLPState)
{
  if(pLPState->NumZeroInt==0)
  {//yes we have just finished doing zero fill
    //check if the next interval will have to be marked with PF flags
    
    if(pLPState->TimeShiftDelta<0)
    {
      if(pLPState->SFlags&LP_SET_FLG_ZTIMEFWD)
        pLPState->SFlags|=LP_INT_FLAG_CLK_FWD;//mark with time shifted forwards flag
      else if(!gFirmwareUpdated)
      {
        //printf("LP_ChkEndZFill\n");
        pLPState->SFlags|=LP_INT_FLAG_PF;//mark with power fail flag
      }
    }
  }
}

/*-------------------------------------------------------------------------------
* Function:    LP_CalcIntervalTime
* Inputs:      pLPState-pointer to LP data set status.
* Outputs:     pLPState-pointer to LP data set status.
* Description: Increments the LastTime in pLPState to the next interval that
*              would occure. This function is used during zero fill.
*------------------------------------------------------------------------------*/
void LP_CalcIntervalTime(_lp_data_set_state_t *pLPState)
{
  unsigned long time;
  unsigned short duration;
  
  //convert time (_ltime_date_t) to posix
  time=RTC_ANSILTimeToPosix(&pLPState->LastTime);
  
  //add interval duration
  duration=pLPState->Max_Int_Time;
  duration*=60;//convert to seconds
  time+=duration;
  
  //convert posix back to _ltime_date_t
  RTC_PosixToANSILTime(time,&pLPState->LastTime);
}

/*-------------------------------------------------------------------------------
* Function:    LP_Enable
* Outputs:     returns zero if LP is successfully enabled.
* Description: Enable the Load Profile.
*------------------------------------------------------------------------------*/
unsigned char LP_Enable()
{
  unsigned char ret=eLPErrPass;
  unsigned char eventbuff[3]={0};
  //unsigned long tl;
  //unsigned short ts;
  unsigned char tc,i;
  
  /*if(gFirmwareUpdatedLP)
    return eLPErrPass;*/
  
  eventbuff[0] = 2;//This is to tell empwin this is a new LP implementation
  if(gLPState.LPFlags[0]&LP_FLG_CRCERR)
  {
    EventLogWrite(eEvtLPStatus,0,3,eventbuff);
    ret = eLPErrCRCErr;
  }
  
  /*
  TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,LP_Memory_Len),sizeof(tl),&tl);
  if((tl!=LP_CHK_MEMLEN) && (ret == eLPErrPass))
  {
  ret = eLPErrMemSize;
}
  
  TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,lp_flags),sizeof(ts),&ts);
  if((ts!=LP_CHK_LPFLAGS) && (ret == eLPErrPass))
  {
  ret = eLPErrFlags;
}
  TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,lp_fmats),sizeof(tc),&tc);
  if((tc!=LP_CHK_LPFMAT) && (ret == eLPErrPass))
  {
  ret = eLPErrFmats;
}
  
  if(ret != eLPErrPass)
  {
#warning Update the eEvtLPStatus here
  return ret;
}*/
  
  tc = 0;
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(gLPState.NumSets &(1<<i))//The set has valid configuration
    {
      if(!(gLPState.LPFlags[i] & LP_FLG_ENABLED))//The set has been disabled
      {
        if(gLPState.sets[i].SFlags & (LP_INT_FLAG_CLK_BACK|LP_INT_FLAG_CLK_FWD))//Time change caused the set to stop
          tc |= (1<<i);//reset the set
      }
    }
  }
  LP_Reset(tc);
  return ret;
  /*
  if(ret == eLPErrPass)
  {
  EventLogWrite(eEvtLPStatus,0,1,&i);
  gLPState.LPFlags&=~(LP_FLG_AUTOSTART);//Reset the LP_FLG_AUTOSTART so that LP can be enabled.
}
  */
}

/*-------------------------------------------------------------------------------
* Function:    LP_ChangeActiveModeFlags
* Inputs:      LP_FLG_ENABLED bit in the global  gLPState.LPFlags.
* Outputs:     Changes flags in std table 63.
* Description: Changes flags in std table 63. Only the Active_Mode_Flag is
*              changed if LP_FLG_CFGOK is set. If LP_FLG_ENABLED is set then the
*              Active_Mode_Flag for each configured data set will be set.
*              Everything cleared if LP_FLG_CFGOK is not set.
*------------------------------------------------------------------------------*/
void LP_ChangeActiveModeFlags(unsigned char SetNum)
{
  unsigned char i;
  _LP_Set_Status_Bfld S63StatusBfld;
  
  if(SetNum == 0)
    return;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(SetNum & (1<<i))
    {
      TableRead(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*i)+offsetof(_LP_Set_Status_RCD,LP_Set_Status_Flags),sizeof(S63StatusBfld),&S63StatusBfld);
      if(gLPState.LPFlags[i]&LP_FLG_ENABLED)
        S63StatusBfld.Active_Mode_Flag=1;
      else
        S63StatusBfld.Active_Mode_Flag=0;
      TableWrite(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*i)+offsetof(_LP_Set_Status_RCD,LP_Set_Status_Flags),sizeof(S63StatusBfld),&S63StatusBfld);
      
      if(!(gLPState.LPFlags[i]&LP_FLG_CFGOK))
        TableWrite(eStdT63_LPStatus,offsetof(_std_table_63_t,LP_Status_Set)+(i*sizeof(_LP_Set_Status_RCD)),sizeof(_LP_Set_Status_RCD),NULL);
    }
  }
}

/*-------------------------------------------------------------------------------
* Function:    LP_Reset
* Description: Resets and clears the Load Profile
*------------------------------------------------------------------------------*/
void LP_Reset(unsigned char ResetFlag)
{
  unsigned char i;
  unsigned long LP_PosixTime;
  _LP_Set_Status_Bfld S63StatusBfld;
  unsigned char eventbuff[3] = {0};
  
  if((ResetFlag == 0))// || (gFirmwareUpdatedLP))
    return;
  for (i=0; i<NUM_LP_DATA_SETS; i++)
  {
    eventbuff[0] = 2;//This is to tell empwin this is a new LP implementation
    if(ResetFlag & (1<<i))
    {
      gLPState.LPFlags[i] &= ~(LP_FLG_ENABLED);
      
      memset((unsigned char*)&S63StatusBfld, 0, sizeof(S63StatusBfld));
      S63StatusBfld.Block_Order = 1;
      S63StatusBfld.Interval_Order = 1;
      S63StatusBfld.List_Type = 1;
      
      //maintain Last_Block_Seq_Nbr, set flags, zero other stuff
      TableWrite(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*i)+offsetof(_LP_Set_Status_RCD,LP_Set_Status_Flags),sizeof(S63StatusBfld),&S63StatusBfld);
      TableWrite(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*i)+offsetof(_LP_Set_Status_RCD,Nbr_Valid_Blocks),4,NULL);//clear Nbr_Valid_Blocks and Last_Block_Element
      TableWrite(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*i)+offsetof(_LP_Set_Status_RCD,Nbr_Unread_Blocks),4,NULL);//clear Nbr_Unread_Blocks and Nbr_Valid_Int
      
      if(gLPState.NumSets &(1<<i))
      {
        //Get the LastTime to the point to last valid interval boundary.
        LP_PosixTime = RTC_GetFunctionalTime(0,0,0,eRTCFT_GMT);
        LP_PosixTime += LP_TimeTillNextInterval(&gLPState.sets[i]);
        LP_PosixTime -= (gLPState.sets[i].Max_Int_Time * 60);
        RTC_PosixToANSILTime(LP_PosixTime,&gLPState.sets[i].LastTime);
        //set up LastVals and LastTime (BkpDeltas and BkpFlags will be filled with junk data)
        memset(&gLPState.sets[i].BkpFlags[0],0x00,(NUM_CH_PER_DATA_SET/2)+1);
        LP_CalcIntervalData(i,&gLPState.sets[i].BkpFlags[0],&gLPState.sets[i].BkpDeltas[0],0,1);
        
        gLPState.sets[i].SFlags=0;
        
        //set up TimeShiftDelta
        gLPState.sets[i].TimeShiftDelta=gLPState.sets[i].Max_Int_Time;
        gLPState.sets[i].TimeShiftDelta*=60;
        gLPState.sets[i].TimeShiftDelta=(signed short)LP_TimeTillNextInterval(&gLPState.sets[i])-gLPState.sets[i].TimeShiftDelta;
        
        
        gLPState.sets[i].ActiveInterval=0;//current interval that is active
        
        /*//set up ActiveBlock and attemp to maintain last value this may help
        //reduce flash page burn out. It's not very accurate. There should
        //only be few calls to LP_SetUpLPStateGlobal() in a meters lifetime.
        if(gLPState.sets[i].ActiveBlock>= gLPState.sets[i].Nbr_Blks)
        gLPState.sets[i].ActiveBlock=0;//current block that is active*/
        
        gLPState.sets[i].ActiveBlock=0;//TODO: IJD- do we want to do this it may increase FLASH burn-out
        
        gLPState.sets[i].NumZeroInt=0;
        
        if(!(gLPState.LPFlags[i] & LP_FLG_AUTOSTART))
          gLPState.LPFlags[i] |= LP_FLG_ENABLED; 
      }
    }
    if(gLPState.LPFlags[i] & LP_FLG_ENABLED)
      eventbuff[(i/2)+1] |= (1<<((i%2)*4));
    else
      eventbuff[(i/2)+1] &= ~(1<<((i%2)*4));
  }
  LP_ChangeActiveModeFlags(ResetFlag);
  EventLogWrite(eEvtLPStatus,0,3,eventbuff);
}

/*-------------------------------------------------------------------------------
* Function:    LP_Disable
* Outputs:     Returns non-zero value if a failure (bad data) occurs.
* Description: Disable the Load Profile.
*------------------------------------------------------------------------------*/
void LP_Disable()
{
  unsigned char eventbuff[3] = {0};
  unsigned char temp=0;
  unsigned char i = 0;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(gLPState.NumSets & (1<<i))
    {
      if(gLPState.LPFlags[i] & LP_FLG_ENABLED)
      {
        gLPState.LPFlags[i]&=~(LP_FLG_ENABLED);
        temp |= (1<<i);
        
      }
    }  
  }
  LP_ChangeActiveModeFlags(temp);
  eventbuff[0] = 2;//This is to tell empwin this is a new LP implementation
  memset(&eventbuff[1],0,sizeof(unsigned short));
  EventLogWrite(eEvtLPStatus,0,3,eventbuff);
}

unsigned char LP_CheckIfInvalidTables()
{
  unsigned short ts;
  unsigned char i,tc,j,numsets,numchan;
  unsigned long memsize,memwork,tl;
  
  TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,LP_Memory_Len),sizeof(tl),&tl);
  if(tl!=LP_CHK_MEMLEN)
    return eLPErrMemSize;
  
  TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,lp_flags),sizeof(ts),&ts);
  if(ts!=LP_CHK_LPFLAGS)
    return eLPErrFlags;
  
  TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,lp_fmats),sizeof(tc),&tc);
  if(tc!=LP_CHK_LPFMAT)
    return eLPErrFmats;
  
  //check how many LP set have been enabled
  numsets=0;
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*i)+offsetof(_lp_config_t,Nbr_Blks),sizeof(ts),&ts);
    if(ts)
    {
      if(numsets!=i)
        return eLPErrSetSeq;//(can't have a gap i.e. Set 1,3,4=enabled set 2=disabled is illegal)
      numsets++;
    }
  }
  
  memsize=0;
  for(i=0;i<numsets;i++)
  {
    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*i)+offsetof(_lp_config_t,Nbr_Chns),sizeof(tc),&tc);
    if((tc==0)||(tc>LP_CHK_NBRCHAN))
      return eLPErrNbrChan;
    numchan=tc;
    memwork=sizeof(_int_fmt1_rcd_t);
    memwork*=tc;//calc interval val array size
    memwork+=(tc/2)+1;//calc unterval flags array size
    
    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*i)+offsetof(_lp_config_t,Nbr_blk_ints),sizeof(ts),&ts);
    if(ts!=LP_CHK_BLKINTS)
      return eLPErrBlkInts;
    memwork*=ts;//calc bytes used by intervals
    memwork+=sizeof(_stime_date_t);//add block header (time stamp)
    
    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*i)+offsetof(_lp_config_t,Nbr_Blks),sizeof(ts),&ts);
    if(ts>LP_CHK_NBRBLKS)
      return eLPErrNbrBlks;
    memwork*=ts;//calc total size for set
    
    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*i)+offsetof(_lp_config_t,Max_Int_Time),sizeof(tc),&tc);
    if(!((tc==1)||(tc==5)||(tc==10)||(tc==15)||(tc==30)||(tc==60)||(tc==90)||(tc==120)||(tc==150)||(tc==180)||(tc==210)||(tc==240)))
      return eLPErrIntTime;
    //TODO: IJD remove 1min LP interval option
    
    //check LP source for each channel
    for(j=0;j<numchan;j++)
    {
      TableRead(eStdT62_LPControl,(sizeof(_Data_Selection_RCD)*i)+offsetof(_Data_Selection_RCD,LP_Sel_Set)+offsetof(_LP_Source_Sel_RCD_t,Chnl_Flag),sizeof(tc),&tc);
      if(tc!=LP_CHK_CHNFLG)
        return eLPErrChnFlg;
      
      TableRead(eStdT62_LPControl,(sizeof(_Data_Selection_RCD)*i)+offsetof(_Data_Selection_RCD,LP_Sel_Set)+offsetof(_LP_Source_Sel_RCD_t,LP_Source_Select),sizeof(tc),&tc);
      if(tc>LP_CHK_SRCSEL)
        return eLPErrSrcSel;
    }
    
    TableRead(eStdT62_LPControl,(sizeof(_Data_Selection_RCD)*i)+offsetof(_Data_Selection_RCD,Int_Fmt_Cde),sizeof(tc),&tc);
    if(tc!=LP_CHK_INTFMTCDE)
      return eLPErrIntFmtCde;
    
    memsize+=memwork;//add LP set size to LP total
    if(memsize>LP_CHK_MEMLEN)
      return eLPErrMemShort;
    
    //adjust mem size for next LP set (may require jump to next page)
    memwork=memsize%EE_PAGE_SIZE;
    memsize+=(EE_PAGE_SIZE-memwork);//jump to next flash page
  }
  return eLPErrPass;
}

unsigned char LP_mirror[NUM_LP_DATA_SETS][DF_DATA_SIZE];
unsigned short LP_active_page[NUM_LP_DATA_SETS]; // last page that was loaded into mirror from flash
unsigned char LP_mod_flag = 0; // this flag is for the possible scenario where we call LP_mirror_shutdown()
// with no calls to LP_flashWrite() since LP_mirror_init() was called.
// ..There is no need to rewrite the initial page if it was not modified.

/*
initialise active pages, load page into RAM
assumes gLPState initialised.
*/
void LP_mirror_init(void) {
  int i;
  unsigned short dummy;
  
  LP_mod_flag = 0;
  for(i=0;i<NUM_LP_DATA_SETS;i++) 
  {
    if(gLPState.NumSets &(1<<i))
    {
      // picking active page corresponding to active interval..
      DF_map_page_and_address(AEMLPDATASTARTADDR + gLPState.sets[i].DataSetOffset +
                              LP_OffsetOfCurrBlock(&gLPState.sets[i])+LP_OffsetOfCurrIntStatus(&gLPState.sets[i]),
                              &LP_active_page[i], &dummy);
      
      DataFlashReadPageBytes(LP_active_page[i], 0, LP_mirror[i], DF_DATA_SIZE);
    }
  }
  //   remote_port_out_str("LP_mirror_init\r\n");
}

/*
write active pages to RAM on shutdown
*/

void LP_mirror_shutdown(unsigned char LPSets) {
  int i;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++) 
  {
    if((gLPState.NumSets &(1<<i)) && (LPSets &(1<<i)))
    {
      if (LP_mod_flag & (1<<i))
        AMPYDFWritePageBytes(LP_active_page[i], 0, LP_mirror[i], DF_DATA_SIZE);
    }
  }
  //   remote_port_out_str("LP_mirror_shutdown\r\n");
}

/*
called from TableReadEx
checks if the time of the active block is within the bounds of the read

the timestamp of a block is the time of the latest interval, so we write LastTime of the set being read.
*/
void LP_time_read(unsigned char *buffer, unsigned long offset, unsigned short size, unsigned char index) {
  unsigned long off_ts;
  long off_into_ts, len;
  
  // find location of timestamp..
  
  if (index >3)
    return;
  
  off_ts = gLPState.sets[index].DataSetOffset + LP_OffsetOfCurrBlock(&gLPState.sets[index]);
  
  if (((offset+size) <= off_ts) || (offset>=(off_ts+sizeof(_stime_date_t)))) {
    return; // timestamp not within bounds
  } else {
    off_into_ts = offset - off_ts;
    if (off_into_ts<0) {
      len = size - (off_ts - offset);
      len = (len>sizeof(_stime_date_t))?sizeof(_stime_date_t):len;
      memcpy(buffer+off_ts-offset, (unsigned char *)&gLPState.sets[index].LastTime, len);
    } else {
      len = (size>(sizeof(_stime_date_t)-off_into_ts))?sizeof(_stime_date_t)-off_into_ts:size;
      memcpy(buffer, (unsigned char *)&gLPState.sets[index].LastTime + off_into_ts, len);
    }
  }
}


unsigned long LP_offset_adjust(unsigned char index) {
  return gLPState.sets[index].DataSetOffset;
}

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
/*------------------------------------------------------------------------------
* Function:    LPPartialMfgProc
* Inputs:      pData[0] and pData[1], parameters used in the procedure call
*              pData[0] is the load profile set in question (1-4) 0=all
*              pData[1..2] is the number of blocks previous to the now 'active'
*                          block to show in the partial LP tables (16-bits)
* Outputs:     Returns 0 on failure, 1 on success (ie at least 1 set was actioned)
* Description: This function handles mfg procedure 17 calls to set the 'last
*              unread lp block pointer'.
* Assumptions: Security system has authorised the use of this procedure by the
*              logged in user on the active comms interface
*-----------------------------------------------------------------------------*/
unsigned char LPPartialMfgProc(unsigned char *pData)
{
  unsigned char retval=0;
  unsigned char set, setend, setstart;
  unsigned short numprevblks;
  
  setend = pData[0];
  numprevblks = pData[1] + ((unsigned short)pData[2] << 8);
  
  if(setend > 4) return 0;
  
  if(setend == 0)
  {
    setstart = 1;
    setend = 4;
  }
  else
    setstart = setend;
#ifdef LPDEBUG
  printf("setstart: %u\n", setstart);
  printf("setend: %u\n", setend);
  printf("numprevblks: %u\n", numprevblks);
#endif
  for(set = setstart; set <= setend; set++)
  {
    if(numprevblks < gLPState.sets[set-1].Nbr_Blks)
    {
      // user passes in the set number and number of previous blocks they want visible
      // we need to ensure that the number of previous blocks specified is less than or equal to the number of 'active' blocks
      if(numprevblks > gLPState.sets[set-1].ActiveBlock)
        numprevblks = gLPState.sets[set-1].ActiveBlock;
      
      gLPState.sets[set-1].LastUnreadBlk = gLPState.sets[set-1].ActiveBlock - numprevblks;
      if(gLPState.sets[set-1].LastUnreadBlk > gLPState.sets[set-1].Nbr_Blks)
        gLPState.sets[set-1].LastUnreadBlk += gLPState.sets[set-1].Nbr_Blks;
#ifdef LPDEBUG
      printf("Setting LastUnreadBlk for set %u to %u\n", set-1, gLPState.sets[set-1].LastUnreadBlk);
#endif
      retval = 1;
    }
  }
  return retval;
}

/*------------------------------------------------------------------------------
* Function:    LP_partialFlashRead
* Inputs:      src-byte address of the beginning of the requested lp data set
*              offset-offset into the available data to begin reading from
*              len-length of requested data, in units of bytes.
*              data_set-specify the desired LP data set, 0,1,2 or 3
* Outputs:     dest-RAM memory location retrieved data will be placed
*              Returns the number of bytes successfully read.
* Description: Used to dynamically present data from std tbls 64-67, allowing
*              offset reads into this virtual data, handling reads that span
*              or wrap the LP data set. Limited to 16-bit reads, in reality
*              this will be much smaller since the kf2 and ansi/psem code
*              will read in chunks of 256 or so bytes
* Assumptions: Assumes that the offset is less than the length of the data set
*-----------------------------------------------------------------------------*/
unsigned short LP_partialFlashRead(unsigned char *dest, unsigned long src, unsigned long offset, unsigned short len, unsigned char data_set)
{
  unsigned short int retval=0, tempw, lastvalidblock, lastunreadblock;
  unsigned long templ1, templ2, templ3;
  // want to read from lastvalidblock to lastunreadblock
  lastvalidblock = LP_CalcLastValidBlock(data_set);
  lastunreadblock = gLPState.sets[data_set].LastUnreadBlk;
  
  // if the data we want isn't 'wrapped'
  if(lastvalidblock >= lastunreadblock)
  {
    // just add the offset corresponding to lastvalidblock and read one continuous block
    templ1 = (gLPState.sets[data_set].Nbr_Blks - lastvalidblock - 1) * LP_CalcSizeOfBlock(&gLPState.sets[data_set]);  //num bytes between start of lp data set and start of lastvalidblock
    retval = LP_flashRead(dest, src + offset + templ1, len, data_set);
  }
  else
  {
    // we need to 'unwrap' the data, and then allow offset reads into the 'continuous' block
    // to 'unwrap', read the most recent data from the end of the buffer (ie read from the last valid block down to block 0). Specified offset may allow us to skip this step...
    templ1 = (lastvalidblock+1) * LP_CalcSizeOfBlock(&gLPState.sets[data_set]);   // length of the data from start of lastvalidblock to end of block 0
    templ2 = (gLPState.sets[data_set].Nbr_Blks - lastvalidblock - 1) * LP_CalcSizeOfBlock(&gLPState.sets[data_set]);// num bytes between start of lp data set and start of lastvalidblock
    // if the supplied offset is greater than or equal to the size of this first chunk, then we can skip it
    // offset passed in includes the lpstart address for this data set, and the offset into the valid data region
    // for comparison below we need to remove all that
    templ3 = offset;
    templ3 -= (LP_offset_adjust(data_set));
    //templ3 -= ((gLPState.sets[data_set].Nbr_Blks - lastvalidblock - 1) * LP_CalcSizeOfBlock(&gLPState.sets[data_set]));
    if(templ3 < templ1)
    {
      tempw = templ1 - offset;                // if we remove the offset, this is the biggest request we can handle
      if(tempw > len) tempw = len;            // trim it down to len if necessary
      retval = LP_flashRead(dest, src + templ2 + offset, tempw, data_set);
      if(tempw <= len)
        len -= tempw;
      else
        len = 0;
      offset = 0;
    }
    else
    {
      offset -= templ1;
    }
    
    // now read from the start of the highest block down to the end of lastunreadblock
    if(len)
      retval += LP_flashRead(&dest[retval], src+offset, len, data_set);
    
  }
  return retval;
}

/*------------------------------------------------------------------------------
* Function:    LP_CalcPartialTableSize
* Inputs:      TableNum-table num of partial LP set 1,2,3,4 (90-93 are valid only)
* Outputs:     Returns the size of the partial LP data set in bytes
* Description: LP data sets each have their own table, these tables are variable
*              in size. In addition to this there are 'partial' LP tables that
*              only present a subset of the available data.
*-----------------------------------------------------------------------------*/
unsigned long LP_CalcPartialTableSize(unsigned short TableNum)
{
  unsigned long temp=0;
  unsigned short set, lastvalidblk, lastunreadblk, tempshort;
  
  set=TableNum-eMfgT90_PartialLPDataSet1;
  if((TableNum>=eMfgT90_PartialLPDataSet1)&&(TableNum<=eMfgT93_PartialLPDataSet4))
  {// do calc for partial LP set 1,2,3 or 4.
    TableRead(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*set)+offsetof(_LP_Set_Status_RCD,Nbr_Valid_Blocks),2,&tempshort);
    if(tempshort > 1) // there must be one, completed block of LP data
    {
      lastvalidblk = LP_CalcLastValidBlock(set);               // find the last valid block
      lastunreadblk = gLPState.sets[set].LastUnreadBlk;        // the last unread block was saved at the time the mfg procedure was called
#ifdef LPDEBUG
      printf("lastvalidblk: %u\n",lastvalidblk);
      printf("lastunreadblk: %u\n",lastunreadblk);
#endif
      tempshort = (lastvalidblk - lastunreadblk) + 1;          // how many blocks will be visible
      if(tempshort > gLPState.sets[set].Nbr_Blks)              // handle wrap
        tempshort += gLPState.sets[set].Nbr_Blks;
      temp = (unsigned long int) tempshort * LP_CalcSizeOfBlock(&gLPState.sets[set]);
    }
  }
#ifdef LPDEBUG
  printf("returns: %ld\n",temp);
#endif
  return temp;
}

/*------------------------------------------------------------------------------
* Function:    LP_CalcLastValidBlock
* Inputs:      set - which LP set we want to calculate last valid block
* Outputs:     Returns the index of the most recently completed block of lp data
* Description: LP data is recorded in blocks, in a circular buffer. This function
*              returns the index of the most recently completed block of lp data
* Note: Assumes that num valid blocks (std 63) is greater than 1
*-----------------------------------------------------------------------------*/
unsigned short int LP_CalcLastValidBlock(unsigned char set)
{
  unsigned short int retval;
#ifdef LPDEBUG
  printf("LP_CalcLastValidBlock() \n");
  printf("gLPState.sets[set].ActiveBlock: %u \n", gLPState.sets[set].ActiveBlock);
  printf("gLPState.sets[set].Nbr_Blks: %u \n", gLPState.sets[set].Nbr_Blks);
#endif
  retval = gLPState.sets[set].ActiveBlock - 1;       // the last completed block is behind the active block
  if(retval > gLPState.sets[set].Nbr_Blks)           // handle wrap
    retval += gLPState.sets[set].Nbr_Blks;
  return retval;
}

#endif //#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
/*------------------------------------------------------------------------------
* Function:    LP_flashRead
* Inputs:      src-byte address of the beginning of the requested data
*              len-length of requested data, in units of bytes.
*              data_set-specify the desired LP data set, 0,1,2 or 3
* Outputs:     dest-RAM memory location retrieved data will be placed
*              Returns the number of bytes successfully read.
* Description: Reads LP data from the external flash or the mirrored RAM copy of
*              the most recently written page of data.
*-----------------------------------------------------------------------------*/
unsigned short LP_flashRead (unsigned char *dest, unsigned long src, unsigned short len, unsigned char data_set)
{
  unsigned short page;
  unsigned short addr;
  unsigned short len_in_page;
  
  DF_map_page_and_address(src, &page, &addr);
  
  //printf("\nmemcpy_rd() Src 0x%lx, Len %u. Page %u, Addr %u", src, len, page, addr);
  
  while (len)
  {
    len_in_page = (len > (DF_DATA_SIZE - addr))
      ? (DF_DATA_SIZE - addr)
        : len;
        len = len - len_in_page;
#if  (1)
        if (page == LP_active_page[data_set]) {
          memcpy(dest, LP_mirror[data_set]+addr, len_in_page);
          //          remote_port_out_str("LP_flashRead set"); remote_port_out_short(data_set); remote_port_out_str(", page("); remote_port_out_short(page);
          //          remote_port_out_str(")==active page("); remote_port_out_short(LP_active_page[data_set]); remote_port_out_str(")\r\n");
        } else {
#endif
          DataFlashReadPageBytes( page, addr, dest, len_in_page );
#if  (1)
          //         remote_port_out_str("LP_flashRead set"); remote_port_out_short(data_set); remote_port_out_str(", page("); remote_port_out_short(page);
          //          remote_port_out_str(")!=active page("); remote_port_out_short(LP_active_page[data_set]); remote_port_out_str(")\r\n");
        }
#endif
        dest += len_in_page;
        
        // prepare dataflash to access addr 0 of next page
        page = page + 1;
        addr = 0;
  }
  return len;
}

/*------------------------------------------------------------------------------
* Function:    LP_flashWrite
* Inputs:      dest-byte address indicating where the data will be written
*              len-length of data that will be written, in units of bytes.
*              data_set-specify the desired LP data set, 0,1,2 or 3
* Outputs:     src-RAM memory location where data will be retrieved
*              Returns the number of bytes successfully written.
* Description: Writes LP data to the external flash and/or the mirrored RAM copy
*              . If a new page is touched then the old RAM mirror will be
*              written to external flash and the new data will go into the RAM
*              mirror.
*-----------------------------------------------------------------------------*/
unsigned short LP_flashWrite(unsigned long dest, unsigned char *src, unsigned short len,unsigned char data_set)
{
  unsigned short page;
  unsigned short addr;
  unsigned short len_in_page;
  
  if((dest < AEMLPDATASTARTADDR) || (dest > AEMLPDATAENDADDR))
  {
#ifdef LPDEBUG
    printf("LPW!!!\n");
#endif
    return 0;
  }
  
  DF_map_page_and_address(dest, &page, &addr);
  
  //printf("\nmemcpy_dr() Dst 0x%lx, Len %u. Page %u, Addr %u", dest, len, page, addr);
  
  while (len)
  {
    len_in_page = (len > (DF_DATA_SIZE - addr))
      ? (DF_DATA_SIZE - addr)
        : len;
        len = len - len_in_page;

#if  (1)
		  //printf("=%d,=%d \n",page,(unsigned short)LP_active_page[data_set]);
        if (page == LP_active_page[data_set]) {
          memcpy(LP_mirror[data_set]+addr, src, len_in_page);
          //         remote_port_out_str("LP_flashWrite set"); remote_port_out_short(data_set); remote_port_out_str(", page("); remote_port_out_short(page);
          //          remote_port_out_str(")==active page("); remote_port_out_short(LP_active_page[data_set]); remote_port_out_str(")\r\n");
        } else {
          AMPYDFWritePageBytes(LP_active_page[data_set], 0, LP_mirror[data_set], DF_DATA_SIZE);
          DataFlashReadPageBytes( page, 0, LP_mirror[data_set], DF_DATA_SIZE );
          memcpy(LP_mirror[data_set]+addr, src, len_in_page);
          
          //         remote_port_out_str("LP_flashWrite set"); remote_port_out_short(data_set); remote_port_out_str(", page("); remote_port_out_short(page);
          //          remote_port_out_str(")!=active page("); remote_port_out_short(LP_active_page[data_set]); remote_port_out_str(")\r\n");
          LP_active_page[data_set] = page;
        }
        LP_mod_flag |= 1 << data_set;
#else
        AMPYDFWritePageBytes( page, addr, src, len_in_page );
#endif
        src += len_in_page;

        if ((DF_refresh_on_write)&&(gLPZeroFilling==0))
        {
          DF_incremental_refresh();
        }
		  
        // prepare dataflash to access addr 0 of next page
        page = page + 1;
        addr = 0;
  }
  return len;
}

/*------------------------------------------------------------------------------
* Function:    LP_GetTableSize
* Inputs:      TableNum-table num of LP set 1,2,3,4 (64,65,66,67 are valid only)
* Outputs:     Returns the size of the LP data set in bytes.
* Description: LP data sets each have their own table, these tables are variable
*              in size therefore the table functions need a means for
*              determining the size base on the users configuration.
*-----------------------------------------------------------------------------*/
unsigned long LP_GetTableSize(unsigned short TableNum)
{
  unsigned long temp=0;
  unsigned short val;
  val=TableNum-eStdT64_LPDataSet1;
  
  if((TableNum>=eStdT64_LPDataSet1)&&(TableNum<=eStdT67_LPDataSet4))
  {// do calc for LP set 1,2,3 or 4.
    //if LP data set has been configured Nbr_Blks will be non-zero
    temp=gLPState.sets[val].Nbr_Blks;
    temp*=LP_CalcSizeOfBlock(&gLPState.sets[val]);
  }
#if defined(SIMULATION_SR)
  if(temp>MAXLPSIZE)
    temp=MAXLPSIZE;
#endif
  return temp;
}

/*------------------------------------------------------------------------------
* Function:    LP_MeterTimeChangeUpdate
*
* Inputs:      Timeshift-time in seconds the system clock has moved forward
*              or behind
* Description: Called every time the system clock is changed. It sets the CLK_FWD
*              and CLK_BACK flags, depending on the direction of time shift
*
*-----------------------------------------------------------------------------*/
void LP_MeterTimeChangeUpdate(signed long Timeshift)
{
  
  unsigned char i;
  gMeterTimeChanged = 1;
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(!(gLPState.LPFlags[i]&LP_FLG_ENABLED))
      continue;
    if(Timeshift > 0)
    {//We have move ahead in time
      gLPState.sets[i].SFlags|= LP_INT_FLAG_CLK_FWD;
      if(Timeshift >= 0x7FFEL)//If the time change delta is more than the range of a signed short
        gLPState.sets[i].TimeShiftDelta = 0x7FFF;
      else
      {
        gLPState.sets[i].TimeShiftDelta -= Timeshift;
        if(gLPState.sets[i].TimeShiftDelta > 0)//check if underflow occured
          gLPState.sets[i].TimeShiftDelta = 0x7FFF;
      }
    }
    else if (Timeshift < 0)
    {//We have moved behind in time
      gLPState.sets[i].SFlags|= LP_INT_FLAG_CLK_BACK;
      if(Timeshift <= -32768L)//If the time change delta is more than the range of a signed short
        gLPState.sets[i].TimeShiftDelta = 0x8000;
      else
      {
        gLPState.sets[i].TimeShiftDelta += (Timeshift * -1l);
        if(gLPState.sets[i].TimeShiftDelta < 0)//check if overflow occured
          gLPState.sets[i].TimeShiftDelta = 0x8000;
      }
    }
  }
  LPZeroFillingTimeChgn(Timeshift, eTimeChange);
}
/*------------------------------------------------------------------------------
* Function:    LP_CheckMissingInt
* Description: Do a check according to the last timestamp saved in LP, if there 
*              is a missing interval.  Do some compensation
*-----------------------------------------------------------------------------*/
unsigned char LP_CheckMissingInt(unsigned char nLPSetIndex, unsigned char nZeroFillingIntNum)
{
   _ltime_date_t  TimeANSI;
   unsigned long	timeLastInt,timeCurrent;
   unsigned char	missingInt;
   unsigned short IntLenSec;
		
   IntLenSec=gLPState.sets[nLPSetIndex].Max_Int_Time*60;
   TableRead(eStdT55_ClockState, offsetof(_std_table_55_t, clock_calendar), sizeof(TimeANSI), &TimeANSI);
   timeCurrent = RTC_ANSILTimeToPosix(&TimeANSI);//convert ansi time to posix
   timeLastInt = RTC_ANSILTimeToPosix(&gLPState.sets[nLPSetIndex].LastTime);
		
   if ((timeCurrent - 1)>timeLastInt)
      missingInt  = (timeCurrent - 1 - timeLastInt)/IntLenSec;  //if we are on the interval boudary, the LP_Sectic can handle this interval
   else
      missingInt = 0;
		
   if (missingInt>nZeroFillingIntNum)
      return 1;
   else
      return 0;
}

/*------------------------------------------------------------------------------
* Function:    LPZeroFillingTimeChgn
*
* Inputs:      Timeshift-time in seconds the system clock has moved forward,
*              behind, or has been powered down.
* Description: Called every time the system clock is changed and on power up.
*              The function computes number of intervals to be zero filled for
*              forward movement of the clock and the time meter was powered down.
*              It performs interval adjustment for reverse time change. Adjustment
*              is only done if the time change is in a current interval, in which
*              the current interval is closed and new one is initiated.If reverse
*              time shift spans for more than 1 interval, no adjustments is performed.
*-----------------------------------------------------------------------------*/
void LPZeroFillingTimeChgn(signed long TimeShift, unsigned char Source)
{
  unsigned char i;
  unsigned long NumCompleteInt;
  unsigned short partial;
  unsigned short IntLenSec;
  unsigned short Deltas[NUM_CH_PER_DATA_SET];
  unsigned char Flags[(NUM_CH_PER_DATA_SET/2)+1];
  unsigned long Positive_Timeshift;
  unsigned char LPDisable_Flag = 0;
  unsigned char LPReset_Flag = 0;
  unsigned char temp = 0;
  _LP_Set_Status_Bfld S63StatusBfld;
  signed short TimeDelta;
  unsigned long LP_PosixTime;
  unsigned char eventbuff[3] = {0};
  memset(Flags,0x00,(NUM_CH_PER_DATA_SET/2)+1);
  
  if(TimeShift < 0L)
    Positive_Timeshift = TimeShift * -1L;
  else
    Positive_Timeshift = TimeShift;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(!(gLPState.LPFlags[i]&LP_FLG_ENABLED))
      continue;
    IntLenSec=gLPState.sets[i].Max_Int_Time;
    IntLenSec*=60;//convert to seconds
    //work out number of completed intervals
    NumCompleteInt=Positive_Timeshift;
    NumCompleteInt/=IntLenSec;//convert to intervals (this is the number of whole intervals)
    //now work out if there is an extra interval (did we cross a boundry?)
    partial=Positive_Timeshift%(IntLenSec);
    
    if(Source == eLPPowerUp)
    {//Calculating Interval Boundary crossing for PowerDown
      TimeDelta = LP_TimeTillNextInterval(&gLPState.sets[i]) + partial;
      if(TimeDelta > IntLenSec)//IJD yes ">" (not ">=") we don't want to double count an interval
        NumCompleteInt++;
    }
    else if(Source == eTimeChange)
    {//Calculating Interval Boundary crossing for Time Shift 
      if(TimeShift > 0)
      {//For Positive Time Shift
        TimeDelta = LP_TimeTillNextInterval(&gLPState.sets[i]) - partial;
        if(TimeDelta < 0)
          NumCompleteInt++;
      }
      else if (TimeShift < 0L)
      {//For Negative Time Shift
        TimeDelta = partial + LP_TimeTillNextInterval(&gLPState.sets[i]);
        if(TimeDelta > IntLenSec)
          NumCompleteInt++;
      }
    }
    
    if((TimeShift < 0) && (NumCompleteInt))
    {//For reverse time shift which has crossed the current interval boundary.
      
      TableRead(eStdT63_LPStatus,(sizeof(_LP_Set_Status_RCD)*i)+offsetof(_LP_Set_Status_RCD,LP_Set_Status_Flags),sizeof(S63StatusBfld),&S63StatusBfld);
      if(S63StatusBfld.Active_Mode_Flag)
      {
        if(TimeShift <= -32768L)//If the time change delta is more than the range of a signed short
          gLPState.sets[i].TimeShiftDelta += 0x8000;
        else
          gLPState.sets[i].TimeShiftDelta += TimeShift;
        
        gLPState.sets[i].TimeShiftDelta-=LP_TimeTillNextInterval(&gLPState.sets[i]);
        LP_CalcIntervalData(i,Flags,Deltas,0,1);
        LP_PosixTime = RTC_GetFunctionalTime(0,0,0,eRTCFT_GMT);
        LP_PosixTime += LP_TimeTillNextInterval(&gLPState.sets[i]);
        RTC_PosixToANSILTime(LP_PosixTime,&gLPState.sets[i].LastTime);
        LP_WriteIntervalData(&gLPState.sets[i],Flags,Deltas,i);
        LP_MoveToNextInterval(&gLPState.sets[i],i);
      }
      LPDisable_Flag |= (1<<i);
      continue;//return;
    }
    
    if(NumCompleteInt > ((unsigned long)gLPState.sets[i].Nbr_Blks)*gLPState.sets[i].Nbr_blk_ints)
    {
      LPReset_Flag |= (1<<i);
      continue;
    }

	 if (LP_CheckMissingInt(i,gLPState.sets[i].NumZeroInt+NumCompleteInt))
	 {
		if (gLPState.sets[i].NumZeroInt>0)//PWR DOWN in the middle of zero filling
		{
        gLPState.sets[i].NumZeroInt++;
		}
		else if (NumCompleteInt>0)//zero filling 
		{
			NumCompleteInt++;
		}
		else
		{
		  gMissingIntFlag[i] = 1;		
		}
	 }
    
    if((gLPState.sets[i].NumZeroInt==0)&&(NumCompleteInt))
    {//first interval is special because its not just be all zero (may have some energy) but only if not in zero fill mode yet
      if(Source == eTimeChange)
        gLPState.sets[i].SFlags|= LP_SET_FLG_ZTIMEFWD;
      
      LP_CalcIntervalData(i,Flags,Deltas,0,1);
      LP_CalcIntervalTime(&gLPState.sets[i]);
      LP_WriteIntervalData(&gLPState.sets[i],Flags,Deltas,i);
      LP_MoveToNextInterval(&gLPState.sets[i],i);
      gLPState.sets[i].TimeShiftDelta=IntLenSec-LP_TimeTillNextInterval(&gLPState.sets[i]);
      gLPState.sets[i].TimeShiftDelta*=-1;//make it negative
      NumCompleteInt--;
      LP_ChkEndZFill(&gLPState.sets[i]);
    }

    gLPState.sets[i].NumZeroInt+=NumCompleteInt;
	 
	 if (!gLPState.sets[i].NumZeroInt)
		LP_FinishZFill(&gLPState.sets[i]);
  }
  
  if(LPDisable_Flag)
  {
    temp = 0;
    for(i=0;i<NUM_LP_DATA_SETS;i++)
    {
      if(LPDisable_Flag & (1<<i))
      {
        if(gLPState.LPFlags[i] & LP_FLG_ENABLED)
        {
          gLPState.LPFlags[i]&=~(LP_FLG_ENABLED);
          temp |= (1<<i);
        }
      }  
    }
    LP_ChangeActiveModeFlags(temp);
    //Commit pending LP Ram mirror to flash(fix for Bug#1841 & #1833)
    LP_mirror_shutdown(temp);
  }
  
  if((LPReset_Flag == 0) && (LPDisable_Flag)) //There is no set to reset we make LP status log here
  {
    eventbuff[0] = 2;//This is to tell empwin this is a new LP implementation
    for(i=0;i<NUM_LP_DATA_SETS;i++)
    {
      if(gLPState.LPFlags[i] & LP_FLG_ENABLED)
         eventbuff[(i/2)+1] |= (1<<((i%2)*4));
      else
         eventbuff[(i/2)+1] &= ~(1<<((i%2)*4));
    }
    EventLogWrite(eEvtLPStatus,0,3,eventbuff);
  }
  else if(LPReset_Flag)
   LP_Reset(LPReset_Flag);    //TODO: can still see the old count for nbr_valid_intervals in std 63 after this guy is executed...
}

/*
* LP QOS profiling mode configuration
* The data contained in lp_qos_cfg determines how the QOS data will be recorded in LP
* i.e. Snapshot, min, max, or average.
*
* NOTE assume a maximum of 4 lp data sets, and max 4 channels per data set.
* format: the nth pair of bits in byte m of lp_qos_cfg determines the mode for qos lp.
* a value of 00 -> snapshot at end of interval.
* 01 -> minimum over interval
* 10 -> maxmimum over interval
* 11 -> average over interval
* (may be calculated over a partial interval)
*
* Nonvolatile storage in mfg_table_36. LP stopped when this table is overwritten.
*/

/*-------------------------------------------------------------------------------
* Function:    LP_MinMaxAvg
* Inputs:      pLPState-pointer to LP data set status
*              index-Specifies LP data set, 0, 1, 2 or 3.
* Outputs:     Update global values.
* Description: Called from LP_MainloopTick every time metrology data has been
*              updated. This function processes new QOS data in ST28 and
*              calculates min/max/average based on the configuration. Average
*              is performed using the accumulated sum method.
*------------------------------------------------------------------------------*/
void LP_MinMaxAvg(_lp_data_set_state_t *pLPState, char index, unsigned char * Flags)
{
  char i;
  unsigned long val;
  signed short wTemp=0;

  for(i=0;i<pLPState->Nbr_Chns;i++)
  {
    unsigned char atError=FALSE;
    char x = pLPState->ValSelect[i];
    
    if(SourceIsQOS(pLPState->ValSelect[i]))
    {
      TableRead(eStdT28_PresRegData,x*4 , sizeof(val), &val);
      if((x >= eT16_ActPwrPhA) && (x <= eT16_ReactPwrTot))
        val /= 10;
    }
    else if (pLPState->ValSelect[i]>=eT16_ATChannel1  && pLPState->ValSelect[i]<=eT16_ATChannel4 )
    {
   	 // ATProfile
       val=GetProfileValue((unsigned char)(pLPState->ValSelect[i])-eT16_ATChannel1);
       atError=!GetProfileValueValid((unsigned char)(pLPState->ValSelect[i])-eT16_ATChannel1);
       if (atError==TRUE)
       {
          Flags[(i+1)/2]|=LP_INT_STAT_ATCOMMERROR<<((i%2)*4);
          // dont store invalid values for min/max/avg
          if (LP_IsQOSMax(index, i))
          {
            val = 0;
          }
          else if (LP_IsQOSMin(index, i))
          {
            val = 0xffff;//7fff
          }
          else if (LP_IsQOSAvg(index, i))
          {
            val = 0;
          }
          else
          {
             val = 0xffff;//7fff
          }
       }
    }
    else if(SysMon_SourceIsSpecial(pLPState->ValSelect[i]))
    {
      TableRead(eMfgT24_TempAppStatus,18+(pLPState->ValSelect[i]-eT16_TempPhA)*50,sizeof(wTemp),&wTemp);
      val = wTemp+1000;  //offset 100 degree to avoid a negative value
    }
    else
    {
      continue;
    }
    
    if (!LP_IsQOSMinMaxAvg(index, i))
      continue;
#if defined(NEUTRAL_INTEGRITY)
    //Move special hack for data collection on phase A reactive to phase C.
#if 1
    if(x==eT16_ReactPwrPhC)
#else
      if(x==eT16_ReactPwrPhA)
#endif
        val=Metrology_NICalcSpecialLPVal();//
#endif
    if (LP_IsQOSMin(index, i))
    {
      if((pLPState->ValSelect[i] >= eT16_PFPhA) &&(pLPState->ValSelect[i] <= eT16_PFTot))
        PowerFactorMinMax(&val, &pLPState->LastVals[i], 1);//1 -Min
      else if (val < pLPState->LastVals[i]) pLPState->LastVals[i] = val;
    }
    else if (LP_IsQOSMax(index, i))
    {
      if((pLPState->ValSelect[i] >= eT16_PFPhA) &&(pLPState->ValSelect[i] <= eT16_PFTot))
        PowerFactorMinMax(&val, &pLPState->LastVals[i], 2);//1 -Max
      else if (val > pLPState->LastVals[i]) pLPState->LastVals[i] = val;
    }
    else if (LP_IsQOSAvg(index, i))
    {
      pLPState->LastVals[i] +=  val;
      gLPState.QOSAvgN[index][i]++;
    }
  }
}

/*------------------------------------------------------------------------------
* Function:    LP_ResetMinMaxAvg
* Inputs:      pLPState-pointer to LP data set status
*              index-Specifies LP data set, 0, 1, 2 or 3.
* Outputs:     Update global values.
* Description: Resets the LastVal[i] if min/max/avg calc being done on LP
*              channel. Must be called at end of interval or when LP is started.
*-----------------------------------------------------------------------------*/
void LP_ResetMinMaxAvg(_lp_data_set_state_t *pLPState,unsigned char index)
{
  int i;
  for(i=0;i<pLPState->Nbr_Chns;i++)
  {
    if(SourceIsQOS(pLPState->ValSelect[i])|| SysMon_SourceIsSpecial(pLPState->ValSelect[i])  || (pLPState->ValSelect[i]>=eT16_ATChannel1  && pLPState->ValSelect[i]<=eT16_ATChannel4))
    {
      if (LP_IsQOSMax(index, i))
      {
        pLPState->LastVals[i] = 0;
      }
      else if (LP_IsQOSMin(index, i))
      {
        pLPState->LastVals[i] = 0x7fffffff;
      }
      else if (LP_IsQOSAvg(index, i))
      {
        gLPState.QOSAvgN[index][i] = 0;
        pLPState->LastVals[i] = 0;
      }
    }
  }
}

/*------------------------------------------------------------------------------
* Function:    LP_InitMinMaxAvg
* Outputs:     Update global values.
* Description: Resets the LastVal[i] if min/max/avg calc being done on LP
*              channel. Must be called when LP is started.
*-----------------------------------------------------------------------------*/
void LP_InitMinMaxAvg(void)
{
  unsigned char i;
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if(gLPState.NumSets &(1<<i))
    {
      LP_ResetMinMaxAvg(&gLPState.sets[i], i);
    }
  }  
}

/*------------------------------------------------------------------------------
* Function:    LP_PrintLPState
* Inputs:      none.
* Outputs:     none.
* Description: Print out the contents of gLPState and lp_qos_cfg[].
*-----------------------------------------------------------------------------*/
void LP_PrintLPState( void )
{
  unsigned char i;
  unsigned char *pDat;
  unsigned short pos;
  
  //print out raw MT76 (gLPState)
  pDat=(unsigned char*)&gLPState;
  pos=0;
  printf("----raw gLPState (MT76)----\n");
  while(pos<sizeof(gLPState))
  {
    printf("0x%03X:",pos);
    for(i=0;i<16;i++)
    {
      if(pos>=sizeof(gLPState))
        break;
      printf(" %02X",pDat[pos]);
      pos++;
    }
    printf("\n");
  }
  printf("\n");
  
  //print out raw lp_qos_cfg[]
  pDat=(unsigned char*)&lp_qos_cfg;
  pos=0;
  printf("----raw lp_qos_cfg (MT36)----\n");
  while(pos<sizeof(lp_qos_cfg))
  {
    printf("0x%03X:",pos);
    for(i=0;i<16;i++)
    {
      if(pos>=sizeof(lp_qos_cfg))
        break;
      printf(" %02X",pDat[pos]);
      pos++;
    }
    printf("\n");
  }
  printf("\n");
  
  //print out gLPState in a human readable form.
  printf("----gLPState----\n");
  printf("NumSets %i\n",gLPState.NumSets);
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    printf("Data Set %i:\n",i+1);
    printf("LPFlags 0x%02X\n",gLPState.LPFlags[i]);
    printf(" Nbr_Blks %u\n",gLPState.sets[i].Nbr_Blks);
    printf(" Nbr_blk_ints %u\n",gLPState.sets[i].Nbr_blk_ints);
    printf(" Nbr_Chns %u\n",gLPState.sets[i].Nbr_Chns);
    printf(" Max_Int_Time %u\n",gLPState.sets[i].Max_Int_Time);
    
    printf(" ValSelect");
    for(pos=0;pos<NUM_CH_PER_DATA_SET;pos++)
      printf(" %u",gLPState.sets[i].ValSelect[pos]);
    printf("\n");
    
    printf(" LastVals");
    for(pos=0;pos<NUM_CH_PER_DATA_SET;pos++)
      printf(" %lu",gLPState.sets[i].LastVals[pos]);
    printf("\n");
    
    printf(" \n",gLPState.sets[i].LastTime);
    printf(" SFlags 0x02X\n",gLPState.sets[i].SFlags);
    printf(" TimeShiftDelta %i\n",gLPState.sets[i].TimeShiftDelta);
    printf(" DataSetOffset %lu\n",gLPState.sets[i].DataSetOffset);
    printf(" ActiveInterval %u\n",gLPState.sets[i].ActiveInterval);
    printf(" ActiveBlock %u\n",gLPState.sets[i].ActiveBlock);
    printf(" NumZeroInt %lu\n",gLPState.sets[i].NumZeroInt);
    
    printf(" BkpDeltas");
    for(pos=0;pos<NUM_CH_PER_DATA_SET;pos++)
      printf(" %u",gLPState.sets[i].BkpDeltas[pos]);
    printf("\n");
    
    printf(" BkpFlags");
    for(pos=0;pos<NUM_CH_PER_DATA_SET;pos++)
      printf(" %lu",gLPState.sets[i].BkpFlags[pos]);
    printf("\n");
    
    printf(" \n",gLPState.sets[i].BkpLastTime);
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
    printf(" LastUnreadBlk %u\n",gLPState.sets[i].LastUnreadBlk);
#endif
    
    printf(" QOSAvgN");
    for(pos=0;pos<NUM_CH_PER_DATA_SET;pos++)
      printf(" %u",gLPState.QOSAvgN[i][pos]);
    printf("\n");
  }
  printf("\n");
  
  //print out lp_qos_cfg[] in a human readable form.
  printf("----lp_qos_cfg----\n");
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    printf("Data Set %i:\n",i+1);
    for(pos=0;pos<NUM_CH_PER_DATA_SET;pos++)
    {
      if(LP_IsQOSMin(i,pos))
        printf(" min");
      else if(LP_IsQOSMax(i,pos))
        printf(" max");
      else if(LP_IsQOSAvg(i,pos))
        printf(" avg");
      else
        printf(" norm");
    }
    printf("\n");
  }
}

/*------------------------------------------------------------------------------
* Function:    LP_IsTimeChgBlkd
* Inputs:      TimeDiff-Time difference between requested time and current meter time
*
* Outputs:     0- Time Change Allowed
*              1- Time Change Not Allowed
* Description: Called when the meter time is updated via Std Proc 10. Blocks time
*              change, if request is to move time backward and crosses only the
*              current interval boundary.
*              This check is done for time change requests from SSN only.
*-----------------------------------------------------------------------------*/
unsigned char LP_IsTimeChgBlkd(signed long TimeDiff)
{
  unsigned char i;
  unsigned char ret = 0;
  unsigned char index = 0;
  unsigned short int_time_sec = 0xFFFF;
  unsigned short int_time_rem = 0;
  
  for(i=0;i<gLPState.NumSets;i++) //Find the smallest interval
  {
    if(int_time_sec > gLPState.sets[i].Max_Int_Time)
    {
      index = i;
      int_time_sec = gLPState.sets[i].Max_Int_Time;
    }
  }
  if(int_time_sec < (unsigned char)0xFF)//Sanity check
  {
    int_time_sec *=  60;//Convert the interval time to seconds.
  }
  
  //If time shift is forward in time
  if(TimeDiff >= 0)
  {
    ret = 0;// Time Change allowed
  }
  else//Time change is backward
  {
    TimeDiff *= -1L;
    int_time_rem = LP_TimeTillNextInterval(&gLPState.sets[index]);
    if(TimeDiff > ((signed long)(int_time_sec/2)))//Time diff is greater than half of the interval length
      ret = 0;
    else if((int_time_rem + (unsigned short)TimeDiff) >= int_time_sec)//We cant cross the current interval backward
    {
      ret = 1;//Time Change Blocked
    }
    else
      ret = 0;
  }
  return ret;
}
unsigned short LP_VerifyConfiguration (unsigned char *buff)
{
  extern unsigned char *LPBuffer;
  extern unsigned char gLPPlatformGet; 
  unsigned char i,j, tempc,rqst;
  unsigned short temps;
  _std_table_60_t *pLPSetConfigN, *pLPSetConfigE,*pLPSetConfigDe;
  _std_table_62_t *pLPDataConfigN, *pLPDataConfigE;
  _mfg_table_36_t *pLPQOSConfigN, *pLPQOSConfigE;
  LPConfigScratchPad *pLPScratchPad;
  
  if(!gLPPlatformGet)
    return 0xFFFF;
  memcpy(&rqst,buff,sizeof(rqst));
  
  pLPSetConfigN = (_std_table_60_t*)LPBuffer;// 31bytes
  pLPDataConfigN =(_std_table_62_t*)&LPBuffer[sizeof(_std_table_60_t)]; //52 bytes
  pLPQOSConfigN = (_mfg_table_36_t*)&LPBuffer[sizeof(_std_table_60_t) + sizeof(_std_table_62_t)]; //4 bytes
  pLPSetConfigE = (_std_table_60_t*)&LPBuffer[sizeof(_std_table_60_t) + sizeof(_std_table_62_t) + sizeof(_mfg_table_36_t)];// 31bytes
  pLPDataConfigE = (_std_table_62_t*)&LPBuffer[(sizeof(_std_table_60_t))*2 + sizeof(_std_table_62_t) + sizeof(_mfg_table_36_t)];//52 bytes
  pLPQOSConfigE = (_mfg_table_36_t*)&LPBuffer[(sizeof(_std_table_60_t))*2 + sizeof(_std_table_62_t)*2 + sizeof(_mfg_table_36_t)];//4 bytes
  pLPScratchPad = (LPConfigScratchPad*)&LPBuffer[(sizeof(_std_table_60_t))*2 + sizeof(_std_table_62_t)*2 + sizeof(_mfg_table_36_t)*2];//41 bytes
  //debug
  pLPSetConfigDe = (_std_table_60_t*)&LPBuffer[(sizeof(_std_table_60_t))*2 + sizeof(_std_table_62_t)*2 + sizeof(_mfg_table_36_t)*2 + sizeof(LPConfigScratchPad)];//31 bytes
  memset(pLPScratchPad,0,sizeof(LPConfigScratchPad));
  
  //Read Existing Configuration
  TableRead(eStdT61_ActualLPLimit,0,sizeof(_std_table_60_t),pLPSetConfigE);
  TableRead(eStdT62_LPControl,0,sizeof(_std_table_62_t),pLPDataConfigE);
  TableRead(eMfgT36_LP_QOS_Cfg,0,sizeof(_mfg_table_36_t),pLPQOSConfigE);
  //debug
  memcpy(pLPSetConfigDe, pLPSetConfigE, sizeof(_std_table_60_t));
  
  //Calculate the size of existing configuration
  LP_SizeofExistingSets(pLPSetConfigE, pLPScratchPad);
  
  //Checking and updating configuration that have changed
  
  //Check for LP Configuration - Std Table 61
  if(pLPSetConfigN->LP_Memory_Len == 0xFFFFFFFF)
  {
    pLPSetConfigN->LP_Memory_Len = pLPSetConfigE->LP_Memory_Len;
  }
  memcpy(&temps, &pLPSetConfigN->lp_flags, sizeof(temps));
  if(temps == 0xFFFF)
  {
    memcpy(&pLPSetConfigN->lp_flags,&pLPSetConfigE->lp_flags, sizeof(temps));
  }
  memcpy(&tempc, &pLPSetConfigN->lp_fmats, sizeof(tempc));
  if(tempc == 0xFF)
  {
    memcpy(&pLPSetConfigN->lp_fmats, &pLPSetConfigE->lp_fmats, sizeof(tempc));
  }
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    if((pLPSetConfigN->Config[i].Nbr_Blks == 0xFFFF) || (!(rqst & (1<<i))))
    {
      pLPSetConfigN->Config[i].Nbr_Blks = pLPSetConfigE->Config[i].Nbr_Blks;
    }
    if((pLPSetConfigN->Config[i].Nbr_Chns == 0xFF)|| (!(rqst & (1<<i))))
    {
      pLPSetConfigN->Config[i].Nbr_Chns = pLPSetConfigE->Config[i].Nbr_Chns;
    }
    if((pLPSetConfigN->Config[i].Nbr_blk_ints == 0xFFFF)|| (!(rqst & (1<<i))))
    {
      pLPSetConfigN->Config[i].Nbr_blk_ints = pLPSetConfigE->Config[i].Nbr_blk_ints;
    }
    if((pLPSetConfigN->Config[i].Max_Int_Time == 0xFF)|| (!(rqst & (1<<i))))
    {
      pLPSetConfigN->Config[i].Max_Int_Time = pLPSetConfigE->Config[i].Max_Int_Time;
    }
    
    //Check for LP Channel Data - Std Table 62
    if((pLPDataConfigN->Format[i].Int_Fmt_Cde == 0xFF) || (!(rqst & (1<<i))))
    {
      pLPDataConfigN->Format[i].Int_Fmt_Cde = pLPDataConfigE->Format[i].Int_Fmt_Cde;
    }
    
    for(j=0; j<NUM_CH_PER_DATA_SET; j++)
    {
      if((pLPDataConfigN->Format[i].LP_Sel_Set[j].Chnl_Flag == 0xFF) || (!(rqst & (1<<i))))
        pLPDataConfigN->Format[i].LP_Sel_Set[j].Chnl_Flag = pLPDataConfigE->Format[i].LP_Sel_Set[j].Chnl_Flag;
      if((pLPDataConfigN->Format[i].LP_Sel_Set[j].LP_Source_Select == 0xFF) || (!(rqst & (1<<i))))
      {
        pLPDataConfigN->Format[i].LP_Sel_Set[j].LP_Source_Select = pLPDataConfigE->Format[i].LP_Sel_Set[j].LP_Source_Select;
      }
      if((pLPDataConfigN->Format[i].LP_Sel_Set[j].End_Blk_Rdg_Source_Select == 0xFF) || (!(rqst & (1<<i))))
      {
        pLPDataConfigN->Format[i].LP_Sel_Set[j].End_Blk_Rdg_Source_Select = pLPDataConfigE->Format[i].LP_Sel_Set[j].End_Blk_Rdg_Source_Select;
      }
    }
    
    //Check for LP QoS  - Mfg Table 36
    //if((pLPQOSConfigN->LP_QOS_Cfg[i] == 0xFF) || (!(rqst & (1<<i))))
    if (!(rqst & (1<<i)))
    {
      pLPQOSConfigN->LP_QOS_Cfg[i] = pLPQOSConfigE->LP_QOS_Cfg[i];
    }
  }
  
  i = LP_CheckIfInvalidTables_Rev(rqst,pLPSetConfigN,pLPDataConfigN,pLPScratchPad);// A good sanity check for the new configuration
  
  if(i != eLPErrPass)
  {
    //printf("New Config Err %d\n",i);
    for(i=0; i < NUM_LP_DATA_SETS; i++)
    {
      if(rqst & (1<<i))
      {
        pLPScratchPad->LP_ReturnCode |= (eLPErrCnfg << (i*4));
      }
    }
    return (pLPScratchPad->LP_ReturnCode);
  }
  
  //Check which sets have been Added or Modified
  for(i=0; i < NUM_LP_DATA_SETS; i++)
  {
    //printf("Set %d Number of Blocks  %d\n",i,pLPSetConfigN->Config[i].Nbr_Blks);
    //Get a list of the existing sets being used. In the existing implementation the Nbr_Blks entry is used to determine is a set is enabled or disabled.
    if(pLPSetConfigE->Config[i].Nbr_Blks)
    {
      pLPScratchPad->LP_ExstFlag |= (1<<i);
    }
    
    if(!(pLPScratchPad->LP_RqstFlag &(1<<i)))// && !(pLPScratchPad->LP_ExstFlag & (1<<i))) //No changes to the existing set
    {
      pLPScratchPad->LP_Sets_Unchg |= (1<<i);
      pLPScratchPad->LP_Set_SizeN[i] = pLPScratchPad->LP_Set_SizeE[i];
    }
    else if((pLPScratchPad->LP_RqstFlag &(1<<i)) && !(pLPScratchPad->LP_ExstFlag & (1<<i))) //A new set has been added
    {
      if(pLPSetConfigN->Config[i].Nbr_Blks == 0)// This could be scenario from full programming
      {
         pLPScratchPad->LP_Sets_Unchg |= (1 << (4+i));
      }
      else
      {
         pLPScratchPad->LP_Sets_Add |= (1<<i);
      }
    }
    else if((pLPScratchPad->LP_RqstFlag & (1<<i)) && (pLPScratchPad->LP_ExstFlag & (1<<i))) //This set needs to be deleted or modified
    {
      if(pLPSetConfigN->Config[i].Nbr_Blks == 0) //The existing set needs to be Deleted.
      {
        pLPScratchPad->LP_Sets_Del |= (1<<i);
      }
      else 
      {
        pLPScratchPad->LP_Sets_Mod |= (1<<i);
      }
    }
  }
  
  
  if(pLPScratchPad->LP_Sets_Del)
  {
    if(pLPScratchPad->LP_Sets_Del & LP_SET1) // Set 1 has been deleted
    {
      memset(pLPSetConfigE->Config, 0, sizeof(pLPSetConfigE->Config));
      pLPScratchPad->LP_Sets_Reset |= (LP_SET1|LP_SET2|LP_SET3|LP_SET4);
      //Delete entire LP
    }
    if(pLPScratchPad->LP_Sets_Del & LP_SET2) // Set 2 has been deleted
    {
      //Delete Set 2. Also 3,4 if present
      pLPScratchPad->LP_Sets_Reset |= (LP_SET2|LP_SET3|LP_SET4);
      memset(&pLPSetConfigE->Config[1], 0, sizeof(pLPSetConfigE->Config[1]));
      memset(&pLPSetConfigE->Config[2], 0, sizeof(pLPSetConfigE->Config[2]));
      memset(&pLPSetConfigE->Config[3], 0, sizeof(pLPSetConfigE->Config[3]));
    }
    if(pLPScratchPad->LP_Sets_Del & LP_SET3) // Set 3 has been deleted
    {
      //Delete Set 3. Also 4 if presetn
      pLPScratchPad->LP_Sets_Reset |= (LP_SET3|LP_SET4);
      memset(&pLPSetConfigE->Config[2], 0, sizeof(pLPSetConfigE->Config[2]));
      memset(&pLPSetConfigE->Config[3], 0, sizeof(pLPSetConfigE->Config[3]));        
    }
    if(pLPScratchPad->LP_Sets_Del & LP_SET4) // Set 4 has been deleted
    {
      //Delete set 4
      pLPScratchPad->LP_Sets_Reset |= LP_SET4;
      memset(&pLPSetConfigE->Config[3], 0, sizeof(pLPSetConfigE->Config[3]));
    }
  }
  
  for(i=0; i < NUM_LP_DATA_SETS; i++)
  {
    if((pLPScratchPad->LP_Sets_Add & (1<<i)) || (pLPScratchPad->LP_Sets_Mod & (1<<i))) 
    {
      if((LP_CheckifSetFits(i, pLPScratchPad,pLPSetConfigN) && (pLPSetConfigN->Config[i].Nbr_Blks < 0xFFFF)))
      {
        if((pLPScratchPad->LP_Set_SizeE[i] != pLPScratchPad->LP_Set_SizeN[i]) && (pLPScratchPad->LP_Set_SizeN[i] <= MAXLPSIZE))
        {
          pLPScratchPad->LP_Sets_Reset |= ((0x0F<<i) & 0x0F);
          //printf("%d RESET %d\n",i,pLPScratchPad->LP_Sets_Reset);
          memcpy(&pLPSetConfigE->Config[i], &pLPSetConfigN->Config[i], sizeof(pLPSetConfigE->Config[i]));
          memset(&pLPSetConfigE->Config[i+1], 0, (sizeof(pLPSetConfigE->Config[i])*(NUM_LP_DATA_SETS-(i+1))));
          memset(&pLPScratchPad->LP_Set_SizeE[i+1],0,(sizeof(pLPScratchPad->LP_Set_SizeE[i])*(NUM_LP_DATA_SETS-(i+1))));
        }
        else if(pLPScratchPad->LP_Set_SizeN[i] <= MAXLPSIZE)
        {
          pLPScratchPad->LP_Sets_Reset |= (1<<i);
          memcpy(&pLPSetConfigE->Config[i], &pLPSetConfigN->Config[i], sizeof(pLPSetConfigE->Config[i]));
        }
      }
      else if(pLPSetConfigN->Config[i].Nbr_Blks < 0xFFFF)
      {
        pLPSetConfigN->Config[i].Nbr_Blks = 0;
      }
    }
  }
  
  memcpy(pLPSetConfigE, pLPSetConfigN, (sizeof(pLPSetConfigE->LP_Memory_Len)+sizeof(pLPSetConfigE->lp_flags)+sizeof(pLPSetConfigE->lp_fmats)));
  
  LP_ComputeReturnCode(rqst,pLPSetConfigN,pLPScratchPad);
  
  flashWriteMeter(GetStd61Addr(),(unsigned char*)pLPSetConfigE, sizeof(_std_table_60_t));
  flashWriteMeter(GetStd62Addr(),(unsigned char*)pLPDataConfigN, sizeof(_std_table_62_t));
  flashWriteMeter(GetMfg36Addr(),(unsigned char*)pLPQOSConfigN, sizeof(_mfg_table_36_t));
  LP_TableUpdate();
  //LP_DebugPrint(pLPSetConfigDe,pLPSetConfigE,pLPSetConfigN,pLPScratchPad);
  gLPResetFlag = pLPScratchPad->LP_Sets_Reset;
  //LP_SetUpLPStateGlobal();
  //LP_Reset(pLPScratchPad->LP_Sets_Reset);
  //LP_PrintLPState();
  return (pLPScratchPad->LP_ReturnCode);
}


unsigned char LP_CheckIfInvalidTables_Rev(unsigned char rqst,_std_table_60_t *LPConfiguration,_std_table_62_t *LPChnlData ,LPConfigScratchPad *pLPScratchPad)
{
  unsigned short temps;
  unsigned char i,j;
  unsigned long memsize=0,memwork=0;
  
  if(LPConfiguration->LP_Memory_Len != LP_CHK_MEMLEN)
    return eLPErrMemSize;
  
  memcpy(&temps,&LPConfiguration->lp_flags, sizeof(temps));
  if(temps != LP_CHK_LPFLAGS)
    return eLPErrFlags;
  
  temps = 0;
  memcpy(&temps,&LPConfiguration->lp_fmats, sizeof(unsigned char));
  if(temps != LP_CHK_LPFMAT)
    return eLPErrFmats;
  
  //check how many LP set have been enabled
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {
    //printf("New SET %d Check", (i+1));
    if(rqst & (1<<i))
    {
      pLPScratchPad->LP_RqstFlag |= (1<<i);
      //pLPScratchPad->LP_Set_SizeN[i] = (MAXLPSIZE +1);
      pLPScratchPad->LP_Set_SizeN[i] = 0;
      if(LPConfiguration->Config[i].Nbr_Blks)
      {
        //pLPScratchPad->LP_ReturnCode |= (eLPErrCnfg << (i*4));//Assuming the configuration is BAD
        
        if((LPConfiguration->Config[i].Nbr_Chns) && (LPConfiguration->Config[i].Nbr_Chns <= LP_CHK_NBRCHAN))
        {
          memwork=sizeof(_int_fmt1_rcd_t);
          memwork*=(unsigned long)LPConfiguration->Config[i].Nbr_Chns;//calc interval val array size
          memwork+=(LPConfiguration->Config[i].Nbr_Chns/2)+1;//calc unterval flags array size  
        }
        else
        {
          //printf("Break1\n");
          LPConfiguration->Config[i].Nbr_Blks = 0;//0xFFFF;
          continue;
        }
        
        if((LPConfiguration->Config[i].Nbr_blk_ints)&&(LPConfiguration->Config[i].Nbr_blk_ints <= LP_CHK_BLKINTS))
        {
          memwork*=(unsigned long)LPConfiguration->Config[i].Nbr_blk_ints;//calc bytes used by intervals
          memwork+=sizeof(_stime_date_t);//add block header (time stamp)
        }
        else
        {
          //printf("Break2\n");
          LPConfiguration->Config[i].Nbr_Blks = 0;//0xFFFF;
          continue;
        }
        
        memwork*=(unsigned long)LPConfiguration->Config[i].Nbr_Blks;
        /*
        if(LPConfiguration->Config[i].Nbr_Blks <= LP_CHK_NBRBLKS)
        {
          memwork*=(unsigned long)LPConfiguration->Config[i].Nbr_Blks;
        }
        else
        {
          //printf("Break3\n");
          LPConfiguration->Config[i].Nbr_Blks = 0xFFFF;
          continue;
        }
        */
        
        if(!((LPConfiguration->Config[i].Max_Int_Time==1)||(LPConfiguration->Config[i].Max_Int_Time==5)||(LPConfiguration->Config[i].Max_Int_Time==10)||
             (LPConfiguration->Config[i].Max_Int_Time==15)||(LPConfiguration->Config[i].Max_Int_Time==30)||(LPConfiguration->Config[i].Max_Int_Time==60)||
               (LPConfiguration->Config[i].Max_Int_Time==90)||(LPConfiguration->Config[i].Max_Int_Time==120)||(LPConfiguration->Config[i].Max_Int_Time==150)||
                 (LPConfiguration->Config[i].Max_Int_Time==180)||(LPConfiguration->Config[i].Max_Int_Time==210)||(LPConfiguration->Config[i].Max_Int_Time==240)))
        {
          //printf("Break4\n");
          LPConfiguration->Config[i].Nbr_Blks = 0;//0xFFFF;
          continue;   
        }
        
        if(LPChnlData->Format[i].Int_Fmt_Cde !=LP_CHK_INTFMTCDE)
        {
          //printf("Break5\n");
          LPConfiguration->Config[i].Nbr_Blks = 0;//0xFFFF;
          continue;
        }
        
        temps=0;
        for(j=0;j<LPConfiguration->Config[i].Nbr_Chns;j++)
        {
          if((LPChnlData->Format[i].LP_Sel_Set[j].Chnl_Flag != LP_CHK_CHNFLG) || (LPChnlData->Format[i].LP_Sel_Set[j].LP_Source_Select > LP_CHK_SRCSEL))
          {
            temps = 1;
            break;
          }
        }
        
        if(temps)
        {
          //printf("Break6\n");
          LPConfiguration->Config[i].Nbr_Blks = 0;//0xFFFF;
          continue;
        }
        
        memsize+=memwork;//add LP set size to LP total
        if(memsize>LP_CHK_MEMLEN)
        {
          //printf("Break7\n");
          LPConfiguration->Config[i].Nbr_Blks = 0;//0xFFFF;
          memsize-=memwork;
          continue;
        }
        pLPScratchPad->LP_Set_SizeN[i] = memwork; // Store the mem size for each 
        //pLPScratchPad->LP_ReturnCode &= ~(eLPErrCnfg << (i*4));//Configuration is Good
        //printf("NEW SET %d Size = %lu", (i+1),pLPScratchPad->LP_Set_SizeN[i]);
        //adjust mem size for next LP set (may require jump to next page)
        memwork=memsize%EE_PAGE_SIZE;
        memsize+=(EE_PAGE_SIZE-memwork);//jump to next flash page
        //printf("MEMSIZE %lu\n",memsize);
      }
    }
  }
  return eLPErrPass;
}

void LP_SizeofExistingSets(_std_table_60_t *pLPSetConfigE, LPConfigScratchPad *pLPScratchPad)
{
  unsigned char i;
  
  for(i=0;i<NUM_LP_DATA_SETS;i++)
  {    
    pLPScratchPad->LP_Set_SizeE[i]=sizeof(_int_fmt1_rcd_t);
    pLPScratchPad->LP_Set_SizeE[i]*=(unsigned long)pLPSetConfigE->Config[i].Nbr_Chns;//calc interval val array size
    pLPScratchPad->LP_Set_SizeE[i]+=(pLPSetConfigE->Config[i].Nbr_Chns/2)+1;//calc unterval flags array size  
    pLPScratchPad->LP_Set_SizeE[i]*=(unsigned long)pLPSetConfigE->Config[i].Nbr_blk_ints;//calc bytes used by intervals
    pLPScratchPad->LP_Set_SizeE[i]+=sizeof(_stime_date_t);//add block header (time stamp)
    pLPScratchPad->LP_Set_SizeE[i]*=(unsigned long)pLPSetConfigE->Config[i].Nbr_Blks;
    //pLPScratchPad->LP_Set_SizeE[i] += (pLPScratchPad->LP_Set_SizeE[i]%EE_PAGE_SIZE);
    //printf("Ex SET %d Size = %lu\n\n", (i+1),pLPScratchPad->LP_Set_SizeE[i]);
  }
}

unsigned char LP_CheckifSetFits(unsigned char LPSet,LPConfigScratchPad *pLPScratchPad, _std_table_60_t *pLPSetConfigN)
{
  unsigned char i=0, ret=1;
  unsigned long memsize = 0;
  unsigned long memwork = 0;
  /*
  What set we are adding/modfying?? For any set size check for Set 1 is required.
  We need to check if a higher order set exists or has been added/modified  
  */
  //printf("Size Check for Set %d\n",(LPSet+1));
  
  //The new set must have a non zero size requirement
  if(!(pLPScratchPad->LP_Set_SizeN[LPSet]))
  {
    return 0;
  }
  
  for(i=0; i<LPSet; i++)
  {
    if(((pLPScratchPad->LP_Sets_Add & (1<<i)) || (pLPScratchPad->LP_Sets_Mod & (1<<i)) || (pLPScratchPad->LP_Sets_Del & (1<<i))) && (pLPSetConfigN->Config[i].Nbr_Blks < 0xFFFF)) //Check size requirements starting from Set 1
      memwork = pLPScratchPad->LP_Set_SizeN[i];
    else
      memwork = pLPScratchPad->LP_Set_SizeE[i];
    
    memsize += memwork;
    memwork=memsize%EE_PAGE_SIZE;
    memsize+=(EE_PAGE_SIZE-memwork);//jump to next flash page
    if(MAXLPSIZE < memsize)//Cant fit requested Set
    {
      ret = 0;
      break;
    }
  }
  
  //printf("MEMSIZE FITS %lu\n",memsize);
  //printf("Used Memory %lu\n",memwork);
  if(((MAXLPSIZE -memsize) < pLPScratchPad->LP_Set_SizeN[LPSet]) && (ret)) //New set requires more than available.
  {
    ret = 0;
  }
  return ret;
}

void LP_ComputeReturnCode(unsigned char rqst,_std_table_60_t *pLPSetConfigN,LPConfigScratchPad *pLPScratchPad)
{
  unsigned char i;
  for(i=0; i<NUM_LP_DATA_SETS; i++)
  {
    if(rqst &(1<<i))//a request was made for changes
    {
      if(pLPScratchPad->LP_Sets_Del &(1<<i))//Request was to delete the set
      {
        pLPScratchPad->LP_ReturnCode |= (eLPSucDel << (4*i));
        //printf("Set %d Deleted\n",(i+1));
        //We obliged.
      }
      else if(pLPScratchPad->LP_Sets_Add &(1<<i))//Request was to Add the set
      {
        if(pLPScratchPad->LP_ReturnCode & (eLPErrCnfg <<(i*4)))
        {
          continue;
        }
        if((pLPSetConfigN->Config[i].Nbr_Blks == 0) || (pLPSetConfigN->Config[i].Nbr_Blks == 0xFFFF))//Request was denied
        {
          if((pLPScratchPad->LP_ExstFlag & (1<<i)) && (pLPScratchPad->LP_Sets_Reset & (1<<i)))
          {
            //printf("Set %d ADD Fail & Existing Set Deleted Due to Higher\n",(i+1));
            pLPScratchPad->LP_ReturnCode |= (eLPHigDel << (4*i));
          }
          else
          {
            //printf("Set %d ADD Fail\n",(i+1));
            pLPScratchPad->LP_ReturnCode |= (eLPErrAdd << (4*i));
          }
          //Sorry not happening today
        }
        else
        {
          //printf("Set %d Added\n",(i+1));
          pLPScratchPad->LP_ReturnCode |= (eLPSucAdd << (4*i));
          //We obliged
        }
      }
      else if(pLPScratchPad->LP_Sets_Mod & (1<<i))
      {
        if(pLPScratchPad->LP_ReturnCode & (eLPErrCnfg <<(i*4)))
        {
          continue;
        }
        
        if((pLPSetConfigN->Config[i].Nbr_Blks == 0)  || (pLPSetConfigN->Config[i].Nbr_Blks == 0xFFFF))//Request was denied
        {
          if((pLPScratchPad->LP_ExstFlag & (1<<i)) && (pLPScratchPad->LP_Sets_Reset & (1<<i)))
          {
            //printf("Set %d MOd Fail & Existing Set Deleted Due to Higher\n",(i+1));
            pLPScratchPad->LP_ReturnCode |= (eLPHigDel << (4*i));
          }
          else
          {
            //printf("Set %d MOD Fail\n",(i+1));
            pLPScratchPad->LP_ReturnCode |= (eLPErrMod << (4*i));
          }
          //Sorry not happening today
        }
        else
        {
          //printf("Set %d Modified\n",(i+1));
          pLPScratchPad->LP_ReturnCode |= (eLPSucMod << (4*i));
          //We obliged
        }
      }
      else if(pLPScratchPad->LP_Sets_Unchg & (1<<(i+4)))
      {
        pLPScratchPad->LP_ReturnCode |= (eLPErrDel << (4*i));
      }
    }
    else if(pLPScratchPad->LP_Sets_Unchg & (1<<i))
    {
      //Check if the set existed and has been disabled because a higher order set has changed in size
      if((pLPScratchPad->LP_Sets_Reset & (1<<i)) && (pLPScratchPad->LP_ExstFlag & (1<<i)))
      {
        //printf("Set %d Deleted by Higer\n",(i+1));
        pLPScratchPad->LP_ReturnCode |= (eLPHigDel << (4*i));
        //OK so we have been asked to go
      }
      else
        pLPScratchPad->LP_ReturnCode |= (eLPNoChng << (4*i)); 
    }
  }
  /*
    else//No request was made to change this set 
  {
  //Check if the set has been disabled because a higher order set has changed in size
  if((pLPScratchPad->LP_Sets_Reset & (1<<i)) && (pLPScratchPad->LP_ExstFlag & (1<<i)))
  {
  printf("Set %d Deleted by Higer\n",(i+1));
  //OK so we have been asked to go
}
}*/
  //printf("Return Code 0x%04X:",pLPScratchPad->LP_ReturnCode);
}

void LP_AutoStart_Activate()
{
  unsigned char i;
  for(i=0; i<NUM_LP_DATA_SETS; i++)
  {
    gLPState.LPFlags[i] |= LP_FLG_AUTOSTART; //LP_FLG_AUTOSTART
  }
}

void LP_AutoStart()
{
  unsigned char i,temp = 0xF0;
  for(i=0; i<NUM_LP_DATA_SETS; i++)
  {
    if(!(gLPState.LPFlags[i] & LP_FLG_AUTOSTART))
    {//The auto start flag should be set across all SETS
      temp &= ~(1<<(i*4));
    }
    else
    {
      gLPState.LPFlags[i] &= ~LP_FLG_AUTOSTART;
      temp |= (1<<i);
    }
  }
  if((temp & 0xF0) == 0xF0)//Alls good the auto start flag is across all SETS
  {
    LP_Reset(temp);
  }
}

void PowerFactorMinMax(unsigned long *NewPF, signed long *OldPF, unsigned char Type)
{
  signed long Local_OldPF = *OldPF;
  signed long Local_NewPF = *NewPF;
  
  if(Local_NewPF >= 300)
    Local_NewPF = 400 - Local_NewPF;
  else if (Local_NewPF >= 200)
    Local_NewPF = Local_NewPF -200;
  else if (Local_NewPF >= 100)
    Local_NewPF = 200 - Local_NewPF;
  
  if((Local_OldPF >= 300) && (Local_OldPF < 401))
    Local_OldPF = 400 - Local_OldPF;
  else if ((Local_OldPF >= 200) && (Local_OldPF < 401))
    Local_OldPF = Local_OldPF -200;
  else if ((Local_OldPF >= 100)  && (Local_OldPF < 401))
    Local_OldPF = 200 - Local_OldPF;
  
  if(Type == 1)
  {
    if(Local_NewPF < Local_OldPF)
    {
      *OldPF = *NewPF;
    }
  }
  else if(Type == 2)
  {
    if(Local_NewPF > Local_OldPF)
    {
      *OldPF = *NewPF;
    }
  }
}
