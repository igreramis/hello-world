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

      Module Name:      Tables.h

      Date:             29th July 2008

      Engineer:         Ian Davis, Marc Circosta

      Description:      ANSI table definitions and functions

      Changes:


************AMPY+EMAIL - CONFIDENTIAL******************************************/

#ifndef TABLESH
#define TABLESH

//tables.h is included in the MS visual studio project for the calibration exe.
//So if MSVS is doing a compile then don't include other header files
#if defined(_MSC_VER)
   #define HUGE
#else
   #include "std.h"
   #include "AMPYEmailCommon.h"
#endif
#include <stddef.h>

//--------------------------------#defs-----------------------------------------
#define MEMBER_SIZEOF(type,member)     sizeof(((type *)0)->member)

#define NUMBER_OF_ELEMENTS             3     // single or polyphase?
#define ANSI_MANFCT_TABLE_OFFSET       2048
#define ANSI_KF_CLASS_TABLE_OFFSET     2148
//(ANSI_MANFCT_TABLE_OFFSET+100)
#define TABLE_HASH_LENGTH              4

//table 0 defs
#define GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED             10 //IJD don't change allow std t00-t79
#define GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED             13 //IJD don't change allow mfg t00-t103
#define GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED             3  //IJD don't change allow std proc00-23
#define GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED             3  //IJD don't change allow mfg proc00-23
#define GEN_CONFIG_TBL_DOT_DIM_MFG_STATUS_USED           4  //IJD don't change allow 32 bits of status flags
#define GEN_CONFIG_TBL_DOT_TM_FORMAT                     2  //IJD don't change this CC requires val=2
#define GEN_CONFIG_TBL_DOT_NBR_PENDING                   10
#define GEN_CONFIG_TBL_DOT_ID_FORM                       0  //set in stone
#define GEN_CONFIG_TBL_DOT_MAX_PROC_PARM_LENGTH          40 //worst case of: 32 byte of table data + overhead (1[ID type]+2[ID]+2[tbl]+2[offset]+1[len])
#ifdef SIM_INTERRUPT
	#define GEN_CONFIG_TBL_DOT_MAX_RESP_DATA_LEN             sizeof(_sim_interrupt_t) //worst case of 2 x time&date
#else
	#define GEN_CONFIG_TBL_DOT_MAX_RESP_DATA_LEN             20 //worst case of 2 x time&date
#endif
#define GEN_CONFIG_TBL_DOT_DATA_ORDER                    0
#define GEN_CONFIG_TBL_DOT_CHAR_FORMAT                   1
#define GEN_CONFIG_TBL_DOT_MODEL_SELECT                  0
#define GEN_CONFIG_TBL_DOT_DATA_ACCESS_METHOD            1
#define GEN_CONFIG_TBL_DOT_INT_FORMAT                    0
#define GEN_CONFIG_TBL_DOT_NI_FORMAT1                    8  //IJD don't change set to INT32 signed
#define GEN_CONFIG_TBL_DOT_NI_FORMAT2                    8  //IJD don't change set to INT32 signed

//table 3 defs
#define STAT_TBL_STD_FLG_UNPROG                          0x0001      //crc hash of hash was found to be wrong
#define STAT_TBL_STD_FLG_CONFIG                          0x0002      //
#define STAT_TBL_STD_FLG_SELF_CHK                        0x0004      //
#define STAT_TBL_STD_FLG_RAM                             0x0008      //
#define STAT_TBL_STD_FLG_ROM                             0x0010      //
#define STAT_TBL_STD_FLG_NV                              0x0020      //
#define STAT_TBL_STD_FLG_CLOCK                           0x0040      //
#define STAT_TBL_STD_FLG_MEASUREMENT                     0x0080      //
#define STAT_TBL_STD_FLG_LOW_BATT                        0x0100      //battery is low but not flat
#define STAT_TBL_STD_FLG_LOW_LOSS                        0x0200      //
#define STAT_TBL_STD_FLG_DMD_OVF                         0x0400      //
#define STAT_TBL_STD_FLG_POWER_FAIL                      0x0800      //
#define STAT_TBL_STD_FLG_TAMPER                          0x1000      //tamper switch went off
#define STAT_TBL_STD_FLG_REV_ROTATE                      0x2000      //phase sequence bad
#define STAT_TBL_STD_FLG_FILLER                          0xC000      //always zero
#define STAT_TBL_MFG_FLG_BATT_FLAT                       0x00000001  //battery flat
#define STAT_TBL_MFG_FLG_PF_COUNTERS                     0x00000002  //power fail counter mismatch
#define STAT_TBL_MFG_FLG_EXP_PH_A                        0x00000004  //export energy detected on phase A
#define STAT_TBL_MFG_FLG_EXP_PH_B                        0x00000008  //export energy detected on phase B
#define STAT_TBL_MFG_FLG_EXP_PH_C                        0x00000010  //export energy detected on phase C
#define STAT_TBL_MFG_FLG_NI                              0x00000020  //Neutral Integrity
#define STAT_TBL_MFG_FLG_VOLT_LOSS_PH_A                  0x00000040  //Voltage loss detected on phase A: 0 means no volt loss;1 means volt loss
#define STAT_TBL_MFG_FLG_VOLT_LOSS_PH_B                  0x00000080  //Voltage loss detected on phase B: 0 means no volt loss;1 means volt loss
#define STAT_TBL_MFG_FLG_VOLT_LOSS_PH_C                  0x00000100  //Voltage loss detected on phase C: 0 means no volt loss;1 means volt loss
#define STAT_TBL_MFG_FLG_SELFREAD_WRITE_FAIL             0x00000200  //Self read record was found corrupt and rewrite was attempted
#define STAT_TBL_MFG_FLG_SELFREAD_REWRITE                0x00000400  //
#define STAT_TBL_MFG_FLG_DC_MAGNETIC_TAMPER              0x00000800  //DC magnetic field tamper
#define STAT_TBL_MFG_FLG_PHASEUNBALANCE                  0x00001000  //phase voltage on neutral unbalance factor greater than threshold
#define STAT_TBL_MFG_FLG_PHASEROTATION                   0x00002000  //phase rotation for polyphase meter
#define STAT_TBL_MFG_FLG_PREPAYSTATUS                    0x00004000  //status for PrePayment systems 0- means prepay is off; 1 means prepay is on
#define STAT_TBL_MFG_FLG_PREPAYOCCPSTATUS                0x00008000  //status for Prepayment Dwelling Status 0- means Occupied 1- means Unoccupied
#define STAT_TBL_MFG_FLG_17                              0x00010000  //
#define STAT_TBL_MFG_FLG_18                              0x00020000  //
#define STAT_TBL_MFG_FLG_19                              0x00040000  //
#define STAT_TBL_MFG_FLG_20                              0x00080000  //
#define STAT_TBL_MFG_FLG_21                              0x00100000  //
#define STAT_TBL_MFG_FLG_22                              0x00200000  //
#define STAT_TBL_MFG_FLG_23                              0x00400000  //
#define STAT_TBL_MFG_FLG_24                              0x00800000  //
#define STAT_TBL_MFG_FLG_25                              0x01000000  //
#define STAT_TBL_MFG_FLG_26                              0x02000000  //
#define STAT_TBL_MFG_FLG_27                              0x04000000  //
#define STAT_TBL_MFG_FLG_28                              0x08000000  //
#define STAT_TBL_MFG_FLG_29                              0x10000000  //
#define STAT_TBL_MFG_FLG_30                              0x20000000  //
#define STAT_TBL_MFG_FLG_31                              0x40000000  //
#define STAT_TBL_MFG_FLG_32                              0x80000000  //

//table 10+11+12 defs
#define ACT_SOURCES_LIM_TBL_DOT_NBR_DEMAND_CTRL_ENTRIES  24  //IJD-set to the number of energy quantities
#define ACT_SOURCES_LIM_TBL_DOT_PF_EXCLUDE_FLAG          0  // was 1
#define ACT_SOURCES_LIM_TBL_DOT_RESET_EXCLUDE_FLAG       1
#define ACT_SOURCES_LIM_TBL_DOT_SLIDING_DEMAND_FLAG      1  //IJD- sorry but Aust customers use sliding demand windows
#define ACT_SOURCES_LIM_TBL_DOT_DATA_CTRL_LENGTH         4
#define ACT_SOURCES_LIM_TBL_DOT_NBR_DATA_CTRL_ENTRIES    50 // maximum number of entries in Table 14 - Data Control
#define ACT_SOURCES_LIM_TBL_DOT_CONSTANTS_SELECTOR       2
#define ACT_SOURCES_LIM_TBL_DOT_SET1_PRESENT_FLAG        0  //IJD We do not allow external CTs and VTs
#define ACT_SOURCES_LIM_TBL_DOT_SET2_PRESENT_FLAG        0  //IJD we dont need two sets of external scaling factors
#define ACT_SOURCES_LIM_TBL_DOT_NBR_CONSTANTS_ENTRIES    16
#define ACT_SOURCES_LIM_TBL_DOT_NBR_SOURCES              51
#define ACT_SOURCES_LIM_TBL_DOT_NBR_UOM_ENTRIES          51     // doesn't focus ax define 24 of these?

//table 20+21+22 defs
#define ACT_REGS_TBL_DOT_SEASON_INFO_FIELD_FLAG          1  //IJD set to 1 don't change. We need to store the sesson with the actual data (it can be difficult to work this out based on time only)
#define ACT_REGS_TBL_DOT_DATE_TIME_FIELD_FLAG            1  //IJD set to 1 don't change. We need to store the time of max demands/self reads/demand resets with the actual data
#define ACT_REGS_TBL_DOT_DEMAND_RESET_CTR_FLAG           0  //IJD set to 0 don't change.
#define ACT_REGS_TBL_DOT_DEMAND_RESET_LOCK_FLAG          0  //IJD set to 0 don't change.
#define ACT_REGS_TBL_DOT_CUM_DEMAND_FLAG                 1  //IJD the customer can set this to 1 or 0 but can't have CUM_DEMAND_FLAG and CONT_CUM_DEMAND_FLAG
#define ACT_REGS_TBL_DOT_CONT_CUM_DEMAND_FLAG            1  //IJD the customer can set this to 1 or 0 but can't have CUM_DEMAND_FLAG and CONT_CUM_DEMAND_FLAG
#define ACT_REGS_TBL_DOT_TIME_REMAINING_FLAG             1  //IJD set to 1 don't change impacts on nothing since we don't report demand at all in tbl 28
#define ACT_REGS_TBL_DOT_SELF_READ_INHIBIT_OVERFLOW_FLAG 0  //IJD set to 0 don't change. We just over rite old self read data
#define ACT_REGS_TBL_DOT_SELF_READ_SEQ_NBR_FLAG          1  //IJD set to 1 don't change. There are sequence numbers for self reads
#define ACT_REGS_TBL_DOT_DAILY_SELF_READ_FLAG            1  //IJD set to 1 don't change. Self read configured via calender
#define ACT_REGS_TBL_DOT_WEEKLY_SELF_READ_FLAG           1  //IJD set to 1 don't change. Self read configured via calender
#define ACT_REGS_TBL_DOT_SELF_READ_DEMAND_RESET          0  //IJD set to 0 don't change. We don't really support demand and we don't want a self read stuffing up our demand.
#define ACT_REGS_TBL_DOT_NBR_SELF_READS                  8
#define ACT_REGS_TBL_DOT_NBR_SUMMATIONS                  6  //IJD set to 6 don't change. number of energy quantaties (Em3300 has 6)
#define ACT_REGS_TBL_DOT_NBR_DEMANDS                     2  //IJD set to 2 don't change. Allow 2 demands Imp and Exp, zigbee requires this
#define ACT_REGS_TBL_DOT_NBR_COIN_VALUES                 0  //IJD set to 0 don't change. no demand coincident PF information (Em3300 does not have coincident)
#define ACT_REGS_TBL_DOT_NBR_OCCUR                       1  //IJD set to 1 don't change. We only have a single max demand value, not an array n highest demands
#define ACT_REGS_TBL_DOT_NBR_TIERS                       6  //IJD set to 6 don't change. number of rates most AMPY Email meters have 4 rates and a total but zigbee says we need 6 +total
#define ACT_REGS_TBL_DOT_NBR_PRESENT_DEMANDS             0  //IJD set to 0 don't change.
#define ACT_REGS_TBL_DOT_NBR_PRESENT_VALUES              51 //set IJD changed from 60 since there will never be temp,or THD

//table 3x defs
#define ACT_DISP_TBL_DOT_ON_TIME_FLAG                    1
#define ACT_DISP_TBL_DOT_OFF_TIME_FLAG                   0
#define ACT_DISP_TBL_DOT_HOLD_TIME_FLAG                  0
#define ACT_DISP_TBL_DOT_WIDTH_DISPL_SOURCES	            2
#define ACT_DISP_TBL_DOT_NBR_DISP_SOURCES	               40
#define ACT_DISP_TBL_DOT_NBR_PRI_DISP_LISTS              3
#define ACT_DISP_TBL_DOT_NBR_PRI_DISP_LIST_ITEMS         40
#define ACT_DISP_TBL_DOT_NBR_SEC_DISP_LISTS              0
#define ACT_DISP_TBL_DOT_NBR_SEC_DISP_LIST_ITEMS         0

//table 4x defs
#define ACT_SECURITY_LIMITING_TBL_DOT_PASSWORD_LEN       4
#define ACT_SECURITY_LIMITING_TBL_DOT_NBR_PASSWORDS      4
#define ACT_SECURITY_LIMITING_TBL_DOT_NBR_PERM_USED      4
#define ACT_SECURITY_LIMITING_TBL_DOT_KEY_LEN            4
#define ACT_SECURITY_LIMITING_TBL_DOT_NBR_KEYS           4

//table 50 defs
#define ACT_TIME_TOU_TBL_DOT_CAP_TM_ZN_OFFSET_FLAG       1
#define ACT_TIME_TOU_TBL_DOT_CAP_DST_AUTO_FLAG           0
#define ACT_TIME_TOU_TBL_DOT_ANCHOR_DATE_FLAG            1
#define ACT_TIME_TOU_TBL_DOT_NBR_NON_RECURR_DATES        100
#define ACT_TIME_TOU_TBL_DOT_NBR_RECURR_DATES            20
#define ACT_TIME_TOU_TBL_DOT_NBR_TIER_SWITCHES           300 //IJD set to 300 don't change
#define ACT_TIME_TOU_TBL_DOT_SEPARATE_WEEKDAYS_FLAG      1
#define ACT_TIME_TOU_TBL_DOT_NBR_SEASONS                 6
#define ACT_TIME_TOU_TBL_DOT_NBR_SPECIAL_SCHED           4
#define ACT_TIME_TOU_TBL_DOT_SEPARATE_SUM_DEMANDS_FLAG   1
#define ACT_TIME_TOU_TBL_DOT_SORT_TIER_SWITCHES_FLAG     0
#define ACT_TIME_TOU_TBL_DOT_SORT_DATES_FLAG             0
#define ACT_TIME_TOU_TBL_DOT_SEASON_CHNG_ARMED_FLAG      0
#define ACT_TIME_TOU_TBL_DOT_SEASON_DEMAND_RESET_FLAG    0
#define ACT_TIME_TOU_TBL_DOT_SEASON_SELF_READ_FLAG       1
#define ACT_TIME_TOU_TBL_DOT_TOU_SELF_READ_FLAG          1
#define TIME_TBL_TIME_DATE_QUAL_DAY_OF_WEEK_MASK         0x07
#define TIME_TBL_TIME_DATE_QUAL_DST_FLAG_MASK            0x08
#define TIME_TBL_TIME_DATE_QUAL_GMT_FLAG_MASK            0x10
#define TIME_TBL_TIME_DATE_QUAL_TM_ZN_APPLIED_FLAG_MASK  0x20
#define TIME_TBL_TIME_DATE_QUAL_DST_APPLIED_FLAG_MASK    0x40
#define TIME_TBL_TIME_DATE_QUAL_ZERO_MASK                0x80

//table 6x defs
#define ACT_LP_TBL_DOT_SCALAR_DIVISOR_FLAG_SET1          0  //IJD don't have scaling info (same as focus AX)
#define LP_CTRL_TBL_DOT_INT_FMT_CDE1                     2
#define ACT_LP_TBL_DOT_NBR_CHNS_SET1                     4
#define ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1                 96//48 //IJD 96 is good because its divisible by 1,2,3,4,6,8 [number of channels per block] (same as focus AX)
#define ACT_LP_TBL_DOT_NBR_BLKS_SET1                     591  //TODO: IJD change value to ~269
#define ACT_LP_TBL_DOT_EXTENDED_INT_STATUS_FLAG          1  //IJD have extended status info (same as focus AX)
#define ACT_LP_TBL_DOT_BLK_END_READ_FLAG                 0  //IJD don't have absolute readings embedded in LP (same as focus AX)
#define ACT_LP_TBL_DOT_BLK_END_PULSE_FLAG                0  //IJD don't have pulse readings embedded in LP (same as focus AX)
#define ACT_LP_TBL_DOT_SIMPLE_INT_STATUS_FLAG            0  //IJD don't have simple status info (same as focus AX)
#define NUM_LP_DATA_SETS                                 4  //in the Paulaner there is 1 data set
#define NUM_CH_PER_DATA_SET                              4  //in the Paulaner there are ?? LP channels?
#define LP_INT_FLAG_DST                                  0x10  //interval extended status flag in the common nibble showing DST is active
#define LP_INT_FLAG_PF                                   0x20  //interval extended status flag in the common nibble showing a power fail occured
#define LP_INT_FLAG_CLK_FWD                              0x40  //interval extended status flag in the common nibble showing the clock was shifted forwards
#define LP_INT_FLAG_CLK_BACK                             0x80  //interval extended status flag in the common nibble showing the clock was shifted backwards
#define LP_INT_STAT_NONE                                 0  //No status flag
#define LP_INT_STAT_OVF                                  1  //Overflow
#define LP_INT_STAT_PART                                 2  //Partial interval due to common state
#define LP_INT_STAT_LONG                                 3  //Long interval due to common state
#define LP_INT_STAT_SKIP                                 4  //Skipped interval due to common state
#define LP_INT_STAT_TEST                                 5  //Interval contains test mode data
#define LP_INT_STAT_ATCOMMERROR                          6  //AT module had missing data from the modem

//table 7x defs
#define ACT_LOG_TBL_DOT_NBR_STD_EVENTS                   7 //IJD allow std events00-55
#define ACT_LOG_TBL_DOT_NBR_MFG_EVENTS                   12 //IJD allow mfg events00-55
#define ACT_LOG_TBL_DOT_HIST_DATA_LENGTH                 0 //IJD
#define ACT_LOG_TBL_DOT_NBR_HISTORY_ENTRIES              0 //IJD
#define ACT_LOG_TBL_DOT_EVENT_DATA_LENGTH                6 //IJD allow room for _stime_date_t structure where (GEN_CONFIG_TBL_DOT_TM_FORMAT==2)
#define ACT_LOG_TBL_DOT_NBR_EVENT_ENTRIES                500 //IJD all event logs fit into this space
#define ACT_LOG_TBL_DOT_EVENT_NUMBER_FLAG                0 //IJD there is no need for a history and event log entry to match
#define ACT_LOG_TBL_DOT_HIST_DATE_TIME_FLAG              1 //IJD make history log look like a event log
#define ACT_LOG_TBL_DOT_HIST_SEQ_NBR_FLAG                1 //IJD make history log look like a event log

//Ripple Configuration
#define RIPPLE_NO_TELEGRAMS                              16 //16 Telegrams allowed
#define RIPPLE_NBR_EVENTS                                150//Total 100 events

#define STIME_DATE_SIZE                                  (sizeof(_stime_date_t))
#define MFG40_TOU_NBR_SELF_READS                         14 //IJD 12 months of snapshots plus 2 for when the meter reader is running late

//-------------------manufacturing table defs-----------------------------------

//MT26
#define EXT_LC_NUM_LC                  2
#define EXT_LC_CFG_ENABLE              0x01  //If set then enable functionality
#define EXT_LC_CFG_CLOSE               0x02  //If set then close relay
#define EXT_LC_CFG_NA2                 0x04  //unused
#define EXT_LC_CFG_NA1                 0x08  //unused
#define EXT_LC_CFG_END_STATE_MSK       0xF0  //Mask for value specifying what should happen when condition ends
#define EXT_LC_UNDRFREQ_LC1_FUNCTIONAL 0x01
#define EXT_LC_UNDRFREQ_LC1_BREACH     0x02
#define EXT_LC_UNDRFREQ_LC1_TRIGGERED  0x04
#define EXT_LC_UNDRFREQ_NA1            0x08
#define EXT_LC_UNDRFREQ_LC2_FUNCTIONAL 0x10
#define EXT_LC_UNDRFREQ_LC2_BREACH     0x20
#define EXT_LC_UNDRFREQ_LC2_TRIGGERED  0x40
#define EXT_LC_UNDRFREQ_NA2            0x80

//MT26 + MT27
#define RAPPLC_NUM_MODULE           12
#define RAPPLC_NUM_MODSPARE         3
#define RAPPLC_NUM_RELAY            2
#define RAPPLC_NUM_SCHED_SW_REC     10
#define RAPPLC_NUM_SCHED_SEAS       2
#define RAPPLC_NUM_SCHED_SEAS_SPARE 2
#define RAPPLC_NUM_SPEC_DATES       12
#define RAPPLC_NUM_SPEC_DATES_SPARE 12

//MT52
#define MFG_CONST_HW_N_EXTRA_CAP_U3300                   0x01
#define MFG_CONST_HW_N_DISABLE_LC_PWRDWN_U1300           0x02
#define MFG_CONST_HW_N_NA6                               0x04
#define MFG_CONST_HW_N_NA5                               0x08
#define MFG_CONST_HW_N_NA4                               0x10
#define MFG_CONST_HW_N_NA3                               0x20
#define MFG_CONST_HW_N_NA2                               0x40
#define MFG_CONST_HW_N_NA1                               0x80
#define MFG_CONST_FE_N_RIPPLE                            0x01
#define MFG_CONST_FE_N_NA7                               0x02
#define MFG_CONST_FE_N_NA6                               0x04
#define MFG_CONST_FE_N_NA5                               0x08
#define MFG_CONST_FE_N_NA4                               0x10
#define MFG_CONST_FE_N_NA3                               0x20
#define MFG_CONST_FE_N_NA2                               0x40
#define MFG_CONST_FE_N_NA1                               0x80

//--------------------------------enums-----------------------------------------
enum
{
   eStdT00_GenConfig=0,    //data format, ansi rev, table types, etc
   eStdT01_GenMfgID,       //Mfger ID, model, HW rev, FW rev, serial No
   eStdT02_DevNamePlate,   //type, accuracy class, freq, max I, V Nom, pulse type, etc
   eStdT03_EDMODEStatus,   //error/warning flags, operating mode, etc
   eStdT04_,               //list of tables with a pending status?? Not required
   eStdT05_DeviceID,       //user defined unique meter ID
   eStdT06_UtilityInfo,    //Info for utility, like name of programmer and SW rev
   eStdT07_ProcInit,       //used for sending commands
   eStdT08_ProcResponse,   //used for getting command response data
   eStdT09_,               //Not Defined by Standard
   eStdT10_DimSrcLimit,    //maximum capabilities of data sources
   eStdT11_ActualSrcLimit, //actual capabilities of data sources
   eStdT12_UnitOfMeasure,  //unit of measure (UOM) describes all the quantities that can be reported by the meter
   eStdT13_DemandCtrl,     //demand control like interval length and flags for power up behaviour
   eStdT14_DataControl,    //Data Control?? just a big array of manufacturer defined data
   eStdT15_Constants,      //array of constants applied to each element?? like CT ratio?
   eStdT16_SrcDef,         //set of flags for each source, controls how sources relate to other tables
   eStdT17_,               //Not Defined by Standard
   eStdT18_,               //Not Defined by Standard
   eStdT19_,               //Not Defined by Standard
   eStdT20_DimRegLimit,    //maximum capabilities of measured data registers
   eStdT21_ActualReg,      //actual capabilities of data sources
   eStdT22_DataSelection,  //Data Selection, contains grouped lists of source indices. These indices point towards array elements in Table 16
   eStdT23_CurrRegData,    //contains TOU SUMMATIONS, DEMANDS
   eStdT24_PrevSeaData,    //snapshot(on season change) of tbl 23 with some flags
   eStdT25_PrevDmdRstData, //snapshot(on demand reset) of tbl 23 with some flags
   eStdT26_SelfReadData,   //array of snapshots(on mfger defined schedule) of tbl 23 with some flags
   eStdT27_PresRegSel,     //contains an array of pointers to data used to fill the table 28 array
   eStdT28_PresRegData,    //contains arrays of the present demand and values as selected by Table 27
   eStdT29_,               //Not Defined by Standard
   eStdT30_DimDisplayLim,  //
   eStdT31_ActualDisplay,  //
   eStdT32_DisplaySrc,     //
   eStdT33_PrimDispLst,    //
   eStdT34_SecndDispLst,   //
   eStdT35_,               //Not Defined by Standard
   eStdT36_,               //Not Defined by Standard
   eStdT37_,               //Not Defined by Standard
   eStdT38_,               //Not Defined by Standard
   eStdT39_,               //Not Defined by Standard
   eStdT40_,               ///Not required, we are using non standard method
   eStdT41_,               ///Not required, we are using non standard method
   eStdT42_,               //Not required, we are using non standard method
   eStdT43_,               //Not required, we are using non standard method
   eStdT44_,               //Not required, we are using non standard method
   eStdT45_,               //Not required, we are using non standard method
   eStdT46_,               //Not Defined by Standard
   eStdT47_,               //Not Defined by Standard
   eStdT48_,               //Not Defined by Standard
   eStdT49_,               //Not Defined by Standard
   eStdT50_DimLimTnTOU,    //maximum capabilities of TOU
   eStdT51_ActLimTnTOU,    //actual TOU settings
   eStdT52_Clock,          //real time clock (RTC) of meter
   eStdT53_TimeOffset,     //time zone and daylight savings (DST) settings
   eStdT54_Calender,       //an array of switch times for TOU
   eStdT55_ClockState,     //RTC used by TOU (i dont know if it differs from tbl 51)
   eStdT56_TimeRemaing,    //time remaining until the next TOU event
   eStdT57_,               //Not Defined by Standard
   eStdT58_,               //Not Defined by Standard
   eStdT59_,               //Not Defined by Standard
   eStdT60_DimLimitLP,     //maximum capabilities of LP
   eStdT61_ActualLPLimit,  //actual LP settings (tbl 62 goes into more detail)
   eStdT62_LPControl,      //LP control like what it is profiling and size of an element etc
   eStdT63_LPStatus,       //status of LP like flags and pointers to last LP block
   eStdT64_LPDataSet1,     //actual LP data
   eStdT65_LPDataSet2,     //NA
   eStdT66_LPDataSet3,     //NA
   eStdT67_LPDataSet4,     //NA
   eStdT68_,               //Not Defined by Standard
   eStdT69_,               //Not Defined by Standard
   eStdT70_LimLogDim,      //maximum capabilities of event and history log
   eStdT71_ActLogDim,      //actual event and history log settings
   eStdT72_EventID,        //specify what events could be supported
   eStdT73_HistLogCtlx,    //specifes what history logs are enabled and what tables/procedures to monitor
   eStdT74_HistLogDatax,   //history log data not currently used
   eStdT75_EventLogCtl,    //specifes what events are enabled and what tables/procedures to monitor
   eStdT76_EventLogData,   //event log data contains 500 events

