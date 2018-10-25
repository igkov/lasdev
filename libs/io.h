#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>
#include "vector.h"

// 
#define OP_I2C_READ          0x00 // i2c read
#define OP_I2C_WRITE         0x01 // i2c write
#define OP_I2C_SEQ_READ      0x02 // i2c read seq

// For GET_MEMORY and SET_MEMORY
#define OP_EEREAD            0x0E // read eeprom
#define OP_EEWRITE           0x0F // write eeprom

// For GET_RAW
#define OP_GET_LSM_ACC       0x17
#define OP_GET_LSM_MAG       0x18
#define OP_GET_LSM_DATA      0x19 // with calibration
#define OP_GET_LSM_TEMP      0x25

#define OP_GET_INFO          0x1A
//#define OP_GET_CONFIG        0x1B // thought ee_read
//#define OP_GET_LOG           0x1C // thought ee_read
#define OP_GET_IMU           0x1D
#define OP_GET_DIST          0x1E 
#define OP_GET_DIST_START    0x2A
#define OP_GET_DIST_END      0x2B 
//#define OP_ERASE_LOG         0x1F // thought ee_read
//#define OP_ERASE_CONFIG      0x20 // thought ee_write

#define OP_LAS_POWER_ON      0x21
#define OP_LAS_POWER_OFF     0x22
#define OP_LAS_ENABLE        0x23
#define OP_LAS_LCD_SEND      0x24
#define OP_LAS_LCD_SEND_LOW  0x28

#define OP_UART_SEND         0x26
#define OP_UART_RECV         0x27
#define OP_I2C_SEND          0x29
#define OP_RES3              0x2A
#define OP_RES4              0x2B
#define OP_RES5              0x2C
#define OP_RES6              0x2D
#define OP_RES7              0x2E
#define OP_RES8              0x2F


// FRW Clear command:
#define OP_FW_SWITCH         0xCB
#define OP_FW_CLEAR          0xCC
#define OP_FW_WRITE_RAM      0xCD
#define OP_FW_WRITE_BLOCK    0xCE
#define OP_FW_GET_HW         0xCF

// ..
#define MEM_LOG_OFFSET       0x200

// ..
#define FW_CLEAR_VECTOR      { 0x12, 0x44, 0x54, 0x77, 0xA4, 0xE3, 0x9A, 0x45 }
#define FW_KILL_VECTOR       { 0xAE, 0x9C, 0x4F, 0xD2, 0x75, 0xAA, 0x38, 0xB6 }

typedef struct
{
	uint8_t bOperation;
	uint8_t bDevAddr;
	uint8_t bAddr;
	uint8_t bLen;
} i2c_seq_t;

typedef struct 
{
	uint8_t frw_version_h;
	uint8_t frw_version_l;
	uint8_t configure;
	uint8_t error;
	uint8_t res1;
	uint8_t res2;
	uint8_t res3;
	uint8_t res4;
	uint32_t serial_number;
	uint32_t memory_size;
	uint32_t log_len; // ����� ������� ������ ����.
	uint32_t hrw_version;
} device_info_t, *pdevice_info_t;

typedef struct 
{
	uint8_t  type;           // ��� ������ - ������, �����.
	uint8_t  crc8;           // ����������� ����� ������.
	int16_t  id;             // 0-32767 - ID ������, ������������� - ����� �������.
	uint16_t dist;           // 1mm = 65536mm ~ 65m
	int16_t  tilt;           // -180.0-180.0 Grad (real: value/10)
	uint16_t azimuth;        // 0.0-360.0 Grad (real: value/10)
	uint8_t res[6];
} log_unit_t, *plog_unit_t;  // 16byte

/*
typedef struct
{
	vector3df_t mag;
	vector3df_t acc;
} unit_t, *punit_t;
*/

#define IMG_SIZE (0x8000-0x1000)

/*
	HARDWARE_VERSION 
	�������� �� 2 �������:
	1. HIGH - �������� ���������� ������, ������������� �� ��������.
	2. LOW - ������ ���������� �����, ��������� ����� � �� ��� ����������, 
	   �� �� ������������� ����������������� ��� �������� ���������.
 */
#define HARDWARE_VERSION 0x00000001

typedef struct 
{
	uint32_t tag;           //
	uint32_t size;          // ������ ������
	uint32_t crc32;         // CRC-������
	uint32_t hw_ver;        //
	uint32_t res[4];        //
	uint8_t data[IMG_SIZE]; // ���� ������.
} img_t, *pimg_t;

