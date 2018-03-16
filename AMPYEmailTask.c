/*******************************************************************************
*                                                                              *
*                     PROPRIETARY RIGHTS NOTICE:                               *
*                                                                              *
*   ALL RIGHTS RESERVED. THIS MATERIAL CONTAINS THE VALUABLE                   *
*                    PROPERTIES AND TRADE SECRETS OF                           *
*                                                                              *
*                            AMPY+EMAIL                                        *
*                         SYDNEY AUSTRALIA                                     *
*                                                                              *
*               EMBODYING SUBSTANTIAL CREATIVE EFFORTS                         *
*   AND CONFIDENTIAL INFORMATION, IDEAS AND EXPRESSIONS, NO PART OF            *
*   WHICH MAY BE REPRODUCED OR TRANSMITTED IN ANY FORM OR BY ANY MEANS         *
*   ELECTRONIC, MECHANICAL, OR OTHERWISE, INCLUDING PHOTOCOPYING AND           *
*   RECORDING OR IN CONNECTION WITH ANY INFORMATION STORAGE OR                 *
*   RETRIEVAL SYSTEM WITHOUT THE PERMISSION IN WRITING FROM                    *
*                            AMPY+EMAIL                                        *
*                                                                              *
*                          COPYRIGHT 2008                                      *
*                     AMPY+EMAIL TECHNOLOGIES, INC.                            *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
           Copyright (C) 2008 AMPY+EMAIL.
         Contains CONFIDENTIAL and TRADE SECRET INFORMATION
********************************************************************************

      Module Name:      AMPYTASK.C

      Date:             30th June 2008

      Engineer:         Ian Davis, Marc Circosta

      Description:      Main task for Ampy Meter Process

      Changes:


************AMPY+EMAIL - CONFIDENTIAL******************************************/

#include "std.h"
#include "AMPYEmailCommon.h"
#include "AMPYDisplayApp.h"
#include "platform.h"
#include "stdio.h"
#include "string.h"
#include "kernel.h"
#include "intrinsics.h"
#include "DisplayDriver.h"
#include "rad_data.h"
#include "CRC.h"
#include "utilinet.h"
#include "zigbee.h"
#include "RTC.h"
#include "LoadProf.h"
#include "Tables.h"
#include "uugflash.h"
#include "MeterComms.h"
#include "uug_hard.h"
#include "RTC.h"
#include "I2C.h"
#include "PushButton.h"
#include "metrologydriver.h"
#include "RelayDriver.h"
#include "driver.h"
#include "eventlog.h"
#include "QOS.h"
#include "TOU.h"
#include "MeterStatus.h"
#include "Metrology.h"
#include "RelayAppLC.h"
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(MAINS_CLOCK)||defined(LC_RSM)
   #include "Ripple.h"
   #include "RippleApp.h"
#endif
#if defined(Simulation) && defined(UnitTesting)
  #include "UnitTests.h"
#endif
#include "MetrologyADCDriver.h"
#include "SysMonitor.h"
#if defined(NEUTRAL_INTEGRITY_POLY)
   #include "PolyPhaseNI.h"
#endif
#if defined(PULSE_OUTPUT)
#include "pulseoutput.h"
#endif
#include "LPQoS.h"
unsigned char gPowerDownStartFlag = 0;
unsigned char gEarlyPwrFailWarning=0;

#ifdef AMPY_METER_R1100
unsigned char gPowerDown_R1100_LEDs_Function_Disable = 0;
#endif
/******************************************************************************
Local constants/#define's
******************************************************************************/
#define BAD_IRQ_SIGNATURE        0xd2af3057
#define AMPY_MET_DATA_LK         0x01
#define AMPY_TBL_RAM_ACS         0x02
#define AMPY_T28_EGY_LK          0x04
#define AMPY_UUGFLASH_LK         0x08
#define AMPY_NV_CRASH_BUF_SIZE   8
#define AMPY_PROC_SKIP_TOU       0x01
#define AMPY_PROC_SKIP_NA7       0x02
#define AMPY_PROC_SKIP_NA6       0x04
#define AMPY_PROC_SKIP_NA5       0x08
#define AMPY_PROC_SKIP_NA4       0x10
#define AMPY_PROC_SKIP_NA3       0x20
#define AMPY_PROC_SKIP_NA2       0x40
#define AMPY_PROC_SKIP_NA1       0x80

#define AMPY_GEN_FLG_DO_PRINT    0x01
#define AMPY_GEN_FLG_DO_VAL_TEST 0x02
#define AMPY_GEN_FLG_DELTA       0x04
#define AMPY_GEN_FLG_INVERSE     0x08
#define AMPY_GEN_FLG_CHK_EQUAL   0x10
#define AMPY_GEN_FLG_SKIP        0x20
#define AMPY_GEN_FLG_SPARE2      0x40
#define AMPY_GEN_FLG_SPARE1      0x80

#define AMPY_TEST_VAL_PATTERN    0x5AD69F0C


//#define FLASH_ERASEWRITE_TIME_DEBUG  //test flash part for erase/write time that is too long.

//#define LOG_COMMS_CMDS

//#define DBG_PRINT_TIME_DBG
//#define DBG_FLASH_PACK
//#define DBG_PWR_FAIL_PIN_OUT
//#define DBG_PWR_FAIL_LC_RLY_CHNG
//#define PRINT_POWER_DN_DELAY
//#define LOG_POWER_DN_DELAY
//#define LOG_POWER_UP_NOT_MAINLOOP
//#define DBG_POWERFAILTEST
//#define PWD_COMMS_PWR_FAIL_DEBUG
// see amyemailcommon.h for DBG_POSTRESETDUMP
// see amyemailcommon.h for PWD_DOWN_RLY_SWITCH_DEBUG def

#if defined(PRINT_POWER_DN_DELAY)
   #if defined(LOG_POWER_DN_DELAY)
      #define P_DN_PRINT_DELAY(line) if(DoPDLog)DataFlashWritePageBytesNoErase(AEMFLASHSTARTPAGE + AEMTOTALPAGES,0,(unsigned char*)&gPDTriggerTime,4);
      #define P_DN_PRINT_MAINLOOP() if(DoPDLog)DataFlashWritePageBytesNoErase(AEMFLASHSTARTPAGE + AEMTOTALPAGES,4,(unsigned char*)gPDTimes,4*20);for(int pdi=0;pdi<20;pdi++)gPDTimes[pdi]=0;
      #define P_DN_PRINT_TIME(line) TimeD=SysTimeGet();if(DoPDLog){DataFlashWritePageBytesNoErase(AEMFLASHSTARTPAGE + AEMTOTALPAGES,DoPDLog*4,(unsigned char*)&TimeD,4);DoPDLog++;}
   #else
      #define P_DN_PRINT_DELAY(line) printf("%u,%lu\n",line,gPDTriggerTime)
      #define P_DN_PRINT_MAINLOOP() for(int pdi=0;pdi<20;pdi++){printf("m=%lu\n",gPDTimes[pdi]);gPDTimes[pdi]=0;}
      #define P_DN_PRINT_TIME(line) printf("%u,%lu\n",line,SysTimeGet())
   #endif

   #define P_DN_LOG_TIME(pos) gPDTimes[pos]=SysTimeGet()

   #if defined(LOG_POWER_UP_NOT_MAINLOOP)
      //#define P_DN_LOG_TIME_UP(pos) gPDTimes[pos]=SysTimeGet()
      #define P_DN_LOG_TIME_UP(pos) printf("%u,%lu\n",pos,SysTimeGet())
      #define P_DN_LOG_TIME_MAINLOOP(pos)
   #else
      #define P_DN_LOG_TIME_MAINLOOP(pos) gPDTimes[pos]=SysTimeGet()
      #define P_DN_LOG_TIME_UP(pos)
   #endif

   unsigned long gPDTriggerTime=0;
   unsigned char LastPFPinState=0;
   unsigned long gPDTimes[20];
#else
   #define P_DN_PRINT_DELAY(line)
   #define P_DN_PRINT_MAINLOOP()
   #define P_DN_LOG_TIME(pos)
   #define P_DN_LOG_TIME_MAINLOOP(pos)
   #define P_DN_LOG_TIME_UP(pos)
#endif

#if defined(DBG_POSTRESETDUMP)
   #define AMPY_POST_RESET_DUMP_ARMED   0x5BAD
#endif

#if defined(DBG_PRINT_TIME_DBG)
   #define DBG_PRINT_TIME(line) if(g_dbg_do_print)printf("%u,%lu\n",line,SysTimeGet())
#elif defined(SIM_INTERRUPT)
   unsigned short dwSim_Temp;
   _PFDebug_t gPFDebug;	
   _sim_interrupt_t gSimInterrupt;
	#define DBG_PRINT_TIME(id)    disable(); \
                                   dwSim_Temp = gPFDebug.tics - gPFDebug.LastTaskStepTmStamp[METERTASK]; \
                                   gPFDebug.LastTaskStepTmStamp[METERTASK] = gPFDebug.tics; gPFDebug.CurrentTaskStepId[METERTASK]=id; \
                                   if (gPFDebug.PFState==2) \
                                   { \
                                      gPFDebug.PFDetectTaskStepId[METERTASK] = id; \
                                      gPFDebug.PFDetectTaskStepList[METERTASK]|=((0x00000001UL)<<id); \
                                      if (dwSim_Temp > gPFDebug.PFDetectTaskMaxDelayStepTm[METERTASK]) \
											     { \
												     gPFDebug.PFDetectTaskMaxDelayStepTm[METERTASK] = dwSim_Temp; \
                                         gPFDebug.PFDetectTaskMaxDelayStepId[METERTASK] = id; \
												  } \
											  } \
										     else if (gPFDebug.PFState==3) \
                                   { \
                                      gPFDebug.PDStartTaskStepId[METERTASK] = id; \
                                      gPFDebug.PDStartTaskStepList[METERTASK] |=((0x00000001UL)<<id); \
                                      if (dwSim_Temp>gSimInterrupt.PDStartTaskMaxDelayStepTm[METERTASK]) \
											     { \
                                         gPFDebug.PDStartTaskMaxDelayStepTm[METERTASK] = dwSim_Temp; \
                                         gPFDebug.PDStartTaskMaxDelayStepId[METERTASK] = id; \
												  } \
                                   } \
										     enable();
   #define P_DN_PRINT_TIME(id)     dwSim_Temp = gPFDebug.tics - gPFDebug.LastTaskStepTmStamp[METERTASK]; \
                                   gPFDebug.LastTaskStepTmStamp[METERTASK] = gPFDebug.tics; gPFDebug.CurrentTaskStepId[METERTASK]=id; \
                                   if (dwSim_Temp>gSimInterrupt.PDProcessMaxDelayStepTm) \
                                   { \
                                      gSimInterrupt.PDProcessMaxDelayStepId = id; \
                                      gSimInterrupt.PDProcessMaxDelayStepTm = dwSim_Temp; \
                                      gPFDebug.SimInfoUpdated = 1; \
                                   }

	unsigned char StopTest = 0;
#else
	#define DBG_PRINT_TIME(line)		
   #define P_DN_PRINT_TIME(line)
#endif

extern unsigned char gSecCheckDone;	
/******************************************************************************
Local enums
******************************************************************************/
//power state of AMPY meter task
enum{eAMPYpsNoPower = 0, eAMPYpsActive, eAMPYpsLPM};

/******************************************************************************
Function protoypes
******************************************************************************/
void MMI_Clock_Init(void);
void AMPYEmail_UpdateTamperStatus();
void AMPYPowerUp (void);
unsigned char AMPYCheckDoPowerUpUndo(unsigned char ForceUndo);
void BadIRQ(unsigned char IRQtype);
void GetBadIRQStatus();
unsigned char AMPYPowerCycleCheck(unsigned int stage);
void AMPYSuspendMetrology();
void AMPYResumeMetrology();
void UpdateTrueRand();
void CheckFwDownloadActivationTime();
void InitFwDl(void);
void MeterApplicationWaitForShutdown(void);           // only needed for the sim only, to be removed...
unsigned char AMPYPowerUpCommon (unsigned long DownTime);
void AMPYCleanForPowerFail(void);
void AMPYCrashLogInit(void);
void AMPYFlashEnablePacking(void);
void AMPYFlashDisablePacking(void);
unsigned char AMPYFlashUnpack(void);
void AMPYFlashPack(unsigned short PageNum, unsigned short offset, unsigned char *pData, unsigned short len);
void DF_init_incremental_refresh(void);
void DF_backup_incremental_refresh(void);
void PrintSysStatus(void);
void PrePaymentPowerDown();

#ifdef PLATFORM_HOG_DEBUG
   extern void LogPlatformTimeTick();
#endif
#ifdef LOG_COMMS_CMDS
   extern void PrintLogMsg();
#endif

#ifdef DBG_POWERFAILTEST
   void TimerB1_Setup();
#endif

#if defined(DBG_POSTRESETDUMP)
   void PostResetChkDoStackDump( void );
#endif

/******************************************************************************
RAM data / memory allocation
******************************************************************************/
//#pragma memory=dataseg(DEBUGGER_RAM)
//#warning I broke Ians BadIRQ stuff
 unsigned long BadIRQSignature;
 unsigned char BadIRQCount;
 unsigned char BadIRQVal;
//#pragma memory=default

// Powerfail?
unsigned char Standby_Mode;

volatile unsigned char AMPYPowerStateMode=eAMPYpsNoPower;//power state of AMPY meter task
volatile unsigned char AMPYForceReset=0;//signal that a power fail backup is required
volatile unsigned char AMPYForceReset2=0;//signal that a reset is required.
extern SEMAPHORE tableRAMAccess;
extern SEMAPHORE metrology_data_lock;
extern SEMAPHORE flash_lock;
unsigned char gAMPY_Metrology_Flg;
unsigned char gZCLoss=0;
unsigned short gRandBase;//used by random number generator
unsigned char gAMPY_NV_crash_buffer[AMPY_NV_CRASH_BUF_SIZE];
unsigned char gProcessSkipped=0;//each bit indicates a mainloop process that was skipped due to power fail signal
unsigned char gFirmwareUpdated = 0;
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
extern unsigned char gRAppLC_CheckSeason;
#endif
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
   extern unsigned char gU1300Ver2;
   extern unsigned char gPllMultiplier;
   extern void Metrology_SPI_Init();
#endif
//Initial value is only significant if a crash occurs before AMPYCrashLogInit()
//is called. If this did happen then info will be written 80% into the log area.
unsigned short gAMPY_NV_crash_index=(AMPY_NV_CRASH_MAX_CHARS*80L)/100L;

// for direct referencing of mfg_table_54_struct on powerfail and restore
extern _mfg_table_54_t mfg_table_54_struct;
extern SEMAPHORE adc_lock;
extern SEMAPHORE encryptLock;
extern SEMAPHORE flash_lock;
extern SEMAPHORE uugflash_lock;
extern TASK_STRUCT Tcb[TASKS_AVAILABLE];

unsigned char gAMPYFlashDoPack=0;//used on power fail to compress flash writes into a single blob
unsigned short gAMPYFlashIdx=0;
unsigned char gFirmwareUpgrade=0; //Used to disable Flash Packing on FirmwareUpgrade
extern _std_table_03_t std_table_03_struct;
extern unsigned char gSecCheckDone;	

unsigned char g_ampy_generic_flags = 0;
unsigned long g_ampy_test_val_pattern;
unsigned char*g_ampy_test_val_ptr;
unsigned long g_ampy_test_val_mask;
unsigned long g_ampy_test_val_last;
unsigned long g_ampy_test_val_ref;

#if defined(DBG_TEST_RELAY_CYCLE)
unsigned long gRelayDbgCycCnt=0;
unsigned char gRelayDbgCycRun=0;
unsigned char gRelayDbgCycTimer=0;
unsigned short gRelayDbgCycGlitch=0;
unsigned char LastPFPinState=0;
unsigned short gRelayDbgCycTemp;
#endif

#if defined(DBG_POWERFAILTEST)
unsigned char glaststate;
unsigned short gcount =0;
unsigned short g_finalcount;
unsigned char do_once = 0;
#endif