   //Manufacturing ANSI tables (no spec on structure)
   eMfgT00_Hash=ANSI_MANFCT_TABLE_OFFSET,//contains hash verification vaules
   eMfgT01_,
   eMfgT02_,
   eMfgT03_,
   eMfgT04_,
   eMfgT05_MeterFeatures,
   eMfgT06_,
   eMfgT07_,
   eMfgT08_,
   eMfgT09_,
   eMfgT10_,
   eMfgT11_,
   eMfgT12_Status,
   eMfgT13_,
   eMfgT14_,
   eMfgT15_PrePayDisplay,
   eMfgT16_PrePayConfig,
   eMfgT17_FETWrite,
   eMfgT18_FETRead,
   eMfgT19_MagneticFieldCal,
   eMfgT20_RippleStatusBackup,
   eMfgT21_TempAppStatusBckp,
   eMfgT22_TempCalData,
   eMfgT23_TempAppCfg,
   eMfgT24_TempAppStatus,
   eMfgT25_CustFeatureCfg,
   eMfgT26_LCConfg,
   eMfgT27_LCStatus,
   eMfgT28_NIBackup,
   eMfgT29_CommsRAMData,
   eMfgT30_FwDlCtrl,
   eMfgT31_FwDlStatus,
   eMfgT32_FwDlDataBlk,
   eMfgT33_NIRAMData,
   eMfgT34_NIConfig,
   eMfgT35_CommsDisplayList,
   eMfgT36_LP_QOS_Cfg,
   eMfgT37_PwdLockoutCfg,
   eMfgT38_ReverseEngyCfg,
   eMfgT39_CalData3Phase,
   eMfgT40_TOUHistory,
   eMfgT41_ActivityStats,
   eMfgT42_CutomerCMAC,
   eMfgT43_PSEMPermission,
   eMfgT44_PSEMXPermission,
   eMfgT45_KF2Permission,
   eMfgT46_PSEMPwdKeys,//focus AX seems to go up to here
   eMfgT47_PSEMXPwdKeys,
   eMfgT48_KF2PwdKeys,
   eMfgT49_AsyncEventNV,
   eMfgT50_QOSConfig,
   eMfgT51_QOSEvent,
   eMfgT52_MfgConst,
   eMfgT53_OtherCal,
   eMfgT54_GnrlMeterNVStatus,
   eMfgT55_LPStatusBkp,
   eMfgT56_LPState,
   eMfgT57_BackEndCommsCfg,
   eMfgT58_ExtEventCtrl,
   eMfgT59_DisplayID,
   eMfgT60_CalData,
   eMfgT61_MetrologyRAM,
   eMfgT62_GenericRAM,
   eMfgT63_MetrologyConfig,
   eMfgT64_TestStatus,
   eMfgT65_NV_Energy,
   eMfgT66_ActiveRelayConfig,
   eMfgT67_FutureRelayConfig,
   eMfgT68_RelaySettings,
   eMfgT69_RelayStatus,
   eMfgT70_RelayStatusBackup,
   eMfgT71_AsyncEventPin,
   eMfgT72_GnrlMeterNVStatusBckp,
   eMfgT73_RippleConfig,
   eMfgT74_RippleEvtLog,
   eMfgT75_RippleResults,
   eMfgT76_LPStateRAM,
   eMfgT77_,
   eMfgT78_DemandLog,
   eMfgT79_CrashLog,
   eMfgT80_PwrCycTiming,
   eMfgT81_PulseOutput,
   eMfgT82_,
   eMfgT83_,
   eMfgT84_,
   eMfgT85_,
   eMfgT86_,
   eMfgT87_,
   eMfgT88_,
   eMfgT89_,
   eMfgT90_PartialLPDataSet1,
   eMfgT91_PartialLPDataSet2,
   eMfgT92_PartialLPDataSet3,
   eMfgT93_PartialLPDataSet4,
   eMfgT94_DiagnosticsData,
   eMfgT95_LPQoSConfig,
   eMfgT96_LPQoSData,

   //AMPY Email Kingfisher(KF) classes
   eCls000_=ANSI_KF_CLASS_TABLE_OFFSET,
   eCls140_RealTime = ANSI_KF_CLASS_TABLE_OFFSET+140,

   // End marker
   eMaxTables
};


enum{
   eHashAllData=0,         //0  0x00
   eHashAllConfig,         //1  0x01
   eHashAllVariableData,   //2  0x02
   eHashEvents,            //3  0x03
   eHashRelay,             //4  0x04
   eHashDisplay,           //5  0x05
   eHashTOU,               //6  0x06
   eHashDemand,            //7  0x07
   eHashLP,                //8  0x08
   eHashMetrology,         //9  0x09
   eHashFirmwareM16,       //10 0x0A
   eHashFirmwareZigbee,    //11 0x0B
   eHashRTC,               //12 0x0C
   eHashCommsOptical,      //13 0x0D
   eHashCommsRF,           //14 0x0E
   eHashQOSLog,            //15 0x0F
   eHashTarrif,            //16 0x10
   eHashHAN,               //17 0x11
   eHashSupplyDisconnect,  //18 0x12
   eHashLoadControl,       //19 0x13
   eHashOtherLoad,         //20 0x14
   eHashSecurity,          //21 0x15
   eHashNI,                //22 0x16
   eHashRippleConfig,      //23 0x17
   eHashTempConfig,        //24 0x18
   eHashGeneralCustCfg,    //25 0x19
   eHashPrePayConfig,      //26 0x1A
   eHashPulseOutput,       //27 0x1B
   eHashSpare4,            //28 0x1C
   eHashSpare3,            //29 0x1D
   eHashSpare2,            //30 0x1E
   eHashSpare1,            //31 0x1F
   eHashNumber             //32
};

enum{
   eBackEndUtilinet=0,
   eBackEnd3G,
   eBackEndSSN,
   eBackEndWiMax,
   eBackModem,
   eBackEndRF_SBS,
   eBackEndUnknown=0xff
};

//index into for _std_table_16_t
enum{
   eT16_WhQ14PhA=0,
   eT16_WhQ14PhB=1,
   eT16_WhQ14PhC=2,
   eT16_WhQ14Tot=3,
   eT16_WhQ23PhA=4,
   eT16_WhQ23PhB=5,
   eT16_WhQ23PhC=6,
   eT16_WhQ23Tot=7,
   eT16_VARhQ12PhA=8,
   eT16_VARhQ12PhB=9,
   eT16_VARhQ12PhC=10,
   eT16_VARhQ12Tot=11,
   eT16_VARhQ34PhA=12,
   eT16_VARhQ34PhB=13,
   eT16_VARhQ34PhC=14,
   eT16_VARhQ34Tot=15,
   eT16_VAhQ14PhA=16,
   eT16_VAhQ14PhB=17,
   eT16_VAhQ14PhC=18,
   eT16_VAhQ14Tot=19,
   eT16_VAhQ23PhA=20,
   eT16_VAhQ23PhB=21,
   eT16_VAhQ23PhC=22,
   eT16_VAhQ23Tot=23,
   eT16_VoltsPhA=24,
   eT16_VoltsPhB=25,
   eT16_VoltsPhC=26,
   eT16_CurrentPhA=27,
   eT16_CurrentPhB=28,
   eT16_CurrentPhC=29,
   eT16_CurrentTot=30,
   eT16_Frequency=31,
   eT16_AnglePhA=32,
   eT16_AnglePhB=33,
   eT16_AnglePhC=34,
   eT16_PFPhA=35,
   eT16_PFPhB=36,
   eT16_PFPhC=37,
   eT16_PFTot=38,
   eT16_ActPwrPhA=39,
   eT16_ActPwrPhB=40,
   eT16_ActPwrPhC=41,
   eT16_ActPwrTot=42,
   eT16_AppPwrPhA=43,
   eT16_AppPwrPhB=44,
   eT16_AppPwrPhC=45,
   eT16_AppPwrTot=46,
   eT16_ReactPwrPhA=47,
   eT16_ReactPwrPhB=48,
   eT16_ReactPwrPhC=49,
   eT16_ReactPwrTot=50,
   eT16_TempPhA=51,
   eT16_TempPhB=52,
   eT16_TempPhC=53,

   eT16_ATChannel1,
   eT16_ATChannel2,
   eT16_ATChannel3,
   eT16_ATChannel4
};

//-----------------------------Structures---------------------------------------

//             ++++++++++Standard ANSI Tables +++++++++++++
//user defined format used in many tables
typedef signed long ni_fmat1_t;//NI_FMAT1 -ANSI only allows for a signed 32 number (thats ok we will just never use the negative)
typedef signed long ni_fmat2_t;//NI_FMAT2 -ANSI only allows for a signed 32 number (thats ok we will just never use the negative)

typedef struct
{
#if (GEN_CONFIG_TBL_DOT_TM_FORMAT==2)
   unsigned char  hour;
   unsigned char  minute;
   unsigned char  second;
#elif(GEN_CONFIG_TBL_DOT_TM_FORMAT==3)
   unsigned long  d_time;
#endif
} _time_t;//TIME

typedef struct
{
#if (GEN_CONFIG_TBL_DOT_TM_FORMAT==2)
   unsigned char  year;
   unsigned char  month;
   unsigned char  day;
   unsigned char  hour;
   unsigned char  minute;
#elif(GEN_CONFIG_TBL_DOT_TM_FORMAT==3)
   unsigned long  u_time;
#endif
} _stime_date_t;//STIME_DATE

typedef struct
{
#if (GEN_CONFIG_TBL_DOT_TM_FORMAT==2)
   unsigned char  year;
   unsigned char  month;
   unsigned char  day;
   unsigned char  hour;
   unsigned char  minute;
   unsigned char  second;
#elif(GEN_CONFIG_TBL_DOT_TM_FORMAT==3)
   unsigned long  u_time;
   unsigned char  second;
#endif
} _ltime_date_t;//LTIME_DATE


typedef struct
{
   unsigned char month    :4;
   unsigned char offset   :4;
   unsigned char weekday  :3;
   unsigned char day      :5;
   // so the union and/or struct below implicity means 'start a new byte', and we end up with a 3 byte rdate
   /*union
   {
      struct
      {
         unsigned char offset   :4;
         unsigned char weekday  :3;
         unsigned char day      :5;
      };
      struct
      {
         unsigned char period   :6;
         unsigned char delta    :6;
      };
   };*/
} _rdate_t;//RDATE

// this struct is necessary because of the problem unioning within a bitfield demonstrated above...
typedef struct
{
   unsigned char month    :4;
   unsigned char period   :6;
   unsigned char delta    :6;
} _rdate_pd_t;//RDATE

typedef struct
{
   unsigned short YEAR  :7;
   unsigned short MONTH :4;
   unsigned short DAY   :5;
} _date_t;//DATE

typedef struct
{
   unsigned short tbl_proc_nbr      : 11;//table or procedure number
   unsigned short std_vs_mfg_flag   : 1;//0=standard table 1=manufacturing table
   unsigned short selector          : 4;//0=on completion post to tbl8 , 1=on exception post to tbl8, 2=dont post to tbl8, 3=immediate post to tbl8 and then on completion post to tbl8 again
} _table_idb_bfld_t;//TABLE_IDB_BFLD

typedef struct
{//used in table 26+74+76
   unsigned char  order_flag              :1;//Self reads are transported in descending order (N is newer than N+1).
   unsigned char  overflow_flag           :1;//An attempt was made to enter a self read such that the number of unread self reads would have exceeded the actual number of possible entries in the log.
   unsigned char  list_type_flag          :1;//Circular list as placed in self read list.
   unsigned char  inhibit_overflow_flag   :1;//The same value as SELF_READ_INHIBIT_OVERFLOW_FLAG (See “Table 21 - Actual Register”
   unsigned char  filler                  :4;//zero
} _list_status_bfld_t;//LIST_STATUS_BFLD


typedef struct
{
   unsigned char  format_control_1;//bitmask bit0=DATA_ORDER bit1-3=CHAR_FORMAT bit4-6=MODEL_SELECT
   unsigned char  format_control_2;//bitmask bit0-2=TM_FORMAT bit3-4=DATA_ACCESS_METHOD bit5=ID_FORM bit6-7=INT_FORMAT
   unsigned char  format_control_3;//bitmask bit0-3=NI_FORMAT1 bit4-7=NI_FORMAT1
   unsigned char  manufacturer[4];//manufacturer ID string "L&G"
   unsigned char  nameplate_type;//2=Electric
   unsigned char  default_set_used;//??
   unsigned char  max_proc_parm_length;//maximum length for argument parameters passed to procedures in Table 07
   unsigned char  max_resp_data_len;//maximum length representing length returned by procedures in Table 08
   unsigned char  std_version_no;//version of standard
   unsigned char  std_revision_no;//revision of standard
   unsigned char  dim_std_tbls_used;//size of std tbl array at end of struct
   unsigned char  dim_mfg_tbls_used;//size of mfg tbl array at end of struct
   unsigned char  dim_std_proc_used;//size of std proc array at end of struct
   unsigned char  dim_mfg_proc_used;//size of mfg proc array at end of struct
   unsigned char  dim_mfg_status_used;//byte allocated for for indicating manufacturer specific status flags.
   unsigned char  nbr_pending;//Number of pending status sets in Table 04
   unsigned char  std_tbls_used[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED];//bitfield showing if table is present
   unsigned char  mfg_tbls_used[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED];//bitfield showing if table is present
   unsigned char  std_proc_used[GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED];//bitfield showing if procedure can be used
   unsigned char  mfg_proc_used[GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED];//bitfield showing if procedure can be used
   unsigned char  std_tbls_write[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED];//bitfield showing if table can be written
   unsigned char  mfg_tbls_write[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED];//bitfield showing if table can be written

} _std_table_00_t;//GEN_CONFIG_TBL

typedef struct
{
   unsigned char  manufacturer[4];//manufacturer ID string "L&G" same as table 00
   unsigned char  ed_model[8];//model string "S1200R" or "S1200RL" "S1200" ??
   unsigned char  hw_version_number;//meter hardware version
   unsigned char  hw_revision_number;//meter hardware revision (minor)
   unsigned char  fw_version_number;//meter firmware revision
   unsigned char  fw_revision_number;//meter firmware revision (minor)
   unsigned char  mfg_serial_number[16];//IJD GEN_CONFIG_TBL.ID_FORM should be set (like the MSN i think)
} _std_table_01_t;//GENERAL_MFG_ID_TBL

typedef struct
{//used in table 02
   unsigned char  e_freq : 3;//2 = 50Hz, 3 =  50Hz or 60Hz, 4 = 60Hz,
   unsigned char  e_no_of_elements : 3;//1 = 1 element, 2 = 2 element, 3 = 2.5 element, 4 = 3 element
   unsigned char  e_base_type_1 : 2;
   unsigned char  e_base_type_2 : 2;
   unsigned char  e_accuracy_class : 6;
} _E_Elements_bfld_t;

typedef struct
{//used in table 02
   unsigned char  e_elements_volts : 4;
   unsigned char  e_ed_supply_volts : 4;
} _E_Volts_bfld_t;

typedef struct
{//used in table 02
   ni_fmat2_t     e_class_max_amps;//End device class or IEC maximum current rating
   ni_fmat2_t     e_ta;//The RMS amperage test amps (TA) specified by Mfgr
} _E_Amps_Rcd_t;//DEVICE_NAMEPLATE_TBL

typedef struct
{
   ni_fmat2_t     e_kh;//Wh per rev
   ni_fmat1_t     e_kt;//rate of test output (i think it may be the same as Kh)
   unsigned char  e_input_scalar;//Divisor used to scale input values ??
   unsigned char  e_ed_config[5];//Form number per ANSI C12.10. Characters??
   _E_Elements_bfld_t e_elements;//bitfield bit0-2=E_FREQ bit3-5=E_NO_OF_ELEMENTS bit6-9=E_BASE_TYPE bit10-15=E_ACCURACY_CLASS
   _E_Volts_bfld_t  e_volts;//bitfield bit0-3=E_ELEMENTS_VOLTS bit 4-7=E_ED_SUPPLY_VOLTS
   _E_Amps_Rcd_t  e_amps;//current rating
   unsigned char junk;// TODO: IJD this byte should be removed it's a hangover from a bug with structure definitions. It's just here for legacy support.
} _std_table_02_t;//DEVICE_NAMEPLATE_TBL

typedef struct
{
   unsigned char  MeteringFlag : 1;
   unsigned char  TestModeFlag : 1;
   unsigned char  MeterShopModeFlag : 1;
   unsigned char  Filler : 5;
} _E_Mode_bfld_t;

typedef struct
{
   unsigned char  UnprogrammedFlag : 1;
   unsigned char  ConfigurationErrorFlag : 1;
   unsigned char  SelfChkErrorFlag : 1;
   unsigned char  RamFailureFlag : 1;
   unsigned char  RomFailureFlag : 1;
   unsigned char  NonvolMemFailureFlag : 1;
   unsigned char  ClockErrorFlag : 1;
   unsigned char  MeasurementErrorFlag : 1;
   unsigned char  LowBatteryFlag : 1;
   unsigned char  LowLossPotentialFlag : 1;
   unsigned char  DemandOverloadFlag : 1;
   unsigned char  PowerFailureFlag : 1;
   unsigned char  TamperDetectFlag : 1;
   unsigned char  ReverseRotationFlag : 1;
   unsigned char  Filler : 2;
} _ED_Std_status1_bfld;

typedef struct
{
   _E_Mode_bfld_t       ed_mode;//bitfield bit0=METERING_FLAG bit1=TEST_MODE_FLAG bit2=METER_SHOP_MODE_FLAG bit3-7=zero
   _ED_Std_status1_bfld ed_std_status1;//bitfield bit0=UNPROGRAMMED_FLAG bit1=CONFIGURATION_ERROR_FLAG bit2=SELF_CHK_ERROR_FLAG bit3=RAM_FAILURE_FLAG bit4=ROM_FAILURE_FLAG bit5=NONVOL_MEM_FAILURE_FLAG bit6=CLOCK_ERROR_FLAG bit7=MEASUREMENT_ERROR_FLAG bit8=LOW_BATTERY_FLAG bit9=LOW_LOSS_POTENTIAL_FLAG bit10=DEMAND_OVERLOAD_FLAG bit11=POWER_FAILURE_FLAG bit12-15=zeor
   unsigned char  ed_std_status2;//bitfield bit0-7=zero
   unsigned char ed_mfg_status[GEN_CONFIG_TBL_DOT_DIM_MFG_STATUS_USED];//bitfield Mfg specific
} _std_table_03_t;//ED_MODE_STATUS_TBL

typedef struct
{//used in table 04
   unsigned char  event_selector;//bitfield bit0-3=EVENT_CODE bit4=SELF_READ_FLAG bit5=DEMAND_RESET_FLAG bit6-7=zero
   unsigned char  event_storage[5];//trigger condition that causes a pending table to become active
} _event_rcd_t;//EVENT_RCD

typedef struct
{//used in table 04
   _event_rcd_t   event;//
   unsigned short table_selector;//bitfield the table number,the standard or manufacturer flag and the pending flag. The pending flag indicates that the associated table has not been activated.
} _entry_activation_rcd_t;//ENTRY_ACTIVATION_RCD

typedef struct
{
   unsigned char  standard_pending[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED];//indicates standard tables that are capable of being written with a pending status.
   unsigned char  manufact_pending[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED];//indicates manufacturing tables that are capable of being written with a pending status.
   _stime_date_t  last_activation_date_time;//Date and time of the last pending table activated.
   unsigned char  nbr_pending_activation;//Number of activation events that have yet to be activated
   _entry_activation_rcd_t pending_tables[GEN_CONFIG_TBL_DOT_NBR_PENDING];//
} _std_table_04_t;//PENDING_STATUS_TBL

typedef struct
{//this table points directly to the NV area of device_id in table 6 using DEVICE_ID_IN_STD06
#if (GEN_CONFIG_TBL_DOT_ID_FORM==0)
   unsigned char  identification[20];//user specified unique ID
#else
#error ID_FORM must be zero
#endif
} _std_table_05_t;//DEVICE_IDENT_TBL

typedef struct
{// all of this data can be set by the user (utility)
   unsigned char  owner_name[20];//
   unsigned char  utility_div[20];//Utility division number
#if (GEN_CONFIG_TBL_DOT_ID_FORM==0)
   unsigned char  service_point_id[20];//ID number attached to the service point
   unsigned char  elec_addr[20];//End device logical electrical address for mapping and study purposes. This is used by the utility as information only
   unsigned char  device_id[20];//The ID attached to the hardware
   unsigned char  util_ser_no[20];//Utility specified serial number. Up to 20 Characters or BCD digits.
   unsigned char  customer_id[20];//Up to 20 CHARs or BCD digits. All shall be printable characters
#else
#error ID_FORM must be zero
#endif
   unsigned char  coordinate_1[10];//mapping coordinate x
   unsigned char  coordinate_2[10];//mapping coordinate y
   unsigned char  coordinate_3[10];//mapping coordinate z
   unsigned char  tariff_id[8];//billing tarrif being used
   unsigned char  ex1_sw_vendor[4];//Name of manufacturer that provided configuration/programming software.
   unsigned char  ex1_sw_version_number;//configuration/programming software.
   unsigned char  ex1_sw_revision_number;//configuration/programming software.
   unsigned char  ex2_sw_vendor[4];//Name of manufacturer that provided configuration/programming software.
   unsigned char  ex2_sw_version_number;//configuration/programming software.
   unsigned char  ex2_sw_revision_number;//configuration/programming software.
   unsigned char  programmer_name[10];//Name of the last programmer or programming device.
   union
   {
      unsigned char  misc_id[30];//generic data- undefined by ANSI
      struct
      {
         unsigned char ProgID[7];//Empwin (meter suite) Program ID
         unsigned char Version[7];//Empwin (meter suite) Program version
      }s;
   }u;
} _std_table_06_t;//TABLE UTIL_INFO_TBL

typedef struct
{
   _table_idb_bfld_t proc;//procedure number std/mfg and response type
   unsigned char     seq_nbr;//provided by initiator ans used for labeling the response (coordination)
   unsigned char     parm[GEN_CONFIG_TBL_DOT_MAX_PROC_PARM_LENGTH];//a bunch of arguments, varies depending on particular procedure
} _std_table_07_t;//PROC_INITIATE_TBL

typedef struct
{
   _table_idb_bfld_t proc;//procedure number std/mfg and response type
   unsigned char     seq_nbr;//provided by initiator ans used for labeling the response (coordination)
   unsigned char     result_code;//0=done, 1=still active, 2=invalid parameter for known proc, 3=conflict with device setup, 4=unable due to timing, 5=no authorisation, 6=unknown proc
   unsigned char     resp_data[GEN_CONFIG_TBL_DOT_MAX_RESP_DATA_LEN];//a bunch of returned arguments, varies depending on particular procedure
} _std_table_08_t;//PROC_RESPONSE_TBL

typedef struct
{//used in table 10 and 11
   unsigned char  PFExcludeFlag : 1;
   unsigned char  ResetExcludeFlag : 1;
   unsigned char  BlockDemandFlag : 1;
   unsigned char  SlidingDemandFlag : 1;
   unsigned char  ThermalDemandFlag : 1;
   unsigned char  Set1PresentFlag : 1;
   unsigned char  Set2PresentFlag : 1;
   unsigned char  Filler : 1;
} _Source_flags_bfld_t;

