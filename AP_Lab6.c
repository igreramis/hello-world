// AP_Lab6.c
// Runs on either MSP432 or TM4C123
// see GPIO.c file for hardware connections 

// Daniel Valvano and Jonathan Valvano
// November 20, 2016
// CC2650 booster or CC2650 LaunchPad, CC2650 needs to be running SimpleNP 2.2 (POWERSAVE)

/* THESE FUCKERS ARE USING A TEMPLATE ON THE ROM FOR ALL MESSAGE BUILDING.
   SEE HOW THAT WORKS. THINK AP.C AND APPLICATIONPROCESSOR ARE CLUE TO THAT */

#include <stdint.h>
#include "../inc/UART0.h"
#include "../inc/UART1.h"
#include "../inc/AP.h"
#include "AP_Lab6.h"
//**debug macros**APDEBUG defined in AP.h********
#ifdef APDEBUG
#define OutString(STRING) UART0_OutString(STRING)
#define OutUHex(NUM) UART0_OutUHex(NUM)
#define OutUHex2(NUM) UART0_OutUHex2(NUM)
#define OutChar(N) UART0_OutChar(N)
#else
#define OutString(STRING)
#define OutUHex(NUM)
#define OutUHex2(NUM)
#define OutChar(N)
#endif

//****links into AP.c**************
extern const uint32_t RECVSIZE;
extern uint8_t RecvBuf[];
typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
extern const uint32_t MAXCHARACTERISTICS;
extern uint32_t CharacteristicCount;
extern characteristic_t CharacteristicList[];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined 
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];

#define PAYLOAD_L     2
#define COMMAND_L     2

void wifi_tx(char *buffer)
{
  broadcom_tx(buffer);
}

void wifi_rx(char *buffer)
{
  broadcom_rx(buffer);
}

uint8_t
utrtb_strlen(const char *str)
{
        const char *s;

        for (s = str; *s; ++s)
                ;
        return (s - str);
}

void
send_pkt(const char *dest)
{
  tx_pkt(dest);
}

void
recv_pkt(const char *src)
{
  rx_pkt(src);
}

void
ack_pkt(const char *dest)
{
  ack_pkt(dest);
}

void utrtb_memcpy_V(void *dest, void *src, uint8_t n)
{
   // Copy contents of src[] to dest[]
   for (int i=0; i<n; i++)
       cdest[j] = csrc[x];
}

void utrtb_memcpy_IV(void *dest, void *src, uint8_t n)
{
   // Copy contents of src[] to dest[]
   for (int i=0; i<n; i++)
       cdest[i] = csrc[i];
}
void utrtb_memcpyIII(void *dest, void *src, uint8_t n)
{

  cdinofunctionmemverify();

}

void utrtb_memcpyII(void *dest, void *src, uint8_t n)
{

  cdinofunctionmemverify();

   // Copy contents of src[] to dest[]
   for (int i=0; i<n; i++)
       cdest[i] = csrc[i];
>>>>>>> f69f36eea2cd669c67f39de2770cb3375e6547ee
}

void utrtb_memcpy(void *dest, void *src, uint8_t n)
{
   // Typecast src and dest addresses to (char *)
   char *csrc = (char *)src;
   char *cdest = (char *)dest;

	cdinofunctionmemverify();

   // Copy contents of src[] to dest[]
   for (int i=0; i<n; i++)
       cdest[i] = csrc[i];
}

void connect_to_bluetooth(void *host)
{
	connect_to_host(host);
}

void setup_bluetooth_connection(void *host)
{
	establish_data_connnection(host);
}

const uint8_t NPI_SetAdvertisementMsg[] = {   
SOF,11,0x00,    // length = 11
0x55,0x43,      // SNP Set Advertisement Data
0x01,           // Not connected Advertisement Data
0x02,0x01,0x06, // GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
0x06,0xFF,      // length, manufacturer specific
0x0D ,0x00,     // Texas Instruments Company ID
0x03,           // TI_ST_DEVICE_ID
0x00,           // TI_ST_KEY_DATA_ID
0x00,           // Key state
0xEE};          // FCS (calculated by AP_SendMessageResponse)