#if defined(DBG_POSTRESETDUMP)
   extern POWER_ON_DATA reboot_data;
#endif

#ifdef PWD_DOWN_RLY_SWITCH_DEBUG
unsigned char gRlyDbgMode=0;
#endif

#ifdef PWD_COMMS_PWR_FAIL_DEBUG
unsigned short g_dbg_comms_pwr_fail=0;
#endif

#if defined(DBG_PRINT_TIME_DBG) || defined(POLYNI_DEBUG)
   unsigned char g_dbg_do_print=0;
   extern _mfg_table_24_t MfgT24_TempStatus;
#endif

#if defined(AMPY_METER_NO_RF)
const unsigned char IDHashArr[9]={0xB3,0x8D,0x7B,0xEA,0xDE,0x70,0x19,0xDB,0x2E};//clear text value of 22,5E,00,48,89,FA,1F,33,3F  (U1300 F7 instead of 3F) pwd=6F,D1,85,5E,AB,8A,06,58,11
#endif

unsigned char system_die_flag = 0;

#ifdef SEMAPHORE_RAMACCESS_DEBUG
unsigned char StackDumpRAM[METER_TASK_STACK_SIZE] ;
unsigned short gRamDumpSemValue;
unsigned long gRamDumpTime;
unsigned char* gRAMDumpSP;
unsigned char* gRAMDumpTOS;
unsigned char gRAMdumpTaskId;
extern unsigned char SemHeadPad;
extern unsigned char SemTailPad;
SemaphoreDebugLog gSemaphoreDebugLog[TASKS_AVAILABLE][2];    //[x][y] where x= taskid, y=0 for Lock ,y=1 for unlock
unsigned char gDisableSemDebug=0;
#endif

#ifdef LCD_TEST
extern unsigned short gLCD_TEST;
extern unsigned char gUpdate;
#endif

#ifdef FLASH_FASTPACK
unsigned char gFlashPageBuff[512];
#endif

/*****************************************************************************/

/*------------------------------------------------------------------------------
* Function:    MeterTask
* Description: AMPY meter task.
* Notes:       Function never returns.
*-----------------------------------------------------------------------------*/
void MeterTask( void )
{
   unsigned char OneSecCounter=0;//accurate RTC 1 sec counter
   unsigned char FakeSeconds=0;//inaccurate seconds counter that does not rely on the I2C RTC chip.
   unsigned long TotalSleepTime,SnapShot;
#ifdef FLASH_ERASEWRITE_TIME_DEBUG
   unsigned char FlashChkFlg=0;
#endif

   AMPYPowerStateMode=eAMPYpsNoPower;

   //queueInit (0, 0, (QUEUE_HDR QH_NEAR *) &LGMeter_Q);

   // ideally, such a RAM sizing problem should be caught with a compile time
   // check. since the necessary math is not possible with the precompiler, do
   // the next best thing and check it at run time
   if(0)// ((&BackupChecksum - &Start_Backup) != BACKUP_DATA_SIZE)
   {
      // check the function DummyVariableAccess() if you see this error
      SysAbort( __FILE__, __LINE__);
   }

#if !defined(SIMULATION_SR)
#pragma segment="PART_INFO"
#ifdef M16C_65
   if((long)__segment_begin("PART_INFO")!=0x40000)
#else
   if((long)__segment_begin("PART_INFO")!=0xA0000)
#endif
   {
      gAMPY_NV_crash_index=0xFFFF;
      printf("xcl file!\n");
      hardReboot();
   }
#endif


#if defined(DBG_POWERFAILTEST)
   TimerB1_Setup();
#endif
   AMPYPowerUp();


   #if defined(Simulation) && defined(UnitTesting)
   UnitTests();
   #endif

   while (TRUE)
   {
     
#ifdef SIM_INTERRUPT		
		if(PressedButtons.Bits.Button2)
		{
			StopTest = 1;	
		}
      if(PressedButtons.Bits.Button1) 
      {
         StopTest = 0;
      }
#endif
#if defined(LCD_TEST)
      if(PressedButtons.Bits.Button2)
      {
         if (gLCD_TEST)
         {
            gLCD_TEST = 0;
         }
         else
         {
            gLCD_TEST = 1;
         }
         PressedButtons.Bits.Button2=0;        
      }
      
      if(PressedButtons.Bits.Button1) 
      {
         gUpdate = 1;
         PressedButtons.Bits.Button1=0;
      }
#endif      
#if defined(LOG_POWER_DN_DELAY)
      if(PressedButtons.Bits.Button2)
      {
         MStatPowerUp();
         Display(edPassed, DISPLAYIDNOSHOW);
         PressedButtons.Bits.Button2=0;
      }
#endif

#ifdef TYPETEST_DEBUG
      if(PressedButtons.Bits.Button2)
      {
         //DumpCtrlRegisters();
        // ((void (*)(void))NULL)();//function call of 0x00000
         //tl=*((unsigned long *)NULL);
         //printf("Addr 0=%lu",tl);
         disable();

         EnableSingleStepIRQ(1);
         EnableSingleStepIRQ(0);
         enable();
         PrintPCHistory();
         PressedButtons.Bits.Button2=0;
      }
#endif
#ifdef PROFILE_ON
      if(PressedButtons.Bits.Button2)
      {
         profilePrint();
         profileReset();
         PressedButtons.Bits.Button2=0;
      }
#endif
#ifdef CHECK_IN_DEBUG
      if(PressedButtons.Bits.Button2)
      {
         StillAlivePrint();
         PressedButtons.Bits.Button2=0;
      }
#endif
#ifdef PLATFORM_USER_LVL_DEBUG
      if(PressedButtons.Bits.Button2)
      {
         PlatformCountPrint();
         PressedButtons.Bits.Button2=0;
      }
#endif
#ifdef FLASH_ERASEWRITE_TIME_DEBUG
      extern unsigned char DisplayNumber(unsigned long int num, unsigned char offset,unsigned char len, unsigned char ShowZero);
      extern void DisplayChar(unsigned char value, unsigned char displacement);
      if(PressedButtons.Bits.Button2)
      {
         int page=0;
         unsigned char buff[2];
         unsigned long tempUL;
         _std_table_01_t ST01Bkp;

         buff[0]=0x50;
         buff[1]=0xFA;
         TableRead(eStdT01_GenMfgID,0,0,&ST01Bkp);
         while(page<4096)
         {
            if((page/10)&0x0001)
               VARHLED_STATE=IO_LO;
            else
               VARHLED_STATE=IO_HI;

            LockTask();
            tempUL=SysTimeGet();
            DataFlashWritePageBytes(page,0,buff,2);
            tempUL=SysTimeGet()-tempUL;
            UnlockTask();
            //printf("page %i=%lu\n",page,tempUL);
            if(tempUL>30)
            {
               FlashChkFlg=1;
               break;
            }

            if((page%41)==0)
            {
               Display(edBlank, DISPLAYIDNOSHOW);
               DisplayChar(eChar_F,6);
               DisplayChar(eChar_L,5);
               DisplayChar(eChar_A,4);
               DisplayChar(eChar_S,3);
               DisplayChar(eChar_H,2);
               DisplayNumber(page/41L,0,2,1);
               DisplayD_Update(gLCDArr);
            }
            page++;
         }
         printf("page %i=%lu\n",page,tempUL);
         if(!FlashChkFlg)
            FlashChkFlg=2;
         TableWrite(eStdT01_GenMfgID,0,0,&ST01Bkp);
         PressedButtons.Bits.Button2=0;
      }
      if(FlashChkFlg)
      {
         if(FlashChkFlg==2)
            Display(edPassed, DISPLAYIDNOSHOW);
         else
            Display(edFailed, DISPLAYIDNOSHOW);
      }
#endif
#ifdef LOG_COMMS_CMDS
      if(PressedButtons.Bits.Button2)
      {
         PrintLogMsg();
         PressedButtons.Bits.Button2=0;
      }
#endif
#ifdef DBG_TEST_RELAY_PULSE
      extern unsigned char DisplayNumber(unsigned long int num, unsigned char offset,unsigned char len, unsigned char ShowZero);
      extern void DisplayChar(unsigned char value, unsigned char displacement);
      if(PressedButtons.Bits.Button2)
      {
         gRelayDbgTime+=5;
         if(gRelayDbgTime>60)
            gRelayDbgTime=25;
         Display(edBlank, DISPLAYIDNOSHOW);
         DisplayChar(eChar_r,4);
         DisplayNumber(gRelayDbgTime,0,2,1);
         PressedButtons.Bits.Button2=0;
      }
#endif
#ifdef DBG_TEST_RELAY_CYCLE
      extern unsigned char DisplayNumber(unsigned long int num, unsigned char offset,unsigned char len, unsigned char ShowZero);
      extern void DisplayChar(unsigned char value, unsigned char displacement);
      if(PressedButtons.Bits.Button2)
      {
         if(Get_Button_Status()&PUSHBUTTON1_BIT)
         {
            Display(edPassed, DISPLAYIDNOSHOW);
            gRelayDbgCycCnt=0;
            gRelayDbgCycRun=0;
            gRelayDbgCycTimer=0;
            gRelayDbgCycGlitch=0;
            AMPYPowerCycleCountReset();
         }
         else
         {
            if(gRelayDbgCycRun)
               gRelayDbgCycRun=0;
            else
               gRelayDbgCycRun=1;
         }
         PressedButtons.Bits.Button2=0;
      }
#endif

#ifdef PWD_DOWN_RLY_SWITCH_DEBUG
      extern unsigned char DisplayNumber(unsigned long int num, unsigned char offset,unsigned char len, unsigned char ShowZero);
      extern void DisplayChar(unsigned char value, unsigned char displacement);
      if(PressedButtons.Bits.Button2)
      {
         gRlyDbgMode++;
         if(gRlyDbgMode>12)//there are 13 options in total
            gRlyDbgMode=0;

         Display(edBlank, DISPLAYIDNOSHOW);
         DisplayChar(eChar_r,5);
         DisplayChar(eChar_L,4);
         DisplayChar(eChar_y,3);
         DisplayNumber(gRlyDbgMode,0,2,1);
         PressedButtons.Bits.Button2=0;
      }
#endif

#ifdef PWD_COMMS_PWR_FAIL_DEBUG
      extern unsigned char DisplayNumber(unsigned long int num, unsigned char offset,unsigned char len, unsigned char ShowZero);
      extern void DisplayChar(unsigned char value, unsigned char displacement);
      if(PressedButtons.Bits.Button2)
      {
         Display(edBlank, DISPLAYIDNOSHOW);
         DisplayChar(eChar_P,6);
         DisplayChar(eChar_F,5);
         DisplayNumber(0,3,1,1);
         MTR_PF_STATE=IO_LO;
         g_dbg_comms_pwr_fail=100;
         PressedButtons.Bits.Button2=0;
      }
      if(LongPressedButtons.Bits.Button2)
      {
         Display(edBlank, DISPLAYIDNOSHOW);
         DisplayChar(eChar_P,6);
         DisplayChar(eChar_F,5);
         DisplayNumber(1,3,1,1);
         MTR_PF_STATE=IO_LO;
         g_dbg_comms_pwr_fail=1000;
         LongPressedButtons.Bits.Button2=0;
      }
#endif
#if defined(DBG_PRINT_TIME_DBG) || defined(POLYNI_DEBUG)
      if(PressedButtons.Bits.Button2)
      {
         if (g_dbg_do_print)
            g_dbg_do_print = 0;
         else
            g_dbg_do_print=MfgT24_TempStatus.AvgAccumCount;
         PressedButtons.Bits.Button2=0;
         printf("print=%d\n",g_dbg_do_print);
      }
#endif

      DBG_PRINT_TIME(0);

      if(AMPYForceReset)
      {
         AMPYPowerFail();

         //R30 change: system_die_flag is set only if the task that crashed == Metrology task.
         // If flag set, Resume Metrology (AMPYPowerFail suspends it)
         if(system_die_flag)                      //Junjie: resume metrology task when meter crash
         {
            Resume(METROLOGY_TASK);
         }

         while(1)
            Pend(0,200);//don't be hog let other lower priority tasks run and finish the sys_reset()
      }
      if(AMPYForceReset2)
         sys_reset(SOFT_SUICIDE_RESET);

      //check for power fail
      if(/*(EPFState_M)||*/AMPYGetFilteredEPF(1))
      {
         if(AMPYPowerStateMode==eAMPYpsActive)
         {//we have lost mains power, save everything and enter low power consumtion
            EventLogWrite(eEvtPwrDn,0,0,NULL);
            AMPYPowerFail();
         }
      }
      else
      {
         if((AMPYPowerStateMode==eAMPYpsLPM)||(AMPYPowerStateMode==eAMPYpsNoPower))
         {//mains has returned after a short outage, restore anything we previously shut down
            AMPYPowerUp();
            OneSecCounter=0;
         }
         else if(AMPYPowerStateMode==eAMPYpsActive)
         {
#if defined(PWD_COMMS_PWR_FAIL_DEBUG)
            if(g_dbg_comms_pwr_fail==0)
#endif
            if(gZCLoss==0)
               RemotePortPowerRestore();
         }
      }

      DBG_PRINT_TIME(1);

      P_DN_LOG_TIME_MAINLOOP(2);
      if(AMPYPowerStateMode==eAMPYpsActive)
      {
         if(gProcessSkipped)
         {
            if(gProcessSkipped&AMPY_PROC_SKIP_TOU)
            {
               if(!TOU_ChkDoRateChange())
                  gProcessSkipped&=~AMPY_PROC_SKIP_TOU;
            }
         }
         CommsMainLoopTick();
         DBG_PRINT_TIME(2);
         P_DN_LOG_TIME_MAINLOOP(3);
         if(TotalSleepTime>=AMPYEMAIL_SLEEPTIME)
         {
            TotalSleepTime=0;
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)				
            LP_SecTick();
#endif
            OneSecCounter+=RTC_MainLoopTick();
            P_DN_LOG_TIME_MAINLOOP(4);
            DBG_PRINT_TIME(3);
            if(OneSecCounter>1)
               SysAbort( __FILE__, __LINE__);//at the moment if we jump more than one seconds then we risk LP not working properly
            MStatMainLoopTick();
            P_DN_LOG_TIME_MAINLOOP(5);
            DBG_PRINT_TIME(4);
            EventMainLoopTick();
            P_DN_LOG_TIME_MAINLOOP(6);
            DBG_PRINT_TIME(5);
            DisplayMainLoopTick();
#ifdef DBG_TEST_RELAY_CYCLE
            if(gRelayDbgCycRun)
            {
               Display(edBlank, DISPLAYIDNOSHOW);
               if(gRelayDbgCycRun==1)
               {
                  DisplayChar(eChar_r,6);
                  DisplayNumber(gRelayDbgCycCnt,0,6,1);
               }
               else if(gRelayDbgCycRun==2)
               {
                  DisplayChar(eChar_P,6);
                  if(MStatPowerCycleCheck(2))
                  {
                     DisplayChar(eChar_minus,0);
                     DisplayChar(eChar_minus,1);
                     DisplayChar(eChar_minus,2);
                     DisplayChar(eChar_minus,3);
                     DisplayChar(eChar_minus,4);
                  }
                  else
                  {
                     TableRead(eMfgT54_GnrlMeterNVStatus,offsetof(_mfg_table_54_t,PFCycleCounts),2,&gRelayDbgCycTemp);
                     DisplayNumber(gRelayDbgCycTemp,0,5,1);
                  }
               }
               else
               {
                  DisplayChar(eChar_g,6);
                  DisplayNumber(gRelayDbgCycGlitch,0,5,1);
               }
            }
#endif
            P_DN_LOG_TIME_MAINLOOP(7);
            DBG_PRINT_TIME(6);
            

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
            RippleMsTick();
            DBG_PRINT_TIME(7);
            RAppLC_MainloopTick();
            DBG_PRINT_TIME(8);
            RippleApp_MainloopTick();
            DBG_PRINT_TIME(9);
#endif

            RelayMainLoopTick();
            DBG_PRINT_TIME(10);
     
            P_DN_LOG_TIME_MAINLOOP(8);
            DBG_PRINT_TIME(11);
            QOSMainLoopTick();
            P_DN_LOG_TIME_MAINLOOP(9);
            LP_MainLoopTick();
            P_DN_LOG_TIME_MAINLOOP(10);
            DBG_PRINT_TIME(12);
            TOU_MainLoopTick();
            P_DN_LOG_TIME_MAINLOOP(11);
            UpdateTrueRand();
            P_DN_LOG_TIME_MAINLOOP(12);
            #ifdef U1300_RIPPLE
            MetrologyADC_ProcMainLoop();
            #endif
            Metrology_NIMainLoopTick();
            DBG_PRINT_TIME(13);
            metrology_driver_energy_update();
#ifdef  PRINT_TABLEADDR_FLASH
            extern unsigned char gFlashTableData;
            extern void TableAddrFlash();

            if(gFlashTableData)
            {
               gFlashTableData = 0;
               TableAddrFlash();
            }
#endif
            //------------------- Update Timers --------------------------------------
            // we'll lose time if this loop doesn't execute every 30ms...
            // At some point it was seen that LP_SecTick() was being executed only ~53 times per minute.
            // probably due to task taking longer to come out of the Pend() below.
            // It was also measured that each mainloop tic was taking ~36ms, with meter comms causing it to rise to
            // ~178ms.

            if(gRTCChanged)
            {//time has been changed tell anyone who cares
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
              gRAppLC_CheckSeason = 1;
#endif
               if(TOU_ChkDoRateChange())//Check if we have to switch to another rate.
                     gProcessSkipped|=AMPY_PROC_SKIP_TOU;
               gRTCChanged=0;
            }
            DBG_PRINT_TIME(14);
            P_DN_LOG_TIME_MAINLOOP(13);

            if(OneSecCounter>=1)
            {
               FakeSeconds++;
               OneSecCounter -= 1;
               gSecCheckDone = 0;

               P_DN_LOG_TIME_MAINLOOP(14);
#if (!(defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)))&&(!defined(DYNAMIC_LP))					
               LP_SecTick();
#endif					
               DBG_PRINT_TIME(15);
               CommsSecTick();
               DBG_PRINT_TIME(16);
               DisplaySecTick();
               P_DN_LOG_TIME_MAINLOOP(15);
               DBG_PRINT_TIME(17);
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
               RAppLC_SecTick();
               DBG_PRINT_TIME(18);
               RippleApp_SecTick();
#endif
               DBG_PRINT_TIME(19);
               RelaySecTick();
               P_DN_LOG_TIME_MAINLOOP(16);
               DBG_PRINT_TIME(20);
               QOSsecTick();
               P_DN_LOG_TIME_MAINLOOP(17);
               DBG_PRINT_TIME(21);
               Metrology_SecTick();
               P_DN_LOG_TIME_MAINLOOP(18);
               DBG_PRINT_TIME(22);
               TOU_SecTick();
#if defined(LP_QOS)
               LpQoS_SecTick();
#endif
               DBG_PRINT_TIME(23);
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(AMPY_METER_U3400)
               TamperDetectSecTick();
#endif         
               DBG_PRINT_TIME(24);
               SysMon_TempCheck();
               DBG_PRINT_TIME(25);
#if defined(METROLOGY_VERIFY_MMI_PARAMS)
               Metrology_MMIParamUpdateTick();
#endif
               DBG_PRINT_TIME(26);
#if defined(PULSE_OUTPUT)
               PulseOutput_ThresholdCtrl_SecTick();
#endif
               DBG_PRINT_TIME(27);
               if(RealTime.TimeFlags & MIN1_FLAG)
               {
                  MCom_MinuteTick();
                  EventMinuteTick();
                  DBG_PRINT_TIME(28);
                  TOU_SelfRead(FALSE);
                  DBG_PRINT_TIME(29);						
                  if(TOU_ChkDoRateChange())
                     gProcessSkipped|=AMPY_PROC_SKIP_TOU;
                  DBG_PRINT_TIME(30);
                  P_DN_LOG_TIME_MAINLOOP(18);
                  //SetLpEoiTask();
                  CheckFwDownloadActivationTime();    // maybe we shouldn't do this bang on a minute?
                  //if (gPolyPhaseMeter) motorDriveMaintenance();            // call every minute, to either charge cap or twitch when required.
                  RealTime.TimeFlags &= ~MIN1_FLAG;
               }
               AMPYUpdateTbl3StatusFlags();
#ifdef DBG_TEST_RELAY_CYCLE
               extern void RelayChange(unsigned char RelayType, unsigned char Command);
               if(gRelayDbgCycRun)
               {
                  gRelayDbgCycRun&=0x03;
                  gRelayDbgCycRun++;

                  gRelayDbgCycTimer++;
                  if(gRelayDbgCycTimer==5)
                  {//close relay
                     RelayChange(DisRelay, cClose);
                  }
                  else if(gRelayDbgCycTimer>=10)
                  {//open realy
                     RelayChange(DisRelay, cOpen);
                     gRelayDbgCycTimer=0;
                     gRelayDbgCycCnt++;
                  }
               }
#endif
            }//end one sec counter
         }//end of sleep time tick ~30ms
      }
      else//AMPYPowerStateMode
      {//what should we do while power is off but super-cap keeps us up
         FakeSeconds++;
         if(FakeSeconds>=30)
         {//a pretend 1 second has elapsed
            FakeSeconds-=30;
            OneSecCounter++;
            ClearAllSegments();
            if(OneSecCounter&0x01)
               DisplayIcon(eIcon_decimalPoint6, FALSE);
            else
               DisplayIcon(eIcon_decimalPoint6, TRUE);
            DisplayD_Update(gLCDArr);
         }
      }//end AMPYPowerStateMode
      P_DN_LOG_TIME_MAINLOOP(19);

      StillAlive();  // pet the watchdog

      //DBG_PRINT_TIME(31);
      //check if there is a power fail skip 30ms pend if there is but only if we are active.
      if(gProcessSkipped||((AMPYPowerStateMode==eAMPYpsActive)&& AMPYGetFilteredEPF(2)))
         TotalSleepTime=0;
      else
      {
         SnapShot=SysTimeGet();
         //force the task to sleep to allow other low priority tasks to run
         Pend( TASK_GENERIC_EVENT, AMPYEMAIL_SLEEPTIME);
         TotalSleepTime+=(SnapShot-SysTimeGet());
      }
   }//end while(1)
}