typedef struct
{//this structure is used for table 10 and table 11
   _Source_flags_bfld_t  source_flags;//bitfield bit0=PF_EXCLUDE_FLAG, bit1=RESET_EXCLUDE_FLAG, bit2=BLOCK_DEMAND_FLAG, bit3=SLIDING_DEMAND_FLAG, bit4=THERMAL_DEMAND_FLAG, bit5=SET1_PRESENT_FLAG, bit6=SET2_PRESENT_FLAG, bit7=zero,
   unsigned char     nbr_uom_entries;//Maximum number of entries in "Table 12 - Unit Of Measure"
   unsigned char     nbr_demand_ctrl_entries;//Maximum number of entries in "Table 13- Demand Control"
   unsigned char     data_ctrl_length;//size in bytes of a single element in the first array"Table 14 - Data Control".
   unsigned char     nbr_data_ctrl_entries;//Maximum number of entries in "Table 14 - Data Control"
   unsigned char     nbr_constants_entries;//Maximum number of entries in "Table 15- Constants"
   unsigned char     constants_selector;//selector for the record structure used in "Table 15 - Constants"
   unsigned char     nbr_sources;//Maximum number of entries in "Table 16- Source Definition"
} _std_table_10_t;//DIM_SOURCES_LIM_TBL + ACT_SOURCES_LIM_TBL

typedef struct
{//this structure is used for table 12
   unsigned char  id_code                 :8;//physical quantity of interest see UOM ID_CODE at end of this file
   unsigned char  time_base               :3;//0=Bulk Quantity of Commodity, 1=Instantaneous, 2=period of a fund freq, 3=Sub-block Average Demand, 4=Block Average Demand, 5=Net Bulk Quantity of Commodity(T1-T2), 6=Thermal quantity (Demand)., 7=Event quantity (a count)
   unsigned char  multiplier              :3;//0=10^0, 1=10^2, 2=10^3, 3=10^6, 4=10^9, 5=10^-2, 6=10^-3, 7=10^-6,
   unsigned char  q1_accountability       :1;//quantity in Quadrant 1 if set
   unsigned char  q2_accountability       :1;//quantity in Quadrant 2 if set
   unsigned char  q3_accountability       :1;//quantity in Quadrant 3 if set
   unsigned char  q4_accountability       :1;//quantity in Quadrant 4 if set
   unsigned char  net_flow_accountability :1;//0=absolute summation, 1=net summation
   unsigned char  segmentation            :3;//Phase 0=no phase, 1=A-B, 2=B-C, 3=C-A, 4=N-G, 5=A-N, 6=B-N, 7=C-N
   unsigned char  harmonic                :1;//0=whole signal, 1=harmonic of signal
   unsigned char  reserved1               :1;//zero
   unsigned char  reserved2               :7;//zero
   unsigned char  nfs                     :1;//Not Following Standard, does not follow ANSI spec
} _uom_entry_bfld_t;//UOM_ENTRY_BFLD

typedef struct
{
   _uom_entry_bfld_t uom_entry[ACT_SOURCES_LIM_TBL_DOT_NBR_UOM_ENTRIES];//Unit Of Measure array
} _std_table_12_t;//UOM_ENTRY_TBL

typedef struct
{//used in table 13
#if (ACT_SOURCES_LIM_TBL_DOT_SLIDING_DEMAND_FLAG==1)
   unsigned char  sub_int;//The number of minutes in the sub-interval
   unsigned char  int_multiplier;//The multiplier by which the SUB_INT is multiplied.
#else
   unsigned short int_length;//The length of the demand interval in minutes.
#endif
} _int_control_rcd_t;

typedef struct
{//It has been decided that the meter will not have demand but zigbee may force us to have it
#if (ACT_SOURCES_LIM_TBL_DOT_RESET_EXCLUDE_FLAG==1)
   unsigned char  reset_exclusion;//Number of minutes after demand reset to exclude additional reset action.
#endif

#if (ACT_SOURCES_LIM_TBL_DOT_PF_EXCLUDE_FLAG==1)
   unsigned char  p_fail_recogntn_tm;//Number of seconds after a power failure occurs until a valid power failure is recorded and a specified action is initiated.
   unsigned char  p_fail_exclusion;//Number of minutes after a valid power failure occurs to inhibit demand calculations.
   unsigned char  cold_load_pickup;//Number of minutes after a valid power failure occurs to provide cold load pickup functions.
#endif
   _int_control_rcd_t   interval_value[ACT_SOURCES_LIM_TBL_DOT_NBR_DEMAND_CTRL_ENTRIES];//Each of the entries in this array is associated to an entry in “Table 16 - Source Definition”
} _std_table_13_t;//DEMAND_CONTROL_TBL

/* table should not exist
typedef struct
{//used in table 14
   unsigned char  source_id[ACT_SOURCES_LIM_TBL_DOT_DATA_CTRL_LENGTH];//Each entry in this array is defined by the manufacturer as to length and content. The length is identical for all entries.
} _data_rcd_t;//DATA_RCD

typedef struct
{
   _data_rcd_t sources_id[ACT_SOURCES_LIM_TBL_DOT_NBR_DATA_CTRL_ENTRIES];//Each of the entries in this array is associated to an entry in “Table 16 - Source Definition”
} _std_table_14_t;//DATA_CONTROL_TBL
*/

typedef struct
{//used in table 15
   unsigned char  set_flags;//bitfield bit0=SET_APPLIED_FLAG, bit1-7=zero
   ni_fmat1_t     ratio_f1;//Ratio of intermediary device to allow interface of commodity flow to utility meters
   ni_fmat1_t     ratio_p1;//Ratio of intermediary device to allow interface of commodity pressure to utility meters
} _set_applied_rcd_t;//SET_APPLIED_RCD

typedef struct
{//used in table 15, ACT_SOURCES_LIM_TBL.CONSTANTS_SELECTOR==2 must be true!!
   ni_fmat1_t  multiplier;//Storage for a value used in multiplication/division adjustment.
   ni_fmat1_t  offset;//Storage for a value used in addition/subtraction adjustment.
#if (ACT_SOURCES_LIM_TBL_DOT_SET1_PRESENT_FLAG==1)
   _set_applied_rcd_t set1_constants;//Storage variable for the first set of constants and associated flags.
#endif
#if (ACT_SOURCES_LIM_TBL_DOT_SET2_PRESENT_FLAG==1)
   _set_applied_rcd_t   set2_constants;//Storage variable for the second set of constants and associated flags.
#endif
} _constants_rcd_t;//CONSTANTS_RCD

typedef struct
{
   _constants_rcd_t  selecion[ACT_SOURCES_LIM_TBL_DOT_NBR_CONSTANTS_ENTRIES];//Each of the entries in this array is associated to an entry in “Table 16 - Source Definition”
} _std_table_15_t;//CONSTANTS_TBL

typedef struct
{//used in table 16
   unsigned char  uom_entry_flag          :1;//UOM tbl12 entry in exists for this source
   unsigned char  demand_ctrl_flag        :1;//demand ctrl tbl13 entry exists for this source
   unsigned char  data_ctrl_flag          :1;//data ctrl tbl14 entry exists for this source
   unsigned char  constants_flag          :1;//constants tbl15 entry exists for this source
   unsigned char  pulse_engr_flag         :1;//0=pulse units, 1=engineering units
   unsigned char  constant_to_be_applied  :1;//if there are associated contants: 0=they have been applied, 1=they must be applied by the user after reading
   unsigned char  filler                  :2;//zero
} _source_link_bfld_t;//REG_FUNC1_BFLD

typedef struct
{
   _source_link_bfld_t  sources_link[ACT_SOURCES_LIM_TBL_DOT_NBR_SOURCES];
} _std_table_16_t;//SOURCES_TBL

typedef struct
{//used in table 20
   unsigned char  season_info_field_flag  :1;//capable of reporting the representative season in tables in this decade
   unsigned char  date_time_field_flag    :1;//capable of providing the date and time in tables in this decade.
   unsigned char  demand_reset_ctr_flag   :1;//capable of counting the number of activations of demand resets.
   unsigned char  demand_reset_lock_flag  :1;//Demand reset lockout is supported
   unsigned char  cum_demand_flag         :1;//Cumulative demand is supported
   unsigned char  cont_cum_demand_flag    :1;//Continuous cumulative demand is supported
   unsigned char  time_remaining_flag     :1;//has ability to report time remaining in demand interval.
   unsigned char  filler                  :1;//zero
} _reg_func1_bfld_t;//REG_FUNC1_BFLD

typedef struct
{//used in table 20
   unsigned char  self_read_inhibit_overflow_flag  :1;//can inhibiting self reads once an overflow occurs.
   unsigned char  self_read_seq_nbr_flag           :1;//capable of providing a self read sequential number
   unsigned char  daily_self_read_flag             :1;//daily self reads are supported. These readings are taken at 00:00:00 hours.
   unsigned char  weekly_self_read_flag            :1;//weekly self reads are supported. These readings are taken at 00:00:00 hours on Sunday.
   unsigned char  self_read_demand_reset           :2;//0=do none, 1=self read on every Demand Reset, 2= Demand Reset on every Self Read, 3=do both
   unsigned char  filler                           :2;//zero
} _reg_func2_bfld_t;//REG_FUNC2_BFLD

typedef struct
{//used for table 20(DIM_REGS_TBL) and 21(ACT_REGS_TBL)
   _reg_func1_bfld_t reg_func1_flags;//
   _reg_func2_bfld_t reg_func2_flags;//
   unsigned char     nbr_self_reads;//Number of Self Reads in use
   unsigned char     nbr_summations;//number of summation registers in each data block.
   unsigned char     nbr_demands;//number of demand registers in each data block.
   unsigned char     nbr_coin_values;//number of coincident values saved concurrently in each data block.
   unsigned char     nbr_occur;//number of occurrences stored for a particular selection.
   unsigned char     nbr_tiers;//number of tiers that data can be stored in.
   unsigned char     nbr_present_demands;//number of present demand values that can be stored.
   unsigned char     nbr_present_values;//number of present values that can be stored.
} _std_table_20_t;//DIM_REGS_TBL

typedef struct
{
   unsigned char  summation_select[ACT_REGS_TBL_DOT_NBR_SUMMATIONS];//A list of source indices that group together summation (bulk energy) sources.
   unsigned char  demand_select[ACT_REGS_TBL_DOT_NBR_DEMANDS];//A list of source indices that groups together demand sources.
   unsigned char  min_or_max_flags[(ACT_REGS_TBL_DOT_NBR_DEMANDS+7)/8];//A set of bit flags, each corresponding to an entry in DEMAND_SELECT. 0=entry is a minimum. 1=entry is a maximum.
#if (ACT_REGS_TBL_DOT_NBR_COIN_VALUES!=0)
   unsigned char  coincident_select[ACT_REGS_TBL_DOT_NBR_COIN_VALUES];//A list of source indices that group together demand sources with time period processing equal to that of the DEMAND_SELECT sources.
   unsigned char  coin_demand_assoc[ACT_REGS_TBL_DOT_NBR_COIN_VALUES];//Each entry corresponds to an entry in COINCIDENT_SELECT and provides an index into DEMAND_SELECT identifying the demand for which this coincident value is taken.
#endif
} _std_table_22_t;//DATA_SELECTION_TBL


typedef struct
{//used in table 23
   ni_fmat2_t           coincident_values[ACT_REGS_TBL_DOT_NBR_OCCUR];//Coincident values selected by COINCIDENT_SELECT
} _coincidents_rcd_t;//COINCIDENTS_RCD

typedef struct
{//used in table 23
#if (ACT_REGS_TBL_DOT_DATE_TIME_FIELD_FLAG==1)
   _stime_date_t        event_time[ACT_REGS_TBL_DOT_NBR_OCCUR];//time of each minimum or maximum recorded. Entry in position 0 corresponds to the entry in position 0 of the DEMAND array.
#endif
#if (ACT_REGS_TBL_DOT_CUM_DEMAND_FLAG==1)
   ni_fmat1_t           cum_demand;//The value of the cumulative demand register.
#endif
//we don't allow "cumulative demand" and "continuous cumulative demand" to exist at the same time
#if ((ACT_REGS_TBL_DOT_CONT_CUM_DEMAND_FLAG==1)&&(ACT_REGS_TBL_DOT_CUM_DEMAND_FLAG!=1))
   ni_fmat1_t           cont_cum_demand;//The value of the continuous cumulative demand register.
#endif
   ni_fmat2_t           demand[ACT_REGS_TBL_DOT_NBR_OCCUR];//min/max values. The value in position zero is the smallest minimum or largest maximum. In position 1 is the second smallest minimum or the second largest maximum and so on.
} _demands_rcd_t;//DEMANDS_RCD

typedef struct
{//used in table 23
   ni_fmat1_t           summations[ACT_REGS_TBL_DOT_NBR_SUMMATIONS];//contains the values of the selected summation measurements
   _demands_rcd_t       demands[ACT_REGS_TBL_DOT_NBR_DEMANDS];//values of the selected minimum/maximum demand measurements.
#if (ACT_REGS_TBL_DOT_NBR_COIN_VALUES!=0)
   _coincidents_rcd_t   coincidents[ACT_REGS_TBL_DOT_NBR_COIN_VALUES];//values of the selected measurement coincident to the selected minimum/maximum demands.
#endif
} _data_blk_rcd_t;//DATA_BLK_RCD

typedef struct
{
#if (ACT_REGS_TBL_DOT_DEMAND_RESET_CTR_FLAG==1)
   unsigned char        nbr_demand_resets;//Indicates the number of demand resets executed by the end device.
#endif
   _data_blk_rcd_t      tot_data_block;//Data block independent of Time of Use structures or other means of selection.
   _data_blk_rcd_t      tier_data_block[ACT_REGS_TBL_DOT_NBR_TIERS];//Data block dependent on Time of Use structures or other means of selection.
} _std_table_23_t;//CURRENT_REG_DATA_TBL

typedef struct
{//used in table 24 and 25 and 26
#if (ACT_REGS_TBL_DOT_DATE_TIME_FIELD_FLAG==1)
   _stime_date_t  end_date_time;//time when the snapshot of the current register data occurred.
#endif
#if (ACT_REGS_TBL_DOT_SEASON_INFO_FIELD_FLAG==1)
   unsigned char  season;//Current season value when the snapshot of the current register data occurred.
#endif
} _register_info_rcd_t;//REGISTER_INFO_RCD

typedef struct
{
   _register_info_rcd_t register_info;//time of snapshot
   _std_table_23_t      prev_season_reg_data;//Contains a snapshot of Table 23 stored at the time of the last season change.
} _std_table_24_t;//PREVIOUS_SEASON_DATA_TBL

typedef struct
{
   _register_info_rcd_t register_info;//time of snapshot
   _std_table_23_t      prev_demand_reset_data;//Contains a snapshot of “Table 23” stored at the time of the last demand reset.
} _std_table_25_t;//PREVIOUS_DEMAND_RESET_DATA_TBL

typedef struct
{//used in table 26
#if (ACT_REGS_TBL_DOT_SELF_READ_SEQ_NBR_FLAG==1)
   unsigned short       self_read_seq_nbr;//This number is initialized to 0 when the end device is configured. This number increments with each self read occurrence.
#endif
   _register_info_rcd_t register_info;//time of snapshot
   _std_table_23_t      self_read_register_data;//A snapshot of current register data taken when requested by any means offered by the end device.
} _self_read_data_rcd_t;//SELF_READ_DATA_RCD

typedef struct
{
   _list_status_bfld_t     list_status;//staus flags
   unsigned char           nbr_valid_entries;//Number of valid self reads stored in the self read array. The range is zero (meaning no self reads in the array) to 255
   unsigned char           last_entry_element;//The array element number of the newest valid entry in the self read array. This field is valid only if NBR_VALID_ENTRIES is greater than zero.
   unsigned short          last_entry_seq_nbr;//The sequence number of the last element in the self read array.
   unsigned char           nbr_unread_entries;//The number of self read entries that have not been read. This number is only updated by a procedure
   _self_read_data_rcd_t   self_reads_entries[ACT_REGS_TBL_DOT_NBR_SELF_READS];//An array of snapshots of register data taken at a prescribed instance in time.
} _std_table_26_t;//SELF_READ_DATA_TBL

typedef struct
{// header struct for working with std table 26
   _list_status_bfld_t     list_status;//staus flags
   unsigned char           nbr_valid_entries;//Number of valid self reads stored in the self read array. The range is zero (meaning no self reads in the array) to 255
   unsigned char           last_entry_element;//The array element number of the newest valid entry in the self read array. This field is valid only if NBR_VALID_ENTRIES is greater than zero.
   unsigned short          last_entry_seq_nbr;//The sequence number of the last element in the self read array.
   unsigned char           nbr_unread_entries;//The number of self read entries that have not been read. This number is only updated by a procedure
} _std_table_26_header_t;

typedef struct
{
#if (ACT_REGS_TBL_DOT_NBR_PRESENT_DEMANDS!=0)
   unsigned char  present_demand_select[ACT_REGS_TBL_DOT_NBR_PRESENT_DEMANDS];//data source selectors used to measure the present demand. The present demand values shall be reported at the same index position in PRESENT_DEMAND see Table 28 - Present Register
#endif
   unsigned char  present_value_select[ACT_REGS_TBL_DOT_NBR_PRESENT_VALUES];//data source selectors used to measure the present value. The present values shall be reported at the same index position in PRESENT_VALUE See Table 28 - Present Register Data
} _std_table_27_t;//PRESENT_REGISTER_SELECT_TBL

typedef struct
{//used in table 28
#if (ACT_REGS_TBL_DOT_TIME_REMAINING_FLAG==1)
   _time_t     time_remaining;//Time remaining to the end of the present demand interval
#endif
   ni_fmat2_t  demand_value;//The present demand value
} _present_demand_rcd_t;//PRESENT_DEMAND_RCD

typedef struct
{
#if (ACT_REGS_TBL_DOT_NBR_PRESENT_DEMANDS!=0)
   _present_demand_rcd_t   present_demand[ACT_REGS_TBL_DOT_NBR_PRESENT_DEMANDS];//Array that contains present demand selected by PRESENT_DEMAND_SELECT (table 27)
#endif
   union
   {
      ni_fmat1_t           present_value[ACT_REGS_TBL_DOT_NBR_PRESENT_VALUES];//Array that contains present values selected by PRESENT_VALUE_SELECT (table 27)
      struct
      {
#if (ACT_REGS_TBL_DOT_NBR_PRESENT_VALUES!=51)
#error "Energy + QOS values in table 28 have a size mismatch"
#endif
         ni_fmat1_t        wh_Q14[4];//
         ni_fmat1_t        wh_Q23[4];//
         ni_fmat1_t        varh_Q12[4];//
         ni_fmat1_t        varh_Q34[4];//
         ni_fmat1_t        vah_Q14[4];//
         ni_fmat1_t        vah_Q23[4];//
         ni_fmat1_t        voltage[3];//
         ni_fmat1_t        current[4];//
         ni_fmat1_t        frequency;//
         ni_fmat1_t        v_angle[3];//
         ni_fmat1_t        pf[4];//
         ni_fmat1_t        act_power[4];//
         ni_fmat1_t        app_power[4];//
         ni_fmat1_t        var_power[4];//
      };
   };
} _std_table_28_t;//PRESENT_REGISTER_DATA_TBL

typedef struct
{//used by table 30
   unsigned char  on_time_flag:1;//is programmable on time supported by the end device
   unsigned char  off_time_flag:1;//is programmable off time supported by the end device
   unsigned char  hold_time_flag:1;//is programmable hold time supported by the end device
   unsigned char  filler:5;
}_display_flag_feld_t;//DISP_FLAG_FELD

typedef struct
{
   _display_flag_feld_t display_flag_bfld;//display bitfields flags
   unsigned short       nbr_disp_sources;//max number of display sources
   unsigned char        width_disp_sources;//max number of octets a manufacturer may use to define the row width of table 32
   unsigned short       nbr_pri_disp_list_items;//max number of primary display items per display list
   unsigned char        nbr_pri_disp_lists;//max number of primary display list defined in table 33
   unsigned short       nbr_sec_disp_list_items;//max number of secondary display items per display list defined in table 34
   unsigned char        nbr_sec_display_lists;//max number of secondary display lists defined in table 34
}_std_table_30_t;//DIM_DISP_TBL

typedef struct
{
   _display_flag_feld_t	display_flag_bfld;//display bitfields flags
   unsigned short       nbr_disp_sources;//actual number of display sources
   unsigned char        width_disp_sources;//actual number of octets a mnufacturer uses to define the row width of all entries
   unsigned short       nbr_pri_disp_list_items;//actual number of primary display items per display list defined in table 33
   unsigned char        nbr_pri_disp_lists;//actual number of primary display lists defined in table 33
   unsigned short       nbr_sec_disp_list_items;//actual number of secondary display items per  sdisplay list defined in table 34
   unsigned char        nbr_sec_display_lists;//actual number of secondary display lists defined in table 34
}_std_table_31_t;//ACT_DISP_TBL

typedef struct
{//used by table 32
   unsigned char   display_source[ACT_DISP_TBL_DOT_WIDTH_DISPL_SOURCES];//manufacturer defined display description entry, size of a single item
}_display_source_t;//DISP_SOURCE_DESC_RCD

typedef struct
{
   _display_source_t display_sources[ACT_DISP_TBL_DOT_NBR_DISP_SOURCES];//contains the manufacturer defined display description entries, number of items
}_std_table_32_t;//DISP_SOURCE_TBL

typedef struct
{//used by table 33
   unsigned char   on_time:4;//the number of seconds each display timem is displayed
   unsigned char   off_time:4;//the number of seconds the display is blank between display items
}_disp_scroll1_bfld_t;//DISP_SCROLL1_BFLD

typedef struct
{//used by table 33
   unsigned char  hold_time:4;//num of minutes to wait before automatilcally restarting the normal display scrol after hold has been initiated.
   unsigned char  default_list:4;//associates this display list with a standard display list type.
}_disp_scroll2_bfld_t;//DISP_SCROLL2_BFLD

typedef struct
{//used by table 33
   _disp_scroll1_bfld_t disp_scroll1_bfld;//contains the on_time and off_time values for the associated list.
   _disp_scroll2_bfld_t disp_scroll2_bfld;//contains hold time and default list values for the associated list.
   unsigned char        nbr_list_items;
}_disp_list_desc_rcd_t;//DISP_LIST_DESC_RCD

typedef struct
{
   _disp_list_desc_rcd_t   pri_disp_list[ACT_DISP_TBL_DOT_NBR_PRI_DISP_LISTS];//array of primary display list descriptors
   unsigned short          pri_disp_sources[ACT_DISP_TBL_DOT_NBR_PRI_DISP_LIST_ITEMS];//array of pointers to table 32
}_std_table_33_t;//PRI_DISP_LIST_TBL

#if (ACT_DISP_TBL_DOT_NBR_SEC_DISP_LISTS!=0)
typedef struct
{
   _disp_list_desc_rcd_t   sec_disp_list[ACT_DISP_TBL_DOT_NBR_SEC_DISP_LISTS];//array of secondary display list descriptors
   unsigned short          sec_disp_sources[ACT_DISP_TBL_DOT_NBR_SEC_DISP_LIST_ITEMS];//array of pointers to table 32
}_std_table_34_t;//SEC_DISP_LIST_TBL
#endif

typedef struct
{//used in std tbl 50/51
   unsigned char  tou_self_read_flag      : 1;
   unsigned char  season_self_read_flag   : 1;
   unsigned char  season_demand_reset_flag: 1;
   unsigned char  season_chng_armed_flag  : 1;
   unsigned char  sort_dates_flag         : 1;
   unsigned char  anchor_date_flag        : 1;
   unsigned char  filler                  : 2;
} _time_func_flag1_bfld_t;

typedef struct
{//used in std tbl 50/51
   unsigned char  cap_dst_auto_flag       : 1;
   unsigned char  separate_weekdays_flag  : 1;
   unsigned char  seperate_sum_demands_flag : 1;
   unsigned char  sort_tier_switches_flag : 1;
   unsigned char  cap_tm_tz_offset_flag   : 1;
   unsigned char  filler                  : 3;
} _time_func_flag2_bfld_t;

typedef struct
{//used in std tbl 50/51
   unsigned char  nbr_seasons       :4;//number of seasons supported
   unsigned char  nbr_special_sched :4;//number of special schedules supported
} _calendar_bfld_t;//CALENDAR_BFLD

