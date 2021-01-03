// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RFM130_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RFM130_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef RFM130_EXPORTS
#define RFM130_API __declspec(dllexport)
#else
#define RFM130_API __declspec(dllimport)
#endif
#include <iostream>
using namespace std;

#define BEEP_CMD_LEN 6
#define BEEP_STU_LEN 5

#define REQ_CMD_LEN 6
#define REQ_STU_LEN 6 // as documentation it should be 7 but ... . 

#define ANTI_CMD_LEN 7
#define ANTI_STU_LEN 9

#define SEL_CMD_LEN 11
#define SEL_STU_LEN 6

#define LOD_CMD_LEN 11
#define LOD_STU_LEN 5

#define AUT_CMD_LEN 11
#define AUT_STU_LEN 5

#define READ_CMD_LEN 7
#define READ_STU_LEN 0 //5+blockQty*16

#define WRITE_STU_LEN 5

// ### ERR ###
#define NORMAL 0X00
#define NO_CARD 0X01
#define ANTICOLL_ERR 0x02
#define BIT_COUNTER_ERR 0x03
#define RETURN_DATA_ERR 0x04
#define AUTHENTICATION_ERR 0x05
#define VALUE_OPERATION_ERR 0x0d
#define CARD_OPERATION_ERR 0x0e
#define CARD_OPERATION_OVERTIME 0x0f
#define COMMAND_OR_PARAMETER_ERR 0x10
#define OTHER_ERR 0x11
#define PORT_OPEN_ERR 0X12

// -=^-=^-=^-=^-=^ body definition -=^-=^-=^-=^-=^
#ifdef __cplusplus
    extern "C" {
#endif

// global port handle
HANDLE port;
 
RFM130_API unsigned int _openPort(int, unsigned long);
RFM130_API unsigned int _closePort(void);
RFM130_API unsigned int _rfbeep(unsigned int);
RFM130_API unsigned int _requestAll (unsigned char&);
RFM130_API unsigned int _anticoll (unsigned char*);
RFM130_API unsigned int _select (const unsigned char*, unsigned char&);
RFM130_API unsigned int _loadKey (unsigned char*);
RFM130_API unsigned int _authenticate (int, int, const unsigned char*);
RFM130_API unsigned int _read (int, int, unsigned char*);
RFM130_API unsigned int _write (int, int, unsigned char*);
RFM130_API int read_card_id (void);
 
#ifdef __cplusplus
    }
#endif