/*------------------------------------------------------------------------------
* Function:    AMPYEmail_1msTickISR
* Inputs:      none
* Outputs:     none
* Description: Function called every 1ms from Kernel ISR.
* Notes:       This function must execute in the shortest possible time. So no
*              mathematical routines using division are allowed nor wait loops
*              nor large processing loops nor RTOS system calls. It would be nice
*              if only inline functions were called from here.
*-----------------------------------------------------------------------------*/
void AMPYEmail_1msTickISR(void)
{
   // early detection of the power fail.  switch off something to save the energy and 
	//	time for power down process.
#if defined(AMPY_METER_U3400)
	unsigned char i;

	for(i=0;i<5;i++)
	{
		if (PWRFAIL_STATE==1) break;
	}
	
	//if (PWRFAIL_STATE==0)
	if (i>=5)  //power fail	
	{
#ifndef AMPY_METER_R1100
		PHASELEDA_STATE = PHASELEDB_STATE = PHASELEDC_STATE =0;
#endif
		DF_refresh_on_write = 0;
		gEarlyPwrFailWarning = 1;
	}
	else
	{
		DF_refresh_on_write = 1;
		gEarlyPwrFailWarning = 0;
	}
   
   #ifdef SIM_INTERRUPT
      if (gPFDebug.PFState>3)
        DF_refresh_on_write = 0;
   #endif
#endif
	AMPYSysValTest();
#ifdef PLATFORM_HOG_DEBUG
   LogPlatformTimeTick();
#endif
	
   Comms1msTickISR();
   Button_chk();
   RTC1msTickISR();
   RelayDriverMsecISR();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   RAppLC_1msTick();
#endif
   
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(MAINS_CLOCK)
   RTC_ZC1msTickISR();
#endif
   
#if defined(PRINT_POWER_DN_DELAY)
   //check for falling edge
   if((LastPFPinState)&&(PWRFAIL_STATE==0))
      gPDTriggerTime=SysTimeGet();//It's a falling edge! log it.
   LastPFPinState=PWRFAIL_STATE;
#endif
#if defined(DBG_TEST_RELAY_CYCLE)
   if((LastPFPinState)&&(PWRFAIL_STATE==0))
      gRelayDbgCycGlitch++;
   LastPFPinState=PWRFAIL_STATE;
#endif
   //RippleMsTick();

#ifdef PWD_COMMS_PWR_FAIL_DEBUG
   if(g_dbg_comms_pwr_fail)
   {
      g_dbg_comms_pwr_fail--;
      if(g_dbg_comms_pwr_fail==0)
         MTR_PF_STATE=IO_HI;
   }
#endif
#ifdef DBG_PWR_FAIL_PIN_OUT
   if(PWRFAIL_STATE==0)
      VARHLED_STATE^=1;
#endif
}


/*-------------------------------------------------------------------------------
* Function:    AMPYEmail_UpdateTamperStatus
* Description: Update test status table with tamper switch state.
*------------------------------------------------------------------------------*/
void AMPYEmail_UpdateTamperStatus()
{
   unsigned char temp;

   //get the status of the tamper switch to help with manufacturing (meter on FTS)
   TableWrite(eMfgT64_TestStatus,0,0,NULL);//zero table
   temp=Get_Button_Status();
   temp>>=1;
   temp&=0x01;//value of 1 means that spring is in place
   TableWrite(eMfgT64_TestStatus,offsetof(_mfg_table_64_t,Button3Count),1,&temp);
}

/*-------------------------------------------------------------------------------
* Function:    MeterApplicationLowLevelInit
* Description: Init some Metrology stuff ASAP, this function is called before
*              many things are initialised.
*------------------------------------------------------------------------------*/
void MeterApplicationLowLevelInit (void)
{
   //Init MMI
   SPI_CS_MMI3_STATE=IO_HI;
   SPI_CS_MMI3_DIR=OUTPUT;//config chip select
   MMI_Clock_Init();//Initialised the Timer to provide a clock for MMI ~1MHz

    //TODO init BUTTON_DIR and RELAY1_AMR_DIR. since has been removed from boot_main()

   //Init Buttons (that is buttons and tamper switches)
   //TODO this may not have to be here if its in the bootloader
#ifndef AMPY_METER_R1100
   PUSHBUTTON1_DIR = INPUT;
   PUSHBUTTON2_DIR = INPUT;
   PUSHBUTTON3_DIR = INPUT;
   PUSHBUTTON4_DIR = INPUT;
#endif
   //Return LED to normal behaviour
   override_AMPY_LED=FALSE;


#if defined(AMPY_METER_NO_RF)
   //check if ID code has not been set
   unsigned char IDArr[9];
   int i;

   //Set ID as real value
   MCom_GetFactoryKeys(0,9,IDArr);
   for(i=0;i<9;i++)
      IDArr[i]+=IDHashArr[i];
   #if !defined(SIMULATION_SR)
   //compare desired ID to actual
   for(i=0;i<9;i++)
   {
      if(IDArr[i]!=((unsigned char *)0xfffdf)[i*4])
         break;
   }
   if(i!=9)
      ReWriteIDBytes(IDArr);
   #endif
#endif

#if defined(U1300_RIPPLE)||defined(MAINS_CLOCK)

   //init ADC to sweep ripple, load side voltage and random number gen inputs

   //turn off Open-Circuit Detection Assist Function Register (AINRST)
   adcmpcr = 0x00;

   //adcon 2 must be set first, selcect AN0-AN7 group, f1 as clock source and clk div=1
#ifndef Simulation
   adcon2 = 0x00;

   //select single sweep mode, software trigger, stop conversion and clk div=1
   adcon0 = 0x10;

   //select AN0-AN3 sweep, not sweep mode 1, enable ADC voltage supply, don't use ANEX0/1 and clk div=1
   adcon1 = 0x31;
#endif

   Ripple_InitTimerB1();
#ifndef Simulation
   adic = 0x05;
   tb1ic = 0x06;
#endif
   tb1s = 1; 
   Ripple_StartADCConversion();
#endif
}

/*
** Function:   MMI_Clock_Init
** Purpose:    Init the MMI
** Inputs:
** Outputs:    Modifies Timer A1 global hardware registers
*/
void MMI_Clock_Init(void)
{
   MMI3_CLOCK_DIR = OUTPUT; // output MMICLK
   ta1s = 0;
   TA1MR = 0x04; // timer mode, pulse output
#if defined(PLL_28M)
   TA1 = 14-1;
#else
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
   TA1 = (7*gPllMultiplier)-1; // f1/(TA1 + 1) ~= 1.05MHz.. TA1 + 1 = 14.7456MHz/2/1MHz ? -----> 2.4576 Mhz with TA1 = 3-1 or 819kHz with TA1=9-1
#else
   TA1 = 7-1; // f1/(TA1 + 1) ~= 1.05MHz.. TA1 + 1 = 14.7456MHz/2/1MHz ? -----> 2.4576 Mhz with TA1 = 3-1 or 819kHz with TA1=9-1
#endif
#endif
   // TABSR |= 0x2; // start counting
   ta1s = 1; // start counting.
}

void AMPYPowerFailTurnOffLeds()                       
{
#ifndef AMPY_METER_R1100
   PHASELEDA_STATE   =IO_LO;  //p0_4
   PHASELEDB_STATE   =IO_LO;  //p0_5
   PHASELEDC_STATE   =IO_LO;  //p0_6
   #endif	
   WHLED_STATE       =IO_LO;  //p0_7
   VARHLED_STATE     =IO_LO;  //p1_0
   PPO_PWR_STATE     =IO_LO;
   
    #ifdef AMPY_METER_R1100
   //LEDRC_STATE
   LED1_G_STATE      =IO_LO;
   LED1_R_STATE      =IO_LO;
   LED2_G_STATE      =IO_LO;
   LED2_R_STATE      =IO_LO;
   #endif
   

}
/*------------------------------------------------------------------------------
* Function:    AMPYPowerFail
* Inputs:      global value AMPYPowerStateMode
* Outputs:     global value AMPYPowerStateMode
* Description: Shutdown AMPY specific stuff in response to a mains power loss.
* Notes:       Does things like turning off the power, saving energy data, and
*              doing other nv house-keeping.
*-----------------------------------------------------------------------------*/
void AMPYPowerFail (void)
{
   unsigned short temp_w;
   
  

#if defined (PF_DEBUG)
   // UNUSED_P011_STATE=0;
    UNUSED_P014_STATE=0;
#endif   

#if defined(PRINT_POWER_DN_DELAY)
   unsigned char DoPDLog;
   unsigned long TimeD;

   if(MStatPowerCycleCheck(2))
      DoPDLog=0;
   else
      DoPDLog=21;
#endif

   gPowerDownStartFlag = 1;
   #if defined(AMPY_METER_R1100)
      gPowerDown_R1100_LEDs_Function_Disable = 1;
  #endif
   AMPYPowerFailTurnOffLeds();
#ifdef SIM_INTERRUPT   
   gPFDebug.PDProcessTmStamp = gPFDebug.tics;
   gPFDebug.LastTaskStepTmStamp[METERTASK] = gPFDebug.tics;
#endif
   if(AMPYPowerStateMode==eAMPYpsActive)
   {//powering fail after a long up-time, system is fully active so do a full backup
      //printf("A->LPM %lu\n",SysTimeGet());
#if defined(PULSE_OUTPUT)
      PulseOutput_PwrDn();
#endif
      EVENT1("PwrDn");
#if defined(NSMP_LOS)      
      P_DN_PRINT_TIME(1);
      RelayLOSPwrDn();
#endif
      P_DN_PRINT_DELAY(__LINE__);
      P_DN_PRINT_MAINLOOP();
      P_DN_PRINT_TIME(2);
      DF_refresh_on_write = FALSE;
      AMPYFlashEnablePacking();
      AMPYSuspendMetrology();
      P_DN_PRINT_TIME(3);
      TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[2]),2, &temp_w);
      temp_w++;
      TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[2]),2, &temp_w);
      TableRead(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[2]),2, &temp_w);
      temp_w++;
      TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[2]),2, &temp_w);
		
	   SysMon_NVBakup(0);