//TOU_SELF_READ_FLAG, trigger a TOU self read if set
//SEASON_SELF_READ_FLAG, trigger a self read on season change if set
//SEASON_DEMAND_RESET_FLAG, automatic demand reset on season change if set
//SEASON_CHNG_ARMED_FLAG, arm meter for a season change if set, does not change until a demand reset occurs
//SORT_DATES_FLAG,requires the NON_RECURR_DATES to be pre-sorted by date (Year, Month, Day) when received (Tbl54 - Calendar). Non-recurring dates starting with 1990 and ending with 2089 shall occur in that order. No sorting of recurring dates, if present, is assumed.
//ANCHOR_DATE_FLAG,device is capable of accepting an Anchor Date for the Period/Delta RDATE type (SRecurring Date Format -RDATE).
//CAP_DST_AUTO_FLAG,capable of handling daylight savings time changes independently of dates supplied by Tbl54 - Calendar.
//SEPARATE_WEEKDAYS_FLAG,capable of having a different schedule for each of the five weekdays.
//SEPARATE_SUM_DEMANDS_FLAG,capable of switching summation and demands independently.
//SORT_TIER_SWITCHES_FLAG,requires that TIER_SWITCHES be sorted (Table 54 - Calendar).
//CAP_TM_ZN_OFFSET_FLAG,Time zone offset capability is supported
typedef struct
{//used for table 50 and 51
   _time_func_flag1_bfld_t     time_func_flag1;//bitfield bit0=TOU_SELF_READ_FLAG, bit1=SEASON_SELF_READ_FLAG, bit2=SEASON_DEMAND_RESET_FLAG, bit3=SEASON_CHNG_ARMED_FLAG, bit4=SORT_DATES_FLAG, bit5=ANCHOR_DATE_FLAG, bit6-7=zero,
   _time_func_flag2_bfld_t     time_func_flag2;//bitfield bit0=CAP_DST_AUTO_FLAG, bit1=SEPARATE_WEEKDAYS_FLAG, bit2=SEPARATE_SUM_DEMANDS_FLAG, bit3=SORT_TIER_SWITCHES_FLAG, bit4=CAP_TM_ZN_OFFSET_FLAG, bit5-7=zero,
   _calendar_bfld_t  calendar_func;//
   unsigned char     nbr_non_recurr_dates;//number of non-recurring dates
   unsigned char     nbr_recurr_dates;//number of recurring dates
   unsigned short    nbr_tier_switches;//number of tier switches supported by the end device calendar. This number comprises the total collection of tier switches for all day types in the DAILY_SCHEDULE_ID_MATRIX (Table 54 - Calendar).
   unsigned short    calendar_tbl_size;//size of data transported by "Table 54 - Calendar" This includes recurring dates, nonrecurring dates and tier switches. Anchor date is counted even if it is unused.
} _std_table_50_t;//TABLE DIM_TIME_TOU_TBL + ACT_TIME_TOU_TBL



//DAY_OF_WEEK, sunday=0 saturday=6 7=unknown
//DST_FLAG, device in DST mode
//GMT_FLAG, device time matched GMT
//TM_ZN_APPLIED_FLAG, time zone offset has been applied to device time
//DST_APPLIED_FLAG, DST offset has been applied to device time
typedef struct
{
   _ltime_date_t  clock_calendar;//current device time
   unsigned char  time_date_qual;//bitfield bit0-2=DAY_OF_WEEK, bit3=DST_FLAG, bit4=GMT_FLAG, bit5=TM_ZN_APPLIED_FLAG, bit6=DST_APPLIED_FLAG, bit7=zero,
   //for DAY_OF_WEEK sunday=0
} _std_table_52_t;//CLOCK_TBL
/*
typedef struct
{
   TimeDate SoftTime;      // this is 6 bytes
   unsigned char TimeDateQual;   // end of Table 52 data - 7 bytes
} _std_table_52_t;*/

typedef struct
{
   _time_t        dst_time_eff;//The time of day the end device shifts to or from daylight saving time. The date of occurrence is based on entries in "Table 54 - Calendar" and the CAP_DST_AUTO_FLAG variable (Tbl51 - Actual Time And TOU Limiting).
   unsigned char  dst_time_amt;//DST time adjustment specified in minutes.
#if (ACT_TIME_TOU_TBL_DOT_CAP_TM_ZN_OFFSET_FLAG!=0)
   signed short   time_zone_offset;//Offset for end device in minutes
#endif
} _std_table_53_t;//TIME_OFFSET_TBL

typedef struct
{//used in table 54
   unsigned char calendar_ctrl      :5;//0=no action, 1=DST on, 2=DST off, 3-18 slect season 0-15, 19-30 special schedule, 31=end of list
   unsigned char demand_reset_flag  :1;//Demand reset shall occur at 00:00 A.M. on the date specified.
   unsigned char self_read_flag     :1;//Self read shall occur at 00:00 A.M. on the date specified.
   unsigned char filler             :1;//zero
} _calendar_action_bfld_t;//CALENDAR_ACTION_BFLD

typedef struct
{//used in table 54
   _rdate_t                recurr_date;//Date definition of a recurring event. can be yearly, monthly, or weekly. The recurring event is referenced to midnight.
   _calendar_action_bfld_t calendar_action;//
} _recurr_date_rcd_t;//RECURR_DATE_RCD

typedef struct
{//used in table 54
   _date_t                 non_recurr_date;//Non-recurring event date. Action shall occur at midnight.
   _calendar_action_bfld_t calendar_action;//
} _non_recurr_date_rcd_t;//NON_RECURR_DATE_RCD

typedef struct
{//used in table 54
   unsigned short new_tier                :3;//Identifies the tier number that becomes active defined by the DAILY_SCHEDULE_ID_MATRIX
   unsigned short summation_switch_flag   :1;//Specifies whether or not this tier switch selects a new tier for summation. Only used when SEPARATE_SUM_DEMANDS_FLAG==1
   unsigned short demands_switch_flag     :1;//Specifies whether or not this tier switch selects a new tier for demands. Only used when SEPARATE_SUM_DEMANDS_FLAG==1
   unsigned short switch_min              :6;//The minute value when the tier switch occurs.
   unsigned short switch_hour             :5;//The hour value when the tier switch occurs.
} _tier_switch_bfld_t;//TIER_SWITCH_BFLD

typedef struct
{//used in table 54
   _tier_switch_bfld_t  tier_switch;//
   unsigned char        day_sch_num;//daily schedule type associated with the tier switch. All identifiers are stored in the DAILY_SCHEDULE_ID_MATRIX.
} _tier_switch_rcd_t;//TIER_SWITCH_RCD

typedef struct
{
#if (ACT_TIME_TOU_TBL_DOT_ANCHOR_DATE_FLAG==1)
   _date_t                 anchor_date;//any recurring date using the PERIOD/OFFSET type shall use this date as a starting date.
#endif
   _non_recurr_date_rcd_t  non_recurr_dates[ACT_TIME_TOU_TBL_DOT_NBR_NON_RECURR_DATES];//non-recurring dates
   _recurr_date_rcd_t      recurr_dates[ACT_TIME_TOU_TBL_DOT_NBR_RECURR_DATES];//recurring dates
   _tier_switch_rcd_t      tier_switches[ACT_TIME_TOU_TBL_DOT_NBR_TIER_SWITCHES];//tier switches
   //The identifier is used to tie tier switch entries to specific daily schedules
#if (ACT_TIME_TOU_TBL_DOT_SEPARATE_WEEKDAYS_FLAG==1)
   unsigned char           daily_schedule_id_matrix[ACT_TIME_TOU_TBL_DOT_NBR_SEASONS][(ACT_TIME_TOU_TBL_DOT_NBR_SPECIAL_SCHED+7)];//
#else
   unsigned char           daily_schedule_id_matrix[ACT_TIME_TOU_TBL_DOT_NBR_SEASONS][(ACT_TIME_TOU_TBL_DOT_NBR_SPECIAL_SCHED+3)];//
#endif
} _std_table_54_t;//CALENDAR_TBL

typedef struct
{//used in table 55
   //union
   //{
     // struct
      //{
         unsigned short curr_summ_tier    :3;//Active tier corresponding to summations. only used when SEPARATE_SUM_DEMANDS_FLAG==1
         unsigned short curr_demand_tier  :3;//Active tier corresponding to demands. only used when SEPARATE_SUM_DEMANDS_FLAG==1
      //};
      //struct
      //{
        // unsigned short curr_tier         :3;//Number representing the tier that is currently active in the meter. only used when SEPARATE_SUM_DEMANDS_FLAG is set
       //  unsigned short filler            :3;//zero
     // };
   //};
   unsigned short tier_drive              :2;//0=Tier selection is controlled by TIER_SWITCHES 1-3=Tier selection is not controlled by this standard.
   unsigned short special_schd_active     :4;//0-11 Active special schedule number 12-14=reserved 15=No special schedule active.
   unsigned short season                  :4;//Current end device season number
} _status_bfld_t;//STATUS_BFLD

typedef struct
{
   _ltime_date_t  clock_calendar;//Current end device time.
   unsigned char  time_date_qual;//bitfield bit0-2=DAY_OF_WEEK, bit3=DST_FLAG, bit4=GMT_FLAG, bit5=TM_ZN_APPLIED_FLAG, bit6=DST_APPLIED_FLAG, bit7=zero,
   //for DAY_OF_WEEK sunday=0
   _status_bfld_t status;
} _std_table_55_t;//CLOCK_STATE_TBL
/*
typedef struct
{
   // start of Table 55 data - 9 bytes
   _std_table_52_t Clock;  // Std Table 52 part of Std Table 55
   unsigned short TOU_Status;   // end of Table 55 data - 9 bytes
} _std_table_55_t;*/

typedef struct
{
#if (ACT_TIME_TOU_TBL_DOT_SEPARATE_SUM_DEMANDS_FLAG==1)
   unsigned short summ_tier_time_remain; //Minutes remaining until the next tier change which affects summations.
   unsigned short demand_tier_time_remain;//Minutes remaining before the next tier change which affects demands.
#else
   unsigned short tier_time_remain;//Minutes remaining before the next tier change.
#endif
   unsigned char  self_read_days_remain;//Days remaining before the next scheduled self read.
} _std_table_56_t;//TIME_REMAIN_TBL

typedef struct
{//struct used in table 60
   unsigned short lp_set1_inhibit_ovf_flag   :1;//device is inhibiting load profile set 1 once an overflow occurs.
   unsigned short lp_set2_inhibit_ovf_flag   :1;//device is inhibiting load profile set 2 once an overflow occurs.
   unsigned short lp_set3_inhibit_ovf_flag   :1;//device is inhibiting load profile set 3 once an overflow occurs.
   unsigned short lp_set4_inhibit_ovf_flag   :1;//device is inhibiting load profile set 4 once an overflow occurs.
   unsigned short blk_end_read_flag          :1;//device is providing block register reading information
   unsigned short blk_end_pulse_flag         :1;//device does have a running pulse accumulator
   unsigned short scalar_divisor_flag_set1   :1;//device does have scalars and divisors associated with set 1 LP
   unsigned short scalar_divisor_flag_set2   :1;//device does have scalars and divisors associated with set 2 LP
   unsigned short scalar_divisor_flag_set3   :1;//device does have scalars and divisors associated with set 3 LP
   unsigned short scalar_divisor_flag_set4   :1;//device does have scalars and divisors associated with set 4 LP
   unsigned short extended_int_status_flag   :1;//device returns extended interval status with load profile interval data
   unsigned short simple_int_status_flag     :1;//device returns simple interval status with load profile interval data
   unsigned short filler                     :4;//zero
} _lp_flags_bfld_t;//LP_FLAGS_BFLD

typedef struct
{//struct used in table 60
   unsigned char  inv_uint8_flag    :1;//format is uint8
   unsigned char  inv_uint16_flag   :1;//format is uint16
   unsigned char  inv_uint32_flag   :1;//format is uint32
   unsigned char  inv_int8_flag     :1;//format is int8
   unsigned char  inv_int16_flag    :1;//format is int16
   unsigned char  inv_int32_flag    :1;//format is int32
   unsigned char  inv_ni_fmat1_flag :1;//format is ni_fmat1 (manufacturer defined)
   unsigned char  inv_ni_fmat2_flag :1;//format is ni_fmat2 (manufacturer defined)
} _lp_fmats_bfld_t;//LP_FMATS_BFLD

typedef struct
{//struct used in table 60
   unsigned short Nbr_Blks;//number of data blocks
   unsigned short Nbr_blk_ints;//number of intevals per data block
   unsigned char  Nbr_Chns;//number of channels
   unsigned char  Max_Int_Time;//interval duration
} _lp_config_t;

//the size of this is 13 in the paulaner
typedef struct
{// table 60 and 61
   unsigned long     LP_Memory_Len;//size in bytes of whole LP storage area
   _lp_flags_bfld_t  lp_flags;//see above
   _lp_fmats_bfld_t  lp_fmats;//see above
   _lp_config_t      Config[NUM_LP_DATA_SETS];//see above
} _std_table_60_t;

typedef struct
{//used for table 62
   unsigned char Chnl_Flag;//bitfield bit0=END_RDG_FLAG, if set The channel does have an associated end reading. bit1-7=unused
   unsigned char LP_Source_Select;//This is an index into “Table 16 - Source Definition” on page 5-27) that identifies source of the interval data
   unsigned char End_Blk_Rdg_Source_Select;//This is an index into Table 16 - Source Definition, that identifies source of the block end time reading data for a specific channel.
} _LP_Source_Sel_RCD_t;//

typedef struct
{//used for table 62
   _LP_Source_Sel_RCD_t LP_Sel_Set[NUM_CH_PER_DATA_SET];//see above
   unsigned char        Int_Fmt_Cde;//format of LP interval: 1=UINT8,2=UINT16,4=UINT32,8=INT8,16=INT16,32=INT32,64=NI_FMAT1,128=NI_FMAT2
#if (ACT_LP_TBL_DOT_SCALAR_DIVISOR_FLAG_SET1==1)
   unsigned short       Scalars_Set[NUM_CH_PER_DATA_SET];//An array of scalars applied to interval data before recording pulse data in LP
   unsigned short       Divisor_Set[NUM_CH_PER_DATA_SET];//An array of divisors applied to interval data before recording pulse data in LP
#endif
} _Data_Selection_RCD;

//the size of this is 25 in the Paulaner
typedef struct
{
   _Data_Selection_RCD Format[NUM_LP_DATA_SETS];//see above
} _std_table_62_t;

typedef struct
{//used for table 63
   unsigned char  Block_Order                   :1;//if set Blocks of load profile data are transported in descending order (N is newer than N+1).
   unsigned char  Overflow_Flag                 :1;//if set An attempt was made to enter an interval in a new data block such that the number of un-read blocks exceeded the actual number of possible blocks in load profile storage.
   unsigned char  List_Type                     :1;//0=FIFO, 1=Circular list as placed in load profile storage.
   unsigned char  Block_Inhibit_Overflow_Flag   :1;//The same value as LP_SETN_INHIBIT_OVF_FLAGs in “Table 61 - Actual Load Profile Limiting”
   unsigned char  Interval_Order                :1;//if set Intervals in each block of load profile are transported in descending order (N is newer than N+1).
   unsigned char  Active_Mode_Flag              :1;//if set This data set is collecting data.
   unsigned char  Test_Mode                     :1;//if set This data set is in the test mode.
   unsigned char  Filler                        :1;//zero
} _LP_Set_Status_Bfld;

typedef struct
{//used for table 63
   _LP_Set_Status_Bfld  LP_Set_Status_Flags;//see above
   unsigned short       Nbr_Valid_Blocks;//Number of valid load profile data blocks in load profile data tables load profile block arrays.
   unsigned short       Last_Block_Element;//The array element of the newest valid data block in the load profile data array.
   unsigned long        Last_Block_Seq_Nbr;//The sequence number of the last element, LAST_BLOCK_ELEMENT, in the load profile data array.
   unsigned short       Nbr_Unread_Blocks;//The number of load profile blocks that have not been read. This number is only updated through a procedure.
   unsigned short       Nbr_Valid_Int;//Number of valid intervals stored in the last load profile block array
} _LP_Set_Status_RCD;

typedef struct
{
   _LP_Set_Status_RCD   LP_Status_Set[NUM_LP_DATA_SETS];//see above
} _std_table_63_t;

typedef struct
{//used in table 64
#if (LP_CTRL_TBL_DOT_INT_FMT_CDE1==1)
   unsigned char     item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==2)
   unsigned short    item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==4)
   unsigned long     item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==8)
   signed char       item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==16)
   signed short      item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==32)
   signed long       item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==64)
   ni_fmat1_t        item;
#elif (LP_CTRL_TBL_DOT_INT_FMT_CDE1==128)
   ni_fmat2_t        item;
#else
#error "LP_CTRL_TBL_DOT_INT_FMT_CDE1 is not defined correctly"
#endif
} _int_fmt1_rcd_t;//INT_FMT1_RCD (INT_FMTN_RCD)

typedef struct
{//used in table 64
#if (ACT_LP_TBL_DOT_EXTENDED_INT_STATUS_FLAG==1)
   //This is one or more single octets of interval status conditions provided for all channels. Four bits of status information are provided for each channel in the set,
   //plus 4 bits for status conditions common to all channels. This common status nibble is always the first nibble of the set.
   //per channel: 0=no status, 1=overflow, 2=partial interval due to common state, 3=Long interval due to common state, 4=Skipped interval due to common state, 5=Interval contains test mode data
   //common: 0=Daylight savings time is in effect during or at start of interval, 1=Power fail within interval, 2=Clock reset forward during interval, 3=Clock reset backwards during interval
   unsigned char     extended_int_status[(ACT_LP_TBL_DOT_NBR_CHNS_SET1/2)+1];
#endif
   _int_fmt1_rcd_t   int_data[ACT_LP_TBL_DOT_NBR_CHNS_SET1];//This is an array of interval values for each channel in the block, recorded at a single time. Channels are presented in the same order. The order of the channels matches their definition in “Table 62 - Load Profile Control”
} _int_set1_rcd_t;//INT_SET1_RCD (INT_SETN_RCD)

typedef struct
{//used in table 64/65/66/67
#if (ACT_LP_TBL_DOT_BLK_END_READ_FLAG==1)
   ni_fmat1_t     block_end_read;//The value of the block end reading both data and readings.
#endif
#if (ACT_LP_TBL_DOT_BLK_END_PULSE_FLAG==1)
   unsigned long  block_end_pulse;//The values of the accumulator at the end of a user defined interval.
#endif
#if ((ACT_LP_TBL_DOT_BLK_END_READ_FLAG==0)&&(ACT_LP_TBL_DOT_BLK_END_READ_FLAG==0))
   unsigned char  compiler_error_stopper;//this is here to stop the IAR compiler from generating an error
#endif
} _readings_rcd_t;//READINGS_RCD

typedef struct
{//used in table 64
   _stime_date_t     blk_end_time;//date and time of the last interval of valid data entered in this data block.
#if ((ACT_LP_TBL_DOT_BLK_END_READ_FLAG==1)||(ACT_LP_TBL_DOT_BLK_END_READ_FLAG==1))
   _readings_rcd_t   end_readings[ACT_LP_TBL_DOT_NBR_CHNS_SET1];//array of readings of the end reading sources, taken at the interval end time of the last interval of data entered in the block. One end reading is provided for each channel in the block. The order of the readings matches their definition in “Table 62
#endif
#if (ACT_LP_TBL_DOT_SIMPLE_INT_STATUS_FLAG==1)
   unsigned char     simple_int_status[(ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1+7)/8];//bitfield This is a set of status bits, one per interval, that specifies whether the corresponding interval is valid.
#endif
   _int_set1_rcd_t   lp_int[ACT_LP_TBL_DOT_NBR_BLK_INTS_SET1];//This is an array of interval records in the block. Intervals are stored in an chronological order, specified by INTERVAL_ORDER, with the oldest reading set appearing in the first record.
} _lp_blk1_dat_rcd_t;//LP_BLK1_DAT_RCD (LP_BLKN_DAT_RCD)

typedef struct
{//actaul LP data
   _lp_blk1_dat_rcd_t   lp_data_sets1[ACT_LP_TBL_DOT_NBR_BLKS_SET1];//This is an array of load profile data blocks.
} _std_table_64_t;//LP_DATA_SET1_TBL (LP_DATA_SETN_TBL)

typedef struct
{//used in table 70
   unsigned char  event_number_flag       :1;//A common event number can be maintained in the History & Event Logs.
   unsigned char  hist_date_time_flag     :1;//Date & time can be maintained in the History Log.
   unsigned char  hist_seq_nbr_flag       :1;//A sequence number can be transported through the History Log.
   unsigned char  hist_inhibit_ovf_flag   :1;//History Log is capable of inhibiting overflow.
   unsigned char  event_inhibit_ovf_flag  :1;//Event Log is capable of being blocked.
   unsigned char  filler                  :3;//
} _log_flags_bfld_t;//LOG_FLAGS_BFLD

typedef struct
{
   _log_flags_bfld_t log_flags;//
   unsigned char     nbr_std_events;//number of octets in the set STD_EVENTS_SUPPORTED
   unsigned char     nbr_mfg_events;//number of octets in the set MFG_EVENTS_SUPPORTED
   unsigned char     hist_data_length;//number of octets in the HISTORY_ARGUMENT
   unsigned char     event_data_length;//number of octets in the EVENT_ARGUMENT
   unsigned short    nbr_history_entries;//number of entries in Table 74 - History Log Data.
   unsigned short    nbr_event_entries;//number of entries in the event log in Table 76 - Event Log Data.
} _std_table_70_t;//DIM_LOG_TBL + ACT_LOG_TBL (tbl 71)

typedef struct
{
   unsigned char  std_events_supported[ACT_LOG_TBL_DOT_NBR_STD_EVENTS];//bitfield indicates which of the standard events are supported in the Event Log (Appendix B, "ANSI C12.19-1997 History Log & Event Log Codes"). Event codes are represented by bits 0 through (8 X NBR_STD_EVENTS - 1), with a one (1) representing a TRUE or implemented condition
   unsigned char  mfg_events_supported[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS];//bitfield like above but for mfg
} _std_table_72_t;//EVENTS_ID_TBL

typedef struct
{
   unsigned char  std_events_monitored_flags[ACT_LOG_TBL_DOT_NBR_STD_EVENTS];//bitfield Bit position is linearly associated with corresponding standard event code. enables event if set
   unsigned char  mfg_events_monitored_flags[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS];//bitfield like above but for mfg
   unsigned char  std_tbls_monitored_flags[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED];//bitfield Bit position is linearly associated with corresponding standard table number
   unsigned char  mfg_tbls_monitored_flags[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED];//bitfield like above
   unsigned char  std_proc_monitored_flags[GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED];//bitfield Bit position is linearly associated with corresponding standard procedure number.
   unsigned char  mfg_proc_monitored_flags[GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED];//bitfield like above
} _std_table_73_t;//HISTORY_LOG_CTRL_TBL + EVENT_LOG_CTRL_TBL (tbl 75)

/*
typedef struct
{//used in table 74
#if (ACT_LOG_TBL_DOT_HIST_DATE_TIME_FLAG==1)
   _ltime_date_t     history_time;//Date and time of History Log entry.
#endif
#if (ACT_LOG_TBL_DOT_EVENT_NUMBER_FLAG==1)
   unsigned short    event_number;//Event number common to both the History and Event Logs.
#endif
#if (ACT_LOG_TBL_DOT_HIST_SEQ_NBR_FLAG==1)
   unsigned short    history_seq_nbr;//Sequence number associated with the History Log only.
#endif
   unsigned short    user_id;//The User ID associated with this History Log entry. It comes from the Log In Procedure or from a communication session initiation sequence. A USER_ID of zero means the end device initiated the event. A USER_ID of one means the event was manually initiated.
   _table_idb_bfld_t history_code;//Event code logged. See TABLE_IDB_BFLD. above
   unsigned char     history_argument[ACT_LOG_TBL_DOT_HIST_DATA_LENGTH];//Argument associated with a specific entry. For standard event arguments, refer to Appendix B, "ANSI C12.19-1997 History Log & Event Log Codes".
} _history_entry_rcd_t;//HISTORY_ENTRY_RCD

typedef struct
{
   _list_status_bfld_t  hist_flags;//See LIST_STATUS_BFLD.
   unsigned short       nbr_valid_entries;//Number of valid entries in the log
   unsigned short       last_entry_element;//The array element number of the newest valid entry in the log.
   unsigned long        last_entry_seq_nbr;//The sequence number of the newest valid entry in the log.
   unsigned short       nbr_unread_entries;//The number of entries in the log that have not yet been read. It is only changed through a procedure.
   _history_entry_rcd_t entries[ACT_LOG_TBL_DOT_NBR_HISTORY_ENTRIES];//Array of History Log entries.
} _std_table_74_t;//HISTORY_LOG_DATA_TBL
*/

typedef struct
{//used in table 76
   _ltime_date_t     event_time;//Date and time of Event Log entry.
#if (ACT_LOG_TBL_DOT_EVENT_NUMBER_FLAG==1)
   unsigned short    event_number;//Event number common to both the History and Event Logs.
#endif
   unsigned short    event_seq_nbr;//Sequence number associated with the Event Log only.
   unsigned short    user_id;//The User ID associated with this Event Log entry. It comes from the Log In Procedure or from a communication session initiation sequence. A USER_ID of zero means the end device initiated the event. A USER_ID of one means the event was manually initiated
   _table_idb_bfld_t event_code;//Event code logged. See TABLE_IDB_BFLD
   unsigned char     event_argument[ACT_LOG_TBL_DOT_EVENT_DATA_LENGTH];//Argument associated with a specific entry.
} _event_entry_rcd_t;//EVENT_ENTRY_RCD

typedef struct
{
   _list_status_bfld_t  event_flags;//See LIST_STATUS_BFLD above.
   unsigned short       nbr_valid_entries;//Number of valid entries in the log.
   unsigned short       last_entry_element;//The array element number of the newest valid entry in the log.
   unsigned long        last_entry_seq_nbr;//The sequence number of the newest valid entry in the log.
   unsigned short       nbr_unread_entries;//The number of entries in the log that have not yet been read. It is only changed through a procedure.
} _std_table_76_hdr_t;//EVENT_LOG_DATA_TBL

typedef struct
{
   _list_status_bfld_t  event_flags;//See LIST_STATUS_BFLD above.
   unsigned short       nbr_valid_entries;//Number of valid entries in the log.
   unsigned short       last_entry_element;//The array element number of the newest valid entry in the log.
   unsigned long        last_entry_seq_nbr;//The sequence number of the newest valid entry in the log.
   unsigned short       nbr_unread_entries;//The number of entries in the log that have not yet been read. It is only changed through a procedure.
   _event_entry_rcd_t   entries[ACT_LOG_TBL_DOT_NBR_EVENT_ENTRIES];//Array of Event Log entries
} _std_table_76_t;//EVENT_LOG_DATA_TBL