typedef struct 
{
	uint32_t tag;                        // 0000: ������ ������
	uint32_t size;                       // 0004: ������ ������
	uint32_t crc32;                      // 0008: CRC-������
	uint32_t hw_ver;                     // 0012: ���������� ������
	uint32_t sp;                         // 0016: Stack Pointer
	uint32_t res1[3];                    // 0020: Res
	// Entry points:                     
	struct                               
	{                                    
		uint32_t Reset_Handler;          // 0032: 
		uint32_t SysTick_Handler;        // 0036: 
		uint32_t NMI_Handler;            // 0040: 
		uint32_t HardFault_Handler;      // 0044: 
		uint32_t MemManage_Handler;      // 0048: 
		uint32_t BusFault_Handler;       // 0052: 
		uint32_t UsageFault_Handler;     // 0056: 
		uint32_t SVC_Handler;            // 0060: 
		uint32_t DebugMon_Handler;       // 0064: 
		uint32_t PendSV_Handler;         // 0068: 
		uint32_t WAKEUP_IRQHandler;      // 0072: 
		uint32_t I2C_IRQHandler;         // 0076: 
		uint32_t TIMER16_0_IRQHandler;   // 0080: 
		uint32_t TIMER16_1_IRQHandler;   // 0084: 
		uint32_t TIMER32_0_IRQHandler;   // 0088: 
		uint32_t TIMER32_1_IRQHandler;   // 0092: 
		uint32_t SSP_IRQHandler;         // 0096: 
		uint32_t UART_IRQHandler;        // 0100: 
		uint32_t USB_IRQHandler;         // 0104: 
		uint32_t USB_FIQHandler;         // 0108: 
		uint32_t ADC_IRQHandler;         // 0112: 
		uint32_t WDT_IRQHandler;         // 0116: 
		uint32_t BOD_IRQHandler;         // 0120: 
		uint32_t FMC_IRQHandler;         // 0124: 
		uint32_t PIOINT3_IRQHandler;     // 0128: 
		uint32_t PIOINT2_IRQHandler;     // 0132: 
		uint32_t PIOINT1_IRQHandler;     // 0136: 
		uint32_t PIOINT0_IRQHandler;     // 0140: 
		uint32_t res[4];                 // 0144: 
	} eps; // 32                         
	// Res:                              
	uint32_t res2[256-(32+8)*4];         // 0160: Res.
	uint8_t  data[];                     // 0256: Code+Data in image
} img_bin_t, *pimg_bin_t; // reserved 256b

typedef struct 
{
	uint8_t status; // 0 - ok, 1 - in move, 2 - res
	uint8_t res[3];
	float azimuth;
	float tilt;
} imu_data_t, *pimu_data_t;

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

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bRes3;
		vector3d_t acc;
		vector3d_t mag;
	} dev2pc_lsm;

	struct 
	{
		uint8_t bOperation;
		uint8_t bDevAddr;
		uint16_t wAddr;
		uint8_t bLen;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bRes3;
		uint8_t abData[];
	} pc2dev_ee;

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		device_info_t info;
	} dev2pc_info;

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		log_unit_t log;
	} dev2pc_getlog;

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		imu_data_t imu;
	} dev2pc_imu;

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		int mm;
	} dev2pc_dist;

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		uint32_t hw_ver;
	} dev2pc_hw;

	struct 
	{
		uint8_t bRetStatus;
		uint8_t bRes1;
		uint8_t bRes2;
		uint8_t bLen;
		uint16_t temp;
	} dev2pc_temp;
	
	struct
	{
		uint8_t  bStatus;
		uint8_t  bLen;
		uint16_t wCount;
		uint32_t abData[64-4];
	} dev2pc_spi_logger;

	uint8_t abBuffer[64];
} io_data_t;

// ��������� ����������� ������.
// �.�. ��� ��������� ���������� ��������� ��������� �� �������� ����� ������, 
// ����������, ������� �� ������������ ���������� � ����������� union.
// ��� �� ������������ ���������� ��� ������� USB/NATIVE.
// ��� ��������� ��������� ��������� ���������� ������ � ����������������.
typedef union
{
	struct
	{
		float  samples[32];         // 4*6*32 = 768;
		double inverse_buffer[6*6]; // 8*6*6  = 288;
		float  matrix[6*32];        // 4*6*32 = 768;
		// ����� 1824 �����.
	} cal;
	struct
	{
		// ������� ��������
		vector3df_t mag_raw;
		vector3df_t acc_raw;
		
		// ����������:
		vector3df_t mag_k;
		vector3df_t mag_o;
		vector3df_t acc_k;
		vector3df_t acc_o;
		
		// ���������� ����� ����������:
		vector3df_t mag;
		vector3df_t acc;
		
		// ������� ��� ��������� � ����������:
		//...
	} imu;
	struct
	{
		uint8_t buffer_in[64];
		uint8_t buffer_out[64];
	} usb;
} memory_t;

#define CONFIG_SIZE          (128)
typedef struct config_struct
{
	uint8_t tag;           // ( 1) ���� ����������.
	uint8_t version;       // ( 1) ������ ���������.
	uint8_t memsize;       // ( 1) ������ ���������� ������ � 8�� ������.
	uint8_t res2;          // ( 1) 
	uint32_t sn;           // ( 4) �������� ����� ����������.
	uint16_t crc16;        // ( 2) ����������� ����� ������.
	uint16_t res3;         // ( 2) 
#if 1
	calibrate_t mag_cal;  // (24) ���������� ������������.
	calibrate_t acc_cal;  // (24) ���������� �������������.
#else
	vector3df_t mag_k;     // (12) ���������� ����������� ������������.
	vector3df_t mag_o;     // (12) �������� ���� ������������.
	vector3df_t acc_k;     // (12) ���������� ����������� �������������.
	vector3df_t acc_o;     // (12) �������� ���� �������������.
#endif
	vector3d_t mag_shuff;  // ( 6) ������������ ���� ������������.
	uint16_t res4;         // ( 2) 
	vector3d_t acc_shuff;  // ( 6) ������������ ���� �������������.
	int16_t correct_dist;  // ( 2) ������������� ����������, +-16�.
	float mag_tilt;        // ( 4) ���� ������� ������������ � ���������.
	float correct_mag;     // ( 4) �������� � �������� ������� (���. ���������).
	float correct_tilt;    // ( 4) �������� � �������� ���� � ���������.
	uint8_t res[40];       // (128-88=40b)
} config_t, *pconfig_t;    // Size: 128b, Max size: 512b

#endif // __IO_H__
