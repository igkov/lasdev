#ifndef __SPI_EEPROM__
#define __SPI_EEPROM__

/** Команды EEPROM */
#define SPI_WREN 0x06
#define SPI_WRDI 0x04
#define SPI_RDSR 0x05	  
#define SPI_WRSR 0x01
#define SPI_READ 0x03
#define SPI_WRITE 0x02

unsigned char ee_read  (unsigned short addr, 
			unsigned char *buff, 
			unsigned char len);			/**< Чтение из EEPROM */

unsigned char ee_write (unsigned short addr, 
			unsigned char *buff, 
			unsigned char len);		/**< Запись в  EEPROM */


unsigned char ee_getstat(void);		/**< Получение статуса EEPROM */
unsigned char ee_setstat(unsigned char stat); /**< Установка статуса EEPROM */
void ee_write_dis(void);	/**< Разрешение записи EEPROM */
void ee_write_en(void);	/**< Запрет записи EEPROM */


#endif // __SPI_EEPROM__