//       ++++++++++Non Standard Manufacturing ANSI Tables +++++++++++++
/*
   Mfg Tables that appear to be in use by the Paulaner/LGMETER
   NOTE: This list may be incorrect or incomplete. It's a mess in there.
   1,2,3,4,5,7,8,12,13,14,15,16,17,18,19,21,22,23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,46
 */

#define DIM_MFG_TBLS_USED   47  // remembering to count tbl 0

// global structures
typedef struct
{
   unsigned char        Fraction : 6;
   unsigned short int   Integer : 10;
} _Q_X_DOT_Y_BFLD;

// mfg table 0 related structures
typedef struct
{//used in mfg hash table
   unsigned char  Hash[TABLE_HASH_LENGTH];
} _hash_entry_t;

// mfg table 0 structure
typedef struct
{//Table of Hash values
   _hash_entry_t  HashArr[eHashNumber];//[NUM_TABLE_HASH_VALUES];
} _mfg_table_00_t;

// mfg table 1 structure
typedef struct
{
   unsigned char  MfgTblVersion[DIM_MFG_TBLS_USED];             // version number of each manufacturing table
} _mfg_table_01_t;

// mfg table 2 related structures
typedef struct
{
   unsigned char Reserved : 7;
   unsigned char Engr_ver_set : 1;
} _FW_ENG_REVISION_RCD;

typedef struct
{
   unsigned char                FW_Ver_Num;
   unsigned char                FW_Rev_Num;
   _FW_ENG_REVISION_RCD         FW_Rev_Info;
   unsigned char                Build_Num;
   unsigned char                Eng_Ver_Num;
   unsigned char                Tbl_Struct_Ver_Num;
} _FW_VERSION_RCD;

// mfg table 2 structure
typedef struct
{
   _FW_VERSION_RCD      FW_Version;
   unsigned char        CE_HW_Version;
   unsigned char        Boot_Loader_Ver;
} _mfg_table_02_t;

// mfg table 3 related structures
typedef struct
{
   unsigned char        IA_Coil_Polarity : 1;
   unsigned char        IB_Coil_Polarity : 1;
   unsigned char        IC_Coil_Polarity : 1;
   unsigned char        _2S_2CT : 1;
   unsigned char        _2S_4S_Current_Option : 1;
   unsigned char        Service_Disconnect_Not_Capable : 1;
   unsigned char        SD_Sensor_IsPresent : 1;
   unsigned char        Filler : 1;
} _CAL_CONFIG_BFLD;

// mfg table 3 structure
typedef struct
{
   unsigned long int    Energy_Multiplier[3];
   unsigned long int    Voltage_Multiplier[3];          // doc says floats but that's crazy talk!
   unsigned long int    Current_Multiplier[3];          //      "       "       "       "
   unsigned long int    Neutral_Current_Multiplier;     //      "       "       "       "
   unsigned long int    Creep_Threshold_0;
   unsigned long int    Creep_Threshold_1;
   unsigned short int   PPMC1_MPU;
   unsigned short int   PPMC2_MPU;
   unsigned long int    Temp_Nom;
   _Q_X_DOT_Y_BFLD      Nominal_Volts;
   unsigned short int   Power_Multiplier;
   _CAL_CONFIG_BFLD     Cal_Config;
   unsigned char        Cal_LED_Ticks;
   unsigned char        Kh_Display;
   unsigned char        SF_Hold_Time;
   unsigned long int    Phadj_0;
   unsigned long int    Phadj_1;
   unsigned long int    Phadj_2;
   unsigned long int    Ext_Temp;
   unsigned long int    PPMC1_CE;
   unsigned long int    PPMC2_CE;
   _Q_X_DOT_Y_BFLD      Nominal_Temp;
   _Q_X_DOT_Y_BFLD      SD_Error_Voltage_Threshold;
} _mfg_table_03_t;

// mfg table 4 structure
typedef struct
{
   unsigned char        Identity;
   unsigned char        AuxBaudRate;
   unsigned char        AuxTwoStopbits;
   unsigned char        AuxNbrPacket;
   unsigned char        AuxPacketSize;
   unsigned char        AuxTrafficTo;
   unsigned char        AuxInterCharTo;
   unsigned char        AuxRespTo;
   unsigned char        AuxNbrRetries;
   unsigned char        AuxTaDelay;
   unsigned char        OptBaudRate;
   unsigned char        OptTwoStopbits;
   unsigned char        OptNbrPacket;
   unsigned char        OptPacketSize;
   unsigned char        OptTrafficTo;
   unsigned char        OptInterCharTo;
   unsigned char        OptRespTo;
   unsigned char        OptNbrRetries;
   unsigned char        OptTaDelay;
} _mfg_table_04_t;

// mfg table 5 - primary rating configuration
typedef struct
{
   unsigned char        RatioF1BitMap[7];//Bitmap indicates if ratioF1 should be applied to the quantities in STD T16
   unsigned char        RatioP1BitMap[7];//Bitmap indicates if ratioP1 should be applied to the quantities in STD T16
   unsigned long        RatioF1;         //The current transformer ratio
   unsigned long        RatioP1;         //The voltage transformer ratio	
   unsigned char        KFactor;         //The number to indicate right shift digits on LCD display 	
   unsigned char        spare[4];	
} _mfg_table_05_t;

// mfg table 7 related structures
typedef struct
{
   unsigned char        As_Left_Three_Element_Full_Load[3];
   unsigned char        As_Left_Left_Element_Full_Load[3];
   unsigned char        As_Left_Centre_Element_Full_Load[3];
   unsigned char        As_Left_Right_Element_Full_Load[3];
   unsigned char        As_Left_Three_Element_Power_Factor[3];
   unsigned char        As_Left_Left_Element_Power_Factor[3];
   unsigned char        As_Left_Centre_Element_Power_Factor[3];
   unsigned char        As_Left_Right_Element_Power_Factor[3];
   unsigned char        As_Left_Three_Element_Light_Load[3];
} _AS_LEFT_CALIBRATION_RCD;

// mfg table 7 structures
typedef struct
{
   _AS_LEFT_CALIBRATION_RCD     AsLeftCalibration;
   _Q_X_DOT_Y_BFLD              CalibrationVoltage;
   unsigned char                CatalogNumber[23];
} _mfg_table_07_t;

// mfg table 8
// NOTE: This table is contains a duplicate of manufacturer specific information from mfg tbl 3
//       This table is only written by the manufacturer at calibration time
//       It is used as a reference for customer adjustment of the calibration
//       Therefore there's no struct defined for it here, we just reuse _mfg_table_03_t

// mfg table 12 structure
typedef struct
{
   unsigned char FirmwareRev[16];//FW rev string is currently made up of 16 chars.
} _mfg_table_12_t;

//mfg table 15 - Prepayment Display Parameters
typedef struct
{
  unsigned char   PrePayDispList[20];
  unsigned short  PrePayDebtReg;
  unsigned long   PrePayCerditReg;
  unsigned char   spare[8];
}_mfg_table_15_t;

//Used in mfg table 16
//mfg table 16 - Prepayment Configuration 
typedef struct
{
  unsigned char   PrePayMode; //0- Prepay Disabled 1- Prepay Enabled
  unsigned short  PrePayHoldOffTime;
  unsigned short  PrePayCommTimeout;
  unsigned char   PrePayDiscOverride;
  unsigned char   spare1[2];
  unsigned char   PrePayCrdtCents;
  unsigned char   PrePayActions;
  unsigned char   spare2[4];
}_mfg_table_16_t;

// mfg table 17&18 structures (Factory Encrypted Tables)
typedef struct
{
   unsigned char  encryption_type;
   unsigned char  protected_block[256];
} _mfg_table_17_t;

typedef struct
{
   signed short CalTemperature[3];     //temperature reading from MMI at time of cal
   signed short CalMagFieldOffset[3];  //dc mag field reading from MMI at time of cal, typically 2046
   unsigned short Ratio;               //sensitivity ratio of field v temperature (dc field units/degree) 1.000 = 1,000
   unsigned char Trigger;              //deviation from no-tamper condition that will trigger a tamper, typically 3
   unsigned char SanityDelta;          //deviation from no-tamper condition will trigger a fast transient special case, typically 50
} _mfg_table_19_t;

// mfg table 22 structures (PredicTemp Calibration)
typedef struct
{
   signed short   Offset;
   signed short   Slope;
} _temp_cal_vals_t;

typedef struct
{
   _temp_cal_vals_t CalVals[3];
} _mfg_table_22_t;

typedef struct
{
  unsigned char   Type;
  signed short    Threshold;
  unsigned char   Action;
} _temp_monitor_cfg_t;

typedef struct
{
  _temp_monitor_cfg_t MonitorsCfg[6];
  unsigned char       MMIReadingMonitor; //bit0: 0-disable;1-Log event;  bit1: 0-disabled;1-open break;
} _mfg_table_23_t;

typedef struct
{
  unsigned char   Status;
  unsigned short  BreachCount;
} _temp_monitor_stats_t;

typedef struct
{
  signed short    CurrTemp;
  signed short    MaxTempArr[4];
  signed short    MinTempArr[4];
  signed short    AvgTempArr[4];
  signed short    CurrSlope;
  signed short    MaxSlopeArr[4];
  signed short    MinSlopeArr[4];
  signed long     AvgAccum;
  unsigned short  MaxCurrent;
} _temp_mmi_stats_t;

typedef struct
{
  _temp_monitor_stats_t MonitorStats[6];
  _temp_mmi_stats_t     MMIStats[3];
  unsigned short        AvgAccumCount;
} _mfg_table_24_t;


typedef struct
{//Customer config
   unsigned char  TamperConfig;        //enable flags for: LSVD, intrusion, Mag tamper enable, mag tamper pmax
   unsigned char  OptoPriorityConfig;  //optical port priority over remote port enable/disable
   unsigned short MagTamperStart;      //Persistance time at the start of a magnetic tamper before condition is flagged
   unsigned short MagTamperEnd;        //Amount of time at end of tamper before condition is cleared
   unsigned char ClockSource;          //Configure which clock source to use(Mains Based or Xtal based or...)
   unsigned char MetrologyGrossNetConfig;  //0= Net Metering , 1= Gross Metering;
   unsigned char MetrologyTotalEnergyRegCfg; //a bit map to indicate the phase presents for the total energy reg in std t28
   unsigned char spare[55];
} _mfg_table_25_t;

