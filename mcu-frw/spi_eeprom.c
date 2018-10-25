
#include "dbg.h"

#include "spi.h"
#include "spi_eeprom.h"

unsigned char ee_getstat(void)
{
	unsigned char stat;
	spi_select();
	spi_send(SPI_RDSR);
	stat = spi_send(0xFF);
	spi_unselect();
	return stat;
}

unsigned char ee_setstat(unsigned char stat)
{
	spi_select();
	spi_send(SPI_WRSR);
	spi_send(stat);
	spi_unselect();
	return stat;
}

void ee_write_dis(void)
{
	spi_select();
	spi_send(SPI_WRDI);
	spi_unselect();
}

void ee_write_en(void)
{
	spi_select();
	spi_send(SPI_WREN);
	spi_unselect();
}

unsigned char ee_write(unsigned short addr, 
			unsigned char *buff, 
			unsigned char len)
{
	int offset;
//	DBG("ee_write(a=%04x, s=%d);\r\n", addr, len);
	ee_write_en();
	spi_select();
	spi_send(SPI_WRITE);
	spi_send(addr>>8);
	spi_send(addr&0xFF);
	for (offset = 0; offset < len; offset++)
		spi_send(buff[offset]);
	spi_unselect();
	while ( ee_getstat() & 0x01 );
	ee_write_dis();
	return 0;
}

unsigned char ee_read(unsigned short addr, 
			unsigned char *buff, 
			unsigned char len)
{
	int offset;
//	DBG("ee_read(a=%04x, s=%d);\r\n", addr, len);
	spi_select();
	spi_send(SPI_READ);
	spi_send(addr>>8);
	spi_send(addr&0xFF);
	for (offset = 0; offset < len; offset++)
		buff[offset] = spi_send(0xFF);
	spi_unselect();
	return 0;
}