const uint8_t NPI_StartAdvertisementMsg[] = {   
  SOF,14,0x00,    // length = 14
  0x55,0x42,      // SNP Start Advertisement
  0x00,           // Connectable Undirected Advertisements
  0x00,0x00,      // Advertise infinitely.
  0x64,0x00,      // Advertising Interval (100 * 0.625 ms=62.5ms)
  0x00,           // Filter Policy RFU
  0x00,           // Initiator Address Type RFU
  0x00,0x01,0x00,0x00,0x00,0xC5, // RFU
  0x02,           // Advertising will restart with connectable advertising when a connection is terminated
0xBB};          // FCS (calculated by AP_SendMessageResponse)
//**************Lab 6 routines*******************
// **********SetFCS**************
// helper function, add check byte to message
// assumes every byte in the message has been set except the FCS
// used the length field, assumes less than 256 bytes
// FCS = 8-bit EOR(all bytes except SOF and the FCS itself)
// Inputs: pointer to message
//         stores the FCS into message itself
// Outputs: none
void SetFCS(uint8_t *msg){
//****You implement this function as part of Lab 6*****
/* It is assumed that the message has SCF */
/* It is assumed that the lenght indicates # of bytes uptill FCS */
  uint8_t fcs = 0;
  uint16_t itr = 0;
  /* This would give the value of L in Fig 6.29 */
  uint32_t size = AP_GetSize(msg);

  /* FCS is the xor of L bytes obtained above as well as 4 bytes of Length and Command as in Fig 6.29 */
  /* Skip the first byte and start from the rest */
  for (itr=1;itr<size+(PAYLOAD_L+COMMAND_L+1);itr++){
    fcs = fcs^msg[itr];
  }

  /* Add FCS at end of msg */
  msg[itr]=fcs;
  
}
//*************BuildGetStatusMsg**************
// Create a Get Status message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetStatusMsg(uint8_t *msg){
// hint: see NPI_GetStatus in AP.c
//****You implement this function as part of Lab 6*****
  //const uint8_t NPI_GetStatus[] =   {SOF,0x00,0x00,0x55,0x06,0x53};
  msg[0]=SOF;
  msg[1]=msg[2]=0x00;
  msg[3]=0x55;
  msg[4]=0x06;
  SetFCS(msg);

  
}
//*************Lab6_GetStatus**************
// Get status of connection, used in Lab 6
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress
uint32_t Lab6_GetStatus(void){volatile int r; uint8_t sendMsg[8];
  OutString("\n\rGet Status");
  BuildGetStatusMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return (RecvBuf[4]<<24)+(RecvBuf[5]<<16)+(RecvBuf[6]<<8)+(RecvBuf[7]);
}

//*************BuildGetVersionMsg**************
// Create a Get Version message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetVersionMsg(uint8_t *msg){
// hint: see NPI_GetVersion in AP.c
//****You implement this function as part of Lab 6*****
  //const uint8_t NPI_GetVersion[] =  {SOF,0x00,0x00,0x35,0x03,0x36};
  msg[0]=SOF;
  msg[1]=msg[2]=0x00;
  msg[3]=0x35;
  msg[4]=0x03;
  SetFCS(msg);
  
}
//*************Lab6_GetVersion**************
// Get version of the SNP application running on the CC2650, used in Lab 6
// Input:  none
// Output: version
uint32_t Lab6_GetVersion(void){volatile int r;uint8_t sendMsg[8];
  OutString("\n\rGet Version");
  BuildGetVersionMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE); 
  return (RecvBuf[5]<<8)+(RecvBuf[6]);
}