typedef struct
{//used in MT26 for LC power loss/restore configuration.
   unsigned short count;   //count of number of tamper events
   unsigned long  first;   //posix time (GMT)
   unsigned long  last[5]; //posix time (GMT)
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _r_app_lc_emer_log_t;

typedef struct
{//used in MT26 for LC power loss/restore configuration.
   unsigned char  action;        //1=open, 0=close.
   unsigned short HoldTimeMin;   //Min random duration that relay will be held in specified state. Units of seconds.
   unsigned short HoldTimeMax;   //Max random duration that relay will be held in specified state. Units of seconds.
   unsigned char  padding[4];
} _r_app_lc_cfg_pwr_fail_t;

typedef struct
{//used in MT26 for under frequency configuration.
   unsigned char  action;           //1=open, 0=close.
   unsigned char  NegativeTrigger;  //Trigger frequency for when frequency is falling. Units of 0.1Hz. Range of 40Hz - 65.5Hz.
   unsigned char  PositiveTrigger;  //Trigger frequency for when frequency is rising. Units of 0.1Hz. Range of 40Hz - 65.5Hz.
   unsigned short UnderBreachDur;   //Duration of frequency being less than NegativeTrigger before under freq action is taken. Units of 0.1 seconds.
   unsigned short NominalBreachDur; //Duration of frequency being more than PositiveTrigger before nominal freq action is taken. Units of 0.1 seconds.
   unsigned short HoldTimeMin;      //Min random duration that relay will be held in specified state after returning to nominal frequency.
   unsigned short HoldTimeMax;      //Max random duration that relay will be held in specified state after returning to nominal frequency.
   unsigned char  padding[4];
} _r_app_lc_cfg_undr_freq_t;

typedef struct
{//used in MT26 for under voltage configuration.
   unsigned char  action;           //1=open, 0=close.
   unsigned short NegativeTrigger;  //Trigger voltage for when frequency is falling. Units of 0.1V.
   unsigned short PositiveTrigger;  //Trigger voltage for when frequency is rising. Units of 0.1V.
   unsigned short UnderBreachDur;   //Duration of frequency being less than NegativeTrigger before under freq action is taken. Units of 0.1 seconds.
   unsigned short NominalBreachDur; //Duration of frequency being more than PositiveTrigger before nominal freq action is taken. Units of 0.1 seconds.
   unsigned short HoldTimeMin;      //Min random duration that relay will be held in specified state after returning to nominal frequency.
   unsigned short HoldTimeMax;      //Max random duration that relay will be held in specified state after returning to nominal frequency.
   unsigned char  padding[4];
} _r_app_lc_cfg_undr_volt_t;

typedef struct
{//used in MT26 for excessive demand (power) configuration.
   unsigned short start_time;    //time of day when monitoring is to commence, units of minutes. 0=midnight GMT time
   unsigned short duration;      //duration of monitoring period where demand functionality is active, units of minutes
   unsigned short limit_wh;      //energy limit in units of 0.01kWh
   unsigned char  measure_period;//measurment period over which total energy is calculated for comparison with limit.
} _r_app_lc_cfg_dmd_seas_t;

typedef struct
{//used in MT26 for excessive demand (power) configuration.
   _r_app_lc_cfg_dmd_seas_t seasons[RAPPLC_NUM_SCHED_SEAS];          //different sets of limits for each season
   _r_app_lc_cfg_dmd_seas_t seas_spare[RAPPLC_NUM_SCHED_SEAS_SPARE]; //different sets of limits for each season
   unsigned char  action;                                            //position of relay during a breach. 0=open, 1=close.
   unsigned char  padding[4];
} _r_app_lc_cfg_dmd_lim_t;

typedef struct
{//used in MT26 for boost configuration.
   unsigned char  action;           //1=open, 0=close, 4=cyclic
   unsigned char  period;           //boost period in units of 0.5 hours
   unsigned char  boost_flags;      //flags for ESCC, terminate boost on second push, redirect to element 1
   unsigned char  padding[5];
} _r_app_lc_cfg_boost_t;

typedef struct
{//used in MT26 for priority override configuration of the legacy command.
   unsigned char  dither_mask;   //dither mask for both open and close commands B0=NA B1=prim close B2=2nd close B3=tertiart close B4=NA B5=prim open B6=2nd open B7=tertiart open
   unsigned short min_start; //minimum random dither time to use for the closed command. units in minutes
   unsigned short max_start; //maximum random dither time to use for the closed command. units in minutes
   unsigned short period;        //duration of override. units of minutes
   unsigned char  padding[2];
} _r_app_lc_cfg_pri_overr_t;

typedef struct
{//used in MT26 for schedule configuration.
   unsigned short action   : 4;  //action to perform, open/close/open with random/cyclic etc.
   unsigned short time     :12;  //time of day when action should take place units of minutes. 0=midnight GMT time
} _r_app_lc_cfg_sw_rec_t;

typedef struct
{//used in MT26 for normal schedule configuration.
   unsigned char           day_type_bit_field;                       //bitfield indication if day is type a or type b bit0=monday, bit1=tuesday etc. a set bit indicates day type b
   _r_app_lc_cfg_sw_rec_t  type_a_sw_list[RAPPLC_NUM_SCHED_SW_REC];  //10 switch entires for day type a
   _r_app_lc_cfg_sw_rec_t  type_b_sw_list[RAPPLC_NUM_SCHED_SW_REC];  //10 switch entires for day type b
   unsigned char  padding[4];
} _r_app_lc_cfg_norm_rec_t;

typedef struct
{//used in MT26 for special schedule configuration.
   unsigned char  date_type; //0- non recurrsive, 1- recurrsive
   union
   {
      _rdate_t   recurr_date;
      _date_t    nonrecurr_date;
   };
}_r_app_lc_cfg_spl_dates_t;

typedef struct
{//used in MT26 for special schedule configuration.
   _r_app_lc_cfg_spl_dates_t  special_dates[RAPPLC_NUM_SPEC_DATES];              //array of dates when a special day becomes effective. LOCAL TIME.
   _r_app_lc_cfg_spl_dates_t  special_dates_spare[RAPPLC_NUM_SPEC_DATES_SPARE];  //spare array of dates when a special day becomes effective. SPARE - 1byte*12 = 12bytes
   _r_app_lc_cfg_sw_rec_t     sw_list[RAPPLC_NUM_SCHED_SW_REC];                  //list of switch entires containing action and time.
   unsigned char  padding[4];
} _r_app_lc_cfg_spec_rec_t;

typedef struct
{//used in MT26 for schedule configuration.
  _rdate_t                    season_sw_date;//date when the season becomes effective
  _r_app_lc_cfg_norm_rec_t    norm;          //schedule for normal days
  _r_app_lc_cfg_spec_rec_t    special;       //schedule for special days
} _r_app_lc_cfg_seas_rec_t;

typedef struct
{//used in MT26 for normal & special schedule configuration.
   unsigned short          min_start; //minimum period over which randomisation can occur units of seconds
   unsigned short          max_start; //maximum period over which randomisation can occur units of seconds
   unsigned char           default_action;
  _r_app_lc_cfg_seas_rec_t seasons[RAPPLC_NUM_SCHED_SEAS];              //switch times for each season
  _r_app_lc_cfg_seas_rec_t seasons_spare[RAPPLC_NUM_SCHED_SEAS_SPARE];  //spare season
} _r_app_lc_cfg_sched_t;

/*
typedef struct
{//used in MT26 for ripple configuration.
   unsigned short tele[7];//telegram and command
} _r_app_lc_cfg_rip_tele_t;*/
/*
typedef struct
{//used in MT26 for ripple configuration.
   //TODO: what to put here? is this required? do we put application level stuff only
   unsigned char              supply_loss_memory;  //0=disbaled (unchanged) 1=enabled (memory)
   unsigned short             failsafe_timeouts[3];//??????
   _r_app_lc_cfg_rip_tele_t   telegrams[10];       //
} _r_app_lc_cfg_ripple_t;*/ //Remove 147bytes

typedef struct
{//used in MT26 for installation boost configuration.
   unsigned char  action;           //1=open, 0=close, 4=cyclic
   unsigned short period;           //boost period in units of minutes, after this period has elapsed install bost will never run again
   unsigned char  boost_flags;      //flags for ESCC, terminate boost on second push, redirect to element 1
   unsigned char  padding[3];
} _r_app_lc_cfg_inst_boost_t;

typedef struct
{
   unsigned short closed_period;       //period that relay stays closed. Specified in seconds
   unsigned short open_period;         //period that relay stays open. Specified in seconds
   unsigned short min_start; //minimum randomisation period for start of cyclic. Specified in seconds
   unsigned short max_start; //maximum randomisation period for start of cyclic. Specified in seconds
   unsigned char  cyclic_type;         //normal or air-conditioner safe
   unsigned char  Overwrite_Perm;      //0-dont allow the working cyclic parameters to be replaced by cyclic parameters defined by cyclic in ripple. 1- allow overwrite
   unsigned char  padding[2];
}_r_app_lc_cyclic_cfg_t;

typedef struct
{
   unsigned char permitted;   //determine if module is enabled
   unsigned char priority;    //priority of module 1 is lowest 200 is highest, zero is invalid
   unsigned char log_enable;  //event logging enabled for particular module
   unsigned char padding[3];
}_r_app_lc_module_cfg_t;

typedef struct
{
   _r_app_lc_module_cfg_t     mod_cfgs[RAPPLC_NUM_MODULE];        //generic configuration
   _r_app_lc_module_cfg_t     mod_cfgs_spare[RAPPLC_NUM_MODSPARE];//spare configuration info
   _r_app_lc_cyclic_cfg_t     cyc_cfg;                            //cyclic mode common to all modules
   _r_app_lc_cfg_pwr_fail_t   pwr_fail;                           //loss of supply and restoration
   _r_app_lc_cfg_undr_freq_t  undr_freq;                          //under frequency
   _r_app_lc_cfg_dmd_lim_t    dmd_lim;                            //demand limit
   _r_app_lc_cfg_undr_volt_t  undr_volt;                          //under voltage
   _r_app_lc_cfg_boost_t      boost;                              //boost
   _r_app_lc_cfg_pri_overr_t  pri_overr;                          //settings for legacy support of override command
   _r_app_lc_cfg_sched_t      sched;                              //switch schedule
   _r_app_lc_cfg_inst_boost_t install_boost;                      //installation boost
}_r_app_lc_dev_cfg_t;//

typedef struct
{//configuration of LC related functions
   _r_app_lc_dev_cfg_t  lc_rly_cfg[RAPPLC_NUM_RELAY]; //configuration data for each LC relay
   unsigned char        LCMode;
   unsigned char        spare[5];
}_mfg_table_26_t;

typedef struct
{
   unsigned char           enabled;
   unsigned char           priority;
   unsigned char           req_rly;
   unsigned char           rly_action;
   unsigned char           log_event;
   unsigned char           padding[2];
}_r_app_lc_module_stat_t;

typedef struct
{
   unsigned char           lc_rly_enabled;
   unsigned char           curr_action;
   unsigned char           curr_controller_id;
   unsigned char           curr_controller_priority;
   _r_app_lc_module_stat_t mod_stats[RAPPLC_NUM_MODULE];
   _r_app_lc_module_stat_t mod_spares[RAPPLC_NUM_MODSPARE];
   unsigned long           num_relay_ops;
   _r_app_lc_emer_log_t    mod_log_undr_freq;
   _r_app_lc_emer_log_t    mod_log_undr_volt;
   _r_app_lc_emer_log_t    mod_log_dmd;
}_r_app_lc_dev_stat_t;

typedef struct
{//status of LC related functions
   _r_app_lc_dev_stat_t    lc_rly_stats[RAPPLC_NUM_RELAY];
}_mfg_table_27_t;

typedef struct
{//used in MT28, MT33 & MT 34 units in mA.
   signed long real;
   signed long img;
} _cmplx_t;

#if defined(NEUTRAL_INTEGRITY_POLY)
   typedef struct
   {
      unsigned char PhaseIDMsk;        //mask used in 1&2 phase systems to select what phases are connected to what, 0=NC, 1=A, 2=B 3=NA. b0-1=phase A, b2-3=phase B, b4-5=phase C, b6-7=unused
      unsigned char MaxCurrent;        //Threshold for when current in a phase or neutral is said to be excessive. Units of 1.0A
      unsigned short SupplyImp[3];     //current estimate of supply impedance (may need to be a long?)
      _cmplx_t var_z_sum;              //Accumulating sum used for varience calculation
      unsigned long var_z_count;       //Count used used for varience calculation
      _cmplx_t no_load_snapshot[3];    //voltage vectors captured at a time of no load
      unsigned short no_load_unbal;    //voltage unbalance at the time of no load
      _cmplx_t lo_load_snapshot[2][3]; //voltage & current vectors captured at a time of low load
      unsigned short lo_load_unbal;    //voltage unbalance at the time of low load
      _cmplx_t hi_load_snapshot[2][3]; //voltage & current vectors captured at a time of high load
      unsigned short hi_load_unbal;    //voltage unbalance at the time of high load
      unsigned long z_mag_min;         //minimum magnitude estimate for z for all time.
      unsigned long z_mag_max;         //maximum magnitude estimate for z for all time.
      unsigned short LoopImp[3];       //loop impedence for phase a,b and c.
   } _ni_bkp1_t;

   typedef struct
   {
      unsigned long i_time;            //time of installation
      unsigned char i_flags;           //flags set at time of install
      _cmplx_t i_vi[2][3];             //readings at time of install [0=voltage, 1=current][phase: 0=a, 1=b, 2=c]
      unsigned short i_unbal;          //voltage unbalance at the time of install
   } _ni_install_t;

   typedef struct
   {// Neutral Integrity NV backup
      _ni_install_t install_readings;  //readings at time of install
      _ni_bkp1_t NVbkp1;               //NV backup of some MT33 RAM values
   } _mfg_table_28_t;
#else //NEUTRAL_INTEGRITY_POLY
   typedef struct
   {// Neutral Integrity NV backup
      _cmplx_t ITestActiveRaw[2];      // Install mode test results, active current, [0]=no load, [1]=load (Iail) mA
      _cmplx_t ITestEarthRaw[2];       // Install mode test results, earth current, [0]=no load, [1]=load (Ieix & Ieil) mA
      unsigned short TestVoltage[2];   // Install mode test results, voltage, [0]=no load, [1]=load mA
      unsigned long TestTime;          // Install mode test time
      _cmplx_t IEarthSystemic;         // Systemic earht current (Ieo) mA
      signed long IEarthMaxConst;      // Constant- maximum nominal earth current (Ixok) mA
      signed long IEarthRatioMin;      // Constant- minimum nominal ratio of current returing via earth (Semn) 10000 is 1:1
      signed long IEarthRatioMax;      // Constant- maximum nominal ratio of current returing via earth (Semx) 10000 is 1:1
      unsigned short LoopImpedance;    // Supply loop impedance calculated at the time of installation. Units of 0.001 Ohms.
      unsigned short InstallSoilResist;// Ratio showing soil resistivity at installation time (Rssp)
      unsigned char ForceNIDisable;    // When set via a procedure NI is disbaled.
      signed long IEarthRevConst;      // Constant- threshold for earth current on a reverse active/neutral site (Irth) mA
      signed long Seil;                // Seil calculated at the time of installation.
      signed long NVIEarthSuspectOK;   // active power thresold for peforming supplimentry Hi-Z earth detection. (Sxax * Ixok) + Ixmn  mA.
      unsigned short NVVoltThresh; // Broken Neutral voltage threshold units of 0.1V
   } _mfg_table_28_t;
#endif //NEUTRAL_INTEGRITY_POLY

typedef struct
{// communication module RAM data
   unsigned char        Data[256];
} _mfg_table_29_t;

// mfg table 30
typedef struct
{
   // want stuff like
   // firmware revision
   // m16c, ember, etc ids
   unsigned long int FWDLActivateTime;         // posix GMT time to switch to new firmware. If meter is powered off it will switch on startup
   // total number of blocks
   // crc?
   // activation date
   // timeout stuff?
} _mfg_table_30_t;

// mfg table 31
typedef struct
{
   // want read only, status stuff
   unsigned short int FWBlkSize;                  // block size
   unsigned long int  FWBlkOffset;              // current offset, auto incremented on successful write
} _mfg_table_31_t;

// mfg table 32
typedef struct
{
   unsigned char FirmwareBlock[4096];
} _mfg_table_32_t;


#if defined(NEUTRAL_INTEGRITY_POLY)
   typedef struct
   {
      unsigned long time_posix;        //time of snapshot in UTC, zero indicates invalid
      unsigned long delta_i_mag;       //magnitude of differential current vector for the phase that triggered the capture
      _cmplx_t v_and_i[2][2][3];       // v_and_i[0=before, 1=after][0=voltage, 1=current][phase: 0=a, 1=b, 2=c]
   } _delta_snapshot_t;

   typedef struct
   {
      _delta_snapshot_t snaps[2][3];   //snaps[diff rank: 0=highest, 1=2nd highest][delta trigger phase: 0=a, 1=b, 2=c]
   } _delta_snapshots_t;

   typedef struct
   {
      unsigned long time_posix;        //time of lasst update posix UTC
      _cmplx_t z_estimates[4];         //estimates of impedance for src phase a,b,c, neutral.
      unsigned short solution_count[4];//number of times a solution was found
      unsigned short unknown_count[4]; //number of time a solution could not be found
      unsigned long z_mag_min[4];      //minimum magnitude estimate for z.
      unsigned long z_mag_max[4];      //maximum magnitude estimate for z.
      unsigned short z_ang_min[4];     //minimum angle estimate for z.
      unsigned short z_ang_max[4];     //maximum angle estimate for z.
      _cmplx_t z_sum_lo[4];            //low part of sum used for calculating average
      _cmplx_t z_sum_hi[4];            //high part of sum used for calculating average
      _cmplx_t z_est_avg[4];           //average z estimates
      _cmplx_t var_z_sum_lo[4];        //low part of varience sum used for calculating varience
      _cmplx_t var_z_sum_hi[4];        //high part of varience sum used for calculating varience
      unsigned short var_z[4];         //varience in z estimates, units of ?
      _cmplx_t z_loop_est[3];          //estimates of loop impedance between src phase a,b,c and n.
   } _deriv_ni_calcs_t;

   typedef struct
   {
      unsigned short z_ang_lim_min[4]; //minimum angle limit for z.
      unsigned short z_ang_lim_max[4]; //maximum angle limit for z.
      unsigned short max_ang_step;     //maximum angle step size for round 1 of curve genration.
      unsigned short max_mag_step;     //maximum magnitude step size for round 2 of curve generation.
      unsigned short delta_trig_lim[3];//minimum threshold to trigger a delta snapshot. 0=phase a monitor, 1=ph2, 2=ph3
      unsigned short deriv_period;     //number of hours a derivative metering monitoring/calc period lasts
      unsigned short noise_floor_i;    //noise floor for current
      unsigned short noise_floor_v;    //noise floor for voltage
      unsigned char src_ok_min_calcs;  //acceptance criteria for accepting derivative metering value. min number of derivate calcs
      unsigned char src_ok_pass_rate;  //acceptance criteria for accepting derivative metering value. min pass rate for derivative calcs
      unsigned char src_ok_max_var;    //acceptance criteria for accepting derivative metering value. max varience
      //?? possible formula for calculating the error offset voltage
      //?? criteria for external earth current trigger (high derivate z varience)
   } _deriv_ni_calc_config_t;

   typedef struct
   {
      unsigned char valid_rding_msk;   //set bit indicates reading snapshot is valid b0=a1,b1=a2,b2=b1,b3=b2,b4=c1,b5=c2,
      //??? position within processing loops
   } _deriv_ni_calc_status_t;

   typedef struct
   {
      _delta_snapshots_t live;         //live values that change instantaneously
      _delta_snapshots_t processed;    //snapshot of live values that are currently being processed.
      _deriv_ni_calcs_t calcs[3];      //derivative metering impedance calculations for 3 periods: 0=now, 1=last period, 2=period before that
      unsigned long z_mag_min[4];      //minimum magnitude estimate for z for all time.
      unsigned long z_mag_max[4];      //maximum magnitude estimate for z for all time.
      unsigned short z_ang_min[4];     //minimum angle estimate for z for all time.
      unsigned short z_ang_max[4];     //maximum angle estimate for z for all time.
      _deriv_ni_calc_config_t nid_cfg; //configuration of ni derivative metering
      _deriv_ni_calc_status_t pstat;   //processing status of curve intersection algorithm.
   } _deriv_ni_t;

   typedef struct
   {
      _cmplx_t VoltageVect[4];         //phase a,b,c and (total/3)
      _cmplx_t CurrentVect[4];         //phase a,b,c and total
      _cmplx_t LoadZ[3];               //load impedance phase a,b,c units of 0.01 ohms
      _cmplx_t NeutralImpedance;
      unsigned short UnbalanceFactor;  //unit in 0.1%
   } _inst_ni_t;

   typedef struct
   {
      union
      {
         unsigned char raw;
         struct
         {
            unsigned char basic  :4;
            unsigned char deriv  :4;
         };
      };
   } _ni_state_nib_t;

   typedef struct
   {
      unsigned short PersistTimeLim;   //Number of seconds NI condition must exist before action is taken.
      //unsigned short SupplyImp[3];     //current estimate of supply impedance (may need to be a long?)
      //unsigned char PhaseIDMsk;        //mask used in 1&2 phase systems to select what phases are connected to what, 0=NC, 1=A, 2=B 3=NA. b0-1=phase A, b2-3=phase B, b4-5=phase C, b6-7=unused
      unsigned char RevANErrorLim;     //Criteria for rev A/N trigger (415 v and bad phase angle). Error in units of 0.1%.
      unsigned char PhaseLossLim;      //Threshold for when a phase is said to be mising. Units of 1.0V
      //unsigned char MaxCurrent;        //Threshold for when current in a phase or neutral is said to be excessive. Units of 1.0A
      unsigned char ZeroIPhaseLim;     //Threshold for when current is said to be zero in phase A,B or C. Units of .01A
      unsigned char ZeroINeutralLim;   //Threshold for when current is said to be zero in neutral. Units of .01A
      unsigned char ITrigUnbalHiRange; //Threshold for when current in phase A,B or C is said to be or neutral is said to be excessive. Units of 1.0A
      unsigned short UnbalHiRangeLim;  //Max voltage unbalance for when current is high.
      unsigned short UnbalLoRangeLim;  //Max voltage unbalance for when current is low.
      unsigned short NBrokenVLim;      //When neutral current is zero, a neutral voltage larger than this that will trigger a broken neutral warning.
      unsigned short NBrokenZLim;      //When neutral current is non-zero, a neutral impedance larger than this that will trigger a broken neutral warning.
      unsigned short NDegradedZLim;    //When neutral current is non-zero, a neutral impedance larger than this that will trigger a broken neutral warning.
      unsigned short NVarFactorLim;    //Excessive neutral impedance volatility limit.
      unsigned short NVarPeriodLim;    //Excessive neutral impedance volatility calculation period.
      unsigned short InstallCountDown; //Countdown for installation mode
      unsigned short DeltaCurrentLim[3]; //Threshold of delta phase current to calculate the loop impedence; 
   } _ni_cfg1_t;

   typedef struct
   {// Neutral Integrity RAM, status and metrology values
      unsigned char Status1;           //b0=available, b1=?, b2=active, b3=derivative active, b4=block relay, b5=force LCD , b6-7= number of phases
      unsigned char Status2;           //b0-5=latched NI state (b0=broken n, b1=degraded n, b2=supply Z, b3=reverse A/N, b4=volatile, b5=loop), b6=, b7=
      _ni_state_nib_t LastState;       //b0-3=last instantaneous basic NI state as per ePolyNIxxx, b4-7=last instantaneous derivative metering NI state as per ePolyNIxxx
      _ni_state_nib_t NewFltState;     //b0-3=current filtered basic NI state as per ePolyNIxxx, b4-7=current filtered derivative metering NI state as per ePolyNIxxx
      _ni_state_nib_t OldFltState;     //b0-3=last filtered basic NI state as per ePolyNIxxx, b4-7=last filtered derivative metering NI state as per ePolyNIxxx
      unsigned short PersistTimeCnt;   //Number of seconds NI condition has existed.
      _cmplx_t volatile_vals[3];       //volatile metrology values. Value set in metrology task, can change at any time.
      unsigned short VRMSSafe[3];      //copy of RMS voltage from ST28
      unsigned short VAngSafe[2];      //copy of voltage angle from ST28
      unsigned short ISafe[3];         //copy of current from ST28
      unsigned short Frequency;        //copy of freqency from ST28
      _inst_ni_t inst_vals[2];         //0=current instantaneous values, 1=last instantaneous values
      _ni_bkp1_t bkp1;                 //Numerous parameters that are backed up in NV.
      _ni_cfg1_t cfg1;                 //Numerous config parameters that are stored in NV.
      _deriv_ni_t deriv;               //derivative metering state variables
   } _mfg_table_33_t;

   typedef struct
   {// Neutral Integrity configuration structure
      unsigned char PermitBasic;       //
      unsigned char PermitDeriv;       //
      unsigned char ActionsBaisc[20];   //Actions to take once NI state changes, eg log, display, disconnect
      unsigned char ActionsDeriv[20];   //Actions to take once NI state changes, eg log, display, disconnect
      _ni_cfg1_t cfg1NV;               //Numerous config parameters that are copied to RAM.
      unsigned char Spare[24];         //1=1+1+1+7+7+23. 64-40=24
      unsigned char Version;           //Version of structure, number is incremented each time the structure changes and there is a code release
   } _mfg_table_34_t;
#else //#NEUTRAL_INTEGRITY_POLY
   typedef struct
   {// Neutral Integrity RAM, status and metrology values

      _cmplx_t IActiveRaw;             //Active current calculated from Wh and VARh total (Ia) mA
      _cmplx_t IEarthRaw;              //Earth current calculated from Wh and VARh E2 (Ieu) mA
      _cmplx_t IEarthCorrected;        //Adjusted version of IEarthRaw, installation offset applied. (Ie & Iev) mA
      _cmplx_t IEarthSystemic;         //Systemic mesured at installation time. (Ieo) mA
      signed long IActiveAbs;          //hypotenuse of IActiveRaw =sqrt( (Ia.real)^2 + (Ia.img)^2) ) (abs(Ia)) mA
      signed long IEarthActPhase;      //Proportion of earth current in phase with corrected earth current (Ieia) mA
      signed long IEarthMin;           //Minimum nominal earth current (Iemn) mA
      signed long IEarthMax;           //Maximum nominal earth current (Iemx) mA
      signed long IEarthMaxConst;      //Constant- maximum nominal earth current (Ixok) mA
      signed long IEarthRatioMin;      //Constant- minimum nominal ratio of current returing via earth (Semn) 10000 is 1:1
      signed long IEarthRatioMax;      //Constant- maximum nominal ratio of current returing via earth (Semx) 10000 is 1:1
      signed long IEarthRevConst;      //Constant- threshold for earth current on a reverse active/neutral site (Irth) mA
      unsigned char Status1;           //Current status calculated by state machine. highest bit indicates force display
      unsigned char Status2;           //
      unsigned char Status3;           //
      unsigned char Status4;           //
      signed long IEarthAbs;           //abs(Ie) mA
      unsigned short FiltTimeLim;      //Constant- threshold for filtering NI states.
      unsigned short FiltTimeCnt;      //Number of seconds NI condition has existed.
      unsigned short VoltThresh;       //Threshold for triggering an undervoltage condition, severity indicating broken neutral, units of 0.1V.
      unsigned short VoltTimeLim;      //Duration in seconds that voltage threshold must be in breach, before any action is taken.
      unsigned short VoltTimeCnt;      //Duration in seconds that voltage threshold has been in breach.
      signed long Seil;                //Seil calculated at the time of installation
      unsigned short Voltage;          //Voltage in units of 0.01V.
      signed long IEarthSuspectOK;     //active power thresold for peforming supplimentry Hi-Z earth detection
      signed long IEarthThreshMin;     //minimum threshold for earth detection (Ixmn) mA.
      unsigned short SuspectEarthCnt;  //Count of 5min blocks that earth has been suspect
      unsigned short SuspectEarthLim;  //trigger level for suspect earth units of 5min
   } _mfg_table_33_t;

   typedef struct
   {// Neutral Integrity configuration structure
      unsigned char  NIEnabled;        //0=disable NI, 1=enabled, 2-255=NA
      unsigned char  IMax;             //Nominal service current (Imax). 100A =100
      unsigned short IReverseRatio;    //Threshold ratio for when a negative earth current is likeley due to reverse active/neutral. (Srth) Max of 1:1=10,000. steps of .0001
      unsigned short IEarthThreshMin;  //minimum threshold for earth detection (Ixmn) mA.
      unsigned short IEarthThreshMax;  //maximum threshold for earth detection (Ixmx) mA.
      unsigned short SoilResistVar;    //Ratio showing soil resistivity variability ratio with weather conditions. (Rssw) Max of 100=10,000. steps of .01
      unsigned short MaxOkRatio;       //Upper limit for normal earth current. Ratio of active current. (Sxet) 1:1=10,000 steps of .0001
      unsigned short Spare2;           //Spare
      unsigned short InstallErthResist;//Ratio showing default earth resistivity at installation time. (Resp) Max of 100=100. steps of 1
      unsigned short SysRatio;         //Systemic ratio used to calculate systemic offset based on installation mode (Sxio) 1:1=10,000 steps of .0001
      unsigned char EarthParabolic;    //Earthing variability model parabolic order (Kevm)
      unsigned char NeutralLoopPcnt;   //Proportion of supply loop impedance attributed to the Neutral line 1=1% (Szn).
      unsigned short MaxEarthResist;   //Maximum limit of acceptable earthing resistance (Relh) units of 0.1 kohm
      unsigned short SuspectEarthLim;  //trigger level for suspect earth, units of 5min.
      unsigned long SeilThresholdHigh; //Threshold Ratio used during installation to check Seil. If Seil is above this value then installation will not complete. 1.0=1000,000 (broken/deggraded Neutral) (Selh)
   } _NI_table_hdr_t;

   typedef struct
   {// Neutral Integrity configuration structure
      _NI_table_hdr_t cfg1;            //Configuration commonly accessed
      unsigned short InstallDuration;  //Number of seconds NI install check remains active
      unsigned char AllowInstallMode;  //Allow the meter to enter install mode if the command is sent
      unsigned char Actions[7];        //Actions to take once NI state changes, eg log, display, disconnect
      unsigned short FilterTime;       //number of seconds to wait before a NI condition is logged.
      unsigned short VoltageThresh;    //Threshold for triggering an undervoltage condition, severity indicating broken neutral, units of 0.1V.
      unsigned short VoltageDuration;  //Duration in seconds that voltage threshold must be in breach, before any action is taken.
      unsigned char  VoltageAction;    //
      unsigned short LoopImpedanceLim; //Installation loop impedance threshold for N code. (Zslh) Units of 0.001 Ohms.
      unsigned char HiZELongTrendAct;  //Action to take when a high impedance earth is detected using the supplimentry Hi-Z earth detection algorithm (large period +48h)
      unsigned short SuspectEarthRatio;//Fraction of Ixok for use in  for peforming supplimentry Hi-Z earth detection. (Sxax) 1=10,000 steps of .0001
      unsigned char Spare[13];         //28+10+2+2+2+1+1+2+1+2=51. 64-51=13
      unsigned char Version;           //Version of structure, number is incremented each time the structure changes and there is a code release
   } _mfg_table_34_t;

#endif //NEUTRAL_INTEGRITY_POLY

typedef struct
{//Display Control and List for SSN display
   unsigned char     SSN_Display_1[8];
   unsigned char     SSN_Display_2[8];
   unsigned char     SSN_Display_3[8];
   unsigned char     SSN_Display_4[8];
}_mfg_table_35_t;

typedef struct
{//configuration for QOS LP
   unsigned char LP_QOS_Cfg[NUM_LP_DATA_SETS];
} _mfg_table_36_t;

typedef struct
{//configuration and status for password error 1 hour lockout
   unsigned char pwr_cycle_reset;//if non-zero reset lockout on a meter power cycle
   unsigned char spare;
} _mfg_table_37_t;

typedef struct
{//configuration for Reverse energy warning events and flags
   unsigned short RevEngyLim[3]; //number of Wh threshold that will trigger an event and/or status bit. 0=disabled.
} _mfg_table_38_t;

typedef struct
{//past ~14 billing reads (TOU snapshots) header
   unsigned char           nbr_valid_entries;//Number of valid self reads stored in the self read array. The range is zero (meaning no self reads in the array) to 14
   unsigned char           last_entry_element;//The array element number of the newest valid entry in the self read array. This field is valid only if NBR_VALID_ENTRIES is greater than zero.
} _mfg_table_40_header_t;

typedef struct
{//past ~14 billing reads (TOU snapshots)
   unsigned char           nbr_valid_entries;//Number of valid self reads stored in the self read array. The range is zero (meaning no self reads in the array) to 14
   unsigned char           last_entry_element;//The array element number of the newest valid entry in the self read array. This field is valid only if NBR_VALID_ENTRIES is greater than zero.
   _self_read_data_rcd_t   history[MFG40_TOU_NBR_SELF_READS];// typically 12 months +2 for when the meter reader is running late.
} _mfg_table_40_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned short count;   //count of number of meter calibrations
   unsigned long  last[5]; //posix time (GMT)
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _ActivityCal_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned long time;     //posix time (GMT)
   unsigned char param[6]; //rev of firmware that has just been activated
} _FirmwareInfo_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned short  count;  //number of firmware downloads
   _FirmwareInfo_t last[5];//last 5 firmware downloads
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _ActivityFirmware_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned long  time;    //posix time (GMT)
   unsigned short user;    //user ID established during login
   unsigned char  type;    //reprogram type
   unsigned char  subtype; //reprogram sub-type
   unsigned char  ProgID[7];//Empwin (meter suite) Program ID
   unsigned char  Version[7];//Empwin (meter suite) Program version
} _ProgInfo_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned short count;   //number of programmings
   _ProgInfo_t    first;   // first reprogramming
   _ProgInfo_t    last[10];//last 10 reprogrammings
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _ActivityProg_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned short count;   //count of number of tamper events
   unsigned long  first;   //posix time (GMT)
   unsigned long  last[5]; //posix time (GMT)
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _ActivityTamper_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned short count;   //count of number of security fail events
   unsigned long  first;    //posix time (GMT)
   unsigned long  last[5];  //posix time (GMT)
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _SecurityInfo_t;

typedef struct
{//used in _mfg_table_41_t
   _SecurityInfo_t stats[3];//stats for 3 protocols types
} _ActivitySec_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned long reads;    //number of table reads
   unsigned long writes;   //number of table writes
   unsigned long procs;    //number of table procs
   unsigned long errors;   //number of comms errors
} _ActivityComms_t;

typedef struct
{//used in _ActivityLoadVltgDetect_t
   unsigned long  TimeStamp;   //posix time (GMT)
   unsigned short VoltageDifference;   //count of number of tamper events
} _ActivityLoadVltgData_t;

typedef struct
{//used in _mfg_table_41_t
   unsigned short count;   //count of number of tamper events
   _ActivityLoadVltgData_t  first;   //posix time (GMT)
   _ActivityLoadVltgData_t  last[5]; //posix time (GMT)
   unsigned char  pos;     //zero based index of last written time value in "last" array
} _ActivityLoadVltgDetect_t;

typedef struct
{// count and log of last; 5 cal,5 firmware, 10 programming, 5 tamper, ,comms counters
   _ActivityCal_t      cal;
   _ActivityFirmware_t fw;
   _ActivityProg_t     prog;
   _ActivityTamper_t   tamper;
   _ActivitySec_t      sec[2];
   _ActivityComms_t    coms[2];
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(AMPY_METER_U3400)
   _ActivityTamper_t   tamper2;
   _ActivityTamper_t   TrackCutTamper;
   _ActivityLoadVltgDetect_t  LoadSideVoltageTamper;
#endif
} _mfg_table_41_t;

typedef struct
{//used in _mfg_table_42_t
   unsigned char KeyCurr[32];//current CMAC key
   unsigned char KeyFuture[32];//future CMAC key
   unsigned long KeySwitchTime;//switch time of copy of future to current , Posix GMT
} _UsrCMAC_t;

typedef struct
{//CMAC keys and possible change over dates for customer specific stuff
   _UsrCMAC_t Auth[2];//CMAC keys 1 and 2.
} _mfg_table_42_t;

typedef struct
{//used in _mfg_table_43_t
   unsigned char StdRead[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED];
   unsigned char MfgRead[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED];
   unsigned char StdWrite[GEN_CONFIG_TBL_DOT_DIM_STD_TBLS_USED];
   unsigned char MfgWrite[GEN_CONFIG_TBL_DOT_DIM_MFG_TBLS_USED];
   unsigned char StdProc[GEN_CONFIG_TBL_DOT_DIM_STD_PROC_USED];
   unsigned char MfgProc[GEN_CONFIG_TBL_DOT_DIM_MFG_PROC_USED];
   unsigned char Special[2];
} _UsrPermission_t;

typedef struct
{//permissions for table read/write and procs. used in mfg 43,44,45
   _UsrPermission_t permission[4];//4 user levels in this table public,restricted1,restricted2,full
} _mfg_table_43_t;

typedef struct
{//used in _mfg_table_46_t
   unsigned char pwd[20];//password
   unsigned char key[32];//key (AES)
} _PwdKeyPair_t;

typedef struct
{//used in _mfg_table_46_t
   _PwdKeyPair_t NewPair;//future key & password pair
   unsigned long SwitchTime;//switch time of copy of future to current, Posix GMT
   unsigned char SwitchType;//type of switch pwd,key or pwd & key
} _PwdKeySwitch_t;

typedef struct
{//passwords, keys and change over dates. used in mfg 46,47,48. 4 user levels in this table public,restricted1,restricted2,full
   _PwdKeyPair_t   curr[4];//current password and Key
   _PwdKeySwitch_t future[4];//future password and key with activation date
} _mfg_table_46_t;

//there is no _mfg_table_49_t it is the same as _mfg_table_71_t

typedef struct
{//Used by mfg table 50 QOS Event Configuration
  unsigned char QOSType;//Type of QOS quanity voltage/current/PF etc.
  unsigned char BreachTime;//Time in seconds that breach must last before event is logged
  unsigned short Min;//threshold that will trigger a mimimum event
  unsigned short Max;//threshold that will trigger a maximum event
  unsigned short MaxEvents;//maxium number of events that can be logged in the event log in a single day
}_QOS_t;

typedef struct
{//QOS Configuration Table
  _QOS_t QOSConfig[6];
}_mfg_table_50_t;

typedef struct
{//Used by mfg table 51 QOS Breach Event
   unsigned short  MinCount; //set when the breach begins
   unsigned short  MaxCount; //set when the breach begins
   unsigned long   LastBreachStartTime;//posix GMT set when the breach begins
   unsigned long   LastBreachEndTime;//posix GMT set to all zero if breach is still active
   unsigned char   LastTypeMinMax;//is the above time stamp for a min or a max? set when the breach begins 0=min, 1=max
   unsigned short  LastMin;//only updated at the end of a breach or on power down?
   unsigned short  LastMax;// only updated at the end of a breach or on power down?
   unsigned short  Min;// only updated at the end of a breach or on power down?
   unsigned short  Max;// only updated at the end of a breach or on power down?
}QOSEvent_t;


typedef struct
{//QOS Breach Events
   QOSEvent_t  QOSEvent[6];
}_mfg_table_51_t;


typedef struct
{// General NV constants set at the time of manufacture
   unsigned char  CatalogueNum[16]; //Catalogue Number, set by FTS
   unsigned char  PhyRelayPresent;  //bitmask of physical relays in meter
   unsigned char  HWFeaturesMask;   //bitmask of random hardware features
   unsigned short RippleVoltMag;    // RMS voltage magnitude calibration. ratio of 1.0 is 32678
   unsigned short LoadSideVoltMag;  // RMS voltage magnitude calibration. ratio of 1.0 is 32678
   unsigned char  FeatureMask;      // Bitmask of features available in the meter that customers have paid for. A cleared bit means enabled, see MFG_CONST_FE_N_??? #defs.
#ifdef RELAYLIFETEST
   unsigned char  Spare[4];        //spare bytes for future expansion
   unsigned char  Relay;
   unsigned char  SwtichCyle;
   unsigned char  PulseWidth;
   unsigned short RelaySwtichCounter;
#else
   unsigned char  Spare[9];
#endif
} _mfg_table_52_t;

