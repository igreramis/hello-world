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

      Module Name:      Tables.c

      Date:             29th July 2008

      Engineer:         Ian Davis, Marc Circosta

      Description:      ANSI table definitions and functions

      Changes:


************AMPY+EMAIL - CONFIDENTIAL******************************************/

#include "std.h"
#include "Tables.h"
#include "uugflash.h"
#include <string.h>
#include "metrologydriver.h"
#include "AMPYDisplayApp.h"
#include "mmi.h"
#include "RTC.h"
#include "RelayDriver.h"
#include "crc.h"
#include "stdio.h"
#include "MeterComms.h"
#include "LoadProf.h"
#include "EventLog.h"
#include "TOU.h"
#include "MeterStatus.h"
#include "Metrology.h"
#include "QOS.h"
#include "LPQoS.h"
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   #include "Ripple.h"
   #include "RippleApp.h"
#endif
#include "RelayAppLC.h"
#include "MetrologyADCDriver.h"
#include "SysMonitor.h"
#if defined(NEUTRAL_INTEGRITY_POLY)
   #include "PolyPhaseNI.h"
#endif
#if defined(PULSE_OUTPUT)
#include "pulseoutput.h"
#endif
#if DBG_DIAGTABLE
#include "TablesDiag.h"
#endif
#if defined(SIMULATION_SR)
   #ifdef __GNUC__ //generate packed structs in GCC
      #pragma pack(push,1)
   #endif
#endif

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
   #define DEBUG_EVENT_LOG_ADD_ON_CMD
#endif

//#define SIZE_TEST
void InitDefaultTableData(void);                // remove me, prototype for some hackery down in mexico

void BuildDynamicMfg12(unsigned char *Buffer,unsigned short Offset,unsigned short Size);
unsigned short BuildDynamicStd62(unsigned char *Buffer,unsigned short Offset,unsigned short Size);
unsigned char GroupTblWrite(unsigned char PortSrc, unsigned char UserLvl,unsigned char *pData);
unsigned char Log3rdPartyEvent(unsigned char *pData);
unsigned short int FWDataBlockWrite(unsigned char *buffer, unsigned short int len);
unsigned char FwDownloadProc(unsigned char* Data);
void FwDlCtrlUpdate(void);
void TablePrintTblInfo(void);
void TableAddrFlash();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
unsigned char *LPBuffer;
unsigned char   gLPPlatformGet = 0;
PLATFORMP LPCnfg_Tbls;
#endif
unsigned char RTC_SSN_Check = 0;

unsigned char TableWriteStopRecursive[MAX_TASK_NUM]; //this is used to stop recursiveness
                                       //during a table write.
unsigned char TableWriteStopRecursiveSingleTask;
extern unsigned char gAMPY_NV_Read_Err_Count;
extern unsigned char gAMPY_NV_Write_Err_Count;
unsigned short    gTableUpdateHandlerPending =0;     //In the event the multi packet write stream stops on error or if there is power failure,this helps us identify if the function handler for that table is to be called
extern unsigned char gFirmwareUpgrade; //used to disabled flash packing on firmware upgrade.

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
extern unsigned char gU1300Ver2;
unsigned char gU1300S2_ValidFirmware = 1; //Always assume that the firmware is valid
#endif

// Instances
//0-device config, ID, Procedure
#ifndef Simulation
const _std_table_00_t std_table_00_struct=
#else
_std_table_00_t std_table_00_struct=
#endif
{
   (GEN_CONFIG_TBL_DOT_DATA_ORDER)|(GEN_CONFIG_TBL_DOT_CHAR_FORMAT<<1)|(GEN_CONFIG_TBL_DOT_MODEL_SELECT<<4),//format_control_1
   (GEN_CONFIG_TBL_DOT_TM_FORMAT)|(GEN_CONFIG_TBL_DOT_DATA_ACCESS_METHOD<<3)|(GEN_CONFIG_TBL_DOT_ID_FORM<<5)|(GEN_CONFIG_TBL_DOT_INT_FORMAT<<6),//format_control_2
   (GEN_CONFIG_TBL_DOT_NI_FORMAT1)|(GEN_CONFIG_TBL_DOT_NI_FORMAT2<<4),//format_control_3
   "L&G ",//manufacturer[4]
   2,//nameplate_type
   0,//default_set_used
   GEN_CONFIG_TBL_DOT_MAX_PROC_PARM_LENGTH,//max_proc_parm_length
   GEN_CONFIG_TBL_DOT_MAX_RESP_DATA_LEN,//max_resp_data_len
   1,//std_version_no
   0,//std_revision_no
   GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED,//dim_std_tbls_used
   GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED,//dim_mfg_tbls_used
   GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED,//dim_std_proc_used
   GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED,//dim_mfg_proc_used
   GEN_CONFIG_TBL_DOT_DIM_MFG_STATUS_USED,//dim_mfg_status_used
   GEN_CONFIG_TBL_DOT_NBR_PENDING,//nbr_pending
   {0xEF,0xBD,0xF1,0xDF,0x03,0x00,0xFC,0xF1,0xCF,0x19},//std_tbls_used[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED]
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
   {0x21,0x10,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x00,0x3C,0x00},//mfg_tbls_used[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED]
   {0xBB,0x07,0x01},//std_proc_used[GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED]
   {0xB7,0xF3,0x07},//mfg_proc_used[GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED]
   {0xE0,0x20,0x60,0x80,0x03,0x00,0x68,0x60,0x00,0x08},//std_tbls_write[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED]
   {0x00,0x00,0xC0,0x66,0x7D,0xFC,0x05,0x0E,0x1C,0x02,0x00,0x00,0x00},//mfg_tbls_write[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED]
#elif defined(AMPY_METER_U3400)
   {0x21,0x10,0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x00,0x3C,0x00},//mfg_tbls_used[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED]
   {0xBB,0x07,0x01},//std_proc_used[GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED]
   {0xB7,0xF3,0x07},//mfg_proc_used[GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED]
   {0xE0,0x20,0x60,0x80,0x03,0x00,0x68,0x60,0x00,0x08},//std_tbls_write[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED]
   {0x00,0x00,0xC0,0x66,0x7D,0xFC,0x05,0x0E,0x1C,0x02,0x00,0x00,0x00},//mfg_tbls_write[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED]
#else
   {0x21,0x10,0xE0,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0x91,0x00,0x00,0x00},//mfg_tbls_used[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED]
   {0xBB,0x07,0x01},//std_proc_used[GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED]
   {0xB7,0x73,0x05},//mfg_proc_used[GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED]
   {0xE0,0x20,0x60,0x80,0x03,0x00,0x68,0x60,0x00,0x08},//std_tbls_write[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED]
   {0x00,0x00,0xC0,0x62,0x7D,0xFC,0x05,0x0E,0x1C,0x00,0x00,0x00,0x00},//mfg_tbls_write[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED]
#endif
};

_std_table_03_t std_table_03_struct;
_std_table_08_t std_table_08_struct;

//1-data source
const _std_table_10_t std_table_10_struct=
{
     {
        ACT_SOURCES_LIM_TBL_DOT_PF_EXCLUDE_FLAG,                //source_flags.PFExcludeFlag
        ACT_SOURCES_LIM_TBL_DOT_RESET_EXCLUDE_FLAG,             //source_flags.ResetExcludeFlag
        0,                                                      //source_flags.BlockDemandFlag
        ACT_SOURCES_LIM_TBL_DOT_SLIDING_DEMAND_FLAG,            //source_flags.SlidingDemandFlag
        0,                                                      //source_flags.ThermalDemandFlag
        ACT_SOURCES_LIM_TBL_DOT_SET1_PRESENT_FLAG,              //source_flags.Set1PresentFlag
        ACT_SOURCES_LIM_TBL_DOT_SET2_PRESENT_FLAG,              //source_flags.Set2PresentFlag
        0,                                                      //source_flags.Filler
     },
     ACT_SOURCES_LIM_TBL_DOT_NBR_UOM_ENTRIES,                   //nbr_uom_entries
     ACT_SOURCES_LIM_TBL_DOT_NBR_DEMAND_CTRL_ENTRIES,           //nbr_demand_ctrl_entries
     ACT_SOURCES_LIM_TBL_DOT_DATA_CTRL_LENGTH,                  //data_ctrl_length
     ACT_SOURCES_LIM_TBL_DOT_NBR_DATA_CTRL_ENTRIES,             //nbr_data_ctrl_entries
     ACT_SOURCES_LIM_TBL_DOT_NBR_CONSTANTS_ENTRIES,             //nbr_constants_entries
     ACT_SOURCES_LIM_TBL_DOT_CONSTANTS_SELECTOR,                //constants_selector
     ACT_SOURCES_LIM_TBL_DOT_NBR_SOURCES
};

const _std_table_12_t std_table_12_struct=
{
   .uom_entry[0]  = {0,0,0,1,0,0,1,0,5,0,0,0,0},     //+Wh Phase A
   .uom_entry[1]  = {0,0,0,1,0,0,1,0,6,0,0,0,0},     //+Wh Phase B
   .uom_entry[2]  = {0,0,0,1,0,0,1,0,7,0,0,0,0},     //+Wh Phase C
   .uom_entry[3]  = {0,0,0,1,0,0,1,0,0,0,0,0,0},     //+Wh Total
   .uom_entry[4]  = {0,0,0,0,1,1,0,0,5,0,0,0,0},     //-Wh Phase A
   .uom_entry[5]  = {0,0,0,0,1,1,0,0,6,0,0,0,0},     //-Wh Phase B
   .uom_entry[6]  = {0,0,0,0,1,1,0,0,7,0,0,0,0},     //-Wh Phase C
   .uom_entry[7]  = {0,0,0,0,1,1,0,0,0,0,0,0,0},     //-Wh Total
   .uom_entry[8]  = {1,0,0,1,1,0,0,0,5,0,0,0,0},     //+VARh Phase A
   .uom_entry[9]  = {1,0,0,1,1,0,0,0,6,0,0,0,0},     //+VARh Phase B
   .uom_entry[10] = {1,0,0,1,1,0,0,0,7,0,0,0,0},     //+VARh Phase C
   .uom_entry[11] = {1,0,0,1,1,0,0,0,0,0,0,0,0},     //+VARh Total
   .uom_entry[12] = {1,0,0,0,0,1,1,0,5,0,0,0,0},     //-VARh Phase A
   .uom_entry[13] = {1,0,0,0,0,1,1,0,6,0,0,0,0},     //-VARh Phase B
   .uom_entry[14] = {1,0,0,0,0,1,1,0,7,0,0,0,0},     //-VARh Phase C
   .uom_entry[15] = {1,0,0,0,0,1,1,0,0,0,0,0,0},     //-VARh Total
   .uom_entry[16] = {3,0,0,1,0,0,1,0,5,0,0,0,0},     //+VAh Phase A
   .uom_entry[17] = {3,0,0,1,0,0,1,0,6,0,0,0,0},     //+VAh Phase B
   .uom_entry[18] = {3,0,0,1,0,0,1,0,7,0,0,0,0},     //+VAh Phase C
   .uom_entry[19] = {3,0,0,1,0,0,1,0,0,0,0,0,0},     //+VAh Total
   .uom_entry[20] = {3,0,0,0,1,1,0,0,5,0,0,0,0},     //-VAh Phase A
   .uom_entry[21] = {3,0,0,0,1,1,0,0,6,0,0,0,0},     //-VAh Phase B
   .uom_entry[22] = {3,0,0,0,1,1,0,0,7,0,0,0,0},     //-VAh Phase C
   .uom_entry[23] = {3,0,0,0,1,1,0,0,0,0,0,0,0},     //-VAh Total
   .uom_entry[24] = {8,1,5,1,1,1,1,0,5,0,0,0,0},     //V Phase A
   .uom_entry[25] = {8,1,5,1,1,1,1,0,6,0,0,0,0},     //V Phase B
   .uom_entry[26] = {8,1,5,1,1,1,1,0,7,0,0,0,0},     //V Phase C
   .uom_entry[27] = {12,1,5,1,1,1,1,0,5,0,0,0,0},    //I Phase A
   .uom_entry[28] = {12,1,5,1,1,1,1,0,6,0,0,0,0},    //I Phase B
   .uom_entry[29] = {12,1,5,1,1,1,1,0,7,0,0,0,0},    //I Phase C
   .uom_entry[30] = {12,1,5,1,1,1,1,0,4,0,0,0,0},    //I Neutral
   .uom_entry[31] = {33,1,5,1,1,1,1,0,4,0,0,0,0},    //Frequency
   .uom_entry[32] = {21,1,0,1,1,1,1,0,1,0,0,0,0},    //Voltage Angle Phase A
   .uom_entry[33] = {21,1,0,1,1,1,1,0,2,0,0,0,0},    //Voltage Angle Phase B
   .uom_entry[34] = {21,1,0,1,1,1,1,0,3,0,0,0,0},    //Voltage Angle Phase C
   .uom_entry[35] = {25,1,5,1,1,1,1,0,5,0,0,0,0},    //PF Phase A
   .uom_entry[36] = {25,1,5,1,1,1,1,0,6,0,0,0,0},    //PF Phase B
   .uom_entry[37] = {25,1,5,1,1,1,1,0,7,0,0,0,0},    //PF Phase C
   .uom_entry[38] = {25,1,5,1,1,1,1,0,0,0,0,0,0},    //PF Total
   .uom_entry[39] = {0,1,5,1,1,1,1,0,5,0,0,0,0},     //active power Phase A
   .uom_entry[40] = {0,1,5,1,1,1,1,0,6,0,0,0,0},     //active power Phase B
   .uom_entry[41] = {0,1,5,1,1,1,1,0,7,0,0,0,0},     //active power Phase C
   .uom_entry[42] = {0,1,5,1,1,1,1,0,0,0,0,0,0},     //active power Total
   .uom_entry[43] = {3,1,5,1,1,1,1,0,5,0,0,0,0},     //apparent power Phase A
   .uom_entry[44] = {3,1,5,1,1,1,1,0,6,0,0,0,0},     //apparent power Phase B
   .uom_entry[45] = {3,1,5,1,1,1,1,0,7,0,0,0,0},     //apparent power Phase C
   .uom_entry[46] = {3,1,5,1,1,1,1,0,0,0,0,0,0},     //apparent power Total
   .uom_entry[47] = {1,1,5,1,1,1,1,0,5,0,0,0,0},     //reactive power Phase A
   .uom_entry[48] = {1,1,5,1,1,1,1,0,6,0,0,0,0},     //reactive power Phase B
   .uom_entry[49] = {1,1,5,1,1,1,1,0,7,0,0,0,0},     //reactive power Phase C
   .uom_entry[50] = {1,1,5,1,1,1,1,0,0,0,0,0,0},     //reactive power Total
   //TODO: IJD add more UOM entries?
   //time
   //date
   //time and date
   //50 Power outages
   //51 Number of demand resets
   //52 Number of times programmed
   //53 Number of minutes on battery carryover
   //interval timer
   //counter
   //voltage swell
   //voltage sag
   //power outage
   //local currency
   //temperature
};

/*table should not exist
const _std_table_14_t std_table_14_struct=
{
   0,
};*/

const _std_table_15_t std_table_15_struct=
{
   //A bunch of constants for power, we need this because of non-standard
   //units of 0.1V or 0.1 Hz
   .selecion[0] = {10,0},     //voltage phase A
   .selecion[1] = {10,0},     //voltage phase B
   .selecion[2] = {10,0},     //voltage phase B
   .selecion[3] = {10,0},     //frequency

   //A bunch of constants for power, we need this because of non-standard
   //units of 0.1W
   .selecion[4]  = {10,0},    //active power Phase A
   .selecion[5]  = {10,0},    //active power Phase B
   .selecion[6]  = {10,0},    //active power Phase C
   .selecion[7]  = {10,0},    //active power Total
   .selecion[8]  = {10,0},    //apparent power Phase A
   .selecion[9]  = {10,0},    //apparent power Phase B
   .selecion[10] = {10,0},    //apparent power Phase C
   .selecion[11] = {10,0},    //apparent power Total
   .selecion[12] = {10,0},    //reactive power Phase A
   .selecion[13] = {10,0},    //reactive power Phase B
   .selecion[14] = {10,0},    //reactive power Phase C
   .selecion[15] = {10,0},    //reactive power Total
};

