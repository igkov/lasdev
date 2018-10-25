#ifndef __IAP_H__
#define __IAP_H__

#include <stdint.h>

/** Коды статуса IAP */
#define IAP_CMD_SUCCESS									0
#define IAP_INVALID_COMMAND								1
#define IAP_SRC_ADDR_ERROR								2
#define IAP_DST_ADDR_ERROR								3
#define IAP_SRC_ADDR_NOT_MAPPED							4
#define IAP_DST_ADDR_NOT_MAPPED							5
#define IAP_COUNT_ERROR									6
#define IAP_INVALID_SECTOR								7
#define IAP_SECTOR_NOT_BLANK							8
#define IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION		9
#define IAP_COMPARE_ERROR								10
#define IAP_BUSY										11
#define IAP_PARAM_ERROR									12
#define IAP_ADDR_ERROR									13
#define IAP_ADDR_NOT_MAPPED								14
#define IAP_CMD_LOCKED									15
#define IAP_INVALID_CODE								16
#define IAP_INVALID_BAUD_RATE							17
#define IAP_INVALID_STOP_BIT							18
#define IAP_CODE_READ_PROTECTION_ENABLED				19

uint32_t iap_prepare(uint32_t sector_start, uint32_t sector_end);
uint32_t iap_erase(uint32_t sector_start, uint32_t sector_end);
uint32_t iap_write16(uint8_t *buffer, uint8_t *flash_address);
uint32_t iap_write256(uint8_t *buffer, uint8_t *flash_address);
uint32_t iap_getid(void);
uint32_t iap_getlastsector(void);


#endif // __IAP_H__