typedef struct
{// Used by mfg table 53 RTC xtal calibration.
   signed short   Offset;        //units of 0.1 ppm
   unsigned short QuadFactor;    //units of 1 ppb/C^2
   signed short   TemperatureRef;//units of 0.1 degrees C
} _Xtal_Cal;//used by mfg table 53

typedef struct
{// mfg table 53 structure RTC xtal and temperature cal.
   _Xtal_Cal      XCal[2];          //14MHz xtal in element 0, 32kHz xtal in element 1
   unsigned short ThermalOffset;    // units of 0.01 degrees C (offset of temperature sensor)
} _mfg_table_53_t;

typedef struct
{//used in _mfg_table_54_t
   unsigned long state_pub    :4;
   unsigned long state_res1   :4;
   unsigned long state_res2   :4;
   unsigned long state_full   :4;
   unsigned long state_fact1  :4;
   unsigned long state_fact2  :4;
   unsigned long state_fact3  :4;
   unsigned long state_spare  :4;
} _pwd_err_state_t;

// mfg table 54 structure
typedef struct
{//General Meter NV read only status information, a place where random quantities come to die, feel free to extend
   unsigned short    PFCycleCounts[4];//old school PF counters from class 199
   unsigned long     BatteryDuration;//number of seconds battery has been running
   unsigned short    FractionalSeconds;//sub-second PF RTC sync, units of ms
   _std_table_52_t   OutagesStart[10];//start times of last 10 outages uses ANSI ref of year 2000
   _std_table_52_t   OutagesEnd[10];//end time of last 10 outages uses ANSI ref of year 2000
   unsigned char     RecentOutageIndex;//zero based index into OutagesStart/End array showing most recent outage
   unsigned char     VoltageFlag;//Bit falg used to flag voltage loss/recover on the phase A,B &C;
   unsigned short    QOSMinEventCount[6];//NV backup of QOS Min Event counters to limit overflow of event log.
   unsigned short    QOSMaxEventCount[6];//NV backup of QOS Max Event counters to limit overflow of event log.
   unsigned short    NumEventsLogged;//NV backup of the number of unacknowledged events logged today
   unsigned short    StatusBkpStd;//NV backup of standard status flags in standard table 3
   unsigned long     StatusBkpMfg;//NV backup of manufacturing status flags in standard table 3
   unsigned short    ExportEngy[3];//NV backup of number of exported Wh since exp was detected
   unsigned long     LPQOS[3];//NV backup of LP QOS items min,max or avg
   unsigned char     TOULastSeas;//NV backup of last season meter was in before power fail
   unsigned char     TOULastSpecialDay;//NV backup of last special day meter was in before power fail
   unsigned char     OldFirmwareRev[16];//NV Backup of the current firmware rev,will be used at powerup to determine if firmware has changed
   _pwd_err_state_t  pwd_fail_stats[3];//status info for psem,psemx,kf2
   unsigned short    LastGoodMainsVoltage;//used mainly by 2E meters with SD open where MMI data becomes invalid
   unsigned char     TOU_Rate;
   unsigned long     Demand_Ref[2];
   unsigned long     DemandArray[2][6];
#if defined(NEUTRAL_INTEGRITY)
   unsigned char     NIStatus[3];//Neutral Integrity backup of status, status2 and status3.
#elif defined(NEUTRAL_INTEGRITY_POLY)
   unsigned char     NIStatus[2];//Neutral Integrity backup of status & status2.
   unsigned char     Spares1;
#else
   unsigned char     Spares1[3];
#endif
   unsigned char     DisRelayCommand;
   unsigned char     TamperStatus;      //NV Tamper status bits
   unsigned short    DF_page_refresh;//flash page refresh index
   unsigned char     ExclusiveRemoteAccessStatus;     //Status of exclusive Remote access
   unsigned short    ExclusiveAccessTimeLeft;      //Time in Secs of exclusive Remote access left
#if defined(NEUTRAL_INTEGRITY)
   unsigned char     NIStatus4;//Neutral Integrity backup of Status4.
   unsigned short    SuspectEarthCnt;//Neutral Integrity Hi-Z earth counter.
#else
   unsigned char     Spares2[3];
#endif
   long              MetroBinAccBak[16];
   unsigned char     MetroEnergyIncrementsBak[16];
   unsigned short    MetroBinAccBakChkSum;
#ifdef SIM_INTERRUPT
   _sim_interrupt_t  pfSimulator;
#else   
   unsigned char     LpQoS_MaxIndex;
   unsigned char     LSM;
   unsigned long     LSMTimeout;
   unsigned char     spare3[39];
#endif
} _mfg_table_54_t;

typedef struct
{//used in internal LP state machine (Mfg 56)
   unsigned short Nbr_Blks;//number of data blocks
   unsigned short Nbr_blk_ints;//number of intevals per data block
   unsigned char  Nbr_Chns;//number of channels
   unsigned char  Max_Int_Time;//interval duration
   unsigned char  ValSelect[NUM_CH_PER_DATA_SET];//selects what quantities to monitor
   signed long    LastVals[NUM_CH_PER_DATA_SET];//last snapshot used for delta calcs
   _ltime_date_t  LastTime;//time stamp for last written interval
   unsigned char  SFlags;//flags for partial interval, time change back/forward, Power Fail
   signed short   TimeShiftDelta;//updated when clock changes occur
   unsigned long  DataSetOffset;//offset of LP data set relative to the shared LP memory area
   unsigned short ActiveInterval;//current inetrval that is active
   unsigned short ActiveBlock;//current block that is active
   unsigned long  NumZeroInt;//number of zero intervals that have to be written
   unsigned short BkpDeltas[NUM_CH_PER_DATA_SET];//backup for when interval ends during zero fill
   unsigned char  BkpFlags[(NUM_CH_PER_DATA_SET/2)+1];//backup for when interval ends during zero fill
   _ltime_date_t  BkpLastTime;//backup for when interval ends during zero fill
#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
   unsigned short LastUnreadBlk;//oldest unread block to be presented in the partial LP mfg tbls
#endif
} _lp_data_set_state_t;

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)|| defined(DYNAMIC_LP)
	typedef struct
	{//LP status NV copy of LP state machine variables
	   unsigned char        NumSets;//Used as bit field, with Sets represented by a bit. Bit0=Set1..Bit3=Set4 
	   unsigned char        LPFlags[NUM_LP_DATA_SETS];
	   _lp_data_set_state_t sets[NUM_LP_DATA_SETS];
	   unsigned short       QOSAvgN[NUM_LP_DATA_SETS][NUM_CH_PER_DATA_SET];//it woul dbe better to put this in sets[] but we don't want to corrupt old firmware.
	}_mfg_table_56_t;
#else
	typedef struct
	{//LP status NV copy of LP state machine variables
	   unsigned char        NumSets;
	   unsigned char        LPFlags;
	   _lp_data_set_state_t sets[NUM_LP_DATA_SETS];
	   unsigned short       QOSAvgN[NUM_LP_DATA_SETS][NUM_CH_PER_DATA_SET];//it woul dbe better to put this in sets[] but we don't want to corrupt old firmware.
	}_mfg_table_56_t;
#endif
typedef struct
{//used in Mfg 57 -Back End Comms Settings (Utilinet Specific)
   unsigned char lan_id[4];
   unsigned char lan_crc[2];
   unsigned char channelMask[33];//CHANNEL_MASK_LENGTH
   unsigned char configBytes[7];
   unsigned char zigbeeMAC[8];
} _UtiliNet_Param_t;

typedef struct
{//used in Mfg 57 -Back End Comms Settings (SSN Specific)
   unsigned char  mac[8];//format is raw binary data
   unsigned char  hw_rev;
   unsigned char  hw_ver;
   unsigned char  hw_mode;
   unsigned char  part_num[8];
   unsigned char  part_num_rev;
   unsigned char  catalog_num[10];
   unsigned char  part_num_dash_rev[2];
   unsigned char  part_num_dash_rev_2[2];
} _SSN_Param_t;

typedef struct
{//used in Mfg 57 -Back End Comms Settings (3G Specific)
   unsigned char none;
} _3G_Param_t;

typedef struct
{//used in Mfg 57 -Back End Comms Settings (Wifi Specific)
   unsigned char none;
} _Wifi_Param_t;

typedef struct
{//Back End Communications Configuration, S+U, 3G, WiFi
   unsigned short flag_poll_rate;//bits 0-13 are poll rate in units of 10 sec, (0x000=disabled, 0x0001-0x2FFF=rate, 0x3000=preceded by cmd). bit14=do display
   unsigned char  timeout;       //timeout for response to AT command. seconds
   unsigned char  cmd_sdx;       //offset into string list, pointer to command to be sent
   unsigned char  delimiter_sdx; //offset into string list, pointer to list of delimiters
} _at_cmd_t;

typedef struct
{//
   unsigned char  flag_cmd_idx;  //bits 0-3=index of command that this filed belongs to. bit4=field is to be shown on LCD, bit5-7=LP quantity (0=NA,1-4=LP channel, 5-7=NA)
   unsigned char  offset;        //offset of this field from the start of the response. bits 0-3=delimiter separated field index, bits 4-7= byte offset from delimiter
   unsigned char  field_sdx;     //offset into string list, pointer to format string for the field.
   unsigned char  ram_offset;    //location in MT29 where result will be placed.
   unsigned char  ram_length;    //maximum amount of RAM in MT29 that result can use.
   unsigned char  bit_offset;    //0=Process an numeric, bits0-3=the offset into the LP interval value, bits 4-7=length,in bits, to be copied across (0=1bit)
} _at_fields_t;

typedef struct
{//used in Mfg 57 -Back End Comms Settings (3G Specific)
   unsigned char  modem_type; //used by empWin to indicate the current configuration type
   unsigned char  reset_hour;  //period of modem reset action. hours 0=midnight, 24=disabled, 25=modem always power failed
   unsigned char  reset_random;  //period over which reset action is randomised. minutes.
   unsigned char  reset_day;  //offset from the start of the week (sun->mon midnight UTC). hours, 7 = every day
   unsigned char  baud_rate;     //default baud rate {e1200b, e9600b, e19200b, e38400b, e57600b, e115200b, e2400b, e4800b, eMaxBaud} eBaudRate
   unsigned char  lcd_periods;   //bits0-3=rate at which AT commands, required for lcd display, are sent (Seconds). bits4-7=period of time LCD AT mode will persist (units of 20 seconds, 0=20sec)
   unsigned char  fast_at_delay; //seconds to wait after alternate display has been shown with AT Display
   unsigned char  cmd_prefix[4]; //AT command prefix
   _at_cmd_t      at_cmds[6];    //list of AT commands to be sent to modem
   _at_fields_t   at_fields[12]; //list of fields returned from modem that need to be decoded
   unsigned char  string_list[128];//series of 0x00 terminated strings for commands, delimiters, fields.
   //unsigned char  escape_char;//Character used for the escape sequence, normally '+'
} _modem_Param_t;
typedef struct
{//Back End Communications Configuration, S+U, 3G, WiFi
   unsigned short ParameterLength;//size in bytes of the parameter data
   unsigned char  CommsType;//selects the type of back end comms system. 1= S+U, 2 = SSN, 3 = Gridnet
   unsigned char  Version;//specifes the version onthe selected comms sytem
   union
   {
      _modem_Param_t modem;
      unsigned char     Parameters[256];
      _UtiliNet_Param_t PU;
      _SSN_Param_t      PS;
      _3G_Param_t       P3G;
      _Wifi_Param_t     PW;
   } CommParams;
}_mfg_table_57_t;

typedef struct
{//extended Event Control
   unsigned char std_event_alarm[ACT_LOG_TBL_DOT_NBR_STD_EVENTS];
   unsigned char mfg_event_alarm[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS];
   unsigned char sub_event_log[10];
   unsigned char sub_event_alarm[10];
}_mfg_table_58_t;

// display config table
// alternative to std table 32
typedef struct
{
   unsigned char energy_decimal_places;
   unsigned char energy_digits;
   unsigned char display_leading_zeros_flag;
   unsigned char display_id[ACT_DISP_TBL_DOT_NBR_PRI_DISP_LIST_ITEMS]; //display ID array
}_mfg_table_59_t;

/*
typedef struct
{//event counters table
   unsigned short       StdTotalCounts[ACT_LOG_TBL_DOT_NBR_STD_EVENTS*8];//a count of the number of times the event occured
   unsigned short       MfgTotalCounts[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS*8];//a count of the number of times the event occured
   unsigned char        StdRecentLimits[ACT_LOG_TBL_DOT_NBR_STD_EVENTS*8/2];//each nibble holds a threshold applied to the recent count (limit=2^val)
   unsigned char        MfgRecentLimits[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS*8/2];//each nibble holds a threshold applied to the recent count (limit=2^val)
   unsigned char        StdRecentCounts[ACT_LOG_TBL_DOT_NBR_STD_EVENTS*8];//a count of the number of times the event has occured recently
   unsigned char        MfgRecentCounts[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS*8];//a count of the number of times the event has occured recently
} _mfg_table_71_t;

typedef struct
{//Alarm event configuration table
   unsigned char        StdAlarmFlags[ACT_LOG_TBL_DOT_NBR_STD_EVENTS]; //if bit is set then the message is sent to CC Immediately
   unsigned char          MfgAlarmFlags[ACT_LOG_TBL_DOT_NBR_MFG_EVENTS]; //if bit is set then the message is sent to CC Immediately
} _mfg_table_72_t;
*/

typedef struct
{
   unsigned short ms100;
   unsigned short ms;
   unsigned short sub_ms;
} _hi_res_time_t;

typedef struct
{//used in MT62
   unsigned long  SampleCount;//Number of samples that have been added to the Squared Sum.
   unsigned short LineFlagsFreq;//bits0-12= frequency in units of 0.01Hz on the ripple input bit13=, bit14=, bit15=.
   unsigned long  CycCount;//number of full cycles that have been counted on the ripple input
   unsigned long  SampleLim;//number of samples to calc UF over
} _UndrFreq_t;

typedef struct
{//used in MT62
	 unsigned short LineFreq;//frequency in units of 0.01Hz on the ripple input
    unsigned char  CycCount;//number of full cycles that have been counted on the ripple input
    _UndrFreq_t    UFreq[2];
} _Frequency_t;

typedef struct
{//used in MT62
	 unsigned short LoadSideRMS;     //RMS voltage on load side voltage input.
    unsigned short RippleRMS;       //RMS voltage on ripple input.
    unsigned long  LoadSideSaved;   //Saved snapshot of sample squared for load side voltage input.
    unsigned long  RippleSaved;     //Saved snapshot of sample squared for ripple input.
    unsigned short CountSaved;      //Saved number of samples that have been added to the Saved Squared Sum.
	 unsigned long  LoadSideSqSum;   //Sum of sample squared for load side voltage input.
    unsigned long  RippleSqSum;     //Sum of sample squared for ripple input.
    unsigned long  LoadSideSampSum; //direct sum of samples from ADC on load side voltage input, used for DC offset removal.
    unsigned long  RippleSampSum;   //direct sum of samples from ADC on ripple input, used for DC offset removeal.
  	 unsigned short LoadSideDC;      //Average ADC value i.e. DC component on load side voltage input.
    unsigned short RippleDC;        //Average ADC value i.e. DC component on ripple input.
    unsigned short SampleCount;     //Number of samples that have been added to the Squared Sum.
    unsigned short LoadSideCal;     //Calibration of magnitude for load side voltage input.
    unsigned short RippleCal;       //Calibration of magnitude for ripple input.
    unsigned short InstantRippleRSSI;  //0.01volt per unit
} _RMSVolts_t;

typedef struct
{//used in MT62
	 unsigned long tics;
	 unsigned short subtics; // TA2 not signed, but range is 0-1842 and we can take differences
} _MetrologyTime_t;

typedef struct
{//metrology debugging info used in MT62
	unsigned long powerfail_count;
	unsigned long checksumerror_count; // since powerup
	unsigned long total_mmi_reads; // since powerup
	unsigned long delay_count; // How many times we don't set MMI3_CSB low on time
	_MetrologyTime_t max_delay;
	unsigned long wait_count; // How often we have to WAIT
	_MetrologyTime_t max_wait_time;
	_MetrologyTime_t min_spi_hog_time;
	unsigned char max_tasks_waiting;
	unsigned long ParamError_count;    //Since Power up
} _MetrologyDebug_t;

typedef struct
{//Geric RAM used for RMS voltage, frequency calculation,debugging
   _RMSVolts_t       RMSVolts;   //RMS voltage for load side voltage detect and ripple
   _Frequency_t      MainsFreq;  //high accuracy frequency
   _MetrologyDebug_t Debug;      //metrology debugging information
} _mfg_table_62_t;

typedef struct
{//Test Status table used only in Test mode
   unsigned short Temperature;
   unsigned char RTCError;
   unsigned char FlashError;
   unsigned char Button1Count;
   unsigned char Button2Count;
   unsigned char Button3Count;
   unsigned char Button4Count;
   unsigned char seconds;
} _mfg_table_64_t;

#if defined(AMPY_METER_U1300)||defined(AMPY_METER_R1100)||defined(AMPY_METER_U3400)
typedef struct
{//Currently Active Relay Configuration Table
  unsigned char         spare[90];
  unsigned short        AutoDisconnectMeasurementTime;
  unsigned short        AutoDisconnectPower;
  unsigned short        AutoDisconnectBreachTime;
  unsigned char         NormalSupplyImportedCapacityLimitMeasurementTime;
  unsigned char         NormalSupplyImportedCapacityLimitPower;
  unsigned char         NormalSupplyExportedCapacityLimitMeasurementTime;
  unsigned char         NormalSupplyExportedCapacityLimitPower;
  unsigned char         NormalSupplyCapacityLimitActiveTime;
  unsigned char         EmergencySupplyCapacityLimitMeasurementTime;
  unsigned char         EmergencySupplyCapacityLimitPower;
  unsigned char         EmergencySupplyLimitActiveTime;
  unsigned char         PolyBreakerTwitchHour;
  unsigned char         PolyBreakerTwitchRandomMax;
  unsigned short        LosLimitPwrDnDuration;   //0: Los disable;  5-300 seconds
  unsigned short        LosMinDelayTime;         //0 - 300 seconds
  unsigned short        LosMaxDelayTime;         //0 - 300 seconds
  unsigned short        BreakerARMTimer;          //0 - 2880 minutes
  unsigned short        TODSCCImportedStartTime;
  unsigned short        TODSCCImportedDuration;
  unsigned char         TODSCCImportedMeasurementTime;
  unsigned char         TODSCCImportedPowerLimit;
  unsigned char         TODSCCImportedActiveTime;
  unsigned short        TODSCCExportedStartTime;
  unsigned short        TODSCCExportedDuraton;
  unsigned char         TODSCCExportedMeasurementTime;
  unsigned char         TODSCCExportedPowerLimit;
  unsigned char         TODSCCExportedActiveTime;
}_mfg_table_66_t;

typedef struct
{//Future Relay Configuration Table
  unsigned char         spare[90];
  unsigned short        AutoDisconnectMeasurementTime;
  unsigned short        AutoDisconnectPower;
  unsigned short        AutoDisconnectBreachTime;
  unsigned char         NormalSupplyImportedCapacityLimitMeasurementTime;
  unsigned char         NormalSupplyImportedCapacityLimitPower;
  unsigned char         NormalSupplyExportedCapacityLimitMeasurementTime;
  unsigned char         NormalSupplyExportedCapacityLimitPower;
  unsigned char         NormalSupplyCapacityLimitActiveTime;
  unsigned char         EmergencySupplyCapacityLimitMeasurementTime;
  unsigned char         EmergencySupplyCapacityLimitPower;
  unsigned char         EmergencySupplyLimitActiveTime;
  unsigned long         ActivationDate;
  unsigned char         PolyBreakerTwitchHour;
  unsigned char         PolyBreakerTwitchRandomMax;
  unsigned short        LosLimitPwrDnDuration;   //0: Los disable;  5-300 seconds
  unsigned short        LosMinDelayTime;         //0 - 300 seconds
  unsigned short        LosMaxDelayTime;         //0 - 300 seconds 
  unsigned short        BreakerARMTimer;          //0 - 2880 minutes
  unsigned short        TODSCCImportedStartTime;
  unsigned short        TODSCCImportedDuration;
  unsigned char         TODSCCImportedMeasurementTime;
  unsigned char         TODSCCImportedPowerLimit;
  unsigned char         TODSCCImportedActiveTime;
  unsigned short        TODSCCExportedStartTime;
  unsigned short        TODSCCExportedDuraton;
  unsigned char         TODSCCExportedMeasurementTime;
  unsigned char         TODSCCExportedPowerLimit;
  unsigned char         TODSCCExportedActiveTime;
}_mfg_table_67_t;

typedef struct
{//Relay Settings
  unsigned char   RelayPresentMask;
  unsigned char   EnableArmForConnect;
  unsigned char   EnableArmForDisconnect;
  unsigned char   spare[2];
  unsigned char   DumpE2BoostIntoE1;
  unsigned short  PrimaryGroupID;
  unsigned short  SecondaryGroupID;
  unsigned short  TertiaryGroupID;
}_mfg_table_68_t;

typedef struct
{//Relay Status - This Table is in eRAM
   unsigned char   DisconnectRelayStatus;
   unsigned char   DisconnectRelayArmedState;
   unsigned short  AutoDisconnectTimeRemaining;
   unsigned short  AutoDisconnectBreachDuration;
   signed short    SupplyCapacityTradingIntervalHistory[10];
   unsigned short  SupplyCapacityRecloseCountDown;
   unsigned char   SupplyCapacityHourHistory[60];
   unsigned char   EmergencySupplyCapacityControlActive;
   unsigned char   ControlledLoadState;
   unsigned long   ConrolledLoadOverrideTimer;
   unsigned char   Spare1[2];
   unsigned char   PendingFutureConfiguration;
   unsigned char   OtherLoadState;
   unsigned long   OtherLoadOverrideTimer;
   unsigned long   TradingImportReference;
   unsigned long   TradingExportReference;
   unsigned long   EmergencyReference;
   unsigned char   MinuteRemainder;   
}_mfg_table_69_t;

typedef struct
{
  unsigned char   RelayControlFlag;
  unsigned char   SccArmBckup;
  unsigned short  BreakerArmTimer;
}Breaker_Backup;

typedef struct
{
  unsigned short  RAppLC_StatusBackup[RAPPLC_NUM_RELAY];
  unsigned char   RAppLC_RelayStateBackup[RAPPLC_NUM_RELAY];  
  unsigned char   RAppLC_OverrideStatus;
}LC_Backup;

