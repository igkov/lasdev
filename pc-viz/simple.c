#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#include "hid.h"
#include "lowpass.h"
#include "vector.h"
#include "io.h"
#include "visualization.h"
#include "calibrate.h"
#include "btproto.h"
#include "com.h"

#if defined( USB )
hid_context device;
#else
com_struct_t com;
#endif

#if 0
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
#endif

/*
	get_raw_data()
	Получение сырых данных с датчиков.
 */
#if defined( USB )	
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
#elif defined( BT )
int get_raw_data(pvector3d_t data_a, pvector3d_t data_m)
{
	int ret;
	int len;
	uint8_t type;
	uint8_t buffer[128];
	// read sensor:
	ret = packet_send( 0x08, NULL, 0 );
	//printf("packet_send() return %d\r\n", ret);
	len = sizeof(buffer)-1;
	ret = packet_receive(&type, buffer, &len);
	//printf("packet_receive() return %d\r\n", ret);
	data_a->x = ((int16_t*)buffer)[0];
	data_a->y = ((int16_t*)buffer)[1];
	data_a->z = ((int16_t*)buffer)[2];
	data_m->x = ((int16_t*)buffer)[3];
	data_m->y = ((int16_t*)buffer)[4];
	data_m->z = ((int16_t*)buffer)[5];
	return ret;
}
#endif

#define OP_GET_LSM_ACC       0x17
#define OP_GET_LSM_MAG       0x18