#ifdef DBG_TEST_RELAY_CYCLE
      TableWrite(eMfgT52_MfgConst, offsetof(_mfg_table_52_t,Spare[0]),4, &gRelayDbgCycCnt);
      TableWrite(eMfgT52_MfgConst, offsetof(_mfg_table_52_t,Spare[4]),2, &gRelayDbgCycGlitch);
#endif

#if defined(DBG_PWR_FAIL_LC_RLY_CHNG)
      if(!(PUSHBUTTON_STATE_PORT & PUSHBUTTON1_BIT))
      {//button is not being pressed
         RELAY2_OPEN_STATE=IO_LO;
         RELAY2_CLOSE_STATE=IO_HI;
         Pend(0,20);
         RELAY2_CLOSE_STATE=IO_LO;
      }
#endif

      P_DN_PRINT_TIME(4);
      RTC_Disable();

      P_DN_PRINT_TIME(5);
      RemotePortPowerFail(FALSE);
      P_DN_PRINT_TIME(6);
      LP_PowerDown();
      P_DN_PRINT_TIME(7);

      // stop pulsing
      Metrology_off();
      P_DN_PRINT_TIME(8);
      QOS_State_DN();
      P_DN_PRINT_TIME(9);
      TOU_PowerDown();
      P_DN_PRINT_TIME(10);
      TableBackUp();
      #ifndef AMPY_METER_R1100
      P_DN_PRINT_TIME(11);
      DisplayD_blank();
      #endif

      Metrology_PowerDown();

      P_DN_PRINT_TIME(12);
      RelayDisable();
      P_DN_PRINT_TIME(13);
      EventPowerDown();

      P_DN_PRINT_TIME(14);
      MStatPowerDown();
   
      P_DN_PRINT_TIME(15);
      CommsDisable();
#ifndef AMPY_METER_R1100
      PrePaymentPowerDown();
#endif
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
      Tampers_Powerdown();
#endif
       
      P_DN_PRINT_TIME(16);
      gProcessSkipped=0;
    
      DF_backup_incremental_refresh();       
      //power fail check value (decrement critical blcok end count)
      TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[3]),2, &temp_w);
      temp_w--;
      TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[3]),2, &temp_w);
      // save mfg table 54 to non volatile
#ifdef SIM_INTERRUPT
      unsigned short dwTemp = gPFDebug.tics - gPFDebug.PDProcessTmStamp;
      if (dwTemp>gSimInterrupt.PDProcessMaxDelay)
      {
         gSimInterrupt.PDProcessMaxDelay = dwTemp;
         gPFDebug.SimInfoUpdated = 1;
      }
      if (gPFDebug.SimInfoUpdated)
         TableWrite(eMfgT54_GnrlMeterNVStatus,offsetof(_mfg_table_54_t,pfSimulator),sizeof(_sim_interrupt_t),&gSimInterrupt);
#endif      
      TableWrite(eMfgT72_GnrlMeterNVStatusBckp, 0,0, &mfg_table_54_struct);

      AMPYFlashDisablePacking();
      gPowerDownStartFlag = 0;
      #if defined(AMPY_METER_R1100)
        gPowerDown_R1100_LEDs_Function_Disable = 0;
      #endif
	
      P_DN_PRINT_TIME(17);
      EVENT1("PwrDn done");
      //printf("LPM start %lu\n",SysTimeGet());
#if defined(DBG_POWERFAILTEST)
#if 0
      while(1)
      {
         printf("%u\n",gcount);
         Pend(0,50);
      }
#endif
#endif
      AMPYPowerStateMode=eAMPYpsLPM;
#ifdef SIM_INTERRUPT
      disable();
	   while (TRUE);
#endif		
#if defined(PWD_DOWN_RLY_SWITCH_DEBUG)
      if(0x01&gRlyDbgMode)
         temp_w=cClose;
      else
         temp_w=cOpen;

      if(gRlyDbgMode==0)//0
      {
         RELAY1_CLOSE_STATE=IO_LO;
         RELAY1_OPEN_STATE=IO_HI;
         RELAY2_CLOSE_STATE=IO_LO;
         RELAY2_OPEN_STATE=IO_HI;
         RELAY3_CLOSE_STATE=IO_LO;
         RELAY3_OPEN_STATE=IO_HI;
         Pend(0,100);
         RELAY1_OPEN_STATE=IO_LO;
         RELAY2_OPEN_STATE=IO_LO;
         RELAY3_OPEN_STATE=IO_LO;
      }
      else if(((gRlyDbgMode+1)>>1)==1)//1,2
      {
         RelayChange(DisRelay,temp_w);
         RelayChange(ConRelay,temp_w);
         RelayChange(OthRelay,temp_w);
      }
      else if(((gRlyDbgMode+1)>>1)==2)//3,4
      {
         RelayChange(DisRelay,temp_w);
         RelayChange(OthRelay,temp_w);
         RelayChange(ConRelay,temp_w);
      }
      else if(((gRlyDbgMode+1)>>1)==3)//5,6
      {
         RelayChange(ConRelay,temp_w);
         RelayChange(DisRelay,temp_w);
         RelayChange(OthRelay,temp_w);
      }
      else if(((gRlyDbgMode+1)>>1)==4)//7,8
      {
         RelayChange(ConRelay,temp_w);
         RelayChange(OthRelay,temp_w);
         RelayChange(DisRelay,temp_w);
      }
      else if(((gRlyDbgMode+1)>>1)==5)//9,10
      {
         RelayChange(OthRelay,temp_w);
         RelayChange(DisRelay,temp_w);
         RelayChange(ConRelay,temp_w);
      }
      else if(((gRlyDbgMode+1)>>1)==6)//11,12
      {
         RelayChange(OthRelay,temp_w);
         RelayChange(ConRelay,temp_w);
         RelayChange(DisRelay,temp_w);
      }
#endif
   }
 #if defined (PF_DEBUG)
  //  UNUSED_P011_STATE=1;
   UNUSED_P014_STATE=1;
#endif

}

/*------------------------------------------------------------------------------
* Function:    AMPYPowerUp
* Inputs:      global value AMPYPowerStateMode
* Outputs:     global value AMPYPowerStateMode
* Description: Turn on AMPY specific stuff in response to mains power presence.
* Notes:       Does things like turning on hardware, restoring RAM, updating RTC
*              Never ever return from this function if EPFRestored_M is not set
*              and AMPYPowerStateMode==eAMPYpsNoPower!
*-----------------------------------------------------------------------------*/
void AMPYPowerUp (void)
{
   unsigned short temp_w;
   unsigned char temp_c;
   unsigned long DownTime;
   _std_table_52_t LocalRTC;
   unsigned long temp_l;
 #if defined(AMPY_METER_R1100)
        gPowerDown_R1100_LEDs_Function_Disable = 0;
 #endif	
   gPowerDownStartFlag = 0;
   g_relay_pf_override = 0;

#if defined(BREAKER_LSVD)
   RelayDischargeLSVD();
#endif
	
#ifdef SIM_INTERRUPT
	gPFDebug.PFState = 0;
#endif
   if(AMPYPowerStateMode==eAMPYpsLPM)
   {//powering up after a short outage RAM is still valid and full init is not required
      //printf("LPM->A %lu\n",SysTimeGet());
      if(/*(EPF_STATE)*/AMPYGetFilteredEPF(3))
      {
         //printf("LPM->A abort %lu\n",SysTimeGet());
         return;//power not present don't even think about trying to power up!
      }
      EVENT1("PwrUp LPM");
      if(AMPYFlashUnpack()!=0)  //gsk:if we have aborted flash unpacking,we can't trust the data,so abort powerup as well
         return;
      
      P_DN_LOG_TIME(0);

      P_DN_LOG_TIME_UP(2);
      MStatPrePowerUp();
      // restore NV backup of mfg table 54 from mfg table 72
      TableRead(eMfgT72_GnrlMeterNVStatusBckp, 0,0, &mfg_table_54_struct);
      
#ifdef SIM_INTERRUPT
      TableRead(eMfgT54_GnrlMeterNVStatus,offsetof(_mfg_table_54_t,pfSimulator),sizeof(_sim_interrupt_t),&gSimInterrupt);
#endif
      
      if(RTC_Init(0,&DownTime))//0-because we want normal mode on the RTC chip
      {//shut down! undo what we just did
         //printf("RTC fail %lu\n",SysTimeGet());
         EVENT1("PwrUp exit1");
         P_DN_LOG_TIME(1);
         //printf("LPM->A abort %lu\n",SysTimeGet());
         return;
      }
      TableRead(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      temp_w++;
      TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      temp_w++;
      TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      P_DN_LOG_TIME_UP(3);

      TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,RecentOutageIndex), sizeof(temp_c), &temp_c);//get index to last outage time
      if(temp_c<10)
      {//make sure that RecentOutageIndex is valid
         RTC_GetFunctionalTime(&LocalRTC,0,sizeof(_std_table_52_t),eRTCFT_GMT);
      }
      SysMon_NVBakup(1);
      if(AMPYPowerUpCommon(DownTime))
      {//commit to writing this important stuff
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,OutagesEnd)+(sizeof(_std_table_52_t)*temp_c), sizeof(_std_table_52_t), &LocalRTC);//set last outage time

         TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,BatteryDuration), sizeof(temp_l), &temp_l);
         temp_l+=DownTime;//TODO: IJD- do we care that the super cap lasts for 5min so BatteryDuration calc should be adjusted
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,BatteryDuration), sizeof(temp_l), &temp_l);
         MStatCheckBattery();
         if(!gFirmwareUpdated)
            EventLogWrite(eEvtPwrUp,0,0,NULL);

         TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         temp_w--;
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         TableRead(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         temp_w--;
         TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         DF_refresh_on_write = TRUE;
         AMPYPowerStateMode=eAMPYpsActive;//we are now officially up and running
         //printf("LPM->A end %lu\n",SysTimeGet());
      }
      else
      {//pretend the power up never happend and forget about the few seconds of energy we saw.
         EVENT1("PwrUp exit2");
         temp_w--;
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
         TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
         //printf("LPM->A abort2 %lu\n",SysTimeGet());
      }
   }
   else if(AMPYPowerStateMode==eAMPYpsNoPower)
   {//powering up after a long outage RAM is reset, full init is required
      //printf("NP->A %lu\n",SysTimeGet());
      EVENT1("PwrUp NoP");
      while(/*(EPF_STATE)*/AMPYGetFilteredEPF(4))
         Pend(0,10);//power not present and since this is the first time we can just sit here waiting

      if(AMPYFlashUnpack()!=0)  //gsk:if we have aborted flash unpacking,we can't  trust the data,so abort powerup as well
         return;
      
#ifdef SIM_INTERRUPT
      TableRead(eMfgT72_GnrlMeterNVStatusBckp,offsetof(_mfg_table_54_t,pfSimulator),sizeof(_sim_interrupt_t),&gSimInterrupt);
#endif  
      
      P_DN_LOG_TIME(0);
      MStatPrePowerUp();
      AMPYResumeMetrology();//AMPYCheckDoPowerUpUndo() may have been called so make sure metrology is running
      I2C_Init();//initialise the I2C BUS ready for the RTC chip and LCD
      DisplayInit();
      Debug(LogDebug,"Meter Task","AMPYEmail_Init()");
      if(AMPYCheckDoPowerUpUndo(FALSE))
      {
         EVENT1("PwrUp exit2");
         P_DN_LOG_TIME(1);
         return;
      }
      TableInit();//this should be called before any call to TableWrite
      Metrology_ChkAndFixFaultyPulseDiv();
      Metrology_InitPolyPhaseMeterFlag();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
      if(gU1300Ver2)
      {
        //Initialize the SPI bus 3 for U1300 series 2 hardware
        Metrology_SPI_Init();
        //Used for debugging the SPI bus 3 on which the MMI(s) are connected for U1300 series 2
        //Metrology_SPI_ReadVersion();
      }
#endif
      Post(METROLOGY_TASK, AMPY_INIT_DONE);//tell metrology it can now use the tables

      temp_c=1;
      while(EPFState_M)//wait for EPF state machine to initialise
      {
         if(temp_c)
         {
            if(AMPYCheckDoPowerUpUndo(FALSE))
               temp_c=0;
         }
         Pend(0,50);
      }
      if(temp_c==0)
      {
         EVENT1("PwrUp exit2");
         P_DN_LOG_TIME(1);
         //printf("NP->A abort %lu\n",SysTimeGet());
         return;//init was cancelled so exit, power up will be taken care of in main loop
      }

      if(AMPYCheckDoPowerUpUndo(FALSE))
      {
         EVENT1("PwrUp exit3");
         P_DN_LOG_TIME(1);
         //printf("NP->A abort %lu\n",SysTimeGet());
         return;
      }

      P_DN_LOG_TIME_UP(2);
      // restore NV backup of mfg table 54 from mfg table 72
      TableRead(eMfgT72_GnrlMeterNVStatusBckp, 0,0, &mfg_table_54_struct);

      if(RTC_Init(0,&DownTime))//0-because we want normal mode on the RTC chip
      {//shut down! undo what we just did
         //printf("RTC fail %lu\n",SysTimeGet());
         EVENT1("PwrUp exit4");
         AMPYCheckDoPowerUpUndo(TRUE);
         P_DN_LOG_TIME(1);
         //printf("NP->A abort %lu\n",SysTimeGet());
         return;
      }
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
      TamperInit();           //Init Tampers
#endif
      SysMon_NVBakup(1);
      TableRead(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      temp_w++;
      TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      temp_w++;
      TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
      P_DN_LOG_TIME_UP(3);

      TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,RecentOutageIndex), sizeof(temp_c), &temp_c);//get index to last outage time
      if(temp_c<10)
      {//make sure that RecentOutageIndex is valid
         RTC_GetFunctionalTime(&LocalRTC,0,sizeof(_std_table_52_t),eRTCFT_GMT);
      }

      if(AMPYPowerUpCommon(DownTime))
      {//commit to writing this important stuff
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,OutagesEnd)+(sizeof(_std_table_52_t)*temp_c), sizeof(_std_table_52_t), &LocalRTC);//set last outage time
         TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,BatteryDuration), sizeof(temp_l), &temp_l);
         temp_l+=DownTime;//TODO: IJD- do we care that the super cap lasts for 5min so BatteryDuration calc should be adjusted
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,BatteryDuration), sizeof(temp_l), &temp_l);
         MStatCheckBattery();

       if(!gFirmwareUpdated)
          EventLogWrite(eEvtPwrUp,0,0,NULL);

         TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         temp_w--;
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         TableRead(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         temp_w--;
         TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[1]),2, &temp_w);
         DF_refresh_on_write = TRUE;
         AMPYPowerStateMode=eAMPYpsActive;//we are now officially up and running
         //printf("NP->A end %lu\n",SysTimeGet());
      }
      else
      {//pretend the power up never happend and forget about the few seconds of energy we saw.
         EVENT1("PwrUp exit5");
         temp_w--;
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
         TableWrite(eMfgT72_GnrlMeterNVStatusBckp, offsetof(_mfg_table_54_t,PFCycleCounts[0]),2, &temp_w);
         //printf("NP->A abort2 %lu\n",SysTimeGet());
      }
   }

   if(AMPYPowerStateMode==eAMPYpsActive)
   {
      #if defined(PWD_DOWN_RLY_SWITCH_DEBUG)
      RelayChange(DisRelay,cClose);
      RelayChange(ConRelay,cClose);
      RelayChange(OthRelay,cClose);
      #endif
      gProcessSkipped=0;
		
      AMPYCrashLogInit();
      EVENT1("PwrUp done");
      if(/*(EPF_STATE)*/AMPYGetFilteredEPF(5))
      {
         EventLogWrite(eEvtPwrDn,0,0,NULL);
         AMPYPowerFail();
      }
   }
}