const _std_table_16_t std_table_16_struct=
{
   .sources_link[0] =  {1,1,0,0,0,0,0},   //+Wh Phase A
   .sources_link[1] =  {1,1,0,0,0,0,0},   //+Wh Phase B
   .sources_link[2] =  {1,1,0,0,0,0,0},   //+Wh Phase C
   .sources_link[3] =  {1,1,0,0,0,0,0},   //+Wh Total
   .sources_link[4] =  {1,1,0,0,0,0,0},   //-Wh Phase A
   .sources_link[5] =  {1,1,0,0,0,0,0},   //-Wh Phase B
   .sources_link[6] =  {1,1,0,0,0,0,0},   //-Wh Phase C
   .sources_link[7] =  {1,1,0,0,0,0,0},   //-Wh Total
   .sources_link[8] =  {1,1,0,0,0,0,0},   //+VARh Phase A
   .sources_link[9] =  {1,1,0,0,0,0,0},   //+VARh Phase B
   .sources_link[10] = {1,1,0,0,0,0,0},   //+VARh Phase C
   .sources_link[11] = {1,1,0,0,0,0,0},   //+VARh Total
   .sources_link[12] = {1,1,0,0,0,0,0},   //-VARh Phase A
   .sources_link[13] = {1,1,0,0,0,0,0},   //-VARh Phase B
   .sources_link[14] = {1,1,0,0,0,0,0},   //-VARh Phase C
   .sources_link[15] = {1,1,0,0,0,0,0},   //-VARh Total
   .sources_link[16] = {1,1,0,0,0,0,0},   //+VAh Phase A
   .sources_link[17] = {1,1,0,0,0,0,0},   //+VAh Phase B
   .sources_link[18] = {1,1,0,0,0,0,0},   //+VAh Phase C
   .sources_link[19] = {1,1,0,0,0,0,0},   //+VAh Total
   .sources_link[20] = {1,1,0,0,0,0,0},   //-VAh Phase A
   .sources_link[21] = {1,1,0,0,0,0,0},   //-VAh Phase B
   .sources_link[22] = {1,1,0,0,0,0,0},   //-VAh Phase C
   .sources_link[23] = {1,1,0,0,0,0,0},   //-VAh Total
   .sources_link[24] = {1,0,0,1,0,0,0},   //V Phase A
   .sources_link[25] = {1,0,0,1,0,0,0},   //V Phase B
   .sources_link[26] = {1,0,0,1,0,0,0},   //V Phase C
   .sources_link[27] = {1,0,0,0,0,0,0},   //I Phase A
   .sources_link[28] = {1,0,0,0,0,0,0},   //I Phase B
   .sources_link[29] = {1,0,0,0,0,0,0},   //I Phase C
   .sources_link[30] = {1,0,0,0,0,0,0},   //I Neutral
   .sources_link[31] = {1,0,0,1,0,0,0},   //Frequency
   .sources_link[32] = {1,0,0,0,0,0,0},   //Voltage Angle Phase A
   .sources_link[33] = {1,0,0,0,0,0,0},   //Voltage Angle Phase B
   .sources_link[34] = {1,0,0,0,0,0,0},   //Voltage Angle Phase C
   .sources_link[35] = {1,0,0,0,0,0,0},   //PF Phase A
   .sources_link[36] = {1,0,0,0,0,0,0},   //PF Phase B
   .sources_link[37] = {1,0,0,0,0,0,0},   //PF Phase C
   .sources_link[38] = {1,0,0,0,0,0,0},   //PF Total
   .sources_link[39] = {1,0,0,1,0,0,0},   //active power Phase A
   .sources_link[40] = {1,0,0,1,0,0,0},   //active power Phase B
   .sources_link[41] = {1,0,0,1,0,0,0},   //active power Phase C
   .sources_link[42] = {1,0,0,1,0,0,0},   //active power Total
   .sources_link[43] = {1,0,0,1,0,0,0},   //apparent power Phase A
   .sources_link[44] = {1,0,0,1,0,0,0},   //apparent power Phase B
   .sources_link[45] = {1,0,0,1,0,0,0},   //apparent power Phase C
   .sources_link[46] = {1,0,0,1,0,0,0},   //apparent power Total
   .sources_link[47] = {1,0,0,1,0,0,0},   //reactive power Phase A
   .sources_link[48] = {1,0,0,1,0,0,0},   //reactive power Phase B
   .sources_link[49] = {1,0,0,1,0,0,0},   //reactive power Phase C
   .sources_link[50] = {1,0,0,1,0,0,0},   //reactive power Total
};
//2-register tables
const _std_table_20_t std_table_20_struct=
{
   {ACT_REGS_TBL_DOT_SEASON_INFO_FIELD_FLAG,ACT_REGS_TBL_DOT_DATE_TIME_FIELD_FLAG,ACT_REGS_TBL_DOT_DEMAND_RESET_CTR_FLAG,ACT_REGS_TBL_DOT_DEMAND_RESET_LOCK_FLAG,ACT_REGS_TBL_DOT_CUM_DEMAND_FLAG,ACT_REGS_TBL_DOT_CONT_CUM_DEMAND_FLAG,ACT_REGS_TBL_DOT_TIME_REMAINING_FLAG,0},//reg_func1_flags;
   {ACT_REGS_TBL_DOT_SELF_READ_INHIBIT_OVERFLOW_FLAG,ACT_REGS_TBL_DOT_SELF_READ_SEQ_NBR_FLAG,ACT_REGS_TBL_DOT_DAILY_SELF_READ_FLAG,ACT_REGS_TBL_DOT_WEEKLY_SELF_READ_FLAG,ACT_REGS_TBL_DOT_SELF_READ_DEMAND_RESET,0},//reg_func2_flags;
   ACT_REGS_TBL_DOT_NBR_SELF_READS,//nbr_self_reads; Number of Self Reads in use
   ACT_REGS_TBL_DOT_NBR_SUMMATIONS,//nbr_summations; number of summation registers in each data block.
   ACT_REGS_TBL_DOT_NBR_DEMANDS,//nbr_demands; number of demand registers in each data block.
   ACT_REGS_TBL_DOT_NBR_COIN_VALUES,//nbr_coin_values; number of coincident values saved concurrently in each data block.
   ACT_REGS_TBL_DOT_NBR_OCCUR,//nbr_occur; number of occurrences stored for a particular selection.
   ACT_REGS_TBL_DOT_NBR_TIERS,//nbr_tiers; number of tiers that data can be stored in.
   ACT_REGS_TBL_DOT_NBR_PRESENT_DEMANDS,//nbr_present_demands; number of present demand values that can be stored.
   ACT_REGS_TBL_DOT_NBR_PRESENT_VALUES,//nbr_present_values; number of present values that can be stored.
};
const _std_table_27_t std_table_27_struct=
{
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,
   29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,}
};
_std_table_28_t std_table_28_struct;

//3-Local Display
const _std_table_30_t std_table_30_struct =
{
   {
       (ACT_DISP_TBL_DOT_ON_TIME_FLAG)|(ACT_DISP_TBL_DOT_OFF_TIME_FLAG<<1)|(ACT_DISP_TBL_DOT_HOLD_TIME_FLAG<<2),//display_flag_bfld; display bitfields flags

   },
   ACT_DISP_TBL_DOT_NBR_DISP_SOURCES,//nbr_disp_sources; max number of display sources
   ACT_DISP_TBL_DOT_WIDTH_DISPL_SOURCES,//width_disp_sources; max number of octets a manufacturer may use to define the row width of table 32
   ACT_DISP_TBL_DOT_NBR_PRI_DISP_LISTS,//nbr_pri_disp_list_items; max number of primary display items per display list
   ACT_DISP_TBL_DOT_NBR_PRI_DISP_LIST_ITEMS,//nbr_pri_disp_lists; max number of primary display list defined in table 33
   ACT_DISP_TBL_DOT_NBR_SEC_DISP_LISTS,//nbr_sec_disp_list_items; max number of secondary display items per display list defined in table 34
   ACT_DISP_TBL_DOT_NBR_SEC_DISP_LIST_ITEMS,//nbr_sec_display_lists; max number of secondary display lists defined in table 34
};

//5-Time and TOU
const _std_table_50_t std_table_50_struct=
{
   .time_func_flag1.tou_self_read_flag = ACT_TIME_TOU_TBL_DOT_TOU_SELF_READ_FLAG,
   .time_func_flag1.season_self_read_flag = ACT_TIME_TOU_TBL_DOT_SEASON_SELF_READ_FLAG,
   .time_func_flag1.season_demand_reset_flag = ACT_TIME_TOU_TBL_DOT_SEASON_DEMAND_RESET_FLAG,
   .time_func_flag1.season_chng_armed_flag = ACT_TIME_TOU_TBL_DOT_SEASON_CHNG_ARMED_FLAG,
   .time_func_flag1.sort_dates_flag = ACT_TIME_TOU_TBL_DOT_SORT_DATES_FLAG,
   .time_func_flag1.anchor_date_flag = ACT_TIME_TOU_TBL_DOT_ANCHOR_DATE_FLAG,
   .time_func_flag1.filler = FALSE,
   .time_func_flag2.cap_dst_auto_flag = ACT_TIME_TOU_TBL_DOT_CAP_DST_AUTO_FLAG,
   .time_func_flag2.separate_weekdays_flag = ACT_TIME_TOU_TBL_DOT_SEPARATE_WEEKDAYS_FLAG,
   .time_func_flag2.seperate_sum_demands_flag = ACT_TIME_TOU_TBL_DOT_SEPARATE_SUM_DEMANDS_FLAG,
   .time_func_flag2.sort_tier_switches_flag = ACT_TIME_TOU_TBL_DOT_SORT_TIER_SWITCHES_FLAG,
   .time_func_flag2.cap_tm_tz_offset_flag = ACT_TIME_TOU_TBL_DOT_CAP_TM_ZN_OFFSET_FLAG,    // dunno?
   .time_func_flag2.filler = FALSE,
   .calendar_func.nbr_seasons = ACT_TIME_TOU_TBL_DOT_NBR_SEASONS,
   .calendar_func.nbr_special_sched = ACT_TIME_TOU_TBL_DOT_NBR_SPECIAL_SCHED,
   .nbr_non_recurr_dates = ACT_TIME_TOU_TBL_DOT_NBR_NON_RECURR_DATES,
   .nbr_recurr_dates = ACT_TIME_TOU_TBL_DOT_NBR_RECURR_DATES,
   .nbr_tier_switches = ACT_TIME_TOU_TBL_DOT_NBR_TIER_SWITCHES,
   .calendar_tbl_size = sizeof(_std_table_54_t)
};

//_std_table_52_t std_table_52_struct;//this just a subset of table 55
_std_table_55_t std_table_55_struct;
_std_table_56_t std_table_56_struct;

//6-LP
const _std_table_60_t std_table_60_struct = {
    MAXLPSIZE, //LP_Memory_Len,
    { // lp_flags
       TRUE, //lp_set1_inhibit_ovf_flag   :1;//device is inhibiting load profile set 1 once an overflow occurs.
       TRUE, //lp_set2_inhibit_ovf_flag   :1;//device is inhibiting load profile set 2 once an overflow occurs.
       TRUE, //lp_set3_inhibit_ovf_flag   :1;//device is inhibiting load profile set 3 once an overflow occurs.
       TRUE, //lp_set4_inhibit_ovf_flag   :1;//device is inhibiting load profile set 4 once an overflow occurs.
       ACT_LP_TBL_DOT_BLK_END_READ_FLAG, //blk_end_read_flag          :1;//device is providing block register reading information
       ACT_LP_TBL_DOT_BLK_END_PULSE_FLAG, //blk_end_pulse_flag         :1;//device does have a running pulse accumulator
       ACT_LP_TBL_DOT_SCALAR_DIVISOR_FLAG_SET1, //scalar_divisor_flag_set1   :1;//device does have scalars and divisors associated with set 1 LP
       ACT_LP_TBL_DOT_SCALAR_DIVISOR_FLAG_SET1, //scalar_divisor_flag_set2   :1;//device does have scalars and divisors associated with set 2 LP
       ACT_LP_TBL_DOT_SCALAR_DIVISOR_FLAG_SET1, //scalar_divisor_flag_set3   :1;//device does have scalars and divisors associated with set 3 LP
       ACT_LP_TBL_DOT_SCALAR_DIVISOR_FLAG_SET1, //scalar_divisor_flag_set4   :1;//device does have scalars and divisors associated with set 4 LP
       ACT_LP_TBL_DOT_EXTENDED_INT_STATUS_FLAG, //extended_int_status_flag   :1;//device returns extended interval status with load profile interval data
       ACT_LP_TBL_DOT_SIMPLE_INT_STATUS_FLAG, //simple_int_status_flag     :1;//device returns simple interval status with load profile interval data
       0, //filler                     :4;//zero
    },
    { // lp_fmats
#if (LP_CTRL_TBL_DOT_INT_FMT_CDE1 == 2)
       FALSE, //inv_uint8_flag    :1;//format is uint8
       TRUE, //inv_uint16_flag   :1;//format is uint16
       FALSE, //inv_uint32_flag   :1;//format is uint32
       FALSE, //inv_int8_flag     :1;//format is int8
       FALSE, //inv_int16_flag    :1;//format is int16
       FALSE, //inv_int32_flag    :1;//format is int32
       FALSE, //inv_ni_fmat1_flag :1;//format is ni_fmat1 (manufacturer defined)
       FALSE, //inv_ni_fmat2_flag :1;//format is ni_fmat2 (manufacturer defined)
#else
#error dunno if we're supporting that
#endif
    },
    { //Config[NUM_LP_DATA_SETS]
       //Nbr_Blks;//number of data blocks
       //Nbr_blk_ints;//number of intevals per data block
       //Nbr_Chns;//number of channels
       //Max_Int_Time;//interval duration
        // defaults taken from sim.
        /*
        these values are chosen such that 1 block, corresponds to 1 day of halfhour intervals.
        i.e. 269 = 77kB / ( sizeof(timestamp) + (48 intervals)*(bytes per interval for 2 channels inc flags/status))
                 = 77*1024 / (5 + 48*6)
        */
        { ACT_LP_TBL_DOT_NBR_BLKS_SET1, ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1, NUM_CH_PER_DATA_SET, 240 }, // ACT_LP_TBL_DOT_NBR_BLKS_SET1=269, ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1=96
        { ACT_LP_TBL_DOT_NBR_BLKS_SET1, ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1, NUM_CH_PER_DATA_SET, 240 },
        { ACT_LP_TBL_DOT_NBR_BLKS_SET1, ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1, NUM_CH_PER_DATA_SET, 240 },
        { ACT_LP_TBL_DOT_NBR_BLKS_SET1, ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1, NUM_CH_PER_DATA_SET, 240 }
    },
};

_std_table_60_t std_table_61_struct;//no this is not a typoo
_std_table_63_t std_table_63_struct;

//7-History and Event Logs
const _std_table_70_t std_table_70_struct=
{
  {0,1,1,0,0,0},
  ACT_LOG_TBL_DOT_NBR_STD_EVENTS,
  ACT_LOG_TBL_DOT_NBR_MFG_EVENTS,
  ACT_LOG_TBL_DOT_HIST_DATA_LENGTH,
  ACT_LOG_TBL_DOT_EVENT_DATA_LENGTH,
  ACT_LOG_TBL_DOT_NBR_HISTORY_ENTRIES,
  ACT_LOG_TBL_DOT_NBR_EVENT_ENTRIES,
};

const _std_table_72_t std_table_72_struct=
{
  {0x1E,//Std  0- 7
  0xFE, //Std  8-15
  0xCC, //Std 16-23
  0x07, //Std 24-31
  0xF0, //Std 32-39
  0x9F, //Std 40-47
  0x01},//Std 48-55
  {0xFF,//Mfg  0- 7
  0xFF, //Mfg  8-15
  0xFF, //Mfg 16-23
  0xFF, //Mfg 24-31
  0xFF, //Mfg 32-39
  0xFF, //Mfg 40-47
  0xFF, //Mfg 48-55
  0xFF, //Mfg 56-63
  0xFF, //Mfg 64-71
  0x03, //Mfg 72-79
  0x00, //Mfg 80-87
  0x00},//Mfg 88-95
};


//mfg tables
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
_mfg_table_27_t mfg_table_27_struct;
_mfg_table_70_t mfg_table_69_struct_special;
#endif
_mfg_table_24_t mfg_table_24_struct;
_mfg_table_29_t mfg_table_29_struct;
_mfg_table_31_t mfg_table_31_struct;
_mfg_table_33_t mfg_table_33_struct;
_mfg_table_35_t mfg_table_35_struct;
_mfg_table_54_t mfg_table_54_struct;
volatile _mfg_table_62_t mfg_table_62_struct;
_mfg_table_64_t mfg_table_64_struct;
_mfg_table_70_t mfg_table_69_struct_special;
_mfg_table_71_t mfg_table_71_struct;
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
_mfg_table_75_t mfg_table_75_struct;
#endif

//place super important NV data at the start of the memory map so that changes
//to table structures are unlikely to impact on the important stuff during a
//firmware upgrade
#define FUTURE_PROOF_PAGE     (256L)
#define ADDR_EXT_FLASH_MFG060 (0)                                                //metrology cal values
#define ADDR_EXT_FLASH_MFG063 (ADDR_EXT_FLASH_MFG060  +(sizeof(struct _CalData)*MMI3_MAX_ELEMENTS)) //metrology configuration
#define ADDR_EXT_FLASH_MFG053 (ADDR_EXT_FLASH_MFG063  + sizeof(struct _MetrologyConfig)) //RTC cal values
#define ADDR_EXT_FLASH_STD01  (ADDR_EXT_FLASH_MFG053  + sizeof(_mfg_table_53_t)) //MSN
#define ADDR_EXT_FLASH_MFG000 (ADDR_EXT_FLASH_STD01   + sizeof(_std_table_01_t)) //CRC hash table
#define ADDR_EXT_FLASH_MFG039 (ADDR_EXT_FLASH_MFG000  + sizeof(_mfg_table_00_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG057 (ADDR_EXT_FLASH_MFG039  + sizeof(struct _CalData3Phase) + FUTURE_PROOF_PAGE) //communication parameters
#define ADDR_EXT_FLASH_MFG041 (ADDR_EXT_FLASH_MFG057  + sizeof(_mfg_table_57_t) + FUTURE_PROOF_PAGE) //statistics like tamper counters

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(AMPY_METER_U3400)
   #define ADDR_EXT_FLASH_STD02     (ADDR_EXT_FLASH_MFG041  + sizeof(_mfg_table_41_t) + FUTURE_PROOF_PAGE-93)
