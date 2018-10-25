#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#include "hid.h"

hid_context device;

typedef union 
{
	struct 
	{
		uint8_t bOperation;
		uint8_t bDevAddr;
		uint8_t bAddr;
		uint8_t bLen;
		uint8_t abData[];
	} pc2dev;
	
	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		uint8_t abData[];
	} dev2pc;
	
	uint8_t abBuffer[64];
} io_data_t;

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} vector3d_t, *pvector3d_t;

/*
	get_raw_data()
	Получение сырых данных с датчиков.
 */
int get_raw_data(int op, pvector3d_t data)
{
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = op;
	data_io->pc2dev.bDevAddr = 0;
	data_io->pc2dev.bAddr = 0;
	data_io->pc2dev.bLen = 0;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0)
	{
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF)
	{
		goto rep_read_answer_w;
	}
	
	memcpy((uint8_t*)data, data_io->dev2pc.abData, sizeof(vector3d_t));
	return data_io->dev2pc.bRetStatus;
}

#define OP_GET_LSM_ACC       0x17
#define OP_GET_LSM_MAG       0x18

int main(int argc, char **argv)
{
	vector3d_t mag;
	vector3d_t acc;
	int ret;

	printf("Open device... ");
	ret = hid_find(&device, 0x1FC9, 0x0007, 0xFF00);
	if (ret == 0)
	{
		printf("ERROR: Device not found!\r\n");
		return 0;
	}
	
	ret = get_raw_data(OP_GET_LSM_ACC, &acc);
	if (ret)
	{
		printf("ERROR: get_raw_data() return %d\r\n", ret);
		goto close_and_exit;
	}
	ret = get_raw_data(OP_GET_LSM_MAG, &mag);
	if (ret)
	{
		printf("ERROR: get_raw_data() return %d\r\n", ret);
		goto close_and_exit;
	}
	
	printf("Sensors RAW data:\r\n");
	printf("ACC: (%d, %d, %d)\r\n", acc.x, acc.y, acc.z);
	printf("MAG: (%d, %d, %d)\r\n", mag.x, mag.y, mag.z);

close_and_exit:
	hid_close(&device);

	return 0;
}
