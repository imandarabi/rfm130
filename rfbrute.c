#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>

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

#define BAUDRATE 9600

int rf_init (int fd)
{
        struct termios options;

        //fcntl(fd, F_SETFL, FNDELAY);
        tcgetattr(fd, &options);
        cfsetispeed(&options, BAUDRATE);
        cfsetospeed(&options, BAUDRATE);
        options.c_cflag |= (CLOCAL | CREAD);//Enable the receiver and set local mode
        options.c_cflag &= ~PARENB;//Mask character size to 8 bits, no parity
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |=  CS8;//Select 8 data bits
        

        options.c_cflag &= ~CRTSCTS;//Disable hardware flow control

        //options.c_lflag |= ~(ICANON | ECHO | ECHOE);//Canonical input
        options.c_lflag &= 0; // ~(ICANON | ECHO | ISIG);//Raw input
        options.c_oflag &= ~OPOST;//Raw output

	options.c_iflag &=0;

        options.c_cc[VTIME] = 10;
        options.c_cc[VMIN] = 100;
        tcsetattr(fd, TCSANOW, &options);

	return 1;
}

int _openPort (char *portName)
{
	int comPort = open (portName, O_RDWR | O_NOCTTY);  
	
	if (!comPort)
		return 0;

	if (!rf_init (comPort))
		return 0;
		
	return comPort;
}

int _closePort (int fd)
{
	return close (fd);
}
int _beep(int port)
{
	int dwWritten, dwRead;
	int dwToWrite = BEEP_CMD_LEN, dwToRead = BEEP_STU_LEN;
	unsigned char cmd[BEEP_CMD_LEN], stu[BEEP_STU_LEN];
	int status;
    int i=0, j;

	cmd[i++]=0x02;
    cmd[i++]=0x00;
    cmd[i++]=0x02;
    cmd[i++]=0x2c;
    cmd[i++]=0x0f;
    cmd[i] = 0;
	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];
	
	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, stu, dwToRead);
	if ( dwToRead != dwRead )
		return OTHER_ERR;

	return stu[3];
}

int _requestAll (int port, unsigned char *card_type)
{            
	int dwWritten, dwRead;
	int dwToWrite = REQ_CMD_LEN, dwToRead = REQ_STU_LEN;
	unsigned char cmd[REQ_CMD_LEN];
	unsigned char stu[REQ_STU_LEN];
	int i=0, j;
	
	cmd[i++]=0x02;
	cmd[i++]=0x00;
	cmd[i++]=0x02;
	cmd[i++]=0x31;
	cmd[i++]=0x52;
	cmd[i] = 0;
	for(j=1;j<i;j++)
		cmd[i]^=cmd[j];
	
	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read (port, stu, dwToRead);
	
	*card_type = stu[4];

	return stu[3];
}

int _anticoll (int port, unsigned char *serialNum)
{            
	int dwWritten, dwRead;
	int dwToWrite = ANTI_CMD_LEN, dwToRead = ANTI_STU_LEN;
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

	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, stu, dwToRead);
	
	if (stu[3] == NORMAL)
		for(j=4;j<8;j++)
			serialNum[j-4]=stu[j];
	
	return stu[3];
}                 

int _select (int port, const unsigned char *serialNum, unsigned char *sak)
{
	int dwWritten, dwRead;
	int dwToWrite = SEL_CMD_LEN, dwToRead = SEL_STU_LEN;
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

	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, stu, dwToRead); 
	if (stu[3] == NORMAL)
		*sak = stu[4];

	return stu[3];
}

int _loadKey (int port, const unsigned char *key)
{
	int dwWritten, dwRead;
	int dwToWrite = LOD_CMD_LEN, dwToRead = LOD_STU_LEN;
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
	
	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, stu, dwToRead); 
	
	return stu[3];
}

