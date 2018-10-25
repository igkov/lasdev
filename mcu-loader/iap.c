/**
	@file 
	iap.c
	
	igorkov / 2012
 */
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "iap.h"

/** Коды команд IAP */
#define IAP_CMD_PREPARE		50
#define IAP_CMD_WRITE		51
#define IAP_CMD_ERASE		52
#define IAP_CMD_BLANK		53
#define IAP_CMD_READID		54
#define IAP_CMD_BOOTVER		55
#define IAP_CMD_COMP		56

/** Точка входа IAP */
#define IAP_LOCATION 0x1fff1ff1
typedef void (*IAP)(uint32_t [],uint32_t []);
static const IAP IapEntry = (IAP)IAP_LOCATION;

/**	Переменные, передаваемые в IAP. */
ZERO_INIT uint32_t command[5]; // = {cmd, p0, p1, p2, p3};
ZERO_INIT uint32_t result[2];

void __IapEntry(void)
{
	IapEntry(command, result);
}

extern uint32_t SystemFrequency;

uint32_t iap_prepare(uint32_t sector_start, uint32_t sector_end)
{
	command[0] = IAP_CMD_PREPARE;
	command[1] = sector_start;
	command[2] = sector_end;
	__IapEntry();
	return result[0];
}

uint32_t iap_erase(uint32_t sector_start, uint32_t sector_end)
{
	command[0] = IAP_CMD_ERASE;
	command[1] = sector_start;
	command[2] = sector_end;
	command[3] = SystemFrequency / 1000;
	__IapEntry();
	return result[0];
}

uint32_t iap_write256(uint8_t *buffer, uint8_t *flash_address)
{
	command[0] = IAP_CMD_WRITE;
	command[1] = (uint32_t)(flash_address);
	command[2] = (uint32_t)buffer;
	command[3] = 256;
	command[4] = SystemFrequency / 1000;
	__IapEntry();
	return result[0];
}

uint32_t iap_getid(void)
{
	command[0] = IAP_CMD_READID;
	__IapEntry();
    return result[1];
}

typedef struct
{
	uint32_t id;
	uint8_t last_sect;
	uint8_t ram;   // Kb
	uint16_t flash; // Kb
} mcu_table_t, *pmcu_table_t;

static const mcu_table_t mcu[] =
{
	{ 0x3D01402B, 3, 4, 16 }, /* LPC1342FHN33, LPC1342FBD48 */
	{ 0x3D00002B, 7, 8, 32 }, /* LPC1343FHN33, LPC1343FBD48 */
	{ 0, 0, 0, 0 }
};

uint32_t iap_getlastsector(void)
{
	int i = 0;
	uint32_t id;
	
	id = iap_getid();

	while (mcu[i].id)
	{
		if (mcu[i].id == id)
		{
			return mcu[i].last_sect;
		}
		i++;
	}

    return 0;
}
