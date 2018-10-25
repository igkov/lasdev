#include <stdio.h>
#include <string.h>

#include <windows.h>

#include "com.h"
#include "btproto.h"

uint8_t comm_switch[]   = ":BTMODE";
uint8_t comm_test[]     = ":00000000\r\n";
//:000F330048656C6C6F206C6173646576210D0A
//        >
uint8_t comm_getinfo[]  = ":01000000\r\n";
uint8_t comm_gettime[]  = ":02000000\r\n";
uint8_t comm_getlsm[]   = ":06000000\r\n";

int main(int argc, void **argv)
{
	int ret;
	int len;
	uint8_t type;
	uint8_t data[128+1];
	uint8_t buff[1024];
	com_struct_t com;
	
	//int port = 9;
	int port = 41;
	
	ret = com_init(&com, port, 115200);
	if (ret)
		return 0;
	proto_init(&com);

#if 1
	// switch:
	ret = com_write(&com, comm_switch, strlen(comm_test));
	if (ret)
		printf("com_write() return %d\r\n", ret);
	printf("Send: %s\r\n", comm_switch);
	ret = com_read(&com, buff, 1024);
	printf("com_getblock() return %d\r\n", ret);
	if (ret)
	{
		buff[ret] = 0x00;
		printf("Recv: %s\r\n", buff);
	}
#endif

#if 0
	ret = com_write(&com, comm_test, strlen(comm_test));
	if (ret)
	{
		printf("com_putblock() return %d\r\n", ret);
		return ret;
	}
	printf("Send: %s", comm_test);
	ret = com_read(&com, buff, 1024);
	printf("com_getblock() return %d\r\n", ret);
	if (ret)
	{
		buff[ret] = 0x00;
		printf("Send: %s", buff);
	}
	
#else
	// test
	ret = packet_send( 0x00, NULL, 0 );
	printf("packet_send() return %d\r\n", ret);
	len = sizeof(data);
	ret = packet_receive(&type, data, &len);
	printf("packet_receive() return %d\r\n", ret);
	if (ret == 0)
		printf("\tRetCode = %d, RetLen = %d\r\n", type, len);
	data[len] = 0x00;
	printf("\tRetData = %s\r\n", data);
	
	while (1)
	{
		// read sensor:
		ret = packet_send( 0x08, NULL, 0 );
		//printf("packet_send() return %d\r\n", ret);
		len = sizeof(data)-1;
		ret = packet_receive(&type, data, &len);
		//printf("packet_receive() return %d\r\n", ret);
		if (ret == 0)
		{
			printf("acc = { %d, %d, %d }, mag = { %d, %d, %d }\r\n", 
				((int16_t*)data)[0],
				((int16_t*)data)[1],
				((int16_t*)data)[2],
				((int16_t*)data)[3],
				((int16_t*)data)[4],
				((int16_t*)data)[5]);
		}
		else
		{
			printf("RetCode = %d, RetLen = %d\r\n", type, len);
		}
		Sleep(100);
	}
#endif

	return 0;
}