int _authenticate (int port, int blockNo, int mode, const unsigned char *serialNum)
{
	int dwWritten, dwRead;
	int dwToWrite = AUT_CMD_LEN, dwToRead = AUT_STU_LEN;
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
		
	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, stu, dwToRead); 

	return stu[3];
}

int _read (int port, int blockNo, int blockQty, unsigned char *buffer)
{
	int dwWritten, dwRead;
	int dwToWrite = READ_CMD_LEN, dwToRead = 5+blockQty*16; //READ_STU_LEN;
	unsigned char cmd[READ_CMD_LEN];
	int i, j, stat;

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

	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, buffer, dwToRead);
	
	stat = buffer[3];

	memmove(buffer, buffer+4, blockQty*16);

	return stat;
}
int _write (int port, int blockNo, int blockQty, unsigned char *buffer)
{
	int dwWritten, dwRead;
	int dwToWrite = 7+blockQty*16, dwToRead = WRITE_STU_LEN;
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

	dwWritten = write (port, cmd, dwToWrite);
	
	dwRead = read(port, buffer, dwToRead);

	return buffer[3];
}

int main(int argc, char **argv)
{
	int i, err;
	unsigned char sr[4];
	unsigned char s, sak; 
	unsigned char key[6];
	int ikey[6];
	unsigned char buffer[128], buff[16];
	int comPort;
	char tmp[100];

	for (i = 0; i<128;i++)
		buffer[i] = 0;
	
//	comPort = _openPort ("/dev/ttyS1");
	comPort = _openPort ("/dev/ttyUSB0");
	printf ("comPort is : %d\n", comPort);
	printf ("\nbeep return: %d \n", _beep (comPort));

	exit(0);
	
	for (ikey[5]=255 ; ikey[5]>=0; ikey[5]--)
	for (ikey[4]=255 ; ikey[4]>=0; ikey[4]--)
	for (ikey[3]=255 ; ikey[3]>=0; ikey[3]--)
	for (ikey[2]=0xe0 ; ikey[2]>=0; ikey[2]--)
	for (ikey[1]=0x1e; ikey[1]>=0; ikey[1]--)
	{
		sprintf (tmp, "\n checking key: %x, %x, %x, %x, %x, [0x00..oxff]", ikey[1], ikey[2], ikey[3], ikey[4], ikey[5]);
		write (1, tmp, strlen (tmp));

		for (ikey[0]=255 ; ikey[0]>=0; ikey[0]--)
		{
			if (_requestAll (comPort, &s))
				goto no_card_exit;
			if (_anticoll (comPort, sr))
				goto no_card_exit;
			if (_select (comPort, sr, &sak))
				goto no_card_exit;

		 	//printf ("\n checking key: %x, %x, %x, %x, %x, %x ", ikey[0], ikey[1], ikey[2], ikey[3], ikey[4], ikey[5]);
			key[0]=ikey[0];
			key[1]=ikey[1];
			key[2]=ikey[2];
			key[3]=ikey[3];
			key[4]=ikey[4];
			key[5]=ikey[5];
			err = _loadKey (comPort, key);
			//printf ("\n_loadKey: %d", err);
			err =  _authenticate (comPort, 32, 0x60, sr);
			//printf ("\n_authenticate: %d", err);
			if (err == NORMAL)
			{
				printf ("\n_authenticate OK!\n");
				printf ("\n\n\nkey is: %x, %x, %x, %x, %x, %x !!!!", ikey[0], ikey[1], ikey[2], ikey[3], ikey[4], ikey[5]);
				exit (0);
			}
			else if (err == NO_CARD)
				goto no_card_exit;
		}
	}

	printf ("\n Unable to find any suitable key!");
	exit(0);

   no_card_exit:		
	printf ("\nNO_CARD ERROR!");
	printf ("\nlast checked key is: %x, %x, %x, %x, %x, %x", ikey[0], ikey[1], ikey[2], ikey[3], ikey[4], ikey[5]);
	return -1;
}