//*************BuildAddServiceMsg**************
// Create an Add service message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        pointer to empty buffer of at least 9 bytes
// Output none
// build the necessary NPI message that will add a service
void BuildAddServiceMsg(uint16_t uuid, uint8_t *msg){
//****You implement this function as part of Lab 6*****
  /* Follow the following pattern */
  // const uint8_t NPI_AddServiceMsg[] = {
  // SOF,3,0x00,     // length = 3
  // 0x35,0x81,      // SNP Add Service
  // 0x01,           // Primary Service
  // 0xF0,0xFF,      // UUID
  // 0xB9};          // FCS (calculated by AP_SendMessageResponse)
msg[0]=SOF;
msg[1]=3;              /*Length*/
msg[2]=0;              /*Length*/
msg[3]=0x35;           /*Command*/
msg[4]=0x81;           /*Command*/
msg[5]=0x01;           /*Payload*/
msg[6]=(uuid & 0xFF);  /*Payload*/
msg[7]=(uuid>>8);      /*Payload*/
SetFCS(msg);           /*FCS*/
  
}
//*************Lab6_AddService**************
// Add a service, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_AddService(uint16_t uuid){ int r; uint8_t sendMsg[12];
  OutString("\n\rAdd service");
  BuildAddServiceMsg(uuid,sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);  
  return r;
}
//*************AP_BuildRegisterServiceMsg**************
// Create a register service message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will register a service
void BuildRegisterServiceMsg(uint8_t *msg){
//****You implement this function as part of Lab 6*****
  /*Register msg template*/
  // const uint8_t NPI_Register[] = {   
  // SOF,0x00,0x00,  // length = 0
  // 0x35,0x84,      // SNP Register Service
  // 0x00};          // FCS (calculated by AP_SendMessageResponse)
  msg[0]=SOF;
  msg[1]=0x00;
  msg[2]=0x00;
  msg[3]=0x35;
  msg[4]=0x84;
  SetFCS(msg);
}
//*************Lab6_RegisterService**************
// Register a service, used in Lab 6
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_RegisterService(void){ int r; uint8_t sendMsg[8];
  OutString("\n\rRegister service");
  BuildRegisterServiceMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

//-------------
//*************BuildAddCharValueMsg**************
// Create a Add Characteristic Value Declaration message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write, 0x10=notify
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a characteristic value
void BuildAddCharValueMsg(uint16_t uuid,  
  uint8_t permission, uint8_t properties, uint8_t *msg){
// set RFU to 0 and
// set the maximum length of the attribute value=512
// for a hint see NPI_AddCharValue in AP.c
// for a hint see first half of AP_AddCharacteristic and first half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
  // uint8_t NPI_AddCharValue[] = {   
  // SOF,0x08,0x00,  // length = 8
  // 0x35,0x82,      // SNP Add Characteristic Value Declaration
  // 0x03,           // 0=none,1=read,2=write, 3=Read+write, GATT Permission
  // 0x0A,0x00,      // 2=read,8=write,0x0A=read+write,0x10=notify, GATT Properties
  // 0x00,           // RFU
  // 0x00,0x02,      // Maximum length of the attribute value=512
  // 0xF1,0xFF,      // UUID
  // 0xBA};          // FCS (calculated by AP_SendMessageResponse)

  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
  msg[0]=SOF;
  msg[1]=0x08;
  msg[2]=0x00;
  msg[3]=0x35;
  msg[4]=0x82;
  msg[5]=permission;
  msg[6]=properties; /*properties*/
  msg[7]=0x00;       /*properties*/
  msg[8]=0x00;       /*RFU=0*/
  msg[9]=0x00;
  msg[10]=0x02;
  msg[11]=(uuid & 0xFF);
  msg[12]=(uuid >> 8);
  SetFCS(msg);
}

//*************BuildAddCharDescriptorMsg**************
// Create a Add Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 32 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// for a hint see NPI_AddCharDescriptor in AP.c
// for a hint see second half of AP_AddCharacteristic
//****You implement this function as part of Lab 6*****
  // uint8_t NPI_AddCharDescriptor[] = {   
  // SOF,0x17,0x00,  // length determined at run time 6+string length
  // 0x35,0x83,      // SNP Add Characteristic Descriptor Declaration
  // 0x80,           // User Description String
  // 0x01,           // GATT Read Permissions
  // 0x11,0x00,      // Maximum Possible length of the user description string
  // 0x11,0x00,      // Initial length of the user description string
  // 'C','h','a','r','a','c','t','e','r','i','s','t','i','c',' ','0',0, // Initial user description string
  // 0x0C,0,0,0};    // FCS (calculated by AP_SendMessageResponse)
  uint8_t l_name, l_msg;
  l_name=utrtb_strlen(name)+1;
  l_msg = l_name + 6;

  msg[0]=SOF;
  msg[1]=l_msg; msg[2]=0x00;/*message length*/
  msg[3]=0x35; msg[4]=0x83; /*SNP add characteristic descriptor declaration*/
  msg[5]=0x80; /*user descriptor string*/
  msg[6]=0x01; /*GATT read permissions*/
  msg[7]=l_name; msg[8]=0x00; /*max length */
  msg[9]=l_name; msg[10]=0x00;/*initial length */
  utrtb_memcpy(msg+11, name, l_name); msg[11+l_name]=0;/*name*/
  SetFCS(msg);
}