int main(int argc, char **argv)
{
	vector3d_t mag;
	vector3d_t acc;
	visual_t vis1;
	visual_t vis2;
	int ret;
	
	calibrate_cntx_t calc_cntx;
	
	filter_t fmx;
	filter_t fmy;
	filter_t fmz;
	filter_t fax;
	filter_t fay;
	filter_t faz;
	
	// filter init
	fmx.a = fmy.a = fmz.a = 0.3f;
	fax.a = fay.a = faz.a = 0.3f;
	fmx.xp = fmy.xp = fmz.xp = 0.0f;
	fax.xp = fay.xp = faz.xp = 0.0f;
	fmx.max_d = fmy.max_d = fmz.max_d = 30.0f;
	fax.max_d = fay.max_d = faz.max_d = 2000.0f;

#if defined( USB )
	printf("Open device... ");
	ret = hid_find(&device, 0x1FC9, 0x0007, 0xFF00);
	if (ret == 0)
	{
		printf("ERROR: Device not found!\r\n");
		return 0;
	}
#elif defined( BT )
	//#define PORT 9
	#define PORT 21
	//#define PORT 41
	ret = com_init(&com, PORT, 115200);
	if (ret)
		return 0;
	proto_init(&com);

	if (1)
	{
		uint8_t comm_switch[]   = ":BTMODE";
		uint8_t buff[128];
		// switch:
		ret = com_write(&com, comm_switch, strlen(comm_switch));
		if (ret)
			printf("com_write() return %d\r\n", ret);
		printf("Send: %s\r\n", comm_switch);
		ret = com_read(&com, buff, sizeof(buff));
		printf("com_getblock() return %d\r\n", ret);
		if (ret)
		{
			buff[ret] = 0x00;
			printf("Recv: %s\r\n", buff);
		}
	}
#endif
	
	graph_init();
	
	ret = graph_create(&vis1, 640, 400, "LSM303 Acceleration", 32000.0f, 0.0f);
	if (ret)
	{
		printf("graph_create (1) return %d!\n", ret);
		goto close_and_exit;
	}
	ret = graph_create(&vis2, 640, 400, "LSM303 Magnified", 1000.0f, 0.0f);
	if (ret)
	{
		printf("graph_create (2) return %d!\n", ret);
		goto close_and_exit;
	}

	printf("Sensors RAW data:\r\n");
	
	calibration_auto_init(&calc_cntx);

	while (1)
	{
		vector3df_t magf;
		vector3df_t accf;
#if defined( BT )
		int len;
		uint8_t type;
		uint8_t data[128];
#endif

#if defined( USB )
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
#elif defined( BT )
		// read sensor:
		ret = packet_send( 0x08, NULL, 0 );
		//printf("packet_send() return %d\r\n", ret);
		len = sizeof(data)-1;
		ret = packet_receive(&type, data, &len);
		//printf("packet_receive() return %d\r\n", ret);
		acc.x = ((int16_t*)data)[0];
		acc.y = ((int16_t*)data)[1];
		acc.z = ((int16_t*)data)[2];
		mag.x = ((int16_t*)data)[3];
		mag.y = ((int16_t*)data)[4];
		mag.z = ((int16_t*)data)[5];
#endif
		
		//printf("ACC: (%d, %d, %d)\r\n", acc.x, acc.y, acc.z);
		//printf("MAG: (%d, %d, %d)\r\n", mag.x, mag.y, mag.z);
		
#if 0
		accf.x = lowpass_smart(acc.x, &fax);
		accf.y = lowpass_smart(acc.y, &fay);
		accf.z = lowpass_smart(acc.z, &faz);
		
		magf.x = lowpass_smart(mag.x, &fmx);
		magf.y = lowpass_smart(mag.y, &fmy);
		magf.z = lowpass_smart(mag.z, &fmz);
#else
		accf.x = acc.x;
		accf.y = acc.y;
		accf.z = acc.z;
		
		magf.x = mag.x;
		magf.y = mag.y;
		magf.z = mag.z;
#endif
		
		// Автоматическая калибровка:
		ret = calibration_auto(&calc_cntx, &accf, &magf);
		if (ret)
		{
			break;
		}
		
		//printf("ACC: (%.1f, %.1f, %.1f)\r\n", accf.x, accf.y, accf.z);
		//printf("MAG: (%.1f, %.1f, %.1f)\r\n", magf.x, magf.y, magf.z);
		
		graph_add(&vis1, accf.x, VIS_COLOR_RED);
		graph_add(&vis1, accf.y, VIS_COLOR_YELLOW);
		graph_add(&vis1, accf.z, VIS_COLOR_GREEN);
		graph_add(&vis1, 0.0, VIS_COLOR_WHITE);
		
		//graph_add(&vis2, acc.x, VIS_COLOR_RED);
		//graph_add(&vis2, acc.y, VIS_COLOR_YELLOW);
		//graph_add(&vis2, acc.z, VIS_COLOR_GREEN);
		//graph_add(&vis2, 0.0, VIS_COLOR_WHITE);
		
		graph_add(&vis2, magf.x, VIS_COLOR_RED);
		graph_add(&vis2, magf.y, VIS_COLOR_YELLOW);
		graph_add(&vis2, magf.z, VIS_COLOR_GREEN);
		graph_add(&vis2, 0.0, VIS_COLOR_WHITE);
		
		//Sleep(100);
	}
	
	printf("Calibrated Ok!\r\n\r\n\r\n");
	graph_scale(&vis1, 2.0f, 0.0f);
	graph_scale(&vis2, 2.0f, 0.0f);

	// Калибровано!
	while (1)
	{
		vector3df_t magf;
		vector3df_t accf;
#if defined( BT )
		int len;
		uint8_t type;
		uint8_t data[128];
#endif

#if defined( USB )
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
#elif defined( BT )
		// read sensor:
		ret = packet_send( 0x08, NULL, 0 );
		//printf("packet_send() return %d\r\n", ret);
		len = sizeof(data)-1;
		ret = packet_receive(&type, data, &len);
		//printf("packet_receive() return %d\r\n", ret);
		acc.x = ((int16_t*)data)[0];
		acc.y = ((int16_t*)data)[1];
		acc.z = ((int16_t*)data)[2];
		mag.x = ((int16_t*)data)[3];
		mag.y = ((int16_t*)data)[4];
		mag.z = ((int16_t*)data)[5];
#endif
		
		//printf("ACC: (%d, %d, %d)\r\n", acc.x, acc.y, acc.z);
		//printf("MAG: (%d, %d, %d)\r\n", mag.x, mag.y, mag.z);
		
		accf.x = lowpass_smart(acc.x, &fax);
		accf.y = lowpass_smart(acc.y, &fay);
		accf.z = lowpass_smart(acc.z, &faz);
		
		magf.x = lowpass_smart(mag.x, &fmx);
		magf.y = lowpass_smart(mag.y, &fmy);
		magf.z = lowpass_smart(mag.z, &fmz);
		
		ret = calibration_procf(&accf, &magf, &calc_cntx.cal_a, &calc_cntx.cal_m);
		
		//printf("ACC: (%.1f, %.1f, %.1f)\r\n", accf.x, accf.y, accf.z);
		//printf("MAG: (%.1f, %.1f, %.1f)\r\n", magf.x, magf.y, magf.z);
		
		graph_add(&vis1, accf.x, VIS_COLOR_RED);
		graph_add(&vis1, accf.y, VIS_COLOR_YELLOW);
		graph_add(&vis1, accf.z, VIS_COLOR_GREEN);
		graph_add(&vis1, 0.0, VIS_COLOR_WHITE);
		
		//graph_add(&vis2, acc.x, VIS_COLOR_RED);
		//graph_add(&vis2, acc.y, VIS_COLOR_YELLOW);
		//graph_add(&vis2, acc.z, VIS_COLOR_GREEN);
		//graph_add(&vis2, 0.0, VIS_COLOR_WHITE);
		
		graph_add(&vis2, magf.x, VIS_COLOR_RED);
		graph_add(&vis2, magf.y, VIS_COLOR_YELLOW);
		graph_add(&vis2, magf.z, VIS_COLOR_GREEN);
		graph_add(&vis2, 0.0, VIS_COLOR_WHITE);
		
		//Sleep(100);
	}

close_and_exit:
#if defined( USB )
	hid_close(&device);
#elif defined( BT )
	com_deinit(&com);
#endif

	return 0;
}