#else
   #define ADDR_EXT_FLASH_STD02     (ADDR_EXT_FLASH_MFG041  + sizeof(_mfg_table_41_t) + FUTURE_PROOF_PAGE)
#endif
#define ADDR_EXT_FLASH_STD06     (ADDR_EXT_FLASH_STD02   + sizeof(_std_table_02_t) + FUTURE_PROOF_PAGE)
#define DEVICE_ID_IN_STD06       (ADDR_EXT_FLASH_STD06 + offsetof(_std_table_06_t,device_id))
#define ADDR_EXT_FLASH_STD21     (ADDR_EXT_FLASH_STD06   + sizeof(_std_table_06_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD22     (ADDR_EXT_FLASH_STD21   + sizeof(_std_table_20_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD31     (ADDR_EXT_FLASH_STD22   + sizeof(_std_table_22_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD32     (ADDR_EXT_FLASH_STD31   + sizeof(_std_table_31_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD33     (ADDR_EXT_FLASH_STD32   + sizeof(_std_table_32_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD51     (ADDR_EXT_FLASH_STD33   + sizeof(_std_table_33_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD53     (ADDR_EXT_FLASH_STD51   + sizeof(_std_table_50_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG028    (ADDR_EXT_FLASH_STD53   + sizeof(_std_table_53_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_SPARE2    (ADDR_EXT_FLASH_MFG028  + sizeof(_mfg_table_28_t) + FUTURE_PROOF_PAGE)
#define SIZE_EXT_FLASH_SPARE2    (428 - sizeof(_mfg_table_28_t) - FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD55BKP  (ADDR_EXT_FLASH_SPARE2   + SIZE_EXT_FLASH_SPARE2  + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD56BKP  (ADDR_EXT_FLASH_STD55BKP+ sizeof(_std_table_55_t) + FUTURE_PROOF_PAGE+2)
#define ADDR_EXT_FLASH_MFG059    (ADDR_EXT_FLASH_STD56BKP+ sizeof(_std_table_56_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD75     (ADDR_EXT_FLASH_MFG059  + sizeof(_mfg_table_59_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG065    (ADDR_EXT_FLASH_STD75   + sizeof(_std_table_73_t) + FUTURE_PROOF_PAGE) // MetrologyDebug // eMfgT77_NV_Energy.  (4*6 below refers to: 6 sets of energy registers, each consisting of 3 phases + total)
#define ADDR_EXT_FLASH_MFG034    (ADDR_EXT_FLASH_MFG065  + (sizeof(ni_fmat1_t)*4*6)+ FUTURE_PROOF_PAGE)    // fixme: could be less 'brittle'.
#define ADDR_EXT_FLASH_MFG066    (ADDR_EXT_FLASH_MFG034  + sizeof(_mfg_table_34_t) + FUTURE_PROOF_PAGE-37)//Current Relay Configuration
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100) || defined(AMPY_METER_U3400)
   #define ADDR_EXT_FLASH_MFG067    (ADDR_EXT_FLASH_MFG066  + sizeof(_mfg_table_66_t) + FUTURE_PROOF_PAGE-24)//add TOD in T66//Future Relay Configuration-10//FUTURE_PROOF_PAGE-8 because the earlier build was FUTURE_PROOF_PAGE-2,now 6 bytes more used up
   #define ADDR_EXT_FLASH_MFG068    (ADDR_EXT_FLASH_MFG067  + sizeof(_mfg_table_67_t) + FUTURE_PROOF_PAGE-24)//Relay Settings
   #define ADDR_EXT_FLASH_MFG070    (ADDR_EXT_FLASH_MFG068  + sizeof(_mfg_table_68_t) + FUTURE_PROOF_PAGE)//Relay Status Backup
#else
   #define ADDR_EXT_FLASH_MFG067    (ADDR_EXT_FLASH_MFG066  + sizeof(_mfg_table_66_t) + FUTURE_PROOF_PAGE-60)//Future Relay Configuration
   #define ADDR_EXT_FLASH_MFG068    (ADDR_EXT_FLASH_MFG067  + sizeof(_mfg_table_67_t) + FUTURE_PROOF_PAGE-60)//Relay Settings
   #define ADDR_EXT_FLASH_MFG070    (ADDR_EXT_FLASH_MFG068  + sizeof(_mfg_table_68_t) + FUTURE_PROOF_PAGE-50)//Relay Status Backup
#endif

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
#define ADDR_EXT_FLASH_MFG030    (ADDR_EXT_FLASH_MFG070  + sizeof(_mfg_table_70_t) + FUTURE_PROOF_PAGE-5)
#else
#define ADDR_EXT_FLASH_MFG030    (ADDR_EXT_FLASH_MFG070  + sizeof(_mfg_table_70_t) + FUTURE_PROOF_PAGE-2)
#endif
#define ADDR_EXT_FLASH_MFG038    (ADDR_EXT_FLASH_MFG030  + sizeof(_mfg_table_30_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG037    (ADDR_EXT_FLASH_MFG038  + sizeof(_mfg_table_38_t) + 32)

#define ADDR_EXT_FLASH_SPARE1    (ADDR_EXT_FLASH_MFG037  + sizeof(_mfg_table_37_t) + 32)
#define SIZE_EXT_FLASH_SPARE1    (sizeof(_mfg_table_31_t) + FUTURE_PROOF_PAGE + 128 + FUTURE_PROOF_PAGE -sizeof(_mfg_table_38_t) -32 -sizeof(_mfg_table_37_t) -32)

#define ADDR_EXT_FLASH_STD13     (ADDR_EXT_FLASH_SPARE1  + SIZE_EXT_FLASH_SPARE1)
#define ADDR_EXT_FLASH_MFG058    (ADDR_EXT_FLASH_STD13   + sizeof(_std_table_13_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG056    (ADDR_EXT_FLASH_MFG058  + sizeof(_mfg_table_58_t) + FUTURE_PROOF_PAGE)

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
   #define ADDR_EXT_FLASH_MFG055    (ADDR_EXT_FLASH_MFG056  + sizeof(_mfg_table_56_t) + FUTURE_PROOF_PAGE-103)
#else
   #define ADDR_EXT_FLASH_MFG055    (ADDR_EXT_FLASH_MFG056  + sizeof(_mfg_table_56_t) + FUTURE_PROOF_PAGE-92)
#endif

#define ADDR_EXT_FLASH_MFG072    (ADDR_EXT_FLASH_MFG055  + sizeof(_std_table_63_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD61     (ADDR_EXT_FLASH_MFG072  + sizeof(_mfg_table_54_t) + FUTURE_PROOF_PAGE-247)
#define ADDR_EXT_FLASH_STD62     (ADDR_EXT_FLASH_STD61   + sizeof(_std_table_60_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG052    (ADDR_EXT_FLASH_STD62   + sizeof(_std_table_62_t) + FUTURE_PROOF_PAGE-24)
#define ADDR_EXT_FLASH_MFG050    (ADDR_EXT_FLASH_MFG052  + sizeof(_mfg_table_52_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG051    (ADDR_EXT_FLASH_MFG050  + sizeof(_mfg_table_50_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG049    (ADDR_EXT_FLASH_MFG051  + sizeof(_mfg_table_51_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG048    (ADDR_EXT_FLASH_MFG049  + sizeof(_mfg_table_71_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG047    (ADDR_EXT_FLASH_MFG048  + sizeof(_mfg_table_46_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG046    (ADDR_EXT_FLASH_MFG047  + sizeof(_mfg_table_46_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG045    (ADDR_EXT_FLASH_MFG046  + sizeof(_mfg_table_46_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG044    (ADDR_EXT_FLASH_MFG045  + sizeof(_mfg_table_43_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG043    (ADDR_EXT_FLASH_MFG044  + sizeof(_mfg_table_43_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG042    (ADDR_EXT_FLASH_MFG043  + sizeof(_mfg_table_43_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD54     (ADDR_EXT_FLASH_MFG042  + sizeof(_mfg_table_42_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG036    (ADDR_EXT_FLASH_STD54   + sizeof(_std_table_54_t) + FUTURE_PROOF_PAGE)
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   #define ADDR_EXT_FLASH_MFG073    (ADDR_EXT_FLASH_MFG036  + sizeof(_mfg_table_36_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_SPARE3    (ADDR_EXT_FLASH_MFG073  + sizeof(_mfg_table_73_t) + FUTURE_PROOF_PAGE-198)
   #define SIZE_EXT_FLASH_SPARE3    (1152 - sizeof(_mfg_table_73_t)-FUTURE_PROOF_PAGE) //954 was size of old MT74, MT74 and MT26 +3*FUTURE_PROOF_PAGE.
   #define ADDR_EXT_FLASH_MFG025    (ADDR_EXT_FLASH_SPARE3  + SIZE_EXT_FLASH_SPARE3   + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG021    (ADDR_EXT_FLASH_MFG025  + sizeof(_mfg_table_25_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG022    (ADDR_EXT_FLASH_MFG021  + sizeof(_mfg_table_24_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG023    (ADDR_EXT_FLASH_MFG022  + sizeof(_mfg_table_22_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG026    (ADDR_EXT_FLASH_MFG023  + sizeof(_mfg_table_23_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG020    (ADDR_EXT_FLASH_MFG026  + sizeof(_mfg_table_26_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG019    (ADDR_EXT_FLASH_MFG020  + sizeof(_mfg_table_20_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG015    (ADDR_EXT_FLASH_MFG019  + sizeof(_mfg_table_19_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG016    (ADDR_EXT_FLASH_MFG015  + sizeof(_mfg_table_15_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG081    (ADDR_EXT_FLASH_MFG016  + sizeof(_mfg_table_16_t) + FUTURE_PROOF_PAGE)
#else
   #define ADDR_EXT_FLASH_MFG021    (ADDR_EXT_FLASH_MFG036  + sizeof(_mfg_table_36_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG022    (ADDR_EXT_FLASH_MFG021  + sizeof(_mfg_table_24_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG023    (ADDR_EXT_FLASH_MFG022  + sizeof(_mfg_table_22_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG025    (ADDR_EXT_FLASH_MFG023  + sizeof(_mfg_table_23_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG019    (ADDR_EXT_FLASH_MFG025  + sizeof(_mfg_table_25_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG015    (ADDR_EXT_FLASH_MFG019  + sizeof(_mfg_table_19_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG016    (ADDR_EXT_FLASH_MFG015  + sizeof(_mfg_table_15_t) + FUTURE_PROOF_PAGE)
   #define ADDR_EXT_FLASH_MFG081    (ADDR_EXT_FLASH_MFG016  + sizeof(_mfg_table_16_t) + FUTURE_PROOF_PAGE)
#endif
#define ADDR_EXT_FLASH_MFG005    (ADDR_EXT_FLASH_MFG081  + sizeof(_mfg_table_81_t) + FUTURE_PROOF_PAGE)
//place new table address # defs above this line and adjust the line below accordingly
#define ADDR_EXT_FLASH_END       (ADDR_EXT_FLASH_MFG005  + sizeof(_mfg_table_05_t) + FUTURE_PROOF_PAGE + AEMTABLESTARTADDR - 1)


// special tables in other segments of serial data flash

//TOU data
#define ADDR_EXT_FLASH_STD23     (0)
#define ADDR_EXT_FLASH_STD24     (ADDR_EXT_FLASH_STD23   + sizeof(_std_table_23_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD25     (ADDR_EXT_FLASH_STD24   + sizeof(_std_table_24_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_STD26     (ADDR_EXT_FLASH_STD25   + sizeof(_std_table_25_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_FLASH_MFG040    (ADDR_EXT_FLASH_STD26   + sizeof(_std_table_26_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_TOU_FLASH_END   (ADDR_EXT_FLASH_MFG040   + sizeof(_mfg_table_40_t) + FUTURE_PROOF_PAGE + AEMTOUSTARTADDR - 1)

//Event log Data
#define ADDR_EXT_FLASH_STD76     (0)
#define ADDR_EXT_LOG_FLASH_END   (ADDR_EXT_FLASH_STD76   + sizeof(_std_table_76_t) + FUTURE_PROOF_PAGE + AEMLOGSTARTADDR - 1)

//LP data
#define ADDR_EXT_FLASH_STD64     (0)
#define ADDR_EXT_LP_FLASH_END    (ADDR_EXT_FLASH_STD64   + sizeof(_lp_blk1_dat_rcd_t)*ACT_LP_TBL_DOT_NBR_BLKS_SET1 + FUTURE_PROOF_PAGE + AEMLPDATASTARTADDR - 1)

//Ripple Event Log
#define ADDR_EXT_FLASH_MFG074    (0)
#define ADDR_EXT_RIPPLE_LOG_END  (ADDR_EXT_FLASH_MFG074  + sizeof(_mfg_table_74_t_) + AEMPRIPPLELOGSTARTADDR -1)

#define ADDR_EXT_FLASH_MFG095    (0)
#define ADDR_EXT_FLASH_MFG096    (ADDR_EXT_FLASH_MFG095 + sizeof(_mfg_table_95_t) + FUTURE_PROOF_PAGE)
#define ADDR_EXT_LPQoS_FLASH_END (ADDR_EXT_FLASH_MFG096 + sizeof(_mfg_table_96_t) + FUTURE_PROOF_PAGE + AEMLPQOSSTARTADDR - 1)

