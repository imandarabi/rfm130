// rfm130.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "rfm130.h"



#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

#ifdef __cplusplus
    extern "C" {
#endif
 

RFM130_API unsigned int _openPort(int portNum, unsigned long boudrate)
{
	LPCWSTR portName; 

	switch (portNum){
		case 1:	portName = L"COM1"; break;
		case 2:	portName = L"COM2";	break;
		case 3:	portName = L"COM3";	break;
		case 4:	portName = L"COM4";	break;
		case 5:	portName = L"COM5";	break;
		case 6:	portName = L"COM6";	break;
		case 7:	portName = L"COM7";	break;
		case 8:	portName = L"COM8";	break;
		case 9:	portName = L"COM9";	break;
		case 10:portName = L"COM10";break;
		case 11:portName = L"COM11";break;
		case 12:portName = L"COM12";break;
		case 13:portName = L"COM13";break;
		case 14:portName = L"COM14";break;
		case 15:portName = L"COM15";break;
	}


	DCB          comSettings;          // Contains various port settings
    COMMTIMEOUTS CommTimeouts;
	int   bStatus;
	
	port = CreateFile( portName, 
						GENERIC_READ | GENERIC_WRITE, 
						0, 
						NULL, 
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	
	if (port == INVALID_HANDLE_VALUE)
		return PORT_OPEN_ERR;
		
	// Set timeouts in milliseconds
    CommTimeouts.ReadIntervalTimeout         = 0; 
    CommTimeouts.ReadTotalTimeoutMultiplier  = 0; 
    CommTimeouts.ReadTotalTimeoutConstant    = 100;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant   = 100;
	bStatus = SetCommTimeouts(port,&CommTimeouts);
    if (bStatus == 0)
		return PORT_OPEN_ERR;

    // Set Port parameters.
    // Make a call to GetCommState() first in order to fill
    // the comSettings structure with all the necessary values.
    // Then change the ones you want and call SetCommState().
	GetCommState(port, &comSettings);
    comSettings.BaudRate = boudrate;
	comSettings.StopBits = ONESTOPBIT;
    comSettings.ByteSize = 8;
    comSettings.Parity   = NOPARITY;
    comSettings.fParity  = FALSE;
	bStatus = SetCommState(port, &comSettings);
    
	if (bStatus == 0) return PORT_OPEN_ERR;
	
	return NORMAL;
}
RFM130_API unsigned int _closePort(void)
{	
	/*
		use of 1 ^ ... will return 0 as normal(true) and 1 as false,
		like other function returnes ... . 
	*/
	return (1 ^ CloseHandle(port) );
}
RFM130_API unsigned int _rfbeep(unsigned int soundLen)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = BEEP_CMD_LEN, dwToRead = BEEP_STU_LEN;
	unsigned char cmd[BEEP_CMD_LEN], stu[BEEP_STU_LEN];
	unsigned int status;
    unsigned int i=0, j;

	cmd[i++]=0x02;
    cmd[i++]=0x00;
    cmd[i++]=0x02;
    cmd[i++]=0x2c;
    cmd[i++]=soundLen;
    cmd[i] = 0;
	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];
	
	status = WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	status = ReadFile(port, stu, dwToRead, &dwRead,  NULL);
	if ( dwToRead != dwRead )
		return OTHER_ERR;

	return stu[3];
}
RFM130_API unsigned int _requestAll (unsigned char &card_type)
{            
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = REQ_CMD_LEN, dwToRead = REQ_STU_LEN;
	unsigned char cmd[REQ_CMD_LEN];
	unsigned char stu[REQ_STU_LEN];
	unsigned int i=0, j;
	
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x02;
	cmd[i++]=0x31;
	cmd[i++]=0x52;
	cmd[i] = 0;
	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];
	
	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	ReadFile(port, stu, dwToRead, &dwRead,  NULL);
	
	card_type = stu[4];
	return stu[3];
}
RFM130_API unsigned int _anticoll (unsigned char* serialNum)
{            
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = ANTI_CMD_LEN, dwToRead = ANTI_STU_LEN;
	unsigned char cmd[ANTI_CMD_LEN], stu[ANTI_STU_LEN];
	int i, j;

	i=0;
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x02;
	cmd[i++]=0x32;
	cmd[i++]=0x93;
	cmd[i] = 0;
	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];

	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	ReadFile(port, stu, dwToRead, &dwRead,  NULL);
	
	if (stu[3] == NORMAL)
		for(j=4;j<8;j++)
			serialNum[j-4]=stu[j];

	return stu[3];
}                 
RFM130_API unsigned int _select (const unsigned char * serialNum, unsigned char &sak)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = SEL_CMD_LEN, dwToRead = SEL_STU_LEN;
	unsigned char cmd[SEL_CMD_LEN], stu[SEL_STU_LEN];
	int i, j;

	i=0;
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x06;
	cmd[i++]=0x33;
	cmd[i++]=0x93;
	for(j=0;j<4;j++)
		cmd[i++]=serialNum[j];
	cmd[i] = 0;

	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];

	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	 
	ReadFile(port, stu, dwToRead, &dwRead,  NULL); 
	if (stu[3] == NORMAL)
		sak = stu[4];

	return stu[3];
}
RFM130_API unsigned int _loadKey (unsigned char * key)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = LOD_CMD_LEN, dwToRead = LOD_STU_LEN;
	unsigned char cmd[LOD_CMD_LEN], stu[LOD_STU_LEN];
	int i, j;

	i=0;
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x07;
	cmd[i++]=0x35;
	for(j=0;j<6;j++)
		cmd[i++]=key[j];
	cmd[i] = 0;

	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];
	
	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	ReadFile(port, stu, dwToRead, &dwRead,  NULL); 
	
	return stu[3];
}
RFM130_API unsigned int _authenticate (int blockNo, int mode, const unsigned char *serialNum)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = AUT_CMD_LEN, dwToRead = AUT_STU_LEN;
	unsigned char cmd[AUT_CMD_LEN], stu[AUT_STU_LEN];
	int i, j;

	i=0;
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x07;
	cmd[i++]=0x37;
	cmd[i++]=mode;
	cmd[i++]=blockNo;
	for(j=0;j<4;j++)
		cmd[i++]=serialNum[j];
	cmd[i] = 0;

	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];
		
	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	ReadFile(port, stu, dwToRead, &dwRead,  NULL); 

	return stu[3];
}
RFM130_API unsigned int _read (int blockNo, int blockQty, unsigned char *buffer)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = READ_CMD_LEN, dwToRead = 5+blockQty*16; //READ_STU_LEN;
	unsigned char cmd[READ_CMD_LEN];
	unsigned char readBuffer[5+16*4];
	int i, j, stat;

	if (blockQty>4)
		return 0x11;

	i=0;
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x03;
	cmd[i++]=0x38;
	cmd[i++]=blockNo;
	cmd[i++]=blockQty;
	cmd[i] = 0;

	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];

	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	ReadFile(port, readBuffer, dwToRead, &dwRead,  NULL);
	
	stat = readBuffer[3];
	
	memcpy (buffer, readBuffer+4, blockQty*16);

	return stat;
}
RFM130_API unsigned int _write (int blockNo, int blockQty, unsigned char *buffer)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten, dwRead;
	DWORD dwToWrite = 7+blockQty*16, dwToRead = WRITE_STU_LEN;
	unsigned char cmd[256];
	int i, j;

	i=0;
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x03+blockQty*16;
	cmd[i++]=0x39;
	cmd[i++]=blockNo;
	cmd[i++]=blockQty;
	for(j=0;j<blockQty*16;j++)
		cmd[i++]=buffer[j];
	cmd[i] = 0;

	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];

	WriteFile (port, cmd, dwToWrite, &dwWritten, &osWrite);
	
	ReadFile(port, buffer, dwToRead, &dwRead,  NULL);

	return buffer[3];
}
RFM130_API int read_card_id (void)
{
	unsigned char sr[4];
	unsigned char s, sak;
	unsigned char key[6] = {180, 250, 41, 197, 134, 30};
	unsigned char buffer[16];
	int status; 
	int i;
	int id;

	for (i=0; i<16; i++)
		buffer[i] = 0;
	
	status = _requestAll (s);
	if (status != NORMAL)
		return -1*status;
	Sleep(1);
	status = _anticoll (sr);
	if (status != NORMAL)
		return -1*status;
	Sleep(1);
	status = _select (sr, sak);
	if (status != NORMAL)
		return -1*status;
	Sleep(1);
	status = _loadKey (key);
	if (status != NORMAL)
		return status;

	/* 
	 * _authenticate changed to _authenticateA,
	 * some crashed would happen when calling read_card_id() within java ... .  
	 */
	Sleep(1);	
	status = _authenticate (32, 0x60, sr);
	if (status != NORMAL)
		return -1*status;

	Sleep(1);
	status = _read (32, 1, buffer);
	if (status != NORMAL)
		return -1*status;
	
	id = buffer[0] * 100000;
	id += buffer[1] * 0x100;
	id += buffer[2];

	return id;
}


#ifdef __cplusplus
    }
#endif