/*------------------------------------------------------------------------------
* Function:    AMPYPowerUpCommon
* Inputs:      global value AMPYPowerStateMode
*              DownTime-duration of outage in ms
* Outputs:     Returns zero if power up is aborted due to power loss
* Description: Poerform power up tasks comm to eAMPYpsNoPower & eAMPYpsLPM.
*-----------------------------------------------------------------------------*/
unsigned char AMPYPowerUpCommon (unsigned long DownTime)
{
   Metrology_on();
   EventPowerUp(DownTime);
   MStatPowerUp();
   CommsReset(FALSE,TOTAL_NUMBER_OF_PORTS);           //gsk: Reset All Ports on powerup
   if(AMPYGetFilteredEPF(6))return 0;
#ifdef DBG_TEST_RELAY_CYCLE
      TableRead(eMfgT52_MfgConst, offsetof(_mfg_table_52_t,Spare[0]),4, &gRelayDbgCycCnt);
      TableRead(eMfgT52_MfgConst, offsetof(_mfg_table_52_t,Spare[4]),2, &gRelayDbgCycGlitch);
#endif
      FirmwareStatusUpdate();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
      RippleSystemInit();
#endif
   if(AMPYPowerStateMode==eAMPYpsLPM)
      LP_PowerUp(DownTime);
   else
      LP_Init(DownTime);
   if(AMPYGetFilteredEPF(7))return 0;
   if(CommsInit()) return 0;
   QOS_State_UP();
   if(AMPYGetFilteredEPF(8))return 0;
   RelayInit(DownTime);
   if(AMPYGetFilteredEPF(9))return 0;
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100) || defined(LC_RSM)
   /* Do not initialize the RSM until NSMP_LOS finish the job */
   if(RAppLC_Init(DownTime)) return 0;
   RippleApp_Init(DownTime);
#endif
   if(AMPYGetFilteredEPF(10))return 0;

   AMPYEmail_UpdateTamperStatus();
   BackEndComms_Init();
   if(AMPYGetFilteredEPF(11))return 0;

   if(AMPYPowerStateMode==eAMPYpsLPM)
      AMPYResumeMetrology();

   InitFwDl();
   if(AMPYGetFilteredEPF(12))return 0;
   if(TOU_Init(DownTime))return 0;
   //if(AMPYGetFilteredEPF())return 0;
   //TOU_SelfRead(TRUE);
   if(AMPYGetFilteredEPF(13))return 0;
   Metrology_PowerUp();
#if defined(PULSE_OUTPUT)
   PulseOutput_MeterPwrupInit();
#endif
   SysMon_MeterPwrupTempInit(DownTime);
#if defined(LP_QOS)   
   LpQos_Init();
#endif
   if(AMPYGetFilteredEPF(14))return 0;
   //FirmwareStatusUpdate();
   P_DN_LOG_TIME(1);
   DF_init_incremental_refresh();
   if(AMPYGetFilteredEPF(15))return 0;

#if defined(DBG_PWR_FAIL_LC_RLY_CHNG)
   if(!(PUSHBUTTON_STATE_PORT & PUSHBUTTON1_BIT))
   {//button is not being pressed
      RELAY2_CLOSE_STATE=IO_LO;
      RELAY2_OPEN_STATE=IO_HI;
      Pend(0,20);
      RELAY2_OPEN_STATE=IO_LO;
   }
#endif

#ifdef SEMAPHORE_RAMACCESS_DEBUG
   gDisableSemDebug=0;   //Enable Debug Flag by default
#endif

   return 1;
}

/*------------------------------------------------------------------------------
* Function:    AMPYCheckDoPowerUpUndo
* Inputs:      EPF_STATE
*              ForceUndo-if non-zero ignore EPF_STATE and perform an undo
* Outputs:     global table structures and hardware states
*              Returns non-zero if EPF_STATE indicates power loss or ForceUndo
*              is non-zero.
* Description: Undo partial power up initialisation if a power loss is detected
*              via EPF_STATE.
*-----------------------------------------------------------------------------*/
unsigned char AMPYCheckDoPowerUpUndo(unsigned char ForceUndo)
{
   if((/*(EPF_STATE)*/AMPYGetFilteredEPF(15))||(ForceUndo))
   {//shut down! undo what we just did
      if(AMPYPowerStateMode==eAMPYpsNoPower)
         AMPYSuspendMetrology();
      //turn off I2C?
      //save energy?
      //TableBackUp();
      DisplayD_blank();
      return 1;
   }
   return 0;
}

/*------------------------------------------------------------------------------
* Function:    AMPYGetFilteredEPF
* Outputs:     Returns non-zero if a power fail is detected via EPF_STATE.
* Description: Samples the EPF_STATE multiple times in order to filter out noise
*-----------------------------------------------------------------------------*/
#define NUM_MS_PF_TESTS 5
unsigned char AMPYGetFilteredEPF(unsigned char nPFFlag)
{
   unsigned char i,j,count;
	
#ifdef SIM_INTERRUPT
	unsigned short pfDelay = 0;
	
	if (gPFDebug.PFState==0xFF)
		return 0;
	
	disable();

	if (nPFFlag==1)//its going to call the pwr process, change the PF state
	{			
      if (gPFDebug.PFState == 3)
      {
         pfDelay = gPFDebug.tics - gPFDebug.PFDetectTmStamp;
         //printf("state=%u,d=%u,tics=%u,%u,max=%u\n",(unsigned short)gPFDebug.PFState,pfDelay,gPFDebug.tics,gPFDebug.PFDetectTmStamp,gSimInterrupt.PDStartMaxDelay);
         if (pfDelay>gSimInterrupt.PDStartMaxDelay)
         {
            gSimInterrupt.PDStartMaxDelay = pfDelay;
               
            for(i=0;i<MAXTASKNUM;i++)
            {
               gSimInterrupt.PDStartTaskStepId[i] = gPFDebug.CurrentTaskStepId[i];
               gSimInterrupt.PDStartTaskStepList[i] = gPFDebug.PDStartTaskStepList[i];
               gSimInterrupt.PDStartTaskMaxDelayStepId[i] = gPFDebug.PDStartTaskMaxDelayStepId[i];
               gSimInterrupt.PDStartTaskMaxDelayStepTm[i] = gPFDebug.PDStartTaskMaxDelayStepTm[i];
            }
            gPFDebug.SimInfoUpdated = 1;
         }
         for (i=0;i<MAXTASKNUM;i++)
         {
               gPFDebug.LastTaskStepTmStamp[i] = gPFDebug.tics;
         }
         gPFDebug.PFState = 4;
         gPFDebug.PDProcessTmStamp = gPFDebug.tics;
      }
      else if (gPFDebug.PFState == 2)
      {
         pfDelay = gPFDebug.tics - gPFDebug.PFStartTmStamp;
         if (pfDelay>gSimInterrupt.PDStartMaxDelay)
			{
            gSimInterrupt.PDStartMaxDelay = pfDelay;
            for (i=0;i<MAXTASKNUM;i++)
            {
               gSimInterrupt.PDStartTaskStepId[i] = gPFDebug.CurrentTaskStepId[i];
               gSimInterrupt.PDStartTaskStepList[i] = gPFDebug.PFDetectTaskStepList[i];
               gSimInterrupt.PDStartTaskMaxDelayStepId[i] = gPFDebug.PFDetectTaskMaxDelayStepId[i];
               gSimInterrupt.PDStartTaskMaxDelayStepTm[i] = gPFDebug.PFDetectTaskMaxDelayStepTm[i];
               //gSimInterrupt.PFDetectTaskLastStepTm[i] = gPFDebug.tics - gPFDebug.LastTaskStepTmStamp[i];
            }
            gPFDebug.SimInfoUpdated = 1;
			}
         for (i=0;i<MAXTASKNUM;i++)
         {
               gPFDebug.LastTaskStepTmStamp[i] = gPFDebug.tics;
         }
         
			gPFDebug.PFState = 4;
         gPFDebug.PDProcessTmStamp = gPFDebug.tics;
      }
	}
	else
	{
		if (gPFDebug.PFState==2)
		{
			pfDelay = gPFDebug.tics - gPFDebug.PFStartTmStamp;
			//printf("2:d=%u,tics=%u,%u,max=%u\n",pfDelay,gPFDebug.tics,gSimInterrupt.pfStartTime,gSimInterrupt.pfMaxDelay1);
			if (pfDelay>gSimInterrupt.PFDetectMaxDelay)
			{
            gSimInterrupt.PFDetectMaxDelay = pfDelay;
            for (i=0;i<MAXTASKNUM;i++)
            {
               gSimInterrupt.PFDetectEPFLocation = nPFFlag;
               gSimInterrupt.PFStartTaskStepId[i] = gPFDebug.PFStartTaskStepId[i];
               gSimInterrupt.PFDetectTaskStepId[i] = gPFDebug.CurrentTaskStepId[i];
               gSimInterrupt.PFDetectTaskStepList[i] = gPFDebug.PFDetectTaskStepList[i];
               gSimInterrupt.PFDetectTaskMaxDelayStepId[i] = gPFDebug.PFDetectTaskMaxDelayStepId[i];
               gSimInterrupt.PFDetectTaskMaxDelayStepTm[i] = gPFDebug.PFDetectTaskMaxDelayStepTm[i];
               //gSimInterrupt.PFDetectTaskLastStepTm[i] = gPFDebug.tics - gPFDebug.LastTaskStepTmStamp[i];
            }
            gPFDebug.SimInfoUpdated = 1;
			}
         for (i=0;i<MAXTASKNUM;i++)
         {
            gPFDebug.PDStartTaskStepId[i] = gPFDebug.CurrentTaskStepId[i]; 
            gPFDebug.LastTaskStepTmStamp[i] = gPFDebug.tics;
         }
         
			gPFDebug.PFState = 3;
         gPFDebug.PFDetectTmStamp = gPFDebug.tics;
		}	
	}
	enable();
	
	if (gPFDebug.PFState>1)
		return 1;
#endif
	
	
   j=0;
   while(j<=NUM_MS_PF_TESTS)
   {
      count=0;
      for(i=0;i<200;i++)
      {
         if(EPF_STATE)
     
            count++;
            
  
      }
      if(count<190)
      {
			return 0;
                        
      }
      if(j++!=NUM_MS_PF_TESTS)

      Pend(0,1);

   }
  
   return 1;
}

/*------------------------------------------------------------------------------
* Function:    AMPYSuspendMetrology
* Outputs:
* Description:
*-----------------------------------------------------------------------------*/
void AMPYSuspendMetrology()
{
   SEMAPHORE mask;
   unsigned char i;

   LockSemaphore (&flash_lock);
   Suspend(METROLOGY_TASK);
   i = disable_save();
   gAMPY_Metrology_Flg=0;
   mask=SEMAPHORE_ONE<<METROLOGY_TASK;
   if(metrology_data_lock & mask)
      gAMPY_Metrology_Flg|=AMPY_MET_DATA_LK;
   if(tableRAMAccess & mask)
      gAMPY_Metrology_Flg|=AMPY_TBL_RAM_ACS;
   if(Table28EngyLock & mask)
      gAMPY_Metrology_Flg|=AMPY_T28_EGY_LK;
   if(uugflash_lock & mask)
      gAMPY_Metrology_Flg|=AMPY_UUGFLASH_LK;

   metrology_data_lock&=~mask;
   tableRAMAccess&=~mask;
   Table28EngyLock&=~mask;
   uugflash_lock&=~mask;
   enable_restore(i);
   UnlockSemaphore (&flash_lock);
}

/*------------------------------------------------------------------------------
* Function:    AMPYResumeMetrology
* Outputs:
* Description:
*-----------------------------------------------------------------------------*/
void AMPYResumeMetrology()
{
   SEMAPHORE mask;
   unsigned char i;

   i = disable_save();
   mask=SEMAPHORE_ONE<<METROLOGY_TASK;
   if(gAMPY_Metrology_Flg & AMPY_MET_DATA_LK)
      metrology_data_lock|=mask;
   if(gAMPY_Metrology_Flg & AMPY_TBL_RAM_ACS)
      tableRAMAccess|=mask;
   if(gAMPY_Metrology_Flg & AMPY_T28_EGY_LK)
      Table28EngyLock|=mask;
   if(gAMPY_Metrology_Flg & AMPY_UUGFLASH_LK)
      uugflash_lock|=mask;
   enable_restore(i);

   Resume(METROLOGY_TASK);
}

/*------------------------------------------------------------------------------
* Function:    AMPYPowerCycleCountReset
* Outputs:     global value AMPYPowerStateMode
* Description: Reset power fail counters and clear past outage log array.
*-----------------------------------------------------------------------------*/
void AMPYPowerCycleCountReset()
{
   unsigned char i;
   unsigned short temp_w;
   _std_table_52_t InitTime;

   memset(&InitTime,0x00,sizeof(InitTime));
   i=0;
   TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,RecentOutageIndex),1, &i);
   for(i=0;i<10;i++)
   {
      if(i&0x01)
         temp_w=0xFFFF;
      else
         temp_w=0x0000;
      if(i<4)
         TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,PFCycleCounts[0])+sizeof(unsigned short)*i,2, &temp_w);
      TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,OutagesStart[0])+sizeof(_std_table_52_t)*i,sizeof(_std_table_52_t), &InitTime);
      TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,OutagesEnd[0])+sizeof(_std_table_52_t)*i,sizeof(_std_table_52_t), &InitTime);
   }
}

/*------------------------------------------------------------------------------
* Function:    MeterAppSysResetHook
* Description: React to a call to sys_reset() trigger a call to AMPYPowerFail().
* Notes:       This is a tricky function since it may be called from any task
*              including meter tasks. It may also be called at any time even
*              before tasks are fully initialised. Some tasks may have little
*              remaining stack space so a direct call to AMPYPowerFail() may be
*              unwise.
*-----------------------------------------------------------------------------*/
void MeterAppSysResetHook()
{
   unsigned int temp;
   unsigned int mask,i;

   if(AMPYPowerStateMode!=eAMPYpsActive)//backup only required if code in MeterTask() is fully up and running.
      return;

   temp = TaskId();
   if(temp==METER_TASK)
   {
      AMPYCleanForPowerFail();
      LockTask();
      // If Task0() is not looping through then watchdog will not be serviced,
      // so do it here just in case.
      farWatchdog();
      AMPYPowerFail();//if it's myself then its safe to call AMPYPowerFail()
      UnlockTask();
   }
   else
   {//let MeterTask() call AMPYPowerFail()
      for (i = 0; i < MAX_TASK_NUM; i++)
      {
         //Don't suspend ourselves, idle task, or our backup task (Meter_task)
         if ((i != TaskId()) && (i != IDLE_TASK)&&(i != METER_TASK))
            Suspend (i);
      }

      //unlock all semaphores that METROLOGY_TASK and METER_TASK commonly uses to communicate with other tasks
      i = disable_save();
      mask=SEMAPHORE_ONE<<METER_TASK;
      temp=0;
      if((adc_lock&mask)!=adc_lock)
         temp=1;
      if((encryptLock&mask)!=encryptLock)
         temp=1;
      if((flash_lock&mask)!=flash_lock)
         temp=1;
      if((Table28EngyLock&mask)!=Table28EngyLock)
         temp=1;
      if((metrology_data_lock&mask)!=metrology_data_lock)
         temp=1;
      if((tableRAMAccess&mask)!=tableRAMAccess)
         temp=1;
      if((uugflash_lock&mask)!=uugflash_lock)
      {
         temp=1;

      //If Meter Task has not grabbed the semaphore or if its pending on it, its safe to
      //assume that something else has the SPI bus, in which case we need to end all transactions by driving CS high
         if(((uugflash_lock&mask)==0) || (Tcb[METER_TASK].Pending&SEMAPHORE_READY))
            DataFlashInit();
      }

      //clear all other task bits in semaphores
      adc_lock&=mask;
      encryptLock&=mask;
      flash_lock&=mask;
      Table28EngyLock&=mask;
      metrology_data_lock&=mask;
      tableRAMAccess&=mask;
      uugflash_lock&=mask;

      if((temp)&&(Tcb[METER_TASK].Pending&SEMAPHORE_READY))
      {
         Post(METER_TASK, SEMAPHORE_READY);
      }

      //Check if Meter Task is still pending on any other event and post it
      temp= Tcb[METER_TASK].Pending;
      if(temp)
      {
         Post(METER_TASK, temp);
      }
      enable_restore(i);

      //As Our Backup Process(AMPYPowerFail) Suspends the METROLOGY_TASK,We need to signal
      //the Meter Task to resume METROLOGY_TASK if thats the task that crashed so that
      //it can then proceed to stackdump and invoke reset.
      if(TaskId() == METROLOGY_TASK)
         system_die_flag = 1;
      else
         system_die_flag = 0;

      //signal that call to AMPYPowerFail() is required
      AMPYForceReset=1;
      MeterApplicationWaitForShutdown();

      //If back up process has failed and we have timed
      //out,we should clear all semaphores that could prevent us from writing to crash log
      //TBD: Should we suspend METER_TASK now that backup has completed/timed out?
      AMPYCleanForPowerFail();

   }
}