//*************Lab6_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used in Lab 6
//        for notify properties, call AP_AddNotifyCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*ReadFunc) called before it responses with data from internal structure
//        (*WriteFunc) called after it accepts data into internal structure
// Output APOK if successful,
//        APFAIL if name is empty, more than 8 characteristics, or if SNP failure
int Lab6_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission,
  uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[32];  
  if(thesize>8) return APFAIL;
  if(name[0]==0) return APFAIL;       // empty name
  if(CharacteristicCount>=MAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,permission,properties,sendMsg);
  OutString("\n\rAdd CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  CharacteristicList[CharacteristicCount].theHandle = handle;
  CharacteristicList[CharacteristicCount].size = thesize;
  CharacteristicList[CharacteristicCount].pt = (uint8_t *) pt;
  CharacteristicList[CharacteristicCount].callBackRead = ReadFunc;
  CharacteristicList[CharacteristicCount].callBackWrite = WriteFunc;
  CharacteristicCount++;
  return APOK; // OK
} 
  

//*************BuildAddNotifyCharDescriptorMsg**************
// Create a Add Notify Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddNotifyCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// set User Description String
// set CCCD parameters read+write
// for a hint see NPI_AddCharDescriptor4 in VerySimpleApplicationProcessor.c
// for a hint see second half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
  // const uint8_t NPI_AddCharDescriptor4[] = {   
  // SOF,12,0x00,    // length = 12
  // 0x35,0x83,      // SNP Add Characteristic Descriptor Declaration
  // 0x84,           // User Description String+CCCD
  // 0x03,           // CCCD parameters read+write
  // 0x01,           // GATT Read Permissions
  // 0x06,0x00,      // Maximum Possible length of the user description string
  // 0x06,0x00,      // Initial length of the user description string
  // 'C','o','u','n','t',0, // Initial user description string
  // 0x0E};          // FCS (calculated by AP_SendMessageResponse)
  uint8_t l_name, l_msg;
  l_name=utrtb_strlen(name)+1;
  l_msg = l_name + 7;

  msg[0]=SOF;
  msg[1]=l_msg; msg[2]=0x00;/*message length*/
  msg[3]=0x35; msg[4]=0x83; /*SNP add characteristic descriptor declaration*/
  msg[5]=0x84; /*user descriptor string*/
  msg[6]=0x03; /*CCCD parameters read+write*/
  msg[7]=0x01; /*GATT Read Permissions */
  msg[8]=l_name; msg[9]=0x00; /*max length */
  msg[10]=l_name; msg[11]=0x00;/*initial length */
  utrtb_memcpy(msg+12, name, l_name); msg[12+l_name]=0;/*name*/
  SetFCS(msg);
}
  
