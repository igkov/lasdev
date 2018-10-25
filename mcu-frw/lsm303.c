#include "lsm303.h"
#include "i2c.h"
#include "rev.h"

//#define LSM303_MAG_ADDR 0x1E
//#define LSM303_ACC_ADDR 0x19
#define LSM303_MAG_ADDR 0x3C
#define LSM303_ACC_ADDR 0x32


int lsm303_init(int preca, int precm, int temp)
{
	uint8_t data[4];
	int ret;

	// 1. Magnetic

	//set CRA_REG_M register
	//Write(0x1E, 0x00, 0x14);
	data[0] = 0x14 | (temp ? 0x80 : 0x00);
	ret = i2c_write(LSM303_MAG_ADDR, 0x00, data, 0x01);
	if (ret) return ret;

	data[0] = ((precm & 0x07) << 5);
	ret = i2c_write(LSM303_MAG_ADDR, 0x01, data, 0x01);
	if (ret) return ret;
	
	//set MR_REG_M register
	//Write(0x1E, 0x02, 0x00);
	data[0] = 0x00;
	ret = i2c_write(LSM303_MAG_ADDR, 0x02, data, 0x01);
	if (ret) return ret;

	// 2. Accel

	//set CTRL_REG1_A register
	//Write(0x18, 0x20, 0x27);
	data[0] = 0x27;
	ret = i2c_write(LSM303_ACC_ADDR, 0x20, data, 0x01);
	if (ret) return ret;
	
	//set CTRL_REG4_A register
	//Write(0x18, 0x23, 0x40);
	data[0] = ((preca & 0x03) << 4);
	ret = i2c_write(LSM303_ACC_ADDR, 0x23, data, 0x01);
	if (ret) return ret;

	return 0;
}
	

int lsm303_get_mag(pvector3d_t data)
{
	int ret;

	ret = i2c_read(LSM303_MAG_ADDR, 0x03, (uint8_t*)data, sizeof(vector3d_t));
	if (ret) return 1;

	rev16((uint16_t*)&data->x);
	rev16((uint16_t*)&data->y);
	rev16((uint16_t*)&data->z);

	return ret;
}

int lsm303_get_acc(pvector3d_t data)
{
	int ret;

	ret = i2c_read(LSM303_ACC_ADDR, 0x80|0x28, (uint8_t*)data, sizeof(vector3d_t));
	if (ret) return 1;

	//rev16((uint16_t*)&data->x);
	//rev16((uint16_t*)&data->y);
	//rev16((uint16_t*)&data->z);

	return ret;
}

int lsm303_get_temp(uint16_t *temp)
{
	int ret;

	ret = i2c_read(LSM303_MAG_ADDR, 0x31, (uint8_t*)temp, sizeof(uint16_t));
	if (ret) return 1;
	
	rev16(temp);
	*temp /= 16;
	
	return ret;
}