/*------------------------------------------------------------------------------
* Function:    AMPYCleanForPowerFail
* Outputs:     Clear some global semaphores.
* Description: Prepare for a call to PowerFail() during a system crash. Ensure
*              that semaphores are clear and SPI BUS is ready for operation.
*-----------------------------------------------------------------------------*/
void AMPYCleanForPowerFail(void)
{
   unsigned char i;

   i = disable_save();
   if(uugflash_lock)
   {//something has hold of the SPI BUS so end all transactions by sending CS high
      DataFlashInit();
   }
   //clear all three semaphores that are known to be used by PowerFail().
   Table28EngyLock=0;
   tableRAMAccess=0;
   uugflash_lock=0;
   //clear one other semaphore not used by PowerFail() but used in mainloop.
   metrology_data_lock=0;
   enable_restore(i);
}

/*------------------------------------------------------------------------------
* Function:    AMPYUpdateTbl3StatusFlags
* Outputs:     global values in table 3 are modified.
* Description: Conducts a system check and updates status flags in standard
*              table 3 accordingly.
*-----------------------------------------------------------------------------*/
void AMPYUpdateTbl3StatusFlags()
{
   GetBadIRQStatus();
}

/*------------------------------------------------------------------------------
* Function:    MeterApplicationWaitForShutdown
* Outputs:     Posts a message to METER_TASK
* Description: Posts a message to(wakes up) METER_TASK and waits for meter task
*              to shut down. This function assumes that some sort of signal has
*              already been sent to METER_TASK telling it to shut down.
*-----------------------------------------------------------------------------*/
void MeterApplicationWaitForShutdown( void )
{
   unsigned short i;
   Post(METER_TASK, TASK_GENERIC_EVENT);
   //wait for MeterTask() to end call to AMPYPowerFail()
   i=0;
   while(AMPYPowerStateMode==eAMPYpsActive)
   {
      if(i>1000)//we have been waiting too long(5sec), give up...
         break;
      Pend(0, 5);
      // If Task0() is not looping through then watchdog will not be serviced,
      // so do it here just in case.
      farWatchdog();
      i++;
   }
}

/*
** Function:   TeridianCheckUART
** Purpose:
** Inputs:
** Outputs:
** Notes:
** Since we don't want the ISR to do any more work than it has too, have a high priority task check to see when the transmit is done
** and to clear (free) the transmit buffer. Typically this will get called once every 8 milliseconds, although it is possible for it
** to get called quicker. So to be safe, this routine will wait for the interrupt routine to finish sending the message. When it
** recognizes that state, it will set a flag so that the next time it checks it will actually clear the transmit. This should
** gaurantee a space between each message sent.
**
** This same routine will also parse incoming messages and put the message in a platform to be fetched later by a calling routine.
*/
void TeridianCheckUART (void)
{
}

/*
** Function:   TeridianInitUART
** Purpose:
** Inputs:
** Outputs:
** Notes:
** This routine will setup the Teridian UART. The rest of the Teridian initialization will be left to the
** Teridian task level functions. This should involve the following:
**    1). Getting status from the Teridian (send a status request, get a response).
**    2). Send Calibration values and check for good status
**    3). Enable meterology messages to start flowing.
*/
void TeridianInitUART (void)
{
}


/*
** Function:   UpdateMetrologyRTC
** Purpose:
** Inputs:
** Outputs:
** Notes:
** Called from mclock.c to update the metrology real time clock, usually once
** per second. this function also indicates if the metrology task has to unbuild
** time before use, in scheduler(), by setting prev_rtc_sec to an invalid value
*/
void UpdateMetrologyRTC( void )
{
  /* if (force_unbuild_flag)
   {
      prev_rtc_sec = 0xFF;       // force time unbuild in scheduler() along with a
                                 // history log event indicating a big time jump
   }
   else
   {
      if (++RTC_SEC == 60)
      {
         RTC_SEC = 0;
         if (++RTC_MIN == 60)
         {
            prev_rtc_sec = 60;   // force time unbuild in scheduler() every hour
         }
      }
   }*/
}


/*****************************************************************************/

/*
** Function:   HistoryLogEvent
** Purpose:    Used to record an event
** Inputs:     userid, eventid, param
** Outputs:    returns nothing, guess it writes event data to eeprom/flash
** Notes:      Copied from LGMETER\LG\Decade7.c+h
*/
/*******************************************************************************
 * userid:
 * 0 (METER_INIT) if meter inits the event
 * 1 (MANUAL_INIT) for manual init
 * for comm inits userid is the communication session user id.
 ******************************************************************************/
void HistoryLogEvent(unsigned short userid, unsigned short eventid, unsigned short param)
{
}

unsigned char  DF_refresh_on_write=FALSE; // If true, do dataflash refreshing on each write to DF.
unsigned short DF_page_refresh;           // next page to refresh

//------------------------------------------------------------------------------------------
//
//  flashReadMeter reads data from dataflash to a RAM buffer
//
//  PRECONDITIONS:
//    prior call to SPI_init_port_pins()
//    prior call to DF_init_incremental_refresh()
//    src is a dataflash-style pointer
//    dest is a pointer
//    len is the number of bytes to copy
//
//  POSTCONDITIONS:
//    returns number of bytes read
//
//   Note: this function is not used by boot loader and has no assembly language counterpart
//
unsigned short flashReadMeter (unsigned char *dest, unsigned long src, unsigned short len)
{
   unsigned short page;
   unsigned short addr;
   unsigned short len_in_page, readlen=len;

   DF_map_page_and_address(src, &page, &addr);

   //printf("\nmemcpy_rd() Src 0x%lx, Len %u. Page %u, Addr %u", src, len, page, addr);

   while (len)
   {
      len_in_page = (len > (DF_DATA_SIZE - addr))
                         ? (DF_DATA_SIZE - addr)
                         : len;
      len = len - len_in_page;

      DataFlashReadPageBytes( page, addr, dest, len_in_page );
      dest += len_in_page;

      // prepare dataflash to access addr 0 of next page
      page = page + 1;
      addr = 0;
   }
   return readlen;
}

//--------------------------------------------------------------
//
//  flashWriteMeter copies data to dataflash from any other type of memory
//
//  PRECONDITIONS:
//    prior call to SPI_init_port_pins()
//    prior call to DF_init_incremental_refresh()
//    dest is a dataflash-style pointer
//    src  is a pointer
//    len is the number of bytes to copy
//
//  POSTCONDITIONS:
//    returns number of bytes written
//
//   Note: this function is not used by boot loader and has no assembly language counterpart
//
unsigned short flashWriteMeter(unsigned long dest, unsigned char *src, unsigned short len)
{
   unsigned short page;
   unsigned short addr;
   unsigned short len_in_page, writelen=len;
   unsigned char temp =0;

   DF_map_page_and_address(dest, &page, &addr);

   //printf("\nmemcpy_dr() Dst 0x%lx, Len %u. Page %u, Addr %u", dest, len, page, addr);

   while (len)
   {
      len_in_page = (len > (DF_DATA_SIZE - addr))
                         ? (DF_DATA_SIZE - addr)
                         : len;
      len = len - len_in_page;

      temp = AMPYDFWritePageBytes( page, addr, src, len_in_page );
      src += len_in_page;

      if (DF_refresh_on_write)
      {
         DF_incremental_refresh();
      }

      // prepare dataflash to access addr 0 of next page
      page = page + 1;
      addr = 0;

      if((gDFSelfReadWriteCheck)&&(!temp))         //If Check Flag Enabled and driver failed,set the flag
      {
         MStatChangeMfgStatus(STAT_TBL_MFG_FLG_SELFREAD_WRITE_FAIL,1);      //Set the Mfg status flag.TODO:gsk: Reset only via Std Proc8?
      }
   }
   return writelen;
}

/////////////////////////////////////////////////////////////
//
//   DF_map_page_and_address() calculates the page and in-page address
//   corresponding to the input flat address.
//   The input is the 20 low bits of the flat address pointer (expected
//   to be in nonstandard 0xDVWXYZ format.  Output is the result mapping this
//   address onto the dataflash 264-byte page format.  The dataflash addressing
//   bytes are also initialized for this specific page and address.
//
//  PRECONDITIONS:
//   flat_addr's least-significant 20 bits fall within the dataflash chip's
//      address space without overflow
//
//  POSTCONDITIONS:
//   page, addr, and df[1..3] are all set appropriately
//
//   Note: this function is not used by boot loader and has no assembly language counterpart
//
void DF_map_page_and_address(unsigned long flat_addr, unsigned short *page, unsigned short *addr)
{
   // null out the bits in the flat address which cannot be address bits
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(AMPY_METER_U33WEP)|| defined(AMPY_METER_U3400)
   flat_addr &= 0x001FFFFFUL; // we have a 2 meg data flash
#else
   flat_addr &= 0x000FFFFFUL; // we have a 1 meg data flash, although the first 448kB is used by Utilinet...
#endif

   // convert to page and address within 256-byte (or 512) page (leaving 8 bytes for housekeeping, not addressable by high level read/writes).
   // map the flat address to the base address in the utilinet address space
   *page = (unsigned short) (flat_addr / DF_DATA_SIZE);// + LG_METER_DF_SPACE;
   *addr = (unsigned short) flat_addr % DF_DATA_SIZE;
}

/*------------------------------------------------------------------------------
* Function:    DF_init_incremental_refresh
* Inputs:      none
* Outputs:     sets up globals
* Description: DF_init_incremental_refresh should be called exactly once on
*              powerup before any dataflash write operations are carried
*              out.  It initializes the private variables used by the
*              incremental refresh subsystem.  If this is called more than
*              once the orderly progression of dataflash page refresh is
*              restarted, risking data retention problems with higher-numbered
*              pages which do not get refreshed between reinitializations.
*-----------------------------------------------------------------------------*/
void DF_init_incremental_refresh(void)
{
   TableRead(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,DF_page_refresh), 2, &DF_page_refresh);
   if (DF_page_refresh >= DF_REFRESH_END_PAGE)
      DF_page_refresh = DF_REFRESH_START_PAGE;
   DF_incremental_refresh();
   DF_backup_incremental_refresh();	
}

/*------------------------------------------------------------------------------
* Function:    DF_backup_incremental_refresh
* Inputs:      none
* Outputs:     none
* Description: Save a global to a NV table
*-----------------------------------------------------------------------------*/
void DF_backup_incremental_refresh(void)
{
   TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,DF_page_refresh), 2, &DF_page_refresh);
}

/*------------------------------------------------------------------------------
* Function:    DF_incremental_refresh
* Inputs:      none
* Outputs:     none
* Description: DF_incremental_refresh() ensures that every page of the dataflash
*              gets refreshed often enough (at least once every 10,000 page
*              writes) to not lose data, according to Atmel dataflash
*              application note AN-4. Algorithm:  maintain a counter identifying
*              a 'next' page to refresh. Any time any dataflash main memory page
*              is changed, call this routine and the 'next' page will be
*              refreshed by a copy/recopy exchange between main memory and the
*              dataflash buffer, then the 'next' counter is incremented. Over
*              time, the counter will iterate through the entire page space fast
*              enough so that the 10,000 limit is not exceeded.
*-----------------------------------------------------------------------------*/
void DF_incremental_refresh(void)
{
   // refresh contents of the page and go to the next page,
   // taking care of a wraparound at the the end
   DataFlashRefreshPage( DF_page_refresh );
   if (++DF_page_refresh >= DF_REFRESH_END_PAGE)
   {
      DF_page_refresh = DF_REFRESH_START_PAGE;
   }
}


#pragma vector=0
__interrupt void intdummy0(void){   BadIRQ(0);}
#pragma vector=1
__interrupt void intdummy1(void){   BadIRQ(1);}
#pragma vector=2
__interrupt void intdummy2(void){   BadIRQ(2);}
#pragma vector=3
__interrupt void intdummy3(void){   BadIRQ(3);}
#pragma vector=4
__interrupt void intdummy4(void){   BadIRQ(4);}
#pragma vector=8
__interrupt void intdummy8(void){   BadIRQ(8);}
#pragma vector=10
__interrupt void intdummy10(void){   BadIRQ(10);}
#pragma vector=11
__interrupt void intdummy11(void){   BadIRQ(11);}
#pragma vector=12
__interrupt void intdummy12(void){   BadIRQ(12);}
#pragma vector=13
__interrupt void intdummy13(void){   BadIRQ(13);}

#if(!(defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)))&&(!defined(MAINS_CLOCK))
#pragma vector=14
__interrupt void intdummy14(void){   BadIRQ(14);}
#endif

#pragma vector=22
__interrupt void intdummy22(void){   BadIRQ(22);}

/* remove. it is used in timer.c for the half duplex comms on opt port
#pragma vector=24
__interrupt void intdummy24(void){   BadIRQ(24);} */

#if(!(defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)))&&(!defined(MAINS_CLOCK))
#pragma vector=27
__interrupt void intdummy27(void){   BadIRQ(27);}
#endif

#pragma vector=28
__interrupt void intdummy28(void){   BadIRQ(28);}
#pragma vector=29
__interrupt void intdummy29(void){   BadIRQ(29);}
#pragma vector=30
__interrupt void intdummy30(void){   BadIRQ(30);}
#pragma vector=31
__interrupt void intdummy31(void){   BadIRQ(31);}
#pragma vector=32
__interrupt void intdummy32(void){   BadIRQ(32);}//start of software IRQ

void BadIRQ(unsigned char IRQtype)
{
#ifdef TYPETEST_DEBUG
   unsigned char flags;
   flags = disable_save ();
   printf ("Bad IRQ %i\n", IRQtype);
   flush_console_out();
   printf ("Ignoring error...\n");
   flush_console_out();
   PrintPCHistory();
   enable_restore (flags);
#else
   BadIRQCount++;
   BadIRQVal=IRQtype;
#endif
}
void GetBadIRQStatus()
{
   if(BadIRQSignature!=BAD_IRQ_SIGNATURE)
   {
      BadIRQSignature=BAD_IRQ_SIGNATURE;
      BadIRQCount=0;
      BadIRQVal=0xFF;
   }
   //TableWrite(eStdT03_EDMODEStatus,offsetof(_std_table_03_t, ed_mfg_status[3]), 1, &BadIRQCount);
   //TableWrite(eStdT03_EDMODEStatus,offsetof(_std_table_03_t, ed_mfg_status[2]), 1, &BadIRQVal);

}

/*------------------------------------------------------------------------------
* Function:    QuickTrueRand
* Inputs:      global value gRandBase
* Outputs:     Return a random number
* Description: Fast executing function that returns a truely random number,
*              unlike the quasi-random valur from rand(). This function is
*              required for security reasons and cases where rand() is not
*              seeded with a uniqe value. This function is similar to
*              rndNumGet() howerever its is available in non-RF builds and
*              allows quick successive calls that return a different value each
*              time.
*-----------------------------------------------------------------------------*/
unsigned short QuickTrueRand()
{
   gRandBase^=rand();
   gRandBase^=SysTimeGet();
   gRandBase^=SysTimeGet()<<9;
   return gRandBase;
}

/*------------------------------------------------------------------------------
* Function:    UpdateTrueRand
* Inputs:      global value gRandBase
* Outputs:     global value gRandBase is updated
* Description: During successive calls fills gRandBase with random noise from a
*              floating pin(88[10_6]) on the ADC.
*-----------------------------------------------------------------------------*/
void UpdateTrueRand()
{
   unsigned short val;

   val=getAtoD(A2D_THERMISTOR);
   gRandBase<<=1;
   gRandBase+=val;
}