typedef struct
{//Relay Status Backup - copy of Relay Status in eExtFlash During Mains OFF
   _mfg_table_69_t   MTbl69;
   //Breaker_Backup    brk_bck;
   //LC_Backup         lc_bck;
   unsigned char   RelayControlFlag;
   unsigned short  RAppLC_StatusBackup[RAPPLC_NUM_RELAY];
   unsigned char   RAppLC_RelayStateBackup[RAPPLC_NUM_RELAY];
   unsigned char   SccArmBckup;
   unsigned short  BreakerArmTimer;
}_mfg_table_70_t;
#else
   typedef struct
   {//Currently Active Relay Configuration Table
     unsigned short        ControlledLoadWDSwtichedonTimes[5];
     unsigned short        ControlledLoadWDSwtichedoffTimes[5];
     unsigned short        ControlledLoadWESwtichedonTimes[5];
     unsigned short        ControlledLoadWESwtichedoffTimes[5];
     unsigned char         ControlledLoadSpreadonTime;
     unsigned char         ControlledLoadSpreadoffTime;
     unsigned short        ControlledLoadOverrideTime;
     unsigned char         ControlledLoadBoostTime;
     unsigned char         ControlledLoadBoostPrimacy;//44
     unsigned short        OtherLoadWDSwitchedonTimes[5];
     unsigned short        OtherLoadWDSwitchedoffTimes[5];
     unsigned short        OtherLoadWESwitchedonTimes[5];
     unsigned short        OtherLoadWESwitchedoffTimes[5];
     unsigned char         OtherLoadSpreadonTime;
     unsigned char         OtherLoadSpreadoffTime;
     unsigned short        OtherLoadOverrideTime;
     unsigned short        AutoDisconnectMeasurementTime;
     unsigned short        AutoDisconnectPower;
     unsigned short        AutoDisconnectBreachTime;
     unsigned char         NormalSupplyImportedCapacityLimitMeasurementTime;
     unsigned char         NormalSupplyImportedCapacityLimitPower;
     unsigned char         NormalSupplyExportedCapacityLimitMeasurementTime;
     unsigned char         NormalSupplyExportedCapacityLimitPower;
     unsigned char         NormalSupplyCapacityLimitActiveTime;
     unsigned char         EmergencySupplyCapacityLimitMeasurementTime;
     unsigned char         EmergencySupplyCapacityLimitPower;
     unsigned char         EmergencySupplyLimitActiveTime;
     unsigned char         PolyBreakerTwitchHour;
     unsigned char         PolyBreakerTwitchRandomMax;
     unsigned short        LosLimitPwrDnDuration;   //0: Los disable;  5-300 seconds
     unsigned short        LosMinDelayTime;         //0 - 300 seconds
     unsigned short        LosMaxDelayTime;         //0 - 300 seconds 
     unsigned short        BreakerARMTimer;          //0 - 2880 minutes
     unsigned char         spare[50];
   }_mfg_table_66_t;

   typedef struct
   {//Future Relay Configuration Table
     unsigned long         ActivationDate;
     unsigned short        ControlledLoadWDSwtichedonTimes[5];
     unsigned short        ControlledLoadWDSwtichedoffTimes[5];
     unsigned short        ControlledLoadWESwtichedonTimes[5];
     unsigned short        ControlledLoadWESwtichedoffTimes[5];
     unsigned char         ControlledLoadSpreadonTime;
     unsigned char         ControlledLoadSpreadoffTime;
     unsigned short        ControlledLoadOverrideTime;
     unsigned char         ControlledLoadBoostTime;
     unsigned char         ControlledLoadBoostPrimacy;//44
     unsigned short        OtherLoadWDSwitchedonTimes[5];
     unsigned short        OtherLoadWDSwitchedoffTimes[5];
     unsigned short        OtherLoadWESwitchedonTimes[5];
     unsigned short        OtherLoadWESwitchedoffTimes[5];
     unsigned char         OtherLoadSpreadonTime;
     unsigned char         OtherLoadSpreadoffTime;
     unsigned short        OtherLoadOverrideTime;
     unsigned short        AutoDisconnectMeasurementTime;
     unsigned short        AutoDisconnectPower;
     unsigned short        AutoDisconnectBreachTime;
     unsigned char         NormalSupplyImportedCapacityLimitMeasurementTime;
     unsigned char         NormalSupplyImportedCapacityLimitPower;
     unsigned char         NormalSupplyExportedCapacityLimitMeasurementTime;
     unsigned char         NormalSupplyExportedCapacityLimitPower;
     unsigned char         NormalSupplyCapacityLimitActiveTime;
     unsigned char         EmergencySupplyCapacityLimitMeasurementTime;
     unsigned char         EmergencySupplyCapacityLimitPower;
     unsigned char         EmergencySupplyLimitActiveTime;
     unsigned char         PolyBreakerTwitchHour;
     unsigned char         PolyBreakerTwitchRandomMax;
     unsigned short        LosLimitPwrDnDuration;   //0: Los disable;  5-300 seconds
     unsigned short        LosMinDelayTime;         //0 - 300 seconds
     unsigned short        LosMaxDelayTime;         //0 - 300 seconds 
     unsigned short        BreakerARMTimer;          //0 - 2880 minutes
     unsigned char         spare[50];
   }_mfg_table_67_t;

   typedef struct
   {//Relay Settings
     unsigned char   RelayPresentMask;
     unsigned char   EnableArmForConnect;
     unsigned char   EnableArmForDisconnect;
     unsigned char   RelayCloseDitherMask;
     unsigned char   RelayOpenDitherMask;
     unsigned char   DumpE2BoostIntoE1;
     unsigned short  PrimaryGroupID;
     unsigned short  SecondaryGroupID;
     unsigned short  TertiaryGroupID;
     unsigned char   spare[50];
   }_mfg_table_68_t;

   typedef struct
   {//Relay Status - This Table is in eRAM
      unsigned char   DisconnectRelayStatus;
      unsigned char   DisconnectRelayArmedState;
      unsigned short  AutoDisconnectTimeRemaining;
      unsigned short  AutoDisconnectBreachDuration;
      signed short    SupplyCapacityTradingIntervalHistory[10];
      unsigned short  SupplyCapacityRecloseCountDown;
      unsigned char   SupplyCapacityHourHistory[60];
      unsigned char   EmergencySupplyCapacityControlActive;
      unsigned char   ControlledLoadState;
      unsigned short  ControlledLoadPendingTimeRemaining;
      unsigned short  ControlledLoadGroupTimeRemaining;
      unsigned short  ControlledLoadBoostTimeRemaining;
      unsigned char   PendingFutureConfiguration;
      unsigned char   OtherLoadState;
      unsigned short  OtherLoadPendingTimeRemaining;
      unsigned short  OtherLoadGroupTimeRemaining;
      unsigned long   TradingImportReference;
      unsigned long   TradingExportReference;
      unsigned long   EmergencyReference;
      unsigned char   MinuteRemainder;
   }_mfg_table_69_t;

   typedef struct
   {//Relay Status Backup - copy of Relay Status in eExtFlash During Mains OFF
      _mfg_table_69_t   MTbl69;
      unsigned char   RelayControlFlag;
      unsigned char   LCControlState;//The functions which control the LC
      unsigned char   LCPwrDwnStat;
      unsigned char   LCUndrFreqStat;
      unsigned char   SccArmBckup;
      unsigned short  BreakerArmTimer;
   }_mfg_table_70_t;
#endif

// mfg table 71 structure
typedef struct
{// Asynchronous Event Pin
	unsigned short int 		nbr_unread_alarms;
	unsigned short int		nbr_unread_events;
	_event_entry_rcd_t		last_unread_alarm;
	unsigned short int		last_alarm_entry_element;
	unsigned char			   async_pin_state;
}_mfg_table_71_t;

typedef struct
{
  unsigned char Telegram[13]; //12.4 bytes are used for Telegrams. 12.5- - flag for command specific. 12.6-12.7 - fixed (00), random(01), fixed+random(10)
  unsigned char Telegram_action; //action to be performed.
  unsigned char Action_retriggerable;//0- action cannot be retriggered 1- action can be retriggered
  unsigned char Telegram_name; //holds a number which is decoded into a telegram name
} _ripple_telegram_t;

typedef struct
{
   unsigned char  RippleSignallingSystem; // eRippleSignallingSystem (stored for EMPWin reporting only!)
   unsigned char  NumberOfCommandPulses;       // number of pulses in the telegram
   unsigned char  NumberofValidBits; //Number of 1s in a Telegram for it to be valid
   unsigned short RippleFrequency;       // actual frequency in Hz x 10
   unsigned short MinSignalStrength;     // to set sensitivity
   unsigned char  NumberPulsesBeforeAbortAllowed; // minimum number of pulses before telegram reset allowed - top bit indicates two abort pulses required
   unsigned char  FilterBandwidth;             // in Hz
   unsigned short NominalPulseWidths[4];       // Ta, Tb, Tc, Td in 10's msecs
   unsigned short MinPulseWidths[4];           // Ta, Tb, Tc, Td in 10's msecs
   signed char    FinePulseWidth[4];         // Ta, Tb, Tc, Td in msecs
   unsigned char  MainFreq;
}_ripple_driver_t;

typedef struct
{
  unsigned char            Failsafe_Type;//0-TrackSpecific, 1-Device Specific and 2- Command Specific Failsafe timers. 3=disabled
  unsigned short           Failsafe_Timeout;//time in Minutes
  unsigned char            Failsafe_Action; //0- do nothing, 1-Open, 2-Close, 3-Scheduler
  unsigned char            Failsafe_Exit;  //0- Manual FailSafe Recover, 1- Automatic Failsafe Recover
  unsigned char            Failsafe_Restore_Action; //To be used with manual recovery and time table action.
}_ripple_failsafe_t;

typedef struct
{
   unsigned char      Num_Telegrams_Prog;//total number of telegrams programmed. should be less than RIPPLE_NO_TELEGRAMS(16)
  _ripple_telegram_t  RippleTelegram[RIPPLE_NO_TELEGRAMS]; //upto 16 telegrams per relay
}_ripple_telegrams_t;

typedef struct
{
   unsigned short Fixed_Delay;//time in seconds
   unsigned short Min_Random_Delay; //time in seconds
   unsigned short Max_Random_Delay; //time in seconds
}_ripple_delay_t;

typedef struct
{
   unsigned short Min_Random_Delay; //time in seconds
   unsigned short Max_Random_Delay; //time in seconds
   unsigned short pulse_time; // pulse time in seconds
}_ripple_pulse_t;

typedef struct
{
  _ripple_delay_t  ripple_pwrup_delay;
  unsigned char    ripple_pwrup_action;
}_ripple_pwrup_t;

typedef struct
{
  _rdate_t                    season_sw_date;//date when the season becomes effective
  _r_app_lc_cfg_norm_rec_t    norm;          //schedule for normal days
} _r_app_ripple_cfg_seas_rec_t;

typedef struct
{//used in MT73 for ripple scheduler configuration.
   unsigned short              min_start; //minimum period over which randomisation can occur units of seconds
   unsigned short              max_start; //maximum period over which randomisation can occur units of seconds
   unsigned char               default_action;
  _r_app_ripple_cfg_seas_rec_t seasons[RAPPLC_NUM_SCHED_SEAS];              //switch times for each season
} ripple_cfg_sched_t;

typedef struct
{
   unsigned char              SupplyLossMemory;  //0=disbaled (unchanged) 1=enabled (memory)
   _ripple_failsafe_t         Ripplefailsafe; //settings for fail safe
   _ripple_delay_t            Delayswtich[2];//fixed &/or random delay before switching LC. element 0 for open to close delay and element 1 for close to open
   _ripple_pwrup_t            Ripple_PwrUp;//Delay and action on power up
   _ripple_pulse_t            PulseTimer; //Pulse operation
   _r_app_lc_cyclic_cfg_t     Cyclic_ripple;//Note that the overwrite_Perm field in ripple is used as 0- dont attempt to copy cyclic_ripple settings to working cyclic parameters. 1- make the attempt
   _ripple_telegrams_t        Telegrams; // Telegrams & Actions
}ripple_relay_config_t;

typedef struct
{// Ripple Configuration Table
   unsigned char            Enable;
   unsigned char            RippleProgramID[7];//ASCII String
   unsigned char            RippleProgramVer[3];//ASCII String
   _ripple_driver_t         RippleDriver;//30 bytes +11bytes
   ripple_relay_config_t    Relay_Config[RAPPLC_NUM_RELAY];
   ripple_cfg_sched_t       Ripple_Sched_Cfg[RAPPLC_NUM_RELAY];//Scheduler for ripple
}_mfg_table_73_t;



typedef struct
{//Used in mfg_table_74.Defines the ripple log structure
   _list_status_bfld_t  event_flags;//See LIST_STATUS_BFLD above.
   unsigned short       nbr_valid_entries;//Number of valid entries in the log.
   unsigned short       last_entry_element;//The array element number of the newest valid entry in the log.
   unsigned long        last_entry_seq_nbr;//The sequence number of the newest valid entry in the log.
   unsigned short       nbr_unread_entries;//The number of entries in the log that have not yet been read. It is only changed through a procedure.
   _event_entry_rcd_t   entries[RIPPLE_NBR_EVENTS];
}_ripple_events_t;

//Ripple Event Log
typedef struct
{
   _list_status_bfld_t  event_flags;//See LIST_STATUS_BFLD above.
   unsigned short       nbr_valid_entries;//Number of valid entries in the log.
   unsigned short       last_entry_element;//The array element number of the newest valid entry in the log.
   unsigned long        last_entry_seq_nbr;//The sequence number of the newest valid entry in the log.
   unsigned short       nbr_unread_entries;//The number of entries in the log that have not yet been read. It is only changed through a procedure.
   _event_entry_rcd_t   entries[RIPPLE_NBR_EVENTS];
   //_ripple_events_t    ripple_eventlog;
}_mfg_table_74_t;


typedef struct
{//Ripple
   unsigned char     RippleConfigStatus;  //Status of Ripple Configuration
   unsigned char     LastKnownTelegramID[2][1];  // last recognised command
   unsigned short    LastKnownSignalStrength;  // last recognised command signal strength
   unsigned short    LastUnkSignalStrength;                // last unknown command signal strength
   unsigned char     LastUnkNumBits;      // so can see when acted on the telegram
   unsigned char     LastUnkReceivedTelegram[7];
   unsigned char     LastActionLC[RAPPLC_NUM_RELAY];//The last action that was performed on a relay [0]-LC 1, [1]- LC 2
   unsigned short    TeleCounts[2][16];
} _mfg_table_75_t;

typedef struct
{//Used in MT20
   unsigned long           num_relay_ops;//number of relay operations
   _r_app_lc_emer_log_t    mod_log_undr_freq;//log of last 5 under frequency events
   _r_app_lc_emer_log_t    mod_log_undr_volt;//log of last 5 under voltage events
   _r_app_lc_emer_log_t    mod_log_dmd;//log of last 5 demand limit events
}_part_of_MT27_t;

typedef struct
{
   _mfg_table_75_t         MfgTb75;
   _part_of_MT27_t         MT27Part[RAPPLC_NUM_RELAY];
   unsigned char           inst_boost_nv_flags;
   unsigned long           inst_boost_time_remain[RAPPLC_NUM_RELAY];
   unsigned long           failsafetimer[RAPPLC_NUM_RELAY];//failsafe timer in seconds
   unsigned char           demand_array[60];//not used yet, energy for demand calculation
}_mfg_table_20_t;


typedef struct
{
   unsigned char           ForceUpdate;//Write a value here to update the table
   unsigned char           ATRandomMinute;//A random number of minutes, generated when table57 random reset offset is written to
   unsigned char           ATState;//The current status of the AT command state machine (see ATComms.c)
   unsigned char           IsFastATActive;//Returns TRUE if the fast AT mode is still enabled
   unsigned char           FastATSamplingStatus;//Returns TRUE when the AT module is in fast AT mode
   unsigned char           PressedButtons; // bit map of buttons -  Button4 =bit 0 ,Button3 =bit 1, Button2 =bit 2, Button1 =bit 3, SpareButton1 =bit 4,SpareButton2 =bit 5,SpareButton3 =bit 6, SpareButton4 =bit 7
   unsigned char           LongPressedButtons; // bit map of buttons -  Button4 =bit 0 ,Button3 =bit 1, Button2 =bit 2, Button1 =bit 3, SpareButton1 =bit 4,SpareButton2 =bit 5,SpareButton3 =bit 6, SpareButton4 =bit 7
   unsigned char           DisplayFields[12];//Same as the display values
   unsigned char           ProfileValues[4];//Same as the profile values
   unsigned char           DivideByZero;//Test divide by zero
   unsigned char           DisplayBuffer[22];//13 used normally, 22 used in U3400
   unsigned char           ClearModemReset;//Write to clear the stored last reset date/time
}_mfg_table_94_t;

typedef struct
{
  unsigned char  Type;	// Quantity for pulse output (1-32 EnergyReg), 101 = EOI, 102 = TOU RATE, 103 = Threshold Ctrl Under Freq, 104 = Threshold Ctrl Under Volt, 105 = Threshold Ctrl Above Demand
  unsigned	short PulseWidth;   // pulse width in 20mSecond units (0 = 50% duty cycle), EOI & RateA_D are set to 50 units
  union
  {
    unsigned char  PulseDiv;	// pulse divider, if 0 then not used
    unsigned char  TouRate;    //Rate A,B,C,D,E,F
    unsigned char  EOI;        //END OF INTERVAL 1,2,3,4,5,6, to 60
  }pulseType;   
  unsigned long  Threshold;
  unsigned short MinEventDuration;
  unsigned char  RelayOperation;
  unsigned short MinDelay;
  unsigned short MaxDelay; 
}_PulseOutputCfg;

typedef struct
{
   _PulseOutputCfg Cfg[4];
}_mfg_table_81_t;

typedef struct
{
  unsigned char      pQoS_Enable;   //0- disable profiling, 1 - enable profiling
  unsigned char      pQoS_Overlap;  //0- disable overlap,   1- enable overlap
  unsigned char      pQoS_Intervals;   //Total number of intervals. Hard Coded to 100
  unsigned char      pQoS_IntLength;   //Interval length. 5min,15min,30min,,60min
  unsigned char      pQoS_Quant[3][3]; //quantities to be profiled. Valid range 24- 50
}_mfg_table_95_t;

//Used in mfg table 83
typedef struct
{
  unsigned long     pQoS_TimeStamp; //GMT time in posix
  unsigned short    pQoS_Element[3][3]; //QoS data [element][value];
}_pQoS_data_t;

//LP QoS profile table
typedef struct
{
  _pQoS_data_t  pQoS_Data[96]; //96 entries
}_mfg_table_96_t;
/*
typedef struct
{//Demand Debug Table. Used for Testing purposes only
    unsigned long dmd_register[ACT_REGS_TBL_DOT_NBR_DEMANDS][6];//array of last 6 demand values
    _stime_date_t MaxDmdTime[2];//time of last max demand value
    ni_fmat1_t CumDmd[2]; //thats right cum
    ni_fmat2_t MaxDmd[2];//maximum demand value that has been calculated so far
}_mfg_table_78_t;
*/
//---------------------------TableConfig defintion-----------------------------------
typedef enum {eRAM, eExtFlash, eEmulEEPROM, eDynamic, eConst } eMemoryTypes;
//eRAM         lives in RAM can be read and written frequently
//eEEPROM      lives in external EEPROM can be read and written semi-frequently
//eExtFlash    lives in External Flash can be read and written infrequently
//eIntFlash    lives in CPU Flash can be read and written infrequently
//eEmulEEPROM  lives in External Flash with RAM buffer can be read and written frequently
//eConst       lives in CPU Flash can be read only
//eDynamic     lives in RAM/CPU Flash can be read only

typedef struct
{
     unsigned short Num;
     unsigned long Size;
     eMemoryTypes  Type;   // eg RAM, EEPROM, EXTERNAL FLASH, INTERNAL FLASH
     void HUGE* Address;
     void (*pFunc)();      // pointer to a functoin that is called once a table
                           // is written. We should be careful with stack usage
                           // and recursiveness here. hence TableWriteStopRecursive
} _TableInfoStruct;

extern const _TableInfoStruct TableConfig[];

//---------------------------------Instances-----------------------------------------

//---------------------------------Functions-----------------------------------------
extern unsigned short TableRead(unsigned short TableNum, unsigned long Offset, unsigned short Size, void *Buffer);
extern unsigned short TableWrite(unsigned short TableNum, unsigned long Offset, unsigned short Size, void *Buffer);
extern unsigned long TableCheckSize(unsigned short TableNum,unsigned char TableEntryNum, unsigned long Offset, unsigned short Size);
extern unsigned short TableSizeOK(unsigned short TableNum, unsigned long Offset, unsigned short Size);
extern unsigned long CheckStd62_Size(unsigned long Offset, unsigned short Size);
extern unsigned char TableFindEntry(unsigned short TableNum);
extern void TableInit();
extern void TableBackUp();
extern void UpdateHashCheck();
extern unsigned char TableHasHash(unsigned short TableNum);
extern unsigned short CalcTableHash(unsigned short TableNum,unsigned long *crc32);
extern unsigned char TableProcExecute(unsigned char PortSrc, unsigned char UserLvl,unsigned short ProcCmd, unsigned char SeqNo, unsigned char *pData);
extern void FwDownloadFixSeq();
extern void FwDownloadFixBootPartNum();
extern unsigned long GetMfg54Addr();
extern unsigned short CalcTableHashPartial(unsigned short TableNum,unsigned long *crc32,unsigned short offset, unsigned short length);
extern void ChkPendingTblWriteUpdate(void);
/* ------------UOM ID_CODE used in table 12-----------
0     Active power:
1     Reactive power:
2     Apparent power:
3     Phasor power:
4     Quantity power:
5     Quantity power:
6     Reserved
7     Reserved
8     RMS volts:
9     Average volts:
10    RMS volts squared:
11    Instantaneous volts:
12    RMS Current:
13    Average Current:
14    RMS Current Squared:
15    Instantaneous current:
16    Voltage T.H.D. as per IEEE standard:
17    Current T.H.D. as per IEEE standard:
18    Voltage T.H.D. as per Industry Canada requirements:
19    Current T.H.D. as per Industry Canada requirements:
50    Event Codes-Power outages
51    Event Codes-Number of demand resets
52    Event Codes-Number of times programmed
53    Event Codes-Number of minutes on battery carryover
20    V-VA, Voltage phase angle
21    Phase Angle Vx-VY where x and y are phases defined in phase selector.
22    Current phase angle (I-VA) relative to voltage phase A.
23    Phase Angle (Ix-IY), where x and y are phases defined in phase selector.
24    Power factor, , computed using apparent power, ID_CODE=2.
25    Power factor, , computed using phasor power, ID_CODE=3.
26    Reserved
27    Reserved
28    Reserved
29    Time of day
30    Date
31    Time of day and Date
32    Interval timer
33    Frequency
34    Counter
35    Sense input (T/F)
36..39 Reserved
40    Voltage sag
41    Voltage swells
42    Power outage
43    Voltage excursion low
44    Voltage excursion high
45    Reserved
46    Voltage imbalance
47    Voltage T.H.D. excess
48    Current T.H.D. excess
49..63 Reserved
----------Gas Industry Units-----------------
64    Cubic meter gas (volume un-corrected,meter index reading) per hour.
65    Corrected cubic meter gas (volume corrected to base conditions) per hour.
66    Corrected cubic meter gas (volume corrected to pressure base, without supercompressibility) per hour.
67    Cubic feet gas (volume corrected, meter index reading) per hour.
68    Corrected cubic feet gas (volume corrected to base conditions) per hour.
69    Corrected cubic feet gas (volume corrected to pressure base, without supercompressibility) per hour.
70    Dry bulb temperature .
71    Wet bulb temperature .
72    Dry bulb temperature .
73    Wet bulb temperature .
74    Dry bulb temperature .
75    Wet bulb temperature .
76    Joules per hour
77    Therm per hour
78    Static pascal
79    Differential pascal
80    Static pound per square inch
81    Differential pound per square inch
82    Gram cm2
83    Meter Hg column
84    Inch Hg column
85    Inch H2O column
86    Bar
87    % relative humidity
88    Parts per million odorant
89..127 Reserved
-----------Water Industry Units----------
128   Cubic meter liquid per hour
129   Cubic feet liquid per hour
130   US gallons per hour
131   IMP gallons per hour
132   Acre feet per hour
133   Parts per million lead
134   Turbidity
135   Parts per million chlorine
136   PH factor
137   Corrosion
138   Ionization
139   Parts per million SO2
140   Litres
141   Cubic feet liquid
142   Pounds per sq. ft differential
143   Inches of Water
144   Feet of water
145   Atmospheres
140..189 Reserved
------------Generic Units--------------
190   Local currency (e.g. $)
191   Inch
192   Foot
193   Meter
194..255 Reserved
*/


/*
----------------------------ANSI Standard Event Codes------------------------------------
Event Description                         Argument Type  Argument Description
00    No Event                            NA             None
01    Primary Power Down                  NA             None
02    Primary Power Up                    NA             None
03    Time Changed (old time)             NA             None, time tag if used equals old time.
04    Time Changed (new time)             NA             None, time tag if used equals new time.
05    Time Changed (old time)             STIME_DATE     Old time
06    Time Changed (new time)             STIME_DATE     New time
07    End Device Accessed for Read        NA             None
08    End Device Accessed for Write       NA             None
09    Procedure Invoked                   TABLE_IDB_BFLD Procedure Number
10    Table Written To                    TABLE_IDC_BFLD Table Number
11    End Device Programmed               NA             None
12    Communication Terminated Normally   NA             None
13    Communication Terminated Abnormally NA             None
14    Reset List Pointers                 INT8           See “Procedure 4 - Reset List Pointers” on page 4-35
15    Updated List Pointers               UINT8          See “Procedure 5 - Update Last Read Entry” on page 4-35.
16    History Log Cleared                 NA             None
17    History Log Pointers Updated        UINT16         Number of entries advanced. See “Procedure 5 - Update Last Read Entry” on page 4-35
18    Event Log Cleared                   NA             None
19    Event Log Pointers Updated          UINT16         Number of entries advanced. See “Procedure 5 - Update Last Read Entry” on page 4-35
20    Demand Reset Occurred               NA             None
21    Self Read Occurred                  NA             None
22    Daylight Savings Time On            NA             None
23    Daylight Savings Time Off           NA             None
24    Season Changed                      UINT8          New Season Number. See “Procedure 9 - Remote Reset” on page 4- 39, “Table 54 - Calendar” on page 9-12 and “Table 55 - Clock State” on page 9-19.
25    Rate Change                         UINT8          New Ratea
26    Special Schedule Activated          UINT8          See “Table 54 - Calendar” on page 9-12.
27    Tier Switch Change                  UINT8[2]       New Current Tier followed by New Demand Tierb, See “Table 54 - Calendar” on page 9-12 and “Table 23 - Current Register Data” on page 6- 11.
28    Pending Table Activation            TABLE_IDA_BFLD Table number activated
29    Pending Table Clear                 TABLE_IDA_BFLD Table number removed from end device prior to activation.
30    Metering mode started               NA             None
31    Metering mode stopped               NA             None
32    Test mode started                   NA             None
33    Test mode stopped                   NA             None
34    Meter shop mode started             NA             None
35    Meter shop mode stopped             NA             None
36    Meter reprogrammed                  NA             None
37    Configuration error detected        NA             None
38    Self check error detected           NA             None
39    RAM failure detected                NA             None
40    ROM failure detected                NA             None
41    Nonvolatile memory failure detected NA             None
42    Clock error detected                NA             None
43    Measurement error detected          NA             None
44    Low battery detected                NA             None
45    Low loss potential detected         NA             None
46    Demand overload detected            NA             None
47    Tamper attempt detected             NA             None
48    Reverse rotation detected           NA             None
*/

/*----------------------------ANSI Manufacturing (L+G defined) Event Codes------------------------------------
Event Description                         Argument Type  Argument Description
01
02
03
04
05
06
07
08
09
10
11
12
13
14
15
*/

#if defined(SIMULATION_SR)
   #ifdef __GNUC__ //generate packed structs in GCC
      #pragma pack(pop)
   #endif
#endif

#endif

