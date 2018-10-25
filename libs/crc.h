#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>

uint8_t  crc8 (uint8_t *block, uint16_t len);
uint16_t crc16(uint8_t *block, uint16_t len);
uint32_t crc32(uint32_t crc, uint8_t *buff, uint32_t len );

#endif // __CRC16_H__