unsigned char FutureFwDlActivated = 0;

/*------------------------------------------------------------------------------
* Function:    InitFwDl
* Inputs:      Uses mfg table 0 and 30
* Outputs:     Modifies the global FutureFwDlActivated to 1 if config is ok
* Description: Calculates the hash across the firmware download control table
*              and compares it to the stored hash in mfg 0. If they match, then
*              FutureFwDlActivated is set to 1, and the future arming feature
*              is enabled
*-----------------------------------------------------------------------------*/
void InitFwDl(void)
{
   unsigned long CRCRef,crc32;
   // calc the crc across mfg
   //eHashFirmwareM16
   TableRead(eMfgT00_Hash, eHashFirmwareM16*sizeof(_hash_entry_t), sizeof(CRCRef), (void *)&CRCRef);

   crc32=0;
   CalcTableHash(eMfgT30_FwDlCtrl,&crc32);

   if(crc32 != CRCRef)
      FutureFwDlActivated = 0;
   else
      FutureFwDlActivated = 1;
}

/*------------------------------------------------------------------------------
* Function:    CheckFwDownloadActivationTime
* Inputs:      reads the activation timestamp from mfg tbl 30 and compares with
*              the meters current time. If the activation time has elapsed, arm
*              the firmware image and reboot/switchover.
* Outputs:     An event will be recorded if the switchover is attempted
* Description: Check if it's time to switchover to a previously armed firmware
*              image
*-----------------------------------------------------------------------------*/
void CheckFwDownloadActivationTime(void)
{
   unsigned long int templ;

   if(FutureFwDlActivated)
   {
      // how do we know this table is correct? we need to play the hash/crc game?
      TableRead(eMfgT30_FwDlCtrl, offsetof(_mfg_table_30_t, FWDLActivateTime), sizeof(templ), &templ);
      // get current time in posix/gmt
      //IsFutureDate

      if(((RTC_GetFunctionalTime(0,0,0,eRTCFT_GMT) >= templ)&&(templ != 0x00000000)) || (templ == 0xfefefefe))
      {
         // we have passed the activation time, so switch into the new firmware
         gFirmwareUpgrade = 1;
         FwDownloadFixSeq();
         FwDownloadFixBootPartNum();
#if defined (AMPY_METER_NO_RF) 
         FwDownloadRelayCRCFix();
#elif defined(AMPY_METER_U3400)
         FwDownloadRelayCRCFix(16);
#else
         if(gU1300Ver2 == 0)
         {
           FwDownloadRelayCRCFix(24);
         }
         else
         {
           FwDownloadRelayCRCFix(16);
         }
#endif
         templ = 0;
         TableWrite(eMfgT30_FwDlCtrl, offsetof(_mfg_table_30_t, FWDLActivateTime), sizeof(templ), &templ);
         UpdateHashCheck();
         // should crc the image and double check before this sys_reset()....
         AMPYArmCrashLog();
         sys_reset (SOFT_SUICIDE_ANSI_PROC);
      }
   }
}

/*------------------------------------------------------------------------------
* Function:    AMPYPrintfHook
* Inputs:      c-character that was intended to be sent out the optical port.
* Outputs:     Returns zero if character should be sent out the optical port.
* Description: This function is called at the beginning of console_out() so that
*              system crash information can be captured in NV memory.
*-----------------------------------------------------------------------------*/
unsigned char AMPYPrintfHook( int c)
{
   unsigned short page, offset;
   SEMAPHORE saved;
   unsigned char flags;
//return 0;//
#ifdef  PRINT_TABLEADDR_FLASH
   return 0;
#endif
#ifdef FLASH_ERASEWRITE_TIME_DEBUG
   return 0;
#endif
#if defined(PRINT_POWER_DN_DELAY)
   #if !defined(LOG_POWER_DN_DELAY)
   return 0;
   #endif
#endif
#ifdef LOG_COMMS_CMDS
   return 0;
#endif
#if defined(DBG_POSTRESETDUMP)
   M16C_PetWatchDog();
   #ifndef AMPY_METER_R1100
   while (!ti_u1c1)
    #else
     while (!ti_u0c1)
    #endif
   {
      M16C_PetWatchDog();
   }
   #ifndef AMPY_METER_R1100
   U1TB = c;
   #else
   U0TB = c;
   #endif
   return 1;
#endif
#if defined(DBG_PRINT_TIME_DBG)
   return 0;
#endif

   if(g_ampy_generic_flags&AMPY_GEN_FLG_DO_PRINT)
      return 0;

   if(gAMPY_NV_crash_index<AMPY_NV_CRASH_MAX_CHARS)
   {
      gAMPY_NV_crash_buffer[gAMPY_NV_crash_index%AMPY_NV_CRASH_BUF_SIZE]=c;
      if( (gAMPY_NV_crash_index%AMPY_NV_CRASH_BUF_SIZE) == (AMPY_NV_CRASH_BUF_SIZE-1) )
      {//small buffer has been filled so commit it to NV

         page=(gAMPY_NV_crash_index-AMPY_NV_CRASH_BUF_SIZE+1)/DF_DATA_SIZE;
         page+=AEMCRASHLOGSTARTPAGE;
         offset=gAMPY_NV_crash_index-AMPY_NV_CRASH_BUF_SIZE+1;
         offset=offset%DF_DATA_SIZE;

         // I would have preferred to use LockTask() instead of disable_save()
         // but LockTask() does not actually prevent the task from swapping out
         // in the case where Pend() is called.
         flags = disable_save();
         DataFlashInit();//halt any SPI BUS comms that may be going on
         saved=uugflash_lock;
         uugflash_lock=0;
         DataFlashWritePageBytesNoErase(page,offset,gAMPY_NV_crash_buffer,AMPY_NV_CRASH_BUF_SIZE);
         uugflash_lock=saved;
         enable_restore(flags);
      }
      gAMPY_NV_crash_index++;
      return 1;
   }
   return 0;
}

/*------------------------------------------------------------------------------
* Function:    AMPYArmCrashLog
* Inputs:      none.
* Outputs:     updates globals
* Description: Erase NV memory in preparation for the writing of system crash
*              information.
*-----------------------------------------------------------------------------*/
void AMPYArmCrashLog(void)
{
   int i;

   for(i=0;i<AEMCRASHLOGNUMPAGES;i++)
      DataFlashPageErase(AEMCRASHLOGSTARTPAGE+i);

   gAMPY_NV_crash_index=0;
}

/*------------------------------------------------------------------------------
* Function:    AMPYCrashLogInit
* Inputs:      none.
* Outputs:     Updates gAMPY_NV_crash_index global.
* Description: Initialise gAMPY_NV_crash_index by doing a search through the
*              crash log area.
*-----------------------------------------------------------------------------*/
void AMPYCrashLogInit(void)
{
   int i,p,val,firstrun;
   unsigned char *buff;

   firstrun=1;
   buff=GetRAMPage();
   //Do a quick check of the first page first if its all FF then assume all pages
   //are FF. Otherwise do a full check.
   val=DF_DATA_SIZE;
   p=AEMCRASHLOGSTARTPAGE;
   while(p>=AEMCRASHLOGSTARTPAGE)
   {//search backwards through crash log area looking for non 0xFF.
      DataFlashReadPageBytes(p,0,buff,DF_DATA_SIZE);
      i=DF_DATA_SIZE;
      do
      {
         if(buff[--i]==0xFF)
            val--;
         else
         {
            if(firstrun)
            {
               val=AMPY_NV_CRASH_MAX_CHARS;
               p=AEMCRASHLOGSTARTPAGE+AEMCRASHLOGNUMPAGES;
               firstrun=0;
               i=0;
            }
            break;
         }
      }while(i>0);

      if(i!=0)
         break;
      p--;
   }
   FreeRAMPage();
   //val=AMPY_NV_CRASH_BUF_SIZE*((gAMPY_NV_crash_index+AMPY_NV_CRASH_BUF_SIZE-1)/AMPY_NV_CRASH_BUF_SIZE);
   gAMPY_NV_crash_index=val;
}

extern SEMAPHORE adc_lock;
extern SEMAPHORE ee_emul_lock;
extern SEMAPHORE encryptLock;
extern SEMAPHORE flash_lock;
extern SEMAPHORE metrology_data_lock;
extern SEMAPHORE nv4k_lock;
extern SEMAPHORE Table28EngyLock;
extern SEMAPHORE tableRAMAccess;
extern SEMAPHORE transparent_inuse;
extern SEMAPHORE update_flag;
extern SEMAPHORE uugflash_lock;
SEMAPHORE backupsemaphore;
const SEMAPHORE *SemList[12]={
   &adc_lock,
   &ee_emul_lock,
   &encryptLock,
   &flash_lock,
   &metrology_data_lock,
   &nv4k_lock,
   &Table28EngyLock,
   &tableRAMAccess,
   &transparent_inuse,
   &update_flag,
   &uugflash_lock,
   &backupsemaphore
};
void AMPYPrintSemaphores(void)
{
   unsigned char i;
   printf("Semaphores:\n");
   for(i=0;i<12;i++)
      printf("0x%04X\n",*(SemList[i]));
}


#if defined(DBG_POWERFAILTEST)
#pragma vector=TIMER_B1_INTNUM
__interrupt void TimerB1ISR( void )
{
   unsigned char state;
   state = PWRFAIL_STATE;
   if(!state)
   {
      if(gcount < 0xFFFF)
         gcount++;

      if(state != glaststate) //falling edge
      {
         gcount = 0;
      }

      if(AMPYPowerStateMode != eAMPYpsLPM) //eAMPYpsLPM = 2
      {
         if(gcount >= 600)
         {
            if(!do_once);
            {
               DataFlashInit();
               backupsemaphore = uugflash_lock;
               uugflash_lock = 0;
               AMPYPrintSemaphores();
               DebugPrintEvents (0, 0);
               PrintStackDump(ZERO_TASK);
               PrintStackDump(METROLOGY_TASK);
               PrintStackDump(METER_TASK);
               PrintStackDump(CONSOLE_TASK);
               PrintStackDump(ACCOUNTANT_TASK);
               do_once=1;
            }
         }
      }
   }
   else
   {
      if(state != glaststate) //rising edge.end of outage
      {
         g_finalcount = gcount;
      }
   }
   glaststate = state;
}

void TimerB1_Setup()
{
   tb1s = 0;
   TB1MR = 0x40; // fclk/32.
   TB1IC = 6;
   TB1 = ((CPU_CLOCK/8)/1000) - 1; // go off every 1ms
   tb1s = 1;
}
#endif

#if defined(DBG_POSTRESETDUMP)
__root void PostResetChkDoStackDump( void )
{
   if(reboot_data.pattern1==AMPY_POST_RESET_DUMP_ARMED)
   {//print out stacks
      printf("!");
      //asm("LDC 0x0000A000, ISP");
      //asm("LDC 0x0000B000, SP");
      disable_save();
      M16C_PetWatchDog();
      //console_init (9600U, NO_PARITY, EIGHT_BITS_PER_CHAR, STOP_BITS_1);
      AMPYPrintSemaphores();
      DebugPrintEvents (0, 0);
      PrintStackDump(ZERO_TASK);
      PrintStackDump(METROLOGY_TASK);
      PrintStackDump(METER_TASK);
      PrintStackDump(CONSOLE_TASK);
      PrintStackDump(ACCOUNTANT_TASK);
   }
   reboot_data.pattern1=AMPY_POST_RESET_DUMP_ARMED;
}
#endif

#define AMPY_FLASH_PAGE_SIG   0x9713
#define AMPY_FLASH_OFFSET_SIG 0xF00D
#define AMPY_FLASH_LENGTH_SIG 0x0000
#define AMPY_FLASH_TAG_LEN 6
unsigned char gAMPYFlashPackBuffer[DF_DATA_SIZE];
void AMPYFlashEnablePacking(void)
{
#if !defined(DBG_FLASH_PACK)
   if(gFirmwareUpgrade)
   {
      gAMPYFlashDoPack=0;
      return;
   }
   gAMPYFlashDoPack=1;
   gAMPYFlashIdx=0;

   //write a signature to show block is valid
   AMPYFlashPack(AMPY_FLASH_PAGE_SIG, AMPY_FLASH_OFFSET_SIG, NULL, AMPY_FLASH_LENGTH_SIG);
#endif
}

unsigned char AMPYFlashUnpack(void)
{
#if !defined(DBG_FLASH_PACK)
   unsigned char abort=0;      //never abort flash unpacking
   unsigned short i;
   unsigned short my_page;
   unsigned short my_addr;
   unsigned short my_len;
   unsigned short len_in_page;
   unsigned short tag_page;
   unsigned short tag_offset;
   unsigned short tag_len;
   unsigned char *pBytes;

   pBytes=gAMPYFlashPackBuffer;
   //test if there is valid data to be restored
   DataFlashReadPageBytes(AEMPACKSTARTPAGE,0,pBytes,AMPY_FLASH_TAG_LEN);
   if( (pBytes[0]==(0xFF&AMPY_FLASH_PAGE_SIG)) && (pBytes[1]==(AMPY_FLASH_PAGE_SIG>>8)) && (pBytes[2]==(0xFF&AMPY_FLASH_OFFSET_SIG)) && (pBytes[3]==(AMPY_FLASH_OFFSET_SIG>>8)) && (pBytes[4]==(0xFF&AMPY_FLASH_LENGTH_SIG)) && (pBytes[5]==(AMPY_FLASH_LENGTH_SIG>>8)) )
   {
      gAMPYFlashIdx=6;     //6 bytes signature already read
      //loop through all the tags and thier data and copy it to the correct place
      while(gAMPYFlashIdx<((AEMPACKNUMPAGES)*(DF_DATA_SIZE)))
      {
         if(AMPYGetFilteredEPF(16))
         {
            abort=1;
            break;
         }
         //3 steps: read tag, read data, write data
         for(i=0;i<3;i++)
         {
            if(i==0)
            {//get ready to read the tag
               my_len=AMPY_FLASH_TAG_LEN;
               DF_map_page_and_address(gAMPYFlashIdx,&my_page,&my_addr);
               my_page+=AEMPACKSTARTPAGE;
            }
            else if(i==1)
            {//get ready to read data
               //save tag information
               memcpy(&tag_page, &gAMPYFlashPackBuffer[0], 2);
               memcpy(&tag_offset, &gAMPYFlashPackBuffer[2], 2);
               memcpy(&tag_len, &gAMPYFlashPackBuffer[4], 2);

               //page should be within a know area.
               if(tag_page>=DF_MAX_FLASH_PAGES)
               {
                  gAMPYFlashIdx=0xFFFF;
                  break;
               }

               //offset should be within a page
               if(tag_offset>=DF_DATA_SIZE)
               {
                  gAMPYFlashIdx=0xFFFF;
                  break;
               }

               //skip zero length data, end process if end of data i.e. all ff.
               if((tag_len==0)||(tag_len>DF_DATA_SIZE))
               {
                  gAMPYFlashIdx=0xFFFF;
                  break;
               }

               my_len=tag_len;
               DF_map_page_and_address(gAMPYFlashIdx,&my_page,&my_addr);
               my_page+=AEMPACKSTARTPAGE;
            }
            else
            {//get ready to write data
               my_len=tag_len;
               my_page=tag_page;
               my_addr=tag_offset;
            }

            pBytes=gAMPYFlashPackBuffer;
            while(my_len)
            {
               len_in_page = (my_len > (DF_DATA_SIZE - my_addr))? (DF_DATA_SIZE - my_addr) : my_len;
               my_len = my_len - len_in_page;

               if(i==2)
                  DataFlashWritePageBytes(my_page,my_addr,pBytes,len_in_page);
               else
               {
                  DataFlashReadPageBytes(my_page,my_addr,pBytes,len_in_page);
                  gAMPYFlashIdx+=len_in_page;
               }
               pBytes+=len_in_page;

               // prepare dataflash to access addr 0 of next page
               my_page++;
               my_addr = 0;
            }
         }//end for loop for 3 steps
      }//end loop for each tag
   }//end if check for valid data




   //erase all pages ready for the next power fail
   if(!abort)
   {
      for(i=AEMPACKSTARTPAGE;i<(AEMPACKNUMPAGES+AEMPACKSTARTPAGE);i++)
      {
         DataFlashPageErase(i);
         if(AMPYGetFilteredEPF(17))
         {
            abort=1;
         }

//gsk:removed checks for powerfail when doing erase.Erase takes around 110ms for 10 pages and our hold up capacitor should atleast "hold" good for that long if we did lose mains during erase.Note that we do a powerfail check for each page while unpacking.
//         if(AMPYGetFilteredEPF())
//            break;
      }
   }

 return abort;
#endif
}

