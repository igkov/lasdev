#ifndef __SPI9_H__
#define __SPI9_H__

#include <stdint.h>

void spi9_select(void);
void spi9_unselect(void);
void spi9_send(uint16_t data);
void spi9_init(void);

#endif