//*************Lab6_AddNotifyCharacteristic**************
// Add a notify characteristic, used in Lab 6
//        for read, write, or read/write characteristic, call AP_AddCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*CCCDfunc) called after it accepts , changing CCCDvalue
// Output APOK if successful,
//        APFAIL if name is empty, more than 4 notify characteristics, or if SNP failure
int Lab6_AddNotifyCharacteristic(uint16_t uuid, uint16_t thesize, void *pt,   
  char name[], void(*CCCDfunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[36];  
  if(thesize>8) return APFAIL;
  if(NotifyCharacteristicCount>=NOTIFYMAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,0,0x10,sendMsg);
  OutString("\n\rAdd Notify CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddNotifyCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  NotifyCharacteristicList[NotifyCharacteristicCount].uuid = uuid;
  NotifyCharacteristicList[NotifyCharacteristicCount].theHandle = handle;
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDhandle = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this CCCD
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDvalue = 0; // notify initially off
  NotifyCharacteristicList[NotifyCharacteristicCount].size = thesize;
  NotifyCharacteristicList[NotifyCharacteristicCount].pt = (uint8_t *) pt;
  NotifyCharacteristicList[NotifyCharacteristicCount].callBackCCCD = CCCDfunc;
  NotifyCharacteristicCount++;
  return APOK; // OK

  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];

//*************BuildSetDeviceNameMsg**************
// Create a Set GATT Parameter message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message to set Device name
void BuildSetDeviceNameMsg(char name[], uint8_t *msg){
// for a hint see NPI_GATTSetDeviceNameMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_GATTSetDeviceName in AP.c
//****You implement this function as part of Lab 6*****
  // const uint8_t NPI_GATTSetDeviceNameMsg[] = {   
  // SOF,18,0x00,    // length = 18
  // 0x35,0x8C,      // SNP Set GATT Parameter (0x8C)
  // 0x01,           // Generic Access Service
  // 0x00,0x00,      // Device Name
  // 'S','h','a','p','e',' ','t','h','e',' ','W','o','r','l','d',
  // 0x77}; //FCS
  
  uint8_t l_name, l_msg;
  l_name=utrtb_strlen(name);
  l_msg = l_name + 3; /*examples referred above seem to add 3 to orig length*/

  msg[0]=SOF;
  msg[1]=l_msg; msg[2]=0x00;/*message length<-???*/
  msg[3]=0x35; msg[4]=0x8C; /*SNP Set GATT parameter*/
  msg[5]=0x01; /*Generic Access Service*/
  msg[6]=0x00; /*Device Name*/
  msg[7]=0x00; /*Device Name*/
  utrtb_memcpy(msg+8, name, l_name);/*name*/
  SetFCS(msg);
}
//*************BuildSetAdvertisementData1Msg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs pointer to empty buffer of at least 16 bytes
// Output none
// build the necessary NPI message for Non-connectable Advertisement Data
void BuildSetAdvertisementData1Msg(uint8_t *msg){
// for a hint see NPI_SetAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisement1 in AP.c
// Non-connectable Advertisement Data
// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR  
// Texas Instruments Company ID 0x000D
// TI_ST_DEVICE_ID = 3
// TI_ST_KEY_DATA_ID
// Key state=0
//****You implement this function as part of Lab 6*****
 utrtb_memcpy(msg, (void *)NPI_SetAdvertisementMsg, sizeof(NPI_SetAdvertisementMsg)/sizeof(uint8_t));

}

//*************BuildSetAdvertisementDataMsg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message for Scan Response Data
void BuildSetAdvertisementDataMsg(char name[], uint8_t *msg){
// for a hint see NPI_SetAdvertisementDataMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisementData in AP.c
//****You implement this function as part of Lab 6*****
//   const uint8_t NPI_SetAdvertisementDataMsg[] = {   
//   SOF,27,0x00,    // length = 27
//   0x55,0x43,      // SNP Set Advertisement Data
//   0x00,           // Scan Response Data
//   16,0x09,        // length, type=LOCAL_NAME_COMPLETE
//   'S','h','a','p','e',' ','t','h','e',' ','W','o','r','l','d',
// // connection interval range
//   0x05,           // length of this data
//   0x12,           // GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE
//   0x50,0x00,      // DEFAULT_DESIRED_MIN_CONN_INTERVAL
//   0x20,0x03,      // DEFAULT_DESIRED_MAX_CONN_INTERVAL
// // Tx power level
//   0x02,           // length of this data
//   0x0A,           // GAP_ADTYPE_POWER_LEVEL
//   0x00,           // 0dBm
//   0x77};          // FCS (calculated by AP_SendMessageResponse)

  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
  uint8_t l_name, l_msg;
  l_name=utrtb_strlen(name);
  l_msg = l_name + 12; /*examples referred above seem to add 12 to orig length*/

  msg[0]=SOF;
  msg[1]=l_msg; /*length*/
  msg[2]=0x00;   /*length*/
  msg[3]=0x55; /*SNP Set Advertisement Data*/
  msg[4]=0x43; /*SNP Set Advertisement Data*/
  msg[5]=0x00; /*Scan Response Data*/
  msg[6]=l_name+1; /* length */
  msg[7]=0x09; /*type=LOCAL_NAME_COMPLETE*/
  utrtb_memcpy(msg+8, name, l_name);
  msg[8+l_name]=0x05; /*lenght of this data<-WHAT FUCK DATA?*/
  msg[9 + l_name]=0x12; /*GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE*/
  msg[10 + l_name]=0x50; /*DEFAULT_DESIRED_MIN_CONN_INTERVAL*/
  msg[11 + l_name]=0x00; /*DEFAULT_DESIRED_MIN_CONN_INTERVAL*/
  msg[12 + l_name]=0x20; /*DEFAULT_DESIRED_MAX_CONN_INTERVAL*/
  msg[13 + l_name]=0x03; /*DEFAULT_DESIRED_MAX_CONN_INTERVAL*/
  msg[14 + l_name]=0x02; /*length of this data*/
  msg[15 + l_name]=0x0A; /*GAP_ADTYPE_POWER_LEVEL*/
  msg[16 + l_name]=0x00; /*0dBm*/
  SetFCS(msg);
}
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
// for a hint see NPI_StartAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_StartAdvertisement in AP.c
//****You implement this function as part of Lab 6*****
  // const uint8_t NPI_StartAdvertisementMsg[] = {   
  // SOF,14,0x00,    // length = 14
  // 0x55,0x42,      // SNP Start Advertisement
  // 0x00,           // Connectable Undirected Advertisements
  // 0x00,0x00,      // Advertise infinitely.
  // 0x64,0x00,      // Advertising Interval (100 * 0.625 ms=62.5ms)
  // 0x00,           // Filter Policy RFU
  // 0x00,           // Initiator Address Type RFU
  // 0x00,0x01,0x00,0x00,0x00,0xC5, // RFU
  // 0x02,           // Advertising will restart with connectable advertising when a connection is terminated
  // 0xBB};          // FCS (calculated by AP_SendMessageResponse)


  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
}
//*************BuildStartAdvertisementMsg**************
// Create a Start Advertisement Data message, used in Lab 6
// Inputs advertising interval
//        pointer to empty buffer of at least 20 bytes
// Output none
// build the necessary NPI message to start advertisement
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
// for a hint see NPI_StartAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_StartAdvertisement in AP.c
//****You implement this function as part of Lab 6*****
  // const uint8_t NPI_StartAdvertisementMsg[] = {   
  // SOF,14,0x00,    // length = 14
  // 0x55,0x42,      // SNP Start Advertisement
  // 0x00,           // Connectable Undirected Advertisements
  // 0x00,0x00,      // Advertise infinitely.
  // 0x64,0x00,      // Advertising Interval (100 * 0.625 ms=62.5ms)
  // 0x00,           // Filter Policy RFU
  // 0x00,           // Initiator Address Type RFU
  // 0x00,0x01,0x00,0x00,0x00,0xC5, // RFU
  // 0x02,           // Advertising will restart with connectable advertising when a connection is terminated
  // 0xBB};          // FCS (calculated by AP_SendMessageResponse)
  utrtb_memcpy(msg, (void *)NPI_StartAdvertisementMsg, sizeof(NPI_StartAdvertisementMsg)/sizeof(uint8_t));
  msg[8]=(interval & 0xFF);
  msg[9]=(interval >> 8);
  SetFCS(msg);

  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
}