unsigned char AMPYDFWritePageBytes(unsigned short PageNum, unsigned short offset, unsigned char *pData, unsigned short len)
{
   if(gAMPYFlashDoPack)
   {
      AMPYFlashPack(PageNum,offset,pData,len);
      return (TRUE);
   }

   return DataFlashWritePageBytes(PageNum,offset,pData,len);
}

#if defined(FLASH_FASTPACK)
void AMPYFlashPackLastPage()
{
   unsigned short my_page;
   unsigned short my_addr;
   
   DF_map_page_and_address(gAMPYFlashIdx,&my_page,&my_addr);
   my_page+=AEMPACKSTARTPAGE;
   if (my_addr)
      DataFlashWritePageBytesNoErase(my_page,0,gFlashPageBuff,my_addr-1);
}
#endif

void AMPYFlashPack(unsigned short PageNum, unsigned short offset, unsigned char *pData, unsigned short len)
{
   unsigned short my_page;
   unsigned short my_addr;
   unsigned short my_len;
   unsigned short len_in_page;
   unsigned char my_buff[AMPY_FLASH_TAG_LEN];
   unsigned char *pBytes;
   unsigned char WriteTag;
#if defined(FLASH_FASTPACK)	
   unsigned short i=0;
#endif

   //each block of data starts with a tag of the form:
   //[page LSB][page MSB][offset LSB][offset MSB][len LSB][len MSB]
   memcpy(&my_buff[0],&PageNum,2);
   memcpy(&my_buff[2],&offset,2);
   memcpy(&my_buff[4],&len,2);

   //calculate page and offset
   DF_map_page_and_address(gAMPYFlashIdx,&my_page,&my_addr);
   my_page+=AEMPACKSTARTPAGE;

   //write the tag
   my_len=AMPY_FLASH_TAG_LEN;
   pBytes=my_buff;
   WriteTag = 1;
   while (my_len)
   {
      len_in_page = (my_len > (DF_DATA_SIZE - my_addr))? (DF_DATA_SIZE - my_addr) : my_len;
      my_len = my_len - len_in_page;

      #if defined(FLASH_FASTPACK)

      for (i=0;i<len_in_page;i++)
         gFlashPageBuff[my_addr++] = pBytes[i];
      
      if (my_addr == DF_DATA_SIZE)
         DataFlashWritePageBytesNoErase(my_page,0,gFlashPageBuff,DF_DATA_SIZE);                  
      #else      
      
      DataFlashWritePageBytesNoErase(my_page,my_addr,pBytes,len_in_page);
      my_addr += len_in_page; 
      #endif
        
      pBytes+=len_in_page;
      gAMPYFlashIdx+=len_in_page;

      // prepare dataflash to access addr 0 of next page
      if(my_len)
      {
         my_page++;
         my_addr = 0;
      }
      else if(WriteTag)//(*pBytes!=pData[len])       // gsk : && (my_len==0)
      {//set up to do data block after the tag
         WriteTag= 0;
         my_len=len;
         pBytes=pData;
      }
   }
}

void AMPYFlashDisablePacking(void)
{
   gAMPYFlashDoPack=0;
   #if defined(FLASH_FASTPACK)
   AMPYFlashPackLastPage();
   #endif
}

/*------------------------------------------------------------------------------
* Function:    PrintSysStatus
* Inputs:      none
* Outputs:     none.
* Description: Print the status of the system and then reset.
*-----------------------------------------------------------------------------*/
void AMPYPrintSysStatus(void)
{
   g_ampy_generic_flags|=AMPY_GEN_FLG_DO_PRINT;
   AMPYPrintSemaphores();
   SysAbort ("Sys Status", 0);
}

/*------------------------------------------------------------------------------
* Function:    AMPYSysValTest
* Inputs:      none
* Outputs:     none.
* Description: Test a RAM value for insanity and do a stack dump if a bad value
* is found.
*-----------------------------------------------------------------------------*/
void AMPYSysValTest(void)
{
   unsigned long tempL1,tempL2;
   unsigned char tempC;

   if((g_ampy_generic_flags&AMPY_GEN_FLG_DO_VAL_TEST)&&(g_ampy_test_val_pattern==AMPY_TEST_VAL_PATTERN))
   {
      memcpy(&tempL1,g_ampy_test_val_ptr,4);
      tempL1&=g_ampy_test_val_mask;

      if(!(g_ampy_generic_flags&AMPY_GEN_FLG_SKIP))
      {
         //test if a delta calculation
         if(g_ampy_generic_flags&AMPY_GEN_FLG_DELTA)
            tempL2=tempL1-g_ampy_test_val_last;
         else
            tempL2=tempL1;

         tempC=0;
         if(g_ampy_generic_flags&AMPY_GEN_FLG_CHK_EQUAL)
         {//test if equal
            if(tempL2==g_ampy_test_val_ref)
               tempC=1;
         }
         else
         {//test if greater than
            if(tempL2>g_ampy_test_val_ref)
               tempC=1;
         }

         //inverse logic
         if(g_ampy_generic_flags&AMPY_GEN_FLG_INVERSE)
            tempC=!tempC;

         //check if condidtion has been breached
         if(tempC)
         {
            //print info and reset
            g_ampy_generic_flags|=AMPY_GEN_FLG_DO_PRINT;
            printf("Bad Test Val at addr %08lX=%08lX",(unsigned long)g_ampy_test_val_ptr,tempL2);
            printf("r=%08lX m=%08lX c=%02X\n",g_ampy_test_val_ref,g_ampy_test_val_mask,g_ampy_generic_flags);
            AMPYPrintSysStatus();
         }
      }
      g_ampy_generic_flags&=~AMPY_GEN_FLG_SKIP;
      g_ampy_test_val_last=tempL1;
   }
}

/*------------------------------------------------------------------------------
* Function:    AMPYSysValInit
* Inputs:      ValAddr- address of value to be monitored
*              ref-reference value that taget is checked against
*              mask-bit mask for ignoring bits in taget
*              cfg-config delata/absolute, equal/greater than, norm/inverse
*              pData- alternative way of passing in data as an array used if not
*                    NULL, other parameters ignored if non-NULL.
* Outputs:     none.
* Description: Test a RAM value for insanity and do a stack dump if a bad value
* is found.
*-----------------------------------------------------------------------------*/
void AMPYSysValInit(unsigned long ValAddr,unsigned long ref,unsigned long mask, unsigned char cfg, unsigned char *pData)
{
   if(pData!=NULL)
   {
      memcpy(&ValAddr,  &pData[0],4);
      memcpy(&ref,      &pData[4],4);
      memcpy(&mask,     &pData[8],4);
      cfg=pData[12];
   }

   g_ampy_generic_flags&=~(AMPY_GEN_FLG_DO_VAL_TEST|AMPY_GEN_FLG_DELTA|AMPY_GEN_FLG_INVERSE|AMPY_GEN_FLG_CHK_EQUAL);
   cfg&=(AMPY_GEN_FLG_DELTA|AMPY_GEN_FLG_INVERSE|AMPY_GEN_FLG_CHK_EQUAL);
   g_ampy_generic_flags|=cfg;

   g_ampy_test_val_ptr=(unsigned char *)ValAddr;
   g_ampy_test_val_mask=mask;
   g_ampy_test_val_ref=ref;
   g_ampy_test_val_pattern=AMPY_TEST_VAL_PATTERN;
   g_ampy_generic_flags|=AMPY_GEN_FLG_DO_VAL_TEST|AMPY_GEN_FLG_SKIP;
}


/*------------------------------------------------------------------------------
* Function:    AMPYEnablePrintToPort
* Inputs:      doit- if non-zero force print to optical port
* Outputs:     none.
* Description: Sets behaviour of the printf function, force it to print to the
*              optical port or allow diversion to the crash log.
*-----------------------------------------------------------------------------*/
void AMPYEnablePrintToPort(unsigned char doit)
{
   if(doit)
      g_ampy_generic_flags|=AMPY_GEN_FLG_DO_PRINT;
   else
      g_ampy_generic_flags&=~AMPY_GEN_FLG_DO_PRINT;
}

/*------------------------------------------------------------------------------
* Function:    AMPYPrintTCB
* Inputs:      Pointer to the task's TCB
* Outputs:     none.
* Description: Prints out the TCB of the task to which it belongs
*-----------------------------------------------------------------------------*/
void AMPYPrintTCB(TASK_STRUCT *pTcb)
{
   int i=0,j=0;
   unsigned char *t;            //KERN_NEAR??

   t = (unsigned char*)pTcb;
   printf("Task%u TCB:\n",pTcb->ID);

   while(i<sizeof(TASK_STRUCT))
   {
      printf("\n%p : ", (unsigned char *)t);
      for (j = 0; ((j < 16)&&((i+j)<sizeof(TASK_STRUCT))); j++)
      {
         farWatchdog();             //TODO: Do we really need to pet the WD here or after each 16 byte block?
         printf("%02X ",(unsigned char) *t++);
      }
      i+=16;
   }

   printf("\n");
}

/*------------------------------------------------------------------------------
* Function:    AMPYPrintST03
* Inputs:      None
* Outputs:     none.
* Description: Prints out the ST3
*-----------------------------------------------------------------------------*/
void AMPYPrintST03(void)
{
   int i=0;

   unsigned char *pMeterStatus;            //KERN_NEAR??

   pMeterStatus = (unsigned char *)&std_table_03_struct;

   printf("ST3:\n");
   for(i=0;((i<sizeof(std_table_03_struct))&&(i<16));i++)
   {
      printf("%02X ",(unsigned char) *pMeterStatus++);
   }

   printf("\n");

}

/*------------------------------------------------------------------------------
* Function:    AMPYMakeTCBEvent
* Inputs:      none.
* Outputs:     none.
* Description: Creates a set of internal events in RAM that may get witten to
*              the crsh log. The events contain info about the Tcb structure.
*-----------------------------------------------------------------------------*/
void AMPYMakeTCBEvent(void)
{
   EVENT7("Tcb1 %04X,%04X,%02X,%04X,%04X,%02X",Tcb[0].Pending,Tcb[0].Posted,Tcb[0].State,Tcb[1].Pending,Tcb[1].Posted,Tcb[1].State);
   EVENT7("Tcb2 %04X,%04X,%02X,%04X,%04X,%02X",Tcb[2].Pending,Tcb[2].Posted,Tcb[2].State,Tcb[3].Pending,Tcb[3].Posted,Tcb[3].State);
   EVENT7("Tcb3 %04X,%04X,%02X",Tcb[4].Pending,Tcb[4].Posted,Tcb[4].State);
}

/*------------------------------------------------------------------------------
* Function:    AMPYMakeSemEvent
* Inputs:      none.
* Outputs:     none.
* Description: Creates a set of internal events in RAM that may get witten to
*              the crsh log. The events contain info about all semaphores.
*-----------------------------------------------------------------------------*/
void AMPYMakeSemEvent(void)
{
   EVENT7("Sem1 %04X,%04X,%04X,%04X,%04X,%04X",*(SemList[0]),*(SemList[1]),*(SemList[2]),*(SemList[3]),*(SemList[4]),*(SemList[5]));
   EVENT7("Sem2 %04X,%04X,%04X,%04X,%04X,%04X",*(SemList[6]),*(SemList[7]),*(SemList[8]),*(SemList[9]),*(SemList[10]),*(SemList[11]));
}

void PrintStackDumptoRAM(unsigned int task)
{

   TASK_STRUCT KERN_NEAR *tcb;
   unsigned char KERN_NEAR *s;
   int               j;

   if (task == 255)
      {
      s = getStackPointer();
      task = TaskId();
      }
   else
      {
      s = (unsigned char KERN_NEAR *)Tcb[task].SP;
      }

   tcb = &Tcb[task];

   //Copy current semaphore value
   gRamDumpSemValue =tableRAMAccess;

   //Copy task Id
   gRAMdumpTaskId=task;

   //Copy SP and TOS
   gRAMDumpSP = s;
   gRAMDumpTOS = (unsigned char *)tcb->TOS;

   //copy systime of the time of sem conflict
   gRamDumpTime =SysTimeGet();

   j = 0;
   while (s < (unsigned char *)tcb->TOS)
   {
      int i;

      for (i = 0; ((i < 16) && (s < (unsigned char *)tcb->TOS)); i++)
      {
         farWatchdog();
         memcpy(&StackDumpRAM[i+(16*j)],s++,1);
      }
         /*
         ** Put a limit on how many times this can go through. Its possible during
         ** a crash that the stack pointer itself got screwed up. If that is the case
         ** this while loop could turn into an infinite loop. Have no proof of that
         ** at this time, but the event display routine became an infinite loop
         ** because of a memory overwrite.
         **
         ** This will limit any stack dump to a maximum of 1024 byte which should
         ** be more than adequate.
         */
      if (++j > 64)
         break;
   }
}

void CopyRAMStackDumptoNV(void)
{

   int  j;
   int x,y;

   //Get RAM dump and commit to Flash
   printf ("Stack Dump Task %u RamCopy, value %lx,Time %lu\n", gRAMdumpTaskId,(unsigned long)gRamDumpSemValue,gRamDumpTime);
   printf ("Sem HeadPad %02X, TailPad %02X\n", SemHeadPad,SemTailPad);


   x=0;
   while(x< TASKS_AVAILABLE)
   {
      printf ("Task %u\n", x);
      for(y=0; y<2 ; y++)
      {
         printf ("Type %u,Time %lu,Value %lx\n",y,gSemaphoreDebugLog[x][y].Time,(unsigned long)gSemaphoreDebugLog[x][y].SemValue);
      }
      x++;
   }

   j = 0;
   while (gRAMDumpSP < gRAMDumpTOS)
   {
      int i;

      printf("\n%p : ", (unsigned char *)gRAMDumpSP);
      for (i = 0; ((i < 16) && (gRAMDumpSP < gRAMDumpTOS)); i++)
      {
         farWatchdog();
         printf("%02X ", StackDumpRAM[i+(16*j)]);
         gRAMDumpSP++;
      }

         /*
         ** Put a limit on how many times this can go through. Its possible during
         ** a crash that the stack pointer itself got screwed up. If that is the case
         ** this while loop could turn into an infinite loop. Have no proof of that
         ** at this time, but the event display routine became an infinite loop
         ** because of a memory overwrite.
         **
         ** This will limit any stack dump to a maximum of 1024 byte which should
         ** be more than adequate.
         */
      if (++j > 64)
         break;
   }

   printf("\n");

   flush_console_out();

}
/*
unsigned char               PrePayMode; //0- Prepay Disabled 1- Prepay Enabled
  unsigned long               PrePayHoldOffTime;
  unsigned short              PrePayCommTimeoutl;
  unsigned char               PrePayDiscOverride;
  _ppm_DisconnectHrs_Struct   PrePayDiscHours;
  unsigned char               PrePayCrdtCents;
  unsigned char               spare[5];*/


void PrePaymentPowerDown()
{
   flashWriteMeter(GetMfg15Addr(), (unsigned char*)&PrePayDisplay, sizeof(_mfg_table_15_t));
}

#if defined(METROLOGY_VERIFY_MMI_PARAMS)
void Set_Debug_FeatureMask(unsigned char* FeatureParams)
{
   TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,SpecialFeatureMask),1, &FeatureParams[0]);
   TableWrite(eMfgT54_GnrlMeterNVStatus, offsetof(_mfg_table_54_t,MMI_paramUpdate_Freq),2, &FeatureParams[1]);
}
#endif