const _TableInfoStruct TableConfig[] =
{
   // Standard Tables
   {eStdT00_GenConfig,     sizeof(_std_table_00_t),eConst,     (void HUGE*)&std_table_00_struct,NULL},
   {eStdT01_GenMfgID,      sizeof(_std_table_01_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD01,ST01_update},
   {eStdT02_DevNamePlate,  sizeof(_std_table_02_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD02,Metrology_InitPolyPhaseMeterFlag},
   {eStdT03_EDMODEStatus,  sizeof(_std_table_03_t),eRAM,       &std_table_03_struct,            NULL},
   //eStdT04_ not required
   {eStdT05_DeviceID,      sizeof(_std_table_05_t),eExtFlash,  (void HUGE*)DEVICE_ID_IN_STD06,  NULL},
   {eStdT06_UtilityInfo,   sizeof(_std_table_06_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD06,NULL},
   {eStdT07_ProcInit,      sizeof(_std_table_07_t),eDynamic,   NULL,                            NULL},
   {eStdT08_ProcResponse,  sizeof(_std_table_08_t),eRAM,       &std_table_08_struct,            NULL},
   //eStdT09_ not required
   {eStdT10_DimSrcLimit,   sizeof(_std_table_10_t),eConst,     (void HUGE*)&std_table_10_struct,NULL},
   {eStdT11_ActualSrcLimit,sizeof(_std_table_10_t),eConst,     (void HUGE*)&std_table_10_struct,NULL},//same as tbl 10
   {eStdT12_UnitOfMeasure, sizeof(_std_table_12_t),eConst,     (void HUGE*)&std_table_12_struct,NULL},
   {eStdT13_DemandCtrl,    sizeof(_std_table_13_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD13,TOU_dmd_cfg_update},
   {eStdT15_Constants,     sizeof(_std_table_15_t),eConst,     (void HUGE*)&std_table_15_struct,NULL},
   {eStdT16_SrcDef,        sizeof(_std_table_16_t),eConst,     (void HUGE*)&std_table_16_struct,NULL},
   //eStdT17_ not required
   //eStdT18_ not required
   //eStdT19_ not required
   {eStdT20_DimRegLimit,   sizeof(_std_table_20_t),eConst,     (void HUGE*)&std_table_20_struct,NULL},
   {eStdT21_ActualReg,     sizeof(_std_table_20_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD21,TOU_cfg_update},//const?
   {eStdT22_DataSelection, sizeof(_std_table_22_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD22,TOU_sel_update},
   {eStdT23_CurrRegData,   sizeof(_std_table_23_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_STD23,NULL},
   {eStdT24_PrevSeaData,   sizeof(_std_table_24_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_STD24,NULL},
   {eStdT25_PrevDmdRstData,sizeof(_std_table_25_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_STD25,NULL},
   {eStdT26_SelfReadData,  sizeof(_std_table_26_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_STD26,NULL},
   {eStdT27_PresRegSel,    sizeof(_std_table_27_t),eConst,     (void HUGE*)&std_table_27_struct,NULL},
   {eStdT28_PresRegData,   sizeof(_std_table_28_t),eRAM,       &std_table_28_struct,            NULL},
   //eStdT28_ not required
   {eStdT30_DimDisplayLim, sizeof(_std_table_30_t),eConst,     (void HUGE*)&std_table_30_struct,NULL},
   {eStdT31_ActualDisplay, sizeof(_std_table_31_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD31,Displaytable_update},
   {eStdT32_DisplaySrc,    sizeof(_std_table_32_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD32,NULL},
   {eStdT33_PrimDispLst,   sizeof(_std_table_33_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD33,Displaytable_update},
   //eStdT34_ not required
   //eStdT35_ not required
   //eStdT36_ not required
   //eStdT37_ not required
   //eStdT38_ not required
   //eStdT39_ not required
   //eStdT40_ not required
   //eStdT41_ not required
   //eStdT42_ not required
   //eStdT43_ not required
   //eStdT44_ not required
   //eStdT45_ not required
   //eStdT46_ not required
   //eStdT47_ not required
   //eStdT48_ not required
   //eStdT49_ not required
   {eStdT50_DimLimTnTOU,   sizeof(_std_table_50_t),eConst,     (void HUGE*)&std_table_50_struct,NULL},
   {eStdT51_ActLimTnTOU,   sizeof(_std_table_50_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD51,NULL},
   {eStdT52_Clock,         sizeof(_std_table_52_t),eRAM,       &std_table_55_struct,            RTC_T52_T55_Write},//table 52 is a sub-set of table 55
   {eStdT53_TimeOffset,    sizeof(_std_table_53_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD53,RTC_T53_Write},
   {eStdT54_Calender,      sizeof(_std_table_54_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD54,TOU_switch_update},
   {eStdT55_ClockState,    sizeof(_std_table_55_t),eRAM,       &std_table_55_struct,            RTC_T52_T55_Write},//needs NV backup
   {eStdT56_TimeRemaing,   sizeof(_std_table_56_t),eRAM,       &std_table_56_struct,            NULL},//needs NV backup
   //eStdT57_ not required
   //eStdT58_ not required
   //eStdT59_ not required
   {eStdT60_DimLimitLP,    sizeof(_std_table_60_t),eConst,     (void HUGE *)&std_table_60_struct, NULL},
   {eStdT61_ActualLPLimit, sizeof(_std_table_60_t),eExtFlash,  (void HUGE *)ADDR_EXT_FLASH_STD61, LP_TableUpdate},
   {eStdT62_LPControl,     sizeof(_std_table_62_t),eExtFlash,  (void HUGE *)ADDR_EXT_FLASH_STD62, LP_TableUpdate},
   {eStdT63_LPStatus,      sizeof(_std_table_63_t),eRAM,       &std_table_63_struct,            NULL},
   {eStdT64_LPDataSet1,    MAXLPSIZE,eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,NULL},
   {eStdT65_LPDataSet2,    MAXLPSIZE,eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,NULL},
   {eStdT66_LPDataSet3,    MAXLPSIZE,eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,NULL},
   {eStdT67_LPDataSet4,    MAXLPSIZE,eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,NULL},
   //eStdT68_ not required
   //eStdT69_ not required
   {eStdT70_LimLogDim,     sizeof(_std_table_70_t),eConst,     (void HUGE*)&std_table_70_struct,NULL},
   {eStdT71_ActLogDim,     sizeof(_std_table_70_t),eConst,     (void HUGE*)&std_table_70_struct,NULL},//same as tbl 70
   {eStdT72_EventID,       sizeof(_std_table_72_t),eConst,     (void HUGE*)&std_table_72_struct,NULL},
   //eStdT73_HistLogCtl not required
   //eStdT74_HistLogData not required
   {eStdT75_EventLogCtl,   sizeof(_std_table_73_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_STD75,ExtLogCtrl_update},
   {eStdT76_EventLogData,  sizeof(_std_table_76_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_STD76,NULL},
   //eStdT77_ not required
   //eStdT78_ not required
   //eStdT79_ not required
   //--------End of standard tables---------


   // Manufacturing Tables
   {eMfgT00_Hash,             sizeof(_mfg_table_00_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG000,  NULL},//2048 Data validity hash check values
   {eMfgT05_MeterFeatures,    sizeof(_mfg_table_05_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG005,  Displaytable_update},//2053
   {eMfgT12_Status,           39/*_mfg_table_12_t*/,  eDynamic,   NULL,                               NULL},//2060
   {eMfgT15_PrePayDisplay,    sizeof(_mfg_table_15_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG015,  PrePayDisplayUpdate},//2063
   {eMfgT16_PrePayConfig,     sizeof(_mfg_table_16_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG016,  PrePayConfigUpdate},//2064
   {eMfgT17_FETWrite,         sizeof(_mfg_table_17_t),eDynamic,   NULL,                               NULL},//2065 Factory Encrypted Table Write
   {eMfgT18_FETRead,          sizeof(_mfg_table_17_t),eDynamic,   NULL,                               NULL},//2066 Factory Encrypted Table Read
#if defined(MAGNETIC_TAMPER)
   {eMfgT19_MagneticFieldCal, sizeof(_mfg_table_19_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG019,  metrology_driver_dc_mag_update},//2067
#endif
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   {eMfgT20_RippleStatusBackup, sizeof(_mfg_table_20_t),eExtFlash,(void HUGE*)ADDR_EXT_FLASH_MFG020,  NULL},//2068 Ripple Status Backup
#endif
   {eMfgT21_TempAppStatusBckp,sizeof(_mfg_table_24_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG021,  NULL},//2069
   {eMfgT22_TempCalData,      sizeof(_mfg_table_22_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG022,  SysMon_MfgT22TempCalUpdate},//2070
   {eMfgT23_TempAppCfg,       sizeof(_mfg_table_23_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG023,  SysMon_MfgT23TempAppCfgUpdate },//2071
   {eMfgT24_TempAppStatus,    sizeof(_mfg_table_24_t),eRAM,       &mfg_table_24_struct,               NULL},//2072
   {eMfgT25_CustFeatureCfg,   sizeof(_mfg_table_25_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG025,  MCom_CustomerConfigUpdate},//2073
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   {eMfgT26_LCConfg,          sizeof(_mfg_table_26_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG026,  RAppLC_UpdateConfig},//2074
   {eMfgT27_LCStatus,         sizeof(_mfg_table_27_t),eRAM     ,  &mfg_table_27_struct,               NULL},//2075
#endif
   {eMfgT28_NIBackup,         sizeof(_mfg_table_28_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG028,  NULL},//2076
   {eMfgT29_CommsRAMData,     sizeof(_mfg_table_29_t),eRAM,       &mfg_table_29_struct,               NULL},//2077
   {eMfgT30_FwDlCtrl,         sizeof(_mfg_table_30_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG030,  FwDlCtrlUpdate},//2078
   {eMfgT31_FwDlStatus,       sizeof(_mfg_table_31_t),eRAM,       &mfg_table_31_struct,               NULL},//2079
   {eMfgT32_FwDlDataBlk,      sizeof(_mfg_table_32_t),eDynamic,   NULL,                               NULL},//2080
   {eMfgT33_NIRAMData,        sizeof(_mfg_table_33_t),eRAM,       &mfg_table_33_struct,               NULL},//2081
#if defined(NEUTRAL_INTEGRITY_POLY)
   {eMfgT34_NIConfig,         sizeof(_mfg_table_34_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG034,  PolyPhaseNI_NITblUpdate},//2082
#else
   {eMfgT34_NIConfig,         sizeof(_mfg_table_34_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG034,  Metrology_NITblUpdate},//2082
#endif
   {eMfgT35_CommsDisplayList, sizeof(_mfg_table_35_t),eRAM,       &mfg_table_35_struct,               CommsDisplayReady},//2083
   {eMfgT36_LP_QOS_Cfg,       sizeof(_mfg_table_36_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG036,  LP_TableUpdate},//2084
   {eMfgT37_PwdLockoutCfg,    sizeof(_mfg_table_37_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG037,  NULL},//2085
   {eMfgT38_ReverseEngyCfg,   sizeof(_mfg_table_38_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG038,  NULL},//2086
   {eMfgT39_CalData3Phase,    sizeof(struct _CalData3Phase), eExtFlash, (void HUGE *)ADDR_EXT_FLASH_MFG039, Calibration_update},//2087
   {eMfgT40_TOUHistory,       sizeof(_mfg_table_40_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG040,  NULL},//2088
   {eMfgT41_ActivityStats,    sizeof(_mfg_table_41_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG041,  NULL},//2089
   {eMfgT42_CutomerCMAC,      sizeof(_mfg_table_42_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG042,  MCom_Dummy},//2090
   {eMfgT43_PSEMPermission,   sizeof(_mfg_table_43_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG043,  MCom_Dummy},//2091
   {eMfgT44_PSEMXPermission,  sizeof(_mfg_table_43_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG044,  MCom_Dummy},//2092
   {eMfgT45_KF2Permission,    sizeof(_mfg_table_43_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG045,  MCom_Dummy},//2093
   {eMfgT46_PSEMPwdKeys,      sizeof(_mfg_table_46_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG046,  MCom_Dummy},//2094
   {eMfgT47_PSEMXPwdKeys,     sizeof(_mfg_table_46_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG047,  MCom_Dummy},//2095
   {eMfgT48_KF2PwdKeys,       sizeof(_mfg_table_46_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG048,  MCom_Dummy},//2096
   {eMfgT49_AsyncEventNV,     sizeof(_mfg_table_71_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG049,  NULL},//2097
   {eMfgT50_QOSConfig,        sizeof(_mfg_table_50_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG050,  QoS_ConfigUpdate},//2098
   {eMfgT51_QOSEvent,         sizeof(_mfg_table_51_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG051,  NULL},//2099
   {eMfgT52_MfgConst,         sizeof(_mfg_table_52_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG052,  MT52_MfgConstUpdate},//2100
   {eMfgT53_OtherCal,         sizeof(_mfg_table_53_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG053,  NULL},//2101
   {eMfgT54_GnrlMeterNVStatus,sizeof(_mfg_table_54_t),eRAM,       (void HUGE*)&mfg_table_54_struct,   NULL},//2102
   {eMfgT55_LPStatusBkp,      sizeof(_std_table_63_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG055,  NULL},//2103
   {eMfgT56_LPState,          sizeof(_mfg_table_56_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG056,  NULL},//2104
   {eMfgT57_BackEndCommsCfg,  sizeof(_mfg_table_57_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG057,  BackEndComms_update},//2105
   {eMfgT58_ExtEventCtrl,     sizeof(_mfg_table_58_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG058,  ExtEvent_update},//2106
   {eMfgT59_DisplayID,        sizeof(_mfg_table_59_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG059,  Displaytable_update},//2107
   {eMfgT60_CalData,          sizeof(struct _CalData)*MMI3_MAX_ELEMENTS,eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG060, Calibration_update},//2108
   {eMfgT61_MetrologyRAM,     sizeof(struct _MetrologyRAM)*MMI3_MAX_ELEMENTS, eRAM, &MetrologyRAM,    NULL},//2109
   {eMfgT62_GenericRAM,       sizeof(_mfg_table_62_t),eDynamic,   (void HUGE*)&mfg_table_62_struct,   NULL},//2110
   {eMfgT63_MetrologyConfig,  sizeof(struct _MetrologyConfig), eExtFlash, (void HUGE *)ADDR_EXT_FLASH_MFG063, Calibration_update},//2111
   {eMfgT64_TestStatus,       sizeof(_mfg_table_64_t),eRAM,       &mfg_table_64_struct,               NULL},//2112
   {eMfgT65_NV_Energy,       (sizeof(ni_fmat1_t)*4*6),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG065,  NULL},//2113
   {eMfgT66_ActiveRelayConfig,sizeof(_mfg_table_66_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG066,  RelaySettingsUpdate},//2114
   {eMfgT67_FutureRelayConfig,sizeof(_mfg_table_67_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG067,  RelaySettingsUpdate67},//2115
   {eMfgT68_RelaySettings,    sizeof(_mfg_table_68_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG068,  RelaySettingsUpdate68},//2116
   {eMfgT69_RelayStatus,      sizeof(_mfg_table_69_t),eRAM,       &mfg_table_69_struct_special,       NULL},//2117
   {eMfgT70_RelayStatusBackup,sizeof(_mfg_table_70_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG070,  NULL},//2118
   {eMfgT71_AsyncEventPin,    sizeof(_mfg_table_71_t),eRAM,   	   &mfg_table_71_struct,		         NULL},//2119 note we don't use the 'internal' version of the struct here...
   {eMfgT72_GnrlMeterNVStatusBckp,sizeof(_mfg_table_54_t),eExtFlash,(void HUGE*)ADDR_EXT_FLASH_MFG072,NULL},//2120
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   {eMfgT73_RippleConfig,     sizeof(_mfg_table_73_t),eExtFlash,(void HUGE*)ADDR_EXT_FLASH_MFG073,ProgRipple},
   {eMfgT74_RippleEvtLog,  sizeof(_mfg_table_74_t),eDynamic,   (void HUGE*)ADDR_EXT_FLASH_MFG074, NULL},
   {eMfgT75_RippleResults,    sizeof(_mfg_table_75_t),eRAM,   	   &mfg_table_75_struct,		         NULL},
#endif
   {eMfgT76_LPStateRAM,       sizeof(_mfg_table_56_t),eRAM,       &gLPState,                          NULL},//2124
   {eMfgT79_CrashLog,         AMPY_NV_CRASH_MAX_CHARS,eDynamic,   NULL,                               NULL},//2127
   {eMfgT80_PwrCycTiming,     264,                    eDynamic,   NULL,                               NULL},//2128
#if defined(PULSE_OUTPUT)
   {eMfgT81_PulseOutput,      sizeof(_mfg_table_81_t),eExtFlash,  (void HUGE*)ADDR_EXT_FLASH_MFG081,  PulseOutputCfg_update},//2129
#endif
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
   {eMfgT90_PartialLPDataSet1,MAXLPSIZE,              eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,    NULL},
   {eMfgT91_PartialLPDataSet2,MAXLPSIZE,              eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,    NULL},
   {eMfgT92_PartialLPDataSet3,MAXLPSIZE,              eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,    NULL},
   {eMfgT93_PartialLPDataSet4,MAXLPSIZE,              eDynamic,  (void HUGE*)ADDR_EXT_FLASH_STD64,    NULL},
#endif
#if DBG_DIAGTABLE
   {eMfgT94_DiagnosticsData,  sizeof(_mfg_table_94_t),eRAM,       &mfg_table_94_struct,               Diagnostics_update},//2124
#endif
   {eMfgT95_LPQoSConfig,      sizeof(_mfg_table_95_t),eDynamic,  (void HUGE*)ADDR_EXT_FLASH_MFG095,  LpQos_CfgChanged},   
   {eMfgT96_LPQoSData,        sizeof(_mfg_table_96_t),eDynamic,  (void HUGE*)ADDR_EXT_FLASH_MFG096,  NULL},
   // Kingfisher Classes (really just an extension of Mfg Tables)
   {eCls140_RealTime, sizeof(_RealTime),eRAM, &RealTime,                                              NULL},


   // End marker
   {eMaxTables, 0,eRAM,0,NULL}
};

unsigned short TableSizeOK(unsigned short TableNum, unsigned long Offset, unsigned short Size)
{
   unsigned char TableEntryNum;
   unsigned short ret;

   TableEntryNum = TableFindEntry(TableNum);
   if(TableEntryNum == 0xFF)
      return 0;

   ret=TableCheckSize(TableNum,TableEntryNum,Offset,Size);
   if(ret==0)
   {//offset + size is too big
      ret=TableCheckSize(TableNum,TableEntryNum,Offset,1);
      if(ret==0)
         ret=0;//offset is too big, you're on your own
      else
         ret=TableCheckSize(TableNum,TableEntryNum,0,0)-Offset;//offset OK, return remaing data
   }
   return ret;
}

unsigned long TableCheckSize(unsigned short TableNum,unsigned char TableEntryNum, unsigned long Offset, unsigned short Size)
{
	unsigned long Siz = 0;

   //compute size for varaible size tables
   switch(TableNum)
   {
      case eStdT22_DataSelection:
      case eStdT23_CurrRegData:
      case eStdT24_PrevSeaData:
      case eStdT26_SelfReadData:
      case eMfgT40_TOUHistory:
         Siz=TOU_GetTableSize(TableNum);
         break;
      case eStdT64_LPDataSet1:
      case eStdT65_LPDataSet2:
      case eStdT66_LPDataSet3:
      case eStdT67_LPDataSet4:
         Siz=LP_GetTableSize(TableNum);
         break;
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
      case eMfgT90_PartialLPDataSet1:
      case eMfgT91_PartialLPDataSet2:
      case eMfgT92_PartialLPDataSet3:
      case eMfgT93_PartialLPDataSet4:
         Siz=LP_CalcPartialTableSize(TableNum);
         break;
#endif
      default:
         Siz=TableConfig[TableEntryNum].Size;
   }

	if(((unsigned long)Size + Offset) > Siz)
               return(0);//Size = Siz - Offset;

	if ( !((Size == 0) && (Offset==0)) )
		Siz=Size;
	return(Siz);
}
unsigned char SemHeadPad=0xCC;
SEMAPHORE tableRAMAccess;
unsigned char SemTailPad=0xDD;

unsigned short TableRead(unsigned short TableNum, unsigned long Offset, unsigned short Size, void *Buffer)
{
   unsigned char TableEntryNum;
   unsigned short RetVal = 0;
   TableEntryNum = TableFindEntry(TableNum);
   if(TableEntryNum == 0xFF)
   {
      return 0xFFFF;
   }

   Size = TableCheckSize(TableNum,TableEntryNum, Offset, Size);

   if(Size)
   {
      // TableFindEntry returned a valid table
      switch(TableConfig[TableEntryNum].Type)
      {
         case(eDynamic):  // eventually dynamic reads will be implemented, for now just treat like a RAM table
            if(TableNum==eStdT23_CurrRegData)
               RetVal=TOU_TblReadHook(Buffer, Offset, Size, (unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTOUSTARTADDR));
            else if((TableNum==eStdT24_PrevSeaData)||(TableNum==eStdT25_PrevDmdRstData)||(TableNum==eStdT26_SelfReadData)||(TableNum==eMfgT40_TOUHistory))
               RetVal=flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTOUSTARTADDR)+Offset, Size);
            else if((TableNum>=eStdT64_LPDataSet1)&&(TableNum<=eStdT67_LPDataSet4))
            {//special case for LP
               Offset += LP_offset_adjust(TableNum-eStdT64_LPDataSet1);
               RetVal=LP_flashRead(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPDATASTARTADDR)+Offset, Size,TableNum-eStdT64_LPDataSet1);
               // LP_time_read will return if time of active block isnt being read.
               // otherwise it gets filled in.
               if (gLPState.sets[TableNum-eStdT64_LPDataSet1].ActiveInterval != 0)
                  LP_time_read(Buffer, Offset, Size,TableNum-eStdT64_LPDataSet1);
               RetVal = Size;
            }
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
            else if((TableNum>=eMfgT90_PartialLPDataSet1)&&(TableNum<=eMfgT93_PartialLPDataSet4))
            {//special case for partial LP
               // we can assume that there is at least some data to read, since TableCheckSize returned a non-zero value
               Offset += LP_offset_adjust(TableNum-eMfgT90_PartialLPDataSet1);
               RetVal=LP_partialFlashRead(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPDATASTARTADDR),Offset,Size,TableNum-eMfgT90_PartialLPDataSet1);
            }
#endif
            else if(TableNum==eStdT76_EventLogData)
            {
               if ((Offset+Size)>(sizeof(_std_table_76_hdr_t)))
						RetVal=flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLOGSTARTADDR)+Offset, Size);
               EventTblReadHook(Buffer, Offset, Size);
            }
            else if(TableNum==eMfgT12_Status)
            {
               BuildDynamicMfg12(Buffer,Offset,Size);
               RetVal=Size;
            }
            else if(TableNum==eMfgT05_MeterFeatures)
            {
               memset(Buffer,0,Size);
               RetVal=Size;
            }
            else if(TableNum==eMfgT41_ActivityStats)
            {
               RetVal=MStatReadActStatTbl(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR),Offset, Size);
            }
            else if((TableNum==eMfgT43_PSEMPermission)||(TableNum==eMfgT44_PSEMXPermission)||(TableNum==eMfgT45_KF2Permission))
            {
               flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR)+Offset, Size);
               RetVal = Size;
            }
            else if((TableNum==eMfgT46_PSEMPwdKeys)||(TableNum==eMfgT47_PSEMXPwdKeys)||(TableNum==eMfgT48_KF2PwdKeys))
            {
               RetVal=MCom_TblReadKeys(TableNum,Offset,Size,Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR));
            }
            else if(TableNum==eMfgT62_GenericRAM)
            {
               RetVal = disable_save();
               memcpy(Buffer, ((unsigned char*)TableConfig[TableEntryNum].Address + Offset), Size);
               enable_restore(RetVal);
               RetVal = Size;
            }
            else if(TableNum==eMfgT79_CrashLog)
            {
               RetVal=flashReadMeter(Buffer,(AEMCRASHLOGSTARTADDR)+Offset, Size);
            }
            else if(TableNum==eMfgT80_PwrCycTiming)
            {
               DataFlashReadPageBytes(AEMFLASHSTARTPAGE + AEMTOTALPAGES,0,Buffer,Size);
               RetVal = Size;
            }
            else if(TableNum==eMfgT15_PrePayDisplay)
            {
              memcpy(Buffer, (&PrePayDisplay + Offset), Size);
              RetVal = Size;
            }
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
            else if(TableNum==eMfgT74_RippleEvtLog)
            {
              RetVal=flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMPRIPPLELOGSTARTADDR)+Offset, Size);
              RippleApp_EventReadHook(Buffer, Offset, Size);
            }
#endif
           else if(TableNum==eMfgT51_QOSEvent)
           {
             memcpy(Buffer,(&_QoS_Events + Offset), Size);
             RetVal = Size;
           }
           else if(TableNum==eMfgT95_LPQoSConfig)
           {
             RetVal=flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPQOSSTARTADDR)+Offset, Size);
           }
           else if(TableNum==eMfgT96_LPQoSData)
           {
             LpQoS_ReadHook();
             RetVal=flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPQOSSTARTADDR)+Offset, Size);
           }
           break;
         case(eConst):  // const data is in flash, so a memcpy will work..
         case(eRAM):
            LockSemaphore(&tableRAMAccess);
            memcpy(Buffer, ((unsigned char*)TableConfig[TableEntryNum].Address + Offset), Size);
            UnlockSemaphore(&tableRAMAccess);
            RetVal = Size;
            break;
         case(eEmulEEPROM):
         case(eExtFlash): 
            if (TableNum==eStdT62_LPControl)
            {
               RetVal = BuildDynamicStd62(Buffer,Offset,Size);
            }
            else
            {
               RetVal = flashReadMeter(Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR)+Offset, Size);
            }
            break;
      }
#ifdef TYPETEST_DEBUG
   CheckIntReg(1);
#endif
   }
#ifdef SIZE_TEST
   if(RetVal != Size)
      printf("RE %d-%d -%d\n",TableNum,RetVal,Size);
#endif
   if(!RetVal)
      Debug(LogDebug,"Meter Task","TableReadEx: Read from table#%i returns %i",TableNum,RetVal);
   return RetVal;
}

unsigned short TableWrite(unsigned short TableNum, unsigned long Offset, unsigned short Size, void *Buffer)
{
#ifdef AMPY_METER_R1100
  unsigned char Tmp[8];
#endif
   unsigned char TableEntryNum;
   unsigned short RetVal = 0;
   unsigned char WritePacketsInStream = (TableNum & TBL_WRITE_PACKET_IN_STREAM) ? 1 : 0;

   TableNum &= ~TBL_WRITE_PACKET_IN_STREAM;  //clear the stream flag to get table number
   TableEntryNum = TableFindEntry(TableNum);
   if(TableEntryNum == 0xFF)
   {
      Debug(LogDebug,"Meter Task","TableWriteEx: Table Not Found!");
      return 0xFFFF;
   }

   Size = TableCheckSize(TableNum,TableEntryNum, Offset, Size);
   if(Size)
   {
      // TableFindEntry returned a valid table
      switch(TableConfig[TableEntryNum].Type)
      {
         case(eDynamic):
            if(TableNum==eStdT23_CurrRegData)
            {//just a dumb write, a write should only called internally by the TOU module
               flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTOUSTARTADDR)+Offset, Buffer, Size);
               RetVal = Size;
            }
            else if((TableNum==eStdT24_PrevSeaData)||(TableNum==eStdT25_PrevDmdRstData)||(TableNum==eStdT26_SelfReadData)||(TableNum==eMfgT40_TOUHistory))
            {
               flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTOUSTARTADDR)+Offset, Buffer, Size);
               RetVal = Size;
            }
            else if((TableNum>=eStdT64_LPDataSet1)&&(TableNum<=eStdT67_LPDataSet4))
            {//special case for LP
                if(Buffer!=NULL) {
                  Offset += LP_offset_adjust(TableNum-eStdT64_LPDataSet1);
                  RetVal=LP_flashWrite((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPDATASTARTADDR)+Offset, Buffer, Size,TableNum-eStdT64_LPDataSet1);
                }
				  RetVal = Size;
            }
            else if(TableNum==eStdT76_EventLogData)
            {
               EventTblWriteHook(Offset, Buffer, Size);
               RetVal = Size;
            }
            else if(TableNum==eMfgT05_MeterFeatures)
               break;
            else if(TableNum==eMfgT32_FwDlDataBlk)
               RetVal = FWDataBlockWrite(Buffer, Size);
            else if(TableNum==eMfgT41_ActivityStats)
            {
               RetVal = MStatWriteActStatTbl((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR), Offset, Buffer, Size);
            }
            else if((TableNum==eMfgT43_PSEMPermission)||(TableNum==eMfgT44_PSEMXPermission)||(TableNum==eMfgT45_KF2Permission))
            {
               RetVal=MCom_TblWritePerm(TableNum,Offset,Size,Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR));
            }
            else if((TableNum==eMfgT46_PSEMPwdKeys)||(TableNum==eMfgT47_PSEMXPwdKeys)||(TableNum==eMfgT48_KF2PwdKeys))
            {
               RetVal=MCom_TblWriteKeys(TableNum,Offset,Size,Buffer,(unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR));
            }
            else if(TableNum==eMfgT62_GenericRAM)
            {
               RetVal = disable_save();
               if(Buffer==NULL)
                  memset((void*)((unsigned char*)TableConfig[TableEntryNum].Address + Offset),0x00,Size);
               else
                  memcpy(((unsigned char*)TableConfig[TableEntryNum].Address + Offset), Buffer, Size);
               enable_restore(RetVal);
               RetVal = Size;
            }
            else if(TableNum==eMfgT15_PrePayDisplay)
            {
              memcpy((&PrePayDisplay + Offset),Buffer, Size);
              RetVal = Size;
            }
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
            else if(TableNum==eMfgT74_RippleEvtLog)
            {
               RippleApp_EventWriteHook(Offset, Buffer, Size);
               RetVal = Size;
            }
#endif
           else if(TableNum==eMfgT51_QOSEvent)
           {
             memcpy((&_QoS_Events + Offset), Buffer, Size);
             RetVal = Size;
           }
           else if(TableNum==eMfgT95_LPQoSConfig)
           {
             flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPQOSSTARTADDR)+Offset, Buffer, Size);
             RetVal = Size;
           }
           else if(TableNum==eMfgT96_LPQoSData)
           {
             flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMLPQOSSTARTADDR)+Offset, Buffer, Size);
             LpQoS_EraseDataOnTableWrite();
             RetVal = Size;
           }
            break;
         case(eRAM):  // RAM only for now
			   LockSemaphore(&tableRAMAccess);
            if(Buffer==NULL)
               memset((void*)((unsigned char*)TableConfig[TableEntryNum].Address + Offset),0x00,Size);
            else
               memcpy(((unsigned char*)TableConfig[TableEntryNum].Address + Offset), Buffer, Size);
            UnlockSemaphore(&tableRAMAccess);
            RetVal = Size;
            break;
         case(eEmulEEPROM):
         case(eExtFlash):
            if(Buffer==NULL)
            {}   // fixme: need a cool way to blank a flash class without blowing the stack
            else
            {
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(DYNAMIC_LP)
               if((TableNum==eStdT61_ActualLPLimit) || (TableNum==eStdT62_LPControl) || (TableNum==eMfgT36_LP_QOS_Cfg))
               {
                  if(gLPPlatformGet == 0)//Haven't got a platform for mirroring new LP configuration
                  {
                      LPCnfg_Tbls = PendTillGetPlatform();
                      LPBuffer = ExportPP(LPCnfg_Tbls);
                      memset(LPBuffer,0xFF, 255);
                      gLPPlatformGet = 1;
                  }
                  switch (TableNum)
                  {
                  case eStdT61_ActualLPLimit:
                     memcpy((LPBuffer + Offset), Buffer, Size);
                     //gLPPlatformGet = 0;
                     break;
                  case eStdT62_LPControl:
                     memcpy((LPBuffer + (sizeof(_std_table_60_t)) + Offset), Buffer, Size);
                     break;
                  case eMfgT36_LP_QOS_Cfg:
                     memcpy((LPBuffer + (sizeof(_std_table_60_t)) + (sizeof(_std_table_62_t)) + Offset), Buffer, Size);
                     break;
                  }
               }
               else
#endif
               if(TableNum==eMfgT50_QOSConfig)
               {
                 QOS_Recover();//This function does updating of the existing breaches
                 RetVal = flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR)+Offset, Buffer, Size);;
               }
              #ifdef AMPY_METER_R1100
               else if (TableNum==eMfgT63_MetrologyConfig)
               {
                 LED_PulsingConfigActive = 1;
                 RetVal = flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR)+Offset, Buffer, Size);
           //      printf("LED_PulsingConfigActive table_c\n");
               }
               else if (TableNum==eMfgT25_CustFeatureCfg)
               {
                 
                 memcpy(Tmp, Buffer, 8);
                 if(Tmp[7] == 0)//Do not write the table in case  Gross metering is set
                 {
                   RetVal = flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR)+Offset, Buffer, Size);
                 //   printf("gross net metering config. = %d\n",Tmp[7]);
                 }
   
                
               }
              #endif
               else
                 RetVal = flashWriteMeter((unsigned long)(TableConfig[TableEntryNum].Address)+(AEMTABLESTARTADDR)+Offset, Buffer, Size);
            }
            //RetVal = Size;
            break;
         case(eConst):
            break;
      }
   }

   if(TableWriteStopRecursive[TaskId()]<2)
   {//its ok to continue. i.e. we are not being called via pFunc()
      TableWriteStopRecursive[TaskId()]++;//stop further calls to pFunc()
      if((TableConfig[TableEntryNum].pFunc!=NULL)&& (!WritePacketsInStream)) {
         TableConfig[TableEntryNum].pFunc();//call table specific handler
      }
      TableWriteStopRecursive[TaskId()]--;
   }
#ifdef SIZE_TEST
   if(RetVal != Size)
      printf("WE %d-%d -%d\n",TableNum,RetVal,Size);
#endif
   if(!RetVal)
      Debug(LogDebug,"Meter Task","TableWriteEx: Write to table#%i returns %i",TableNum,RetVal);

   return RetVal;
}

unsigned char TableFindEntry(unsigned short TableNum)
{
   // This search routine could be a little smarter...
   unsigned short a;
   for(a=0; a<(sizeof(TableConfig)/sizeof(_TableInfoStruct)) ;a++)
   {
      if(TableConfig[a].Num == TableNum)
         return(a);
   }
   return (0xFF);
}

void TableInit()
{
   ni_fmat1_t buff[4*6];
   unsigned char i;
   unsigned long CRCAll,CRCSum;

#ifndef Simulation
   //make sure the area allocated for table date is big enough.
   //check done in code since compiler is too stupid to do calc at compile time
   if(ADDR_EXT_FLASH_END > AEMTOUSTARTADDR)
      SysAbort( __FILE__, __LINE__);

   if(ADDR_EXT_TOU_FLASH_END > AEMLOGSTARTADDR)
      SysAbort( __FILE__, __LINE__);

   if(ADDR_EXT_LOG_FLASH_END > AEMLPDATASTARTADDR)
      SysAbort( __FILE__, __LINE__);

   if(ADDR_EXT_LP_FLASH_END > AEMLPDATAENDADDR)
      SysAbort( __FILE__, __LINE__);

   if((unsigned long)(UNUSED_SPACE2) > DF_MAX_FLASH_PAGES)
      SysAbort( __FILE__, __LINE__);
#endif


   memset(TableWriteStopRecursive,0x00,sizeof(TableWriteStopRecursive));

   //restore backed up versions of energy RAM tables
   TableRead(eMfgT65_NV_Energy, 0, (sizeof(ni_fmat1_t)*4*6), buff);
   TableWrite(eStdT28_PresRegData, 0, (sizeof(ni_fmat1_t)*4*6), buff);

   //restore LP tables
   TableRead(eMfgT55_LPStatusBkp, 0,sizeof(std_table_63_struct),&std_table_63_struct);

   for(i=0;i<eHashNumber;i++)
   {
      TableRead(eMfgT00_Hash, i*sizeof(_hash_entry_t), sizeof(_hash_entry_t), buff);
      if(i==0)//i==eHashAllData
      {
         memcpy(&CRCAll,buff,4);
         CRCSum=0;
      }
      else
         CRCSum = crc32BufferSlow((unsigned char*)buff, sizeof(_hash_entry_t), CRCSum, CRC32REV_POLY);
   }

   if(CRCAll!=CRCSum)//check for data validity
   {//this is the first time the meter was powered up or there was a corruption
      //clear the hash values.
      printf("Invalid Hash table, Ref=%lX,Calc=%lX\n",CRCAll,CRCSum);
      MStatChangeStdStatus(STAT_TBL_STD_FLG_UNPROG,1);
      buff[0]=0xFFFFFFFF;
      CRCSum=0;
      for(i=1;i<eHashNumber;i++)
      {
         CRCSum = crc32BufferSlow((unsigned char*)buff, sizeof(_hash_entry_t), CRCSum, CRC32REV_POLY);
      }
      for(i=0;i<eHashNumber;i++)
      {
         if(i==eHashAllData)//i==0
            TableWrite(eMfgT00_Hash, i*sizeof(_hash_entry_t), 4, &CRCSum);
         else if(i!=eHashMetrology)//don't kill metrology it may or may not be already be dead
            TableWrite(eMfgT00_Hash, i*sizeof(_hash_entry_t), 4, buff);//all F's
      }
   }

#ifdef Simulation
   // let the hackery begin
   //InitDefaultTableData();
#endif
}

void TableBackUp()
{
   TableWrite(eMfgT65_NV_Energy, 0, (sizeof(ni_fmat1_t)*4*6), &std_table_28_struct);//get values from eStdT28_PresRegData directly
   //backup LP tables
   TableWrite(eMfgT55_LPStatusBkp, 0,sizeof(std_table_63_struct),&std_table_63_struct);
}

void UpdateHashCheck()
{
   unsigned char i;
   unsigned long Ref,crc32=0;
   unsigned char size;
   unsigned char buff[10];

   TableRead(eMfgT00_Hash, 0, 4, &Ref);
   i=sizeof(_hash_entry_t);//skip the first value (this is the Hash check value)
   while(i<sizeof(_mfg_table_00_t))
   {
      if((i+10)<=sizeof(_mfg_table_00_t))
         size=10;
      else
         size=sizeof(_mfg_table_00_t)-i;

      TableRead(eMfgT00_Hash, i, size, buff);
      crc32 = crc32BufferSlow(buff, size, crc32, CRC32REV_POLY);
      i+=size;
   }
   if(Ref!=crc32)
      TableWrite(eMfgT00_Hash, 0, 4, &crc32);
}

unsigned char TableHasHash(unsigned short TableNum)
{
   unsigned char TableEntryNum;

   TableEntryNum = TableFindEntry(TableNum);
   if(TableEntryNum == 0xFF)
      return 0;
   //if no funtion handler the table can't have a hash
   if(TableConfig[TableEntryNum].pFunc!=NULL)
   {
      if((TableNum!=eStdT52_Clock)&&(TableNum!=eStdT55_ClockState))//if not an exception
         return 1;
   }
   return 0;
}

unsigned short CalcTableHash(unsigned short TableNum,unsigned long *crc32)
{
   return CalcTableHashPartial( TableNum, crc32, 0, 0);
}

unsigned short CalcTableHashPartial(unsigned short TableNum,unsigned long *crc32,unsigned short offset, unsigned short length)
{
   unsigned char buff[30];
   unsigned short i=0;
   unsigned short len;
   unsigned short ret;
   unsigned long size;
   unsigned char temp=0;

   size = TableFindEntry(TableNum);
   if(size == 0xFF)
      return 0xFFFF;

   size = TableCheckSize(TableNum,size, 0, 0);
   if((size==0)||(size>0x0000FFFF))
      return 100;
   if(length>size)
      return 0x200;
   if(offset>=size)
      return 0x300;
   if((offset+length)>size)
      return 0x400;
   size-=offset;
   if(length!=0)
      size=length;

   while(i<size)
   {
      if((i+30)<=size)
         len=30;
      else
         len=size-i;

      //gsk:Bug fix for partial table write to passwords/permissions table when NOT using Factory/Full access
      //temporarily disable optical and remote user as this is a internal read.This will let us do a Factory level3 access in MCom_TblReadKeys
      if(gMCommStat.OptActive)
      {
         temp =1;
         gMCommStat.OptActive=0;
      }
      else if(gMCommStat.RemActive)
      {
         temp =2;
         gMCommStat.RemActive=0;
      }

      ret=TableRead(TableNum, i+offset , len, buff);

      //Restore optical and remote user
      if(temp ==1)
         gMCommStat.OptActive=1;
      else if(temp ==2)
         gMCommStat.RemActive=1;

      temp=0;

      if(ret!=len)
         return 0xFFFF;
      *crc32 = crc32BufferSlow(buff, len, *crc32, CRC32REV_POLY);
      i+=len;
   }
   return 0;
}

extern unsigned char gSecCheckDone;
unsigned char TableProcExecute(unsigned char PortSrc, unsigned char UserLvl,unsigned short ProcCmd, unsigned char SeqNo, unsigned char *pData)
{
   unsigned char RetVal=OK;
   unsigned char Response=ANSI_PROC_OK;
   unsigned char tempc;
   unsigned short tempw;
   unsigned long templ;
   _std_table_08_t tbl8_tmp;

   //by the time this function is called security permissions have already been established so there is not need to do
   //a security check here.


   //The focus meter has the GEN_CONFIG_TBL_DOT_DATA_ORDER set but we have it cleared (M16C is a little endian processor)
   //The DCW is assuming that its set so either we change the DCW for all procedures
   //or we do a special endian swap for the S1200 this special swap should occur before the call to this funcion.
   //mc: do this until I can figure out where the endian issue is
   //if(ProcCmd == 0x0A00) ProcCmd = 0x000A;


#ifdef ANSI_PROC_DEBUG
   EVENT2("ANSI Proc %u",ProcCmd&0x0FFF);
#endif

   switch (ProcCmd>>12)
   {
   case 0://on completion post result into table 8
   case 1://on exception post result into table 8
   case 2://don't post result into table 8
   case 3://post response in table 8 immediately and then again on completion
      break;
   default:
      RetVal=ONP;
      break;
   }

   if(RetVal!=OK)
   {
      MCom_ErrorLog();
      return RetVal;
   }

   if(MCom_IsChanOptical(PortSrc))
   {
      if(gMC_Num_Opto_Changes<0xFF)
         gMC_Num_Opto_Changes++;

      MC_COMPACT_USR(gMCommStat.OptUsr,UserLvl);
      gMCommStat.OptActive=1;
   }
   else if(MCom_IsChanRemote(PortSrc))
   {
      MC_COMPACT_USR(gMCommStat.RemUsr,UserLvl);
      gMCommStat.RemActive=1;
   }

   memset(&tbl8_tmp.resp_data,0x00,sizeof(tbl8_tmp.resp_data));

   //perform command
   switch (ProcCmd&0x0FFF)
   {
   //-----------------Std Procedures---------------------------------------
   case STD_PROC00:// Cold Start
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      //ANSI says "This procedure causes the end device to return to a manufacturer default known state. All
      //data values, programs and conditions may be lost. Communication may be broken. If an
      //Event Log exists, an attempt shall be made to retain it."

      //reset accumulated energy
      TableWrite(eStdT28_PresRegData, 0, 0, NULL);
      TableWrite(eMfgT65_NV_Energy, 0, (sizeof(ni_fmat1_t)*4*6), &std_table_28_struct);//get values from eStdT28_PresRegData directly

      //put the relay config tables and relay state machine into a know state
      TableRead(eMfgT00_Hash, eHashRelay*sizeof(_hash_entry_t), sizeof(templ), (void *)&templ);
      templ+=1;//invalidate CRC so RelayInit() is triggered to set defaults
      TableWrite(eMfgT00_Hash, eHashRelay*sizeof(_hash_entry_t), sizeof(templ), (void *)&templ);
      RelayInit(0);

      //reset self read stuff
      TOU_Clear();
      TOU_SelfRead_Reset();
      //UpdateHashCheck();

      break;
   case STD_PROC01://Warm Start (reset)
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      sys_reset (SOFT_SUICIDE_ANSI_PROC);//reset meter
      break;
   case STD_PROC03:// Clear Data Master Reset
      //ANSI says "This procedures causes the end device to purge generated data fields but retain programming fields."
      AMPYPowerCycleCountReset();
      //TableWrite(eStdT28_PresRegData, 0, 0, NULL);
      TableWrite(eStdT28_PresRegData, 0, 96, NULL);
      TableWrite(eMfgT65_NV_Energy, 0, (sizeof(ni_fmat1_t)*4*6), &std_table_28_struct);//get values from eStdT28_PresRegData directly
      TOU_Clear();
      TOU_SelfRead_Reset();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
      LP_Reset(0x0F);
#else
      LP_Reset();
#endif
      //EventReset(); //by order of Paul Collins this will no longer happen
      ExpEngyClear();
      QOSClearStats(0);
      Full_PartialProg(0x1F); //Reset the relay state machine
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
      RippleApp_StatusReset(); //Reset the Ripple Status Table MfgT75
      RAppLC_StatusReset();//Reset the Emergency function log and relay switch count in MfgT27
#endif
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      break;
   case STD_PROC04:// Reset List Pointer
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      EventLogWrite(eEvtResetList,GetCommsUserID(),1,&pData[0]);
      //ANSI says "When invoked, the end device attempts to reset list control variables to their initial state"
      if(pData[0]==1)//reset event log
         EventReset();
      else if(pData[0]==2)//reset self read
         TOU_SelfRead_Reset();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
      else if(pData[0]==3)//reset LP Set 1
         LP_Reset(0x01);
      else if(pData[0]==4)//reset LP Set 2
         LP_Reset(0x02);
      else if(pData[0]==5)//reset LP Set 3
         LP_Reset(0x04);
      else if(pData[0]==6)//reset LP Set 4
         LP_Reset(0x08);
      else if(pData[0]==7)//reset all LP
         LP_Reset(0x0F);
#else
      else if(pData[0]==7)//reset all LP
         LP_Reset();
#endif
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
      else if(pData[0]==34)//reset ripple event log
         RipplApp_EventReset();
#endif
      else
         Response=ANSI_PROC_PAR;
      break;
   case STD_PROC05:// Update Last Read Entry
      //don't log an event for the update of the event log pointers, others like LP and self read are OK
      if(pData[0]!=1)
         EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      Update_Last_Read_Entry(pData);
      break;
   case STD_PROC07:     //Clear Std Status Flag, hack for dcw
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      //ANSI says "When invoked, the end device attempts to clear all standard status flags"
      TableWrite(eStdT03_EDMODEStatus,offsetof(_std_table_03_t,ed_std_status1),sizeof(_ED_Std_status1_bfld)+1,NULL);
      MStatCheckBattery();
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
      MeterStdStatusResetHandler(FORCE_LOG_ONLY);
#endif
#if defined(METROLOGY_MMI_ERROR_REPORTING)
      g_Metrology_Tot_MMI_ErrCount=0; //Reset  MMI error count to allow event/status retrigger
#endif
      break;
   case STD_PROC08:     //Clear Mfg Status Flag, hack for dcw
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      //ANSI says "When invoked, the end device attempts to clear all manufacturer status flags"
      TableWrite(eStdT03_EDMODEStatus,offsetof(_std_table_03_t,ed_mfg_status),GEN_CONFIG_TBL_DOT_DIM_MFG_STATUS_USED,NULL);
      MStatCheckBattery();
      if(PP_Comms.PrePayNotSupported == 0)
      {
         if(PP_Comms.PrePayState & PREPAYMODEENABLE) //Prepay is enabled
            MStatChangeMfgStatus(STAT_TBL_MFG_FLG_PREPAYSTATUS,1);
        
         if(!(PP_Comms.PrePayState & PREPAYOCCUPIED))
            MStatChangeMfgStatus(STAT_TBL_MFG_FLG_PREPAYOCCPSTATUS,1);
      }
      gAMPY_NV_Write_Err_Count=0;
      gAMPY_NV_Read_Err_Count=0;
      break;
   case STD_PROC09: //Reset Demand
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      if(pData[0] & 0x01)
         Response=TOU_demand_reset(1);
      else
         Response=ANSI_PROC_PAR;
      break;
   case STD_PROC10:// Set Meter Date and Time
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      EventLogWrite(eEvtTimeOld,GetCommsUserID(),0,NULL);
      TableRead(eStdT55_ClockState, offsetof(_std_table_55_t, clock_calendar), sizeof(_ltime_date_t), tbl8_tmp.resp_data);

      //Check for SSN Time change request
      if(MCom_IsChanRemote(PortSrc))
      {
         TableRead(eMfgT57_BackEndCommsCfg, offsetof(_mfg_table_57_t,CommsType), sizeof(tempc), &tempc);
         if(tempc == eBackEndSSN)
            RTC_SSN_Check = 1;
         else
            RTC_SSN_Check = 0;
      }
		
      Response=RTC_SetTimeProc(pData);
		
      TableRead(eStdT55_ClockState, offsetof(_std_table_55_t, clock_calendar), sizeof(_ltime_date_t), tbl8_tmp.resp_data+sizeof(_ltime_date_t));
      EventLogWrite(eEvtTimeNew,GetCommsUserID(),0,NULL);
      gSecCheckDone = 0;
      break;

   case STD_PROC16:// Start Load Profile
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      tbl8_tmp.resp_data[0]=LP_Enable();
      if(tbl8_tmp.resp_data[0]!=eLPErrPass)
         Response=ANSI_PROC_DEV;
      break;

   //-----------------Mfg Procedures---------------------------------------
   case MFG_PROC00://statistics reset
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      if(StatisticsResetProc(pData)!=0)
         Response=ANSI_PROC_PAR;
      break;
   case MFG_PROC01:// Add an event to the event log (mainly used for U series HAN and RF events)
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      Response=Log3rdPartyEvent(pData);
      break;
   case MFG_PROC02:// Partial table write via broadcast group command
      Response=GroupTblWrite(PortSrc, UserLvl, pData);
      if(Response==ANSI_PROC_OK)
         EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      break;
   case MFG_PROC04:// unpdate num unread alarms (AX=Battery Control)
      //don't log an event for the update of the event log pointers
      Response=Update_Last_Read_Alarm(pData);
      break;
   case MFG_PROC05:// Change energy LED direction.
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      Response=MetrologyChangeLEDDirection(pData);
      break;
   case MFG_PROC07:// Update full/partial programming date
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
      if(UpdateProgEvent(pData,GetCommsUserID()))
         Response=ANSI_PROC_PAR;
      else
      {
        tbl8_tmp.resp_data[0] = pData[2];
        tbl8_tmp.resp_data[1] = pData[3];
      }
#else
		if(UpdateProgEvent(pData,GetCommsUserID()))
         Response=ANSI_PROC_PAR;
#endif
      break;
   case MFG_PROC08:// Relay Operation
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      tempw=RelayCommsCmdProc(pData,PortSrc);

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
   if(((pData[3] == 0xFF) && (pData[4] > 3)))//Programming the LC Control for U1300
         tempw = 0;
#endif
      tbl8_tmp.resp_data[0]=tempw;
		
      if((tempw==0)||(tempw==8))
         Response=ANSI_PROC_OK;//all is good
      else if((tempw==4)||(tempw==5)||(tempw==eFailed_CmdParam))
         Response=ANSI_PROC_PAR;//one of the parameters is not known
      else if(tempw==7)
         Response=ANSI_PROC_AUTH;//fail due to security settings
      else if(tempw==2)
         Response=ANSI_PROC_NDONE;//fail due to priority
      else if(tempw==11)
         Response=ANSI_PROC_TIME;
      else
         Response=ANSI_PROC_DEV;//meter configuration blocked this command
      break;
   case MFG_PROC09:// update live display symbols
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      //LAN/HAN Commands: 0 -OFF, 1-SLOW FLASH (approx 2secs), 2-FAST FLASH - (approx 300ms), 3-Continous ON
      switch (pData[0])
      {
      case 255:
         break;
      case 1:
         gDisplayQuadrant&=~LIVE_DISP_LAN_BIT;
         gDisplayQuadrant|= LIVE_DISP_LAN_BIT_FLASH; //LAN SLOW FLASH
         break;
      case 2:
         gDisplayQuadrant|=(LIVE_DISP_LAN_BIT | LIVE_DISP_LAN_BIT_FLASH); //HAN FAST FLASH
         break;
      case 3:
         gDisplayQuadrant|= LIVE_DISP_LAN_BIT; //LAN ON
         gDisplayQuadrant&=~LIVE_DISP_LAN_BIT_FLASH;
         break;
      case 0:
      default:
         gDisplayQuadrant&=~(LIVE_DISP_LAN_BIT | LIVE_DISP_LAN_BIT_FLASH);//HAN OFF
         break;
      }

      switch (pData[1])
      {
      case 255:
         break;
      case 1:
         gDisplayQuadrant&=~LIVE_DISP_HAN_BIT;
         gDisplayQuadrant|= LIVE_DISP_HAN_BIT_FLASH; //HAN SLOW FLASH
         break;
      case 2:
         gDisplayQuadrant|=(LIVE_DISP_HAN_BIT | LIVE_DISP_HAN_BIT_FLASH); //HAN FAST FLASH
         break;
      case 3:
         gDisplayQuadrant|= LIVE_DISP_HAN_BIT; //HAN ON
         gDisplayQuadrant&=~LIVE_DISP_HAN_BIT_FLASH;
         break;
      case 0:
      default:
         gDisplayQuadrant&=~(LIVE_DISP_HAN_BIT | LIVE_DISP_HAN_BIT_FLASH);//HAN OFF
         break;
      }
      break;

   case MFG_PROC10:// Factory only special commands (internal documentation only)
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      if(FactoryProc(pData,&tbl8_tmp)!=0)
         Response=ANSI_PROC_PAR;
      break;
    case MFG_PROC12:// Firmware Download
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      tempw=FwDownloadProc(pData);
      if(tempw==0)
      {
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)   
      if((gU1300Ver2) && (gU1300S2_ValidFirmware == 0)) //if U1300 series 2 and Firmware invalid
        Response=ANSI_PROC_DEV;
      else
        Response=ANSI_PROC_PAR;
#else
      Response=ANSI_PROC_PAR;
#endif
      }
      break;
   case MFG_PROC13:// Special undocumented customer command to kill a factory1/2 level user
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      if(pData[0]==0x83)
         MCom_ToggleFactoryUserAccess(pData[1],pData[2],UserLvl);//set/clear, bitmask, user level
      else if(pData[0]==0x10)
         SysMon_TempMeasurementStatsInitProc();//Reset temp measurement Stats
#if defined(DEBUG_EVENT_LOG_ADD_ON_CMD)
      else if(pData[0]==0x01)
      {//add a fake event to the event log
         // proc comd data is [0x01][event ID][event ID][Usr ID][Usr ID][Evnt data][Evnt data][Evnt data][Evnt data][Evnt data][Evnt data]
         tempw=get16(&pData[1]);

         if( ((tempw>0)&&(tempw<eEvtMaxStd)) || ((tempw>=eEvtVoltLossA)&&(tempw<eEvtMaxMfg)) )
            EventLogWrite(tempw,get16(&pData[3]),6,&pData[5]);
      }
#endif
      else if(pData[0]==0x11)
      {
         //print internal event log to crash log
         DebugPrintEvents (0, 0);
      }
      else if(pData[0]==0x12)
      {
         //print internal event log direct to optical port
         AMPYEnablePrintToPort(1);
         DebugPrintEvents (0, 0);
         AMPYEnablePrintToPort(0);
      }
      break;
   case MFG_PROC14: //Async Events to trigger comms to do something
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      if(pData[0] >= 0x0A)
      {
         EventLogWrite(eEvtCommsUpdate,GetCommsUserID(),6,&pData[0]);
      }
      break;
#if defined(NEUTRAL_INTEGRITY) || defined(NEUTRAL_INTEGRITY_POLY)
   case MFG_PROC15: //NI command to Enable/Disable Installation mode and or the entire NI module
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
   #if defined(NEUTRAL_INTEGRITY)
      if(Metrology_NICommand(pData))
   #elif defined(NEUTRAL_INTEGRITY_POLY)
      if(PolyPhaseNI_Command(pData))
   #endif
         Response=ANSI_PROC_DEV;
      break;
#endif
   case MFG_PROC16:
      EventLogWrite(eEvtProcExe,GetCommsUserID(),2,&ProcCmd);
      if(pData[0] == 1)
         TOUFix(pData[1]);
      break;
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
   case MFG_PROC17: //Update the last unread pointer for partial LP data readout via mfg tables 90-93
      if(!LPPartialMfgProc(pData))
         Response=ANSI_PROC_PAR;
      break;
#endif
   case MFG_PROC18:
      if(pData[0]<2)
         Response = Mcom_SetupExclusiveRemoteAccess(pData[0]);
      else
         Response = ANSI_PROC_PAR;
      break;
   case MFG_PROC19:
     if(pData[0] > ePPM_Occupied)
     {
       Response = ANSI_PROC_PAR;
       break;
     }
     if(PrePayStateChange(pData))
     {
       EventLogWrite(eEvtPrePay,0,1,&pData[0]);
       Response = ANSI_PROC_OK;
     }
     else
       Response = ANSI_PROC_DEV;
     break;
   default:
      RetVal=ONP;
      Response=ANSI_PROC_UNKN;
      break;
   }

   tempw=eMSComActPortRemote;
   if(MCom_IsChanOptical(PortSrc))
   {
      gMCommStat.OptActive=0;
      tempw=eMSComActPortOptical;
   }
   else if(MCom_IsChanRemote(PortSrc))
      gMCommStat.RemActive=0;

   if(Response==ANSI_PROC_OK)
      MStatIncCommsActivity(tempw,eMSComActProc);
   else
      MCom_ErrorLog();

   switch (ProcCmd>>12)
   {
   case 0://on completion post result into table 8
   case 3://post response in table 8 immediatley and then again on completion
      TableWrite(eStdT08_ProcResponse, 0, 0, NULL);
      *((unsigned short*)&(tbl8_tmp.proc))=ProcCmd&0x0FFF;
      tbl8_tmp.seq_nbr=SeqNo;
      tbl8_tmp.result_code=Response;
      TableWrite(eStdT08_ProcResponse, 0, sizeof(tbl8_tmp), &tbl8_tmp);
      break;
   case 1://on exception post result into table 8
      if(Response!=ANSI_PROC_OK)
      {
         TableWrite(eStdT08_ProcResponse, 0, 0, NULL);
         *((unsigned short*)&(tbl8_tmp.proc))=ProcCmd&0x0FFF;
         tbl8_tmp.seq_nbr=SeqNo;
         tbl8_tmp.result_code=Response;
         TableWrite(eStdT08_ProcResponse, 0, sizeof(tbl8_tmp), &tbl8_tmp);
      }
      break;
   case 2://don't post result into table 8
      break;
   }

   return RetVal;
}


/*-------------------------------------------------------------------------------
* Function:    BuildDynamicMfg12
* Inputs:      Buffer-pointer to buffer that will be filled with data.
*              Offset-offset into table that is being requested.
*              Size-number of bytes being requested.
* Outputs:     Buffer-Fills up Buffer with data.
* Description: Build up dynamic table data for Mfg Tbl 12 (status).
*------------------------------------------------------------------------------*/
void BuildDynamicMfg12(unsigned char *Buffer,unsigned short Offset,unsigned short Size)
{
   unsigned char buff[16];//in the future don't make this bigger otherwise you risk stack corruption.
   unsigned char i;

   memset(buff,0,sizeof(buff));
   memcpy(&buff[0],PART_NUMBER_M,7);
   memcpy(&buff[7],PN_VERSION,2);
   buff[9]='.';
   memcpy(&buff[10],PN_REVISION,2);
   memcpy(&buff[12],TEST_REV,4);

   //TODO: IJD-undo hack for mfg 12 size due to init push
   for(i=0;i<Size;i++)
   {
      if((Offset+i)<sizeof(_mfg_table_12_t))
         Buffer[i]=buff[Offset+i];
      else
         Buffer[i]=0;
   }
      //memcpy(Buffer,&buff[Offset],Size);
}

/*-------------------------------------------------------------------------------
* Function:    CheckStd62_Size
* Inputs:      None
* Returns:     The calculated size of STD Table 62 in bytes
* Description: Calculate STD table 62 size, will adjust the size if the request
* is from a Gridstream comms module. Gets called by ANSIComms.c
*------------------------------------------------------------------------------*/
unsigned long CheckStd62_Size(unsigned long Offset, unsigned short Size)
{
  unsigned long dynTblSize = 0;
  unsigned char set, numChanSet, temp = 0;

  /* Determine the Comms type connected to the meter on the remote port */
  TableRead(eMfgT57_BackEndCommsCfg, offsetof(_mfg_table_57_t,CommsType), sizeof(temp), &temp);

  if ( ((temp==eBackEnd3G) || (temp==eBackEndRF_SBS)) )
  {
    /* Run through all the sets */
	  for (set = 0; set < NUM_LP_DATA_SETS; set++)
	  {
	    /* Get the number of channels in this set */
	    TableRead(eStdT61_ActualLPLimit,offsetof(_std_table_60_t,Config)+(sizeof(_lp_config_t)*set)+offsetof(_lp_config_t,Nbr_Chns),sizeof(numChanSet),&numChanSet);
	    dynTblSize += (numChanSet * sizeof(_LP_Source_Sel_RCD_t));
	    dynTblSize ++; /* Add the Interval format code length */
	  }

    /* Adjust the size to the table end if it exceeds */
    if (Offset > dynTblSize)
    {
      Size = 0;
    }
    /* Full table read or requested size out of bound*/
    else if ((Size == 0) || (Size > (dynTblSize - Offset)))
    {
      Size = dynTblSize - Offset;
    }
    else
    {
      /* Parameters ok */
    }
  }
  else
  {
    Size=TableSizeOK(eStdT62_LPControl, Offset, Size);
  }

  return(Size);
}

/*-------------------------------------------------------------------------------
* Function:    BuildDynamicStd62
* Inputs:      Buffer-pointer to buffer that will be filled with data.
*              Offset-offset into table that is being requested.
*              Size-number of bytes being requested.
* Outputs:     Buffer-Fills up Buffer with data.
* Returns:     The number of bytes read
* Description: Read Std table 62 and build dynamic if required
*------------------------------------------------------------------------------*/
unsigned short BuildDynamicStd62(unsigned char *Buffer, unsigned short Offset, unsigned short Size)
{
  unsigned char temp;
  unsigned char* src_ptr;
  unsigned char* dst_ptr;
  unsigned short RetVal = 0;
  unsigned char numChanSet, set, chan;
  PLATFORMP Mem_blob;
  unsigned char *tempBuff;
  unsigned short dynTblSize = sizeof(_std_table_62_t); /* Initialise to the full size of the table */

  Mem_blob = PendTillGetPlatform();
  tempBuff = ExportPP(Mem_blob);

  /* Read the full static structure table into the buffer*/
  RetVal = flashReadMeter(tempBuff, (unsigned long) (ADDR_EXT_FLASH_STD62 + AEMTABLESTARTADDR), dynTblSize);

  if (RetVal < Size)
  {
    Size = RetVal;
  }

  /* If the request was initiated by a Gridstream Comms module, change the structure to a dynamic one as
   * per Ansi standard */
  if ((gMCommStat.RemActive == 1) && (Size != 0))
  {
    /* Determine the Comms type connected to the meter on the remote port */
    TableRead(eMfgT57_BackEndCommsCfg, offsetof(_mfg_table_57_t, CommsType), sizeof(temp), &temp);

    /* Don't adjust if the request is KF2  */
    if (((temp == eBackEnd3G) || (temp == eBackEndRF_SBS)) && !MCom_IsChanKF2(UARTCommChannel))
    {
      src_ptr = (unsigned char*) tempBuff;
      dst_ptr = (unsigned char*) tempBuff;

      for (set = 0; set < NUM_LP_DATA_SETS; set++)
      {
        /* Get the number of channels in this set */
        TableRead(eStdT61_ActualLPLimit,
                  offsetof(_std_table_60_t, Config) + (sizeof(_lp_config_t) * set) + offsetof(_lp_config_t, Nbr_Chns), sizeof(numChanSet),
                  &numChanSet);

        /* Include only the configured channels */
        for (chan = 0; chan < NUM_CH_PER_DATA_SET; chan++)
        {
          if (chan < numChanSet)
          {
            memmove(dst_ptr, src_ptr, sizeof(_LP_Source_Sel_RCD_t));
            dst_ptr += sizeof(_LP_Source_Sel_RCD_t);
          }
          src_ptr += sizeof(_LP_Source_Sel_RCD_t);
        }

        /* Copy the Interval format code */
        *dst_ptr++ = *src_ptr++;
      }

      /* Calculate table size */
      dynTblSize = (unsigned long) (dst_ptr) - (unsigned long) tempBuff;
    }
  }

  /* Adjust the size to the table end if it exceeds */
  if (Offset > dynTblSize)
  {
    Size = 0;
  }
  else if (Size > (dynTblSize - Offset))
  {
    Size = dynTblSize - Offset;
  }
  else
  {
    /* Parameters ok */
  }

  /* Copy to output buffer */
  memcpy(Buffer, &tempBuff[Offset], Size);
  FreePlatform(Mem_blob);

  return (Size);
}

//unsigned long int FWBlockOffset = 0;
unsigned char FwDownloadArm(unsigned char *pData);

unsigned short int FWDataBlockWrite(unsigned char *buffer, unsigned short int len)
{
  unsigned short int FWBlkSize;
  unsigned long int FWBlkOffset;
  FLASH_DATA *nmpheader = NULL;  
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
  unsigned char U1300S2_Base_Firmware[3] = {0x52,0x34,0x34}; //0x52 = 'R', 0x34 = '4', 0x34 = '4'
#endif

  TableRead(eMfgT31_FwDlStatus, offsetof(_mfg_table_31_t, FWBlkOffset), sizeof(FWBlkOffset), &FWBlkOffset);
  TableRead(eMfgT31_FwDlStatus, offsetof(_mfg_table_31_t, FWBlkSize), sizeof(FWBlkSize), &FWBlkSize);
  
  if(FWBlkOffset > 500L*1024L) // put the right number here...
    return 0;
  
  if((FWBlkOffset == 0) && (len > 100))		// is all the important stuff within the first 100 bytes?
  {
    nmpheader = (FLASH_DATA *) &buffer[2];		// the first 2 bytes are CRC (see the CompressedImgHdr struct)
    //memcpy(FirmwareCheck,(void*)nmpheader->part_number[13],sizeof(FirmwareCheck));
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)
    if(gU1300Ver2) //Only do this for the U1300 Series 2
    {
     // printf("Part Num %d %d %d\n",nmpheader->part_number[13],nmpheader->part_number[14],nmpheader->part_number[15]);
      // The part_number[] is 20 bytes in length. but the part number we use is 16 bytes
      if(nmpheader->part_number[13] == U1300S2_Base_Firmware[0]) //Perform a check on R builds only
      {
        if(nmpheader->part_number[14] < U1300S2_Base_Firmware[1]) //if firmware is older than R4x
        {
          //printf("Older than R4x\n");
          gU1300S2_ValidFirmware = 0;
        }
        else
        {
          if(nmpheader->part_number[15] < U1300S2_Base_Firmware[2])//if firmware is older than R44
          {
            gU1300S2_ValidFirmware = 0;
            //printf("Older than R44\n");
          }
          else
            gU1300S2_ValidFirmware = 1;
        }
      }
      else
        gU1300S2_ValidFirmware = 1;
    }
    else
      gU1300S2_ValidFirmware = 1;
#endif
#ifndef Simulation
    flashRead (FLASH_NONE, nmpheader->prom_part_number, PART_NUM_LEN, BOOT_CODE_OFFSET + BOOT_PART_OFFSET);
#endif
    // the dev type?
    //nmpheader->dev_type = 1; // hey I reckon the pc tool should do this for us, it's not always 1 (and we won't know what it should be)
  }
  
  // the windowing parameters
  if((FWBlkOffset < 0xa0) && ((FWBlkOffset + len) >= 0xa1))
  {
    // we are writing to the range we're interested
    buffer[0xa0 - FWBlkOffset] = 14;
    buffer[0xa1 - FWBlkOffset] = 5;
  }
  
  Debug(LogDebug,"Fw Dl","Block written at offset %ld", FWBlkOffset);
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)   
   if((gU1300Ver2 == 0) || (gU1300S2_ValidFirmware)) //if U1300 series 1 or Firmware is valid
   {
     flashWrite(FLASH_1, buffer, FWBlkSize, FWBlkOffset);
   }
   else
     len = 0;//return len of 0
#else
  flashWrite(FLASH_1, buffer, FWBlkSize, FWBlkOffset);
#endif
  
  FWBlkOffset += FWBlkSize;
  
  TableWrite(eMfgT31_FwDlStatus, offsetof(_mfg_table_31_t, FWBlkOffset), sizeof(FWBlkOffset), &FWBlkOffset);
  return len;
}

typedef enum { eFwDlSwitchNow=1, eFwDlArmNow, eFwDlArmFuture, eFwDlUnArm, eFwDlErase } eFwDlArmCmds;
typedef enum { eFwDlArmFailed=0, eFwDlArmComplete, eFwDlArmFutureDateFailure, eFwDlArmed, FwDlUnknownCmd, FwDlArmUnknownCmd  } eFwDlErr;

// Attempts to arm the firmware image sitting in external flash
// Return 0 on no error
// Returns errors from enum eFwDlArmErr

unsigned char FwDownloadArm(unsigned char *pData)
{
   unsigned char retVal = eFwDlArmFailed;
   unsigned long int templ;
/*
 *  unsigned char temp;
#if defined(AMPY_METER)
   temp=;//mesh radio & meter FW changed
#elif (defined(AMPY_METER_NO_RF)||defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100) || defined(AMPY_METER_U33WEP))
   temp=;//meter FW changed
#endif*/
   
   switch (pData[0])
   {
   case eFwDlSwitchNow: // reboot now and firmware download should occur on startup
     Debug(LogDebug,"Fw Dl","Firmware Download Arm (switch now)");
     FwDownloadFixSeq();
     FwDownloadFixBootPartNum();
#if defined (AMPY_METER_NO_RF)    
     FwDownloadRelayCRCFix();//to back compatible old fw version is too complex, instead I add 100 spare bytes to prevent tablesize change in the future
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
     //EventLogWrite(eEvtFWChanged,0,1,&temp);
     retVal = eFwDlArmComplete;    // the user won't see this, since we're rebooting but anyway...
     AMPYArmCrashLog();
     gFirmwareUpgrade = 1;
     sys_reset (SOFT_SUICIDE_ANSI_PROC);
     break;
   case eFwDlArmNow:    // arm firmware download now to occur on the next reboot
     Debug(LogDebug,"Fw Dl","Firmware Download Arm (arm for next reboot)");
     templ = 0xfefefefe;
     TableWrite(eMfgT30_FwDlCtrl, offsetof(_mfg_table_30_t,  FWDLActivateTime), sizeof(templ), &templ);
     UpdateHashCheck();
     // need to clac hash, store it, and call InitFwDl()
     FwDownloadFixSeq();
     // don't fix the bootload part number until the switchover date occurs
     FutureFwDlActivated = 1;
     //FwDownloadFixBootPartNum();
     retVal = eFwDlArmComplete;
     AMPYArmCrashLog();
     //EventLogWrite(eEvtFWChanged,0,1,&temp);
     break;
   case eFwDlArmFuture: // arm firmware download to occur on a specified date/time
     Debug(LogDebug,"Fw Dl","Firmware Download Arm (future switch date)");
     // first check the date/time sent is actually in the future
     if(IsFutureLTime((_ltime_date_t*)&pData[1],eRTCFT_GMT))
     {
       templ = RTC_ANSILTimeToPosix((_ltime_date_t*)&pData[1]);
       TableWrite(eMfgT30_FwDlCtrl, offsetof(_mfg_table_30_t, FWDLActivateTime), sizeof(templ), &templ);
       UpdateHashCheck();
       // need to clac hash, store it, and call InitFwDl()
       FwDownloadFixSeq();
       // don't fix the bootload part number until the switchover date occurs
       FutureFwDlActivated = 1;
       gFirmwareUpgrade = 1;
       retVal = eFwDlArmComplete;
     }
     else
     {
       // error out, date specified by the user has already passed, so we're not arming...
       retVal =  eFwDlArmFutureDateFailure;
     }
     break;
   default:             // Unknown command
     retVal = FwDlArmUnknownCmd;
     break;
   }
   
   return retVal;
}

// Does the bootloader only accept seq+1? nah i think it just wants newseq > currseq
void FwDownloadFixSeq()
{
#ifndef Simulation
   unsigned long NewSeqNum;
   flashRead(FLASH_NONE, (unsigned char*)&NewSeqNum, sizeof(unsigned long), offsetof(FLASH_DATA, sequence_number));
/*#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)   
   if((gU1300Ver2 == 0) && (gU1300S2_ValidFirmware)) //if U1300 series 1 or Firmware is valid
   {
     NewSeqNum++;
   }
#else
   NewSeqNum++;
#endif*/
  NewSeqNum++; 
   flashWrite(FLASH_1, (unsigned char*)&NewSeqNum, sizeof(unsigned long), offsetof(FLASH_DATA, sequence_number)+2);
#endif
}

void FwDownloadFixBootPartNum()
{
#ifndef Simulation
   unsigned char BootPromPartNum[PART_NUM_LEN];

   flashRead (FLASH_NONE, BootPromPartNum, PART_NUM_LEN, BOOT_CODE_OFFSET + BOOT_PART_OFFSET);
   Debug(LogDebug,"Fw Dl","Fixing bootloader part number in image: %s",BootPromPartNum);
   flashWrite(FLASH_1, BootPromPartNum, PART_NUM_LEN, offsetof(FLASH_DATA, prom_part_number)+2);
#endif
}

// TODO: S12: should be playing the firmware download game with the radio firmware? ie set flash_download_state?
// TODO: S12: do we need to call ReInitNmpHeader or similar?
// TODO: Maybe we don't set sequence or bootloader part number until we're armed?
typedef enum { eFwDlStart=1, eFwDlStatus, eFwDlSync, eFwDlCheck, eFwDlArm } eFwDlProcCmds;
unsigned char FwDownloadProc(unsigned char* pData)
{
  unsigned char RetVal = 0;        // will result in ANSI_PROC_PAR (Invalid parameter for a known procedure.Request ignored.)
  unsigned long int FWBlkOffset;
  unsigned short int FWBlkSize;
  
  switch (pData[0])
  {
  case eFwDlStart:     // clear the first page in the tank to clear the header?
    // clear all so we can skip sending ff's
    Debug(LogDebug,"Fw Dl","Firmware Download Start");
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)  
    if(gU1300S2_ValidFirmware == 0)
      gU1300S2_ValidFirmware = 1;//This is to handle a situaition where the firmware download has been reinitiated after a failure
#endif
    RetVal = 1;
    break;
  case eFwDlSync:      // get offset and block length
    memcpy(&FWBlkOffset, &pData[1], sizeof(FWBlkOffset));
    memcpy(&FWBlkSize, &pData[5], sizeof(FWBlkSize));
    TableWrite(eMfgT31_FwDlStatus, offsetof(_mfg_table_31_t, FWBlkOffset), sizeof(FWBlkOffset), &FWBlkOffset);
    TableWrite(eMfgT31_FwDlStatus, offsetof(_mfg_table_31_t, FWBlkSize), sizeof(FWBlkSize), &FWBlkSize);
    Debug(LogDebug,"Fw Dl","Sync to offset %i",FWBlkOffset);
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)   
    if((gU1300Ver2) && (gU1300S2_ValidFirmware == 0)) //if U1300 series 2 and Firmware invalid
    {
      RetVal = 0; // will result in  ANSI_PROC_DEV (Request conflicts with current device setup. Request ignored)
    }
    else
      RetVal = 1;
#else
    RetVal = 1;
#endif           
    break;
   case eFwDlArm:       
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)   
     if((gU1300Ver2) && (gU1300S2_ValidFirmware == 0)) //if U1300 series 2 and Firmware invalid
     {
       RetVal = 0; // will result in  ANSI_PROC_DEV (Request conflicts with current device setup. Request ignored)
     }
     else
       RetVal = FwDownloadArm(&pData[1]);
#else
     RetVal = FwDownloadArm(&pData[1]);
#endif
     break;
     //case eFwDlBlank:   // blank from offset to the end of the tank...
     //break;
  default:             RetVal = 0;     // unknown sub command
  break;
  }
  return RetVal;
}

void FwDlCtrlUpdate(void)
{
   unsigned long crc32;

   crc32=0;
   CalcTableHashPartial(eMfgT30_FwDlCtrl,&crc32,0,0);
   TableWrite(eMfgT00_Hash, eHashFirmwareM16*sizeof(_hash_entry_t), sizeof(crc32), (void *)&crc32);
}

#if defined(AMPY_METER_NO_RF) 
  void FwDownloadRelayCRCFix()
  {
    unsigned short TempS;
    unsigned long crc32 = 0;
    
    TempS = sizeof(_mfg_table_66_t) - 60;
    CalcTableHashPartial(eMfgT66_ActiveRelayConfig,&crc32,0,TempS);//we have added two bytes
    TempS = sizeof(_mfg_table_67_t) - 60;
    CalcTableHashPartial(eMfgT67_FutureRelayConfig,&crc32,0,TempS);
    TempS = sizeof(_mfg_table_68_t) - 50;
    CalcTableHashPartial(eMfgT68_RelaySettings,&crc32,0,TempS);
    TableWrite(eMfgT00_Hash, eHashRelay*sizeof(_hash_entry_t), sizeof(unsigned long), &crc32);
    UpdateHashCheck();
  }
#elif defined(AMPY_METER_U3400)
  void FwDownloadRelayCRCFix(unsigned char size)
  {
    unsigned short TempS;
    unsigned long crc32 = 0;
    
    TempS = sizeof(_mfg_table_66_t) - size;
    CalcTableHashPartial(eMfgT66_ActiveRelayConfig,&crc32,0,TempS);//we have added two bytes
    TempS = sizeof(_mfg_table_67_t) - size;
    CalcTableHashPartial(eMfgT67_FutureRelayConfig,&crc32,0,TempS);
    CalcTableHashPartial(eMfgT68_RelaySettings,&crc32,0,0);
    TableWrite(eMfgT00_Hash, eHashRelay*sizeof(_hash_entry_t), sizeof(unsigned long), &crc32);
    UpdateHashCheck();
  }
#else
void FwDownloadRelayCRCFix(unsigned char size)
{
  unsigned short TempS;
  unsigned long crc32 = 0;
  
  TempS = sizeof(_mfg_table_66_t) - size;
  CalcTableHashPartial(eMfgT66_ActiveRelayConfig,&crc32,0,TempS);//we have added two bytes
  TempS = sizeof(_mfg_table_67_t) - size;
  CalcTableHashPartial(eMfgT67_FutureRelayConfig,&crc32,0,TempS);
  CalcTableHashPartial(eMfgT68_RelaySettings,&crc32,0,0);
  TableWrite(eMfgT00_Hash, eHashRelay*sizeof(_hash_entry_t), sizeof(unsigned long), &crc32);
  
  if(gU1300Ver2 == 0)
  {
    crc32 = 0;
    CalcTableHashPartial(eMfgT73_RippleConfig,&crc32,0,(sizeof(_mfg_table_73_t) - (sizeof(ripple_cfg_sched_t)*2)));
    TableWrite(eMfgT00_Hash, eHashRippleConfig*sizeof(_hash_entry_t), sizeof(crc32), (void *)&crc32);
  }
  UpdateHashCheck();
}
#endif
/*-------------------------------------------------------------------------------
* Function:    GetMfg54Addr
* Outputs:     Returns address of Manufacturing table 54 in external flash
* Description: Return address of Manufacturing table 54 in external flash since
*              macro only works in this c file.
*------------------------------------------------------------------------------*/
unsigned long GetMfg54Addr()
{
   return ADDR_EXT_FLASH_MFG072;    // return the flash/NV copy of this table now it's RAM based?
}

/*-------------------------------------------------------------------------------
* Function:    GroupTblWrite
* Inputs:      PortSrc-Who is asking for access.
*              UserLvl-User login level.
*              pData[0]-type of group ID Primary/Secondary/Tertiary
*              pData[1],pData[2]- group ID
*              pData[3],pData[4]- table number
*              pData[5],pData[6]- table offset
*              pData[7]- size of data to be written
*              pData[8]..pData[n]--data to be written to table
* Outputs:     Returns ANSI_PROC_OK on success
* Description: Calculates interval values and and flags and places the results
*              in the provided arrays. If the interval is an energy quantity a
*              delta is calculated otherwise its a direct snapshot. Intervals
*              are determined based on value in table 28.
*------------------------------------------------------------------------------*/
unsigned char GroupTblWrite(unsigned char PortSrc, unsigned char UserLvl,unsigned char *pData)
{
   union{
      struct{
         unsigned short tempw1,tempw2,tempw3;
      }s;
      unsigned char buff[6];
   }u;
   unsigned short tempw4;  // probably not necessary?

   //check group ID type
   if((pData[0]==0)||(pData[0]>3))
      return ANSI_PROC_PAR;//not a primary, secondary or tertiary group ID

   //get group ID
   memcpy(&u.s.tempw3, &pData[1], 2);

   //check group ID
   TableRead(eMfgT68_RelaySettings, offsetof(_mfg_table_68_t,PrimaryGroupID)+2*(pData[0]-1), 2, &u.s.tempw1);
   if((u.s.tempw1!=u.s.tempw3)&&(u.s.tempw3!=0xFFFF))
      return ANSI_PROC_DEV;//message was not for me so pretend everything was OK

   //check if size too big.
   if((pData[7]>(GEN_CONFIG_TBL_DOT_MAX_PROC_PARM_LENGTH-8)) || (pData[7] == 0))//8 is the overhead for
      return ANSI_PROC_PAR;

   memcpy(&u.s.tempw1, &pData[3], 2);//table
   memcpy(&u.s.tempw2, &pData[5], 2);//offset

   //check for table write permissions
   if(!MCom_TblWriteAccess(PortSrc,UserLvl,u.s.tempw1))
      return ANSI_PROC_AUTH;

   u.s.tempw2=TableWrite(u.s.tempw1, u.s.tempw2, pData[7], &pData[8]);
   if(u.s.tempw2!=pData[7])
   {//TableWrite failed
      return ANSI_PROC_PAR;
   }

   //arrange data for event log squeze 2 12bit values into 24 bits
   //u.s.tempw1 (u.buff[0] u.buff[1]) already set with table number
   memcpy(&tempw4, &pData[5], 2);
   u.buff[1]&=0x0F;//mask out upper bits of 2 byte little endian value
   u.buff[1]|=(tempw4&0x000F)<<4;
   u.buff[2]=tempw4>>4;
   u.buff[3]=pData[7];//set length
   //u.buff[4] & u.buff[5] already set with ID

   //log event
   EventLogWrite(eEvtPrimGrpTbWrite+pData[0]-1,GetCommsUserID(),6,u.buff);
   return ANSI_PROC_OK;
}

/*-------------------------------------------------------------------------------
* Function:    Log3rdPartyEvent
* Inputs:      pData[0],pData[1]- event number
*              pData[2]..pData[8]--data to be written to event log parameter
* Outputs:     Returns ANSI_PROC_OK on success
* Description: Allows user to login to the meter and generate specific events in
*              the event log.
*------------------------------------------------------------------------------*/
unsigned char Log3rdPartyEvent(unsigned char *pData)
{
   unsigned short event;

   event=get16(&pData[0]);

   if((event==eEvtRFRegComplete)||
      (event==eEvtRFParamMod)||
      (event==eEvtRFCongestBAD)||
      (event==eEvtRFCongestEase)||
      (event==eEvtRFSigLoss)||
      (event==eEvtRFSigReturn)||
      (event==eEvtHANJoinOK)||
      (event==eEvtHANJoinBAD)||
      (event==eEvtHANMsgConfim)||
      (event==eEvtHANLoadreport))
   {
      EventLogWrite(event,GetCommsUserID(),6,&pData[2]);
      return ANSI_PROC_OK;
   }
   return ANSI_PROC_PAR;
}

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(LC_RSM)
/*-------------------------------------------------------------------------------
* Function:    GetMfg74Addr
* Outputs:     Returns address of Manufacturing table 74 in external flash
* Description: Return address of Manufacturing table 76 in external flash since
*              macro only works in this c file.
*------------------------------------------------------------------------------*/
unsigned long GetMfg74Addr()
{
   return AEMPRIPPLELOGSTARTADDR + ADDR_EXT_FLASH_MFG074;
}
#endif

/*-------------------------------------------------------------------------------
* Function:    GetStd76Addr
* Outputs:     Returns address of Standard table 76 in external flash
* Description: Return address of Standard table 76 in external flash since
*              macro only works in this c file.
*------------------------------------------------------------------------------*/
unsigned long GetStd76Addr()
{
   return AEMLOGSTARTADDR + ADDR_EXT_FLASH_STD76;
}

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
	unsigned long GetStd61Addr()
	{
	  return AEMTABLESTARTADDR + ADDR_EXT_FLASH_STD61;
	}

	unsigned long GetStd62Addr()
	{
	  return AEMTABLESTARTADDR + ADDR_EXT_FLASH_STD62;
	}

	unsigned long GetMfg36Addr()
	{
	   return AEMTABLESTARTADDR + ADDR_EXT_FLASH_MFG036;
	}
#endif
unsigned long GetMfg15Addr()
{
  return AEMTABLESTARTADDR + ADDR_EXT_FLASH_MFG015;
}
unsigned long GetMfg16Addr()
{
  return AEMTABLESTARTADDR + ADDR_EXT_FLASH_MFG016;
}

unsigned long GetMfg51Addr()
{
  return AEMTABLESTARTADDR + ADDR_EXT_FLASH_MFG051;
}
unsigned long GetMfg95Addr()
{
  return AEMLPQOSSTARTADDR + ADDR_EXT_FLASH_MFG095;
}
unsigned long GetMfg96Addr()
{
	return AEMLPQOSSTARTADDR + ADDR_EXT_FLASH_MFG096;
}
void TablePrintTblInfo(void)
{
   unsigned char TableEntryNum;
   unsigned short i;

   i=0;
   while(i<(2048+100))
   {
      TableEntryNum = TableFindEntry(i);
      if(TableEntryNum != 0xFF)
         printf("Tbl %i Addr=%08lX, Size=%i\n",i,TableConfig[TableEntryNum].Address,TableConfig[TableEntryNum].Size);
      i++;
      if(i==100)
         i=2048;
   }
}

/*-------------------------------------------------------------------------------
* Function:    ChkPendingTblWriteUpdate
* Outputs:     None
* Description: Calls table specific handler if pending .this function is to be
* called after write stream interrupted due to comms or power fail.
*------------------------------------------------------------------------------*/
void ChkPendingTblWriteUpdate(void)
{
   unsigned char TableEntryNum;

   if(gTableUpdateHandlerPending & TBL_WRITE_PACKET_IN_STREAM)      //streaming write ?
   {
      gTableUpdateHandlerPending &= ~TBL_WRITE_PACKET_IN_STREAM;    //Get table Num by stripping "stream" bit
      TableEntryNum = TableFindEntry(gTableUpdateHandlerPending);
      if(TableEntryNum == 0xFF)
         return;
      //Table has a function handler?
      if(TableConfig[TableEntryNum].pFunc!=NULL)
      {
         TableConfig[TableEntryNum].pFunc();//call table specific handler
      }

      gTableUpdateHandlerPending = 0;    //clear table info too
   }

}

#ifdef PRINT_TABLEADDR_FLASH
void TableAddrFlash()//to be called from a factory procedure
{
   unsigned short TableNum = 0;
   unsigned long FlatAddr = 0xFFFFFFFF;
   unsigned short Page;
   unsigned short PageAddr;
   for(TableNum=0; TableNum<(sizeof(TableConfig)/sizeof(_TableInfoStruct)) ;TableNum++)
   {
      switch(TableConfig[TableNum].Type)
      {
         case(eDynamic):
            if((TableConfig[TableNum].Num==eStdT23_CurrRegData)||(TableConfig[TableNum].Num==eStdT24_PrevSeaData)||(TableConfig[TableNum].Num==eStdT25_PrevDmdRstData)
               ||(TableConfig[TableNum].Num==eStdT26_SelfReadData)||(TableConfig[TableNum].Num==eMfgT40_TOUHistory))
            {
               FlatAddr = (unsigned long)(TableConfig[TableNum].Address)+(AEMTOUSTARTADDR);
            }
            else if((TableConfig[TableNum].Num>=eStdT64_LPDataSet1)&&(TableConfig[TableNum].Num<=eStdT67_LPDataSet4))
            {
               FlatAddr = (unsigned long)(TableConfig[TableNum].Address)+(AEMLPDATASTARTADDR);
            }
            else if(TableConfig[TableNum].Num==eStdT76_EventLogData)
            {
               FlatAddr = (unsigned long)(TableConfig[TableNum].Address)+(AEMLOGSTARTADDR);
            }
            else if(TableConfig[TableNum].Num==eMfgT32_FwDlDataBlk)
            {
               //RetVal = FWDataBlockWrite(Buffer, Size);
            }
            else if((TableConfig[TableNum].Num==eMfgT43_PSEMPermission)||(TableConfig[TableNum].Num==eMfgT44_PSEMXPermission)||(TableConfig[TableNum].Num==eMfgT45_KF2Permission)
                    ||(TableConfig[TableNum].Num==eMfgT46_PSEMPwdKeys)||(TableConfig[TableNum].Num==eMfgT47_PSEMXPwdKeys)||(TableConfig[TableNum].Num==eMfgT48_KF2PwdKeys))
               FlatAddr = (unsigned long)(TableConfig[TableNum].Address)+(AEMTABLESTARTADDR);
            break;
            case(eExtFlash):
               FlatAddr = (unsigned long)(TableConfig[TableNum].Address)+(AEMTABLESTARTADDR);
               break;
      }
      if(FlatAddr < 0xFFFFFFFF)
      {
         DF_map_page_and_address(FlatAddr, &Page, &PageAddr);
         printf("TableNum %d FlatAddr %lu PageNum %d PageAddr %d\n",TableConfig[TableNum].Num,FlatAddr,Page,PageAddr);
         FlatAddr = 0xFFFFFFFF;
      }
   }
}
#endif