void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
// for a hint see NPI_StartAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_StartAdvertisement in AP.c
//****You implement this function as part of Lab 6*****
  // const uint8_t NPI_StartAdvertisementMsg[] = {   
  // SOF,14,0x00,    // length = 14
  // 0x55,0x42,      // SNP Start Advertisement
  // 0x00,           // Connectable Undirected Advertisements
  // 0x00,0x00,      // Advertise infinitely.
  // 0x64,0x00,      // Advertising Interval (100 * 0.625 ms=62.5ms)
  // 0x00,           // Filter Policy RFU
  // 0x00,           // Initiator Address Type RFU
  // 0x00,0x01,0x00,0x00,0x00,0xC5, // RFU
  // 0x02,           // Advertising will restart with connectable advertising when a connection is terminated
  // 0xBB};          // FCS (calculated by AP_SendMessageResponse)
  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
}

int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  return r;
}

int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  OutString("\n\rSet Device name");
  return r;
}
int Lab6_StartAdvertisement(void kheyvoid);
//*************Lab6_StartAdvertisement**************
// Start advertisement, used in Lab 6
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  OutString("\n\rSet Device name");
  BuildSetDeviceNameMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("Shape the World",sendMsg);
  extern const uint32_t NOTIFYMAXCHARACTERISTICS;
   BuildSetDeviceNameMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("Shape the World",sendMsg);
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rStartAdvertisement");
  BuildStartAdvertisementMsg(100,sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

int sigma(void pi)
{
  return epsilon;
}

void pi(int sigma)
{
  return epsilon;
}