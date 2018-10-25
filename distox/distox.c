/*
	distox.c
	
	DistoX protocol emulator.
	
	Ёмул€ци€ протокола DistoX. ѕосылает псевдоизмерени€ в программу PocketTopo,
	имитиру€ тем самым работу прибора.
	
	igorkov / 2012
 */

#include <stdint.h>
#include <stdio.h>

#include "comn.h"

//Start Calibration Mode (1 byte):
//Byte 0: 00110001

//Stop Calibration Mode (1 byte):
//Byte 0: 00110000

//Start Silent Mode (1 byte):
//Byte 0: 00110011

//Stop Silent Mode (1 byte):
//Byte 0: 00110010

//Read Memory (3 bytes):
//Byte 0: 00111000
//Byte 1: low byte of address
//Byte 2: high byte of address

//Write Memory (7 bytes):
//Byte 0: 00111001
//Byte 1: low byte of address
//Byte 2: high byte of address
//Byte 3: data byte 0
//Byte 4: data byte 1
//Byte 5: data byte 2
//Byte 6: data byte 3

// Acknowledge Packet
// An acknowledge packet consists of a single byte:
// Byte 0: bit 7: sequence bit, bits 0-6: 1010101

#pragma push
#pragma pack(1)
typedef struct
{
	uint8_t type;  // type, bit 7: sequence bit, bit 6: bit 16 of distance, bits 0-5: 000001 (packet type)
	uint16_t dist; // laser
	uint16_t decl; // compass
	uint16_t incl;  // inclinometer
	uint8_t roll;  // ???
} distox_t, *pdistox_t; // 8 byte
#pragma pop

int main(int argc, void **argv)
{
	int ret;
	int seq = 0x00;
	com_struct_t com;
	distox_t data;
	
	printf("DistoX data structure size: %d\r\n", sizeof(distox_t));
	
	// Open communication interface:
	ret = com_init(&com, 6, 115200);
	if (ret)
	{
		printf("Communication port initialization fail (%d)!\r\n");
		return 0;
	}
	printf("Connected!\r\n");
	
	while (1)
	{
		uint8_t ch;
		int distant = 1000;
		
		seq = seq ? 0x00 : 0x80;
		
		data.type = (0x1F & 0x01) | // type = measurement data
					(seq) | // sequence bit
					((0x10000 & distant) ? 0x40 : 0x00); // 17 bit of distance
		
		data.dist = distant & 0xFFFF; // 1m
		
		// The roll angle is in radiant * 2^7 / Pi (full circle = 2^8).
		data.roll = 0x0000; // roll didn't support
		
		// For inclination: 0x0000 = horizontal, 0x4000 = up, 0xC000 = down.
		// The 16 bit angles are in radiant * 2^15 / Pi (full circle = 2^16).
		data.incl = 0x0000; // inclination didn't support
		
		// For declination: 0x0000 = north, 0x4000 = east, 0x8000 = south, 0xC000 = west.
		// The 16 bit angles are in radiant * 2^15 / Pi (full circle = 2^16).
		data.decl = 0x2950;
		
		printf("Send data to device...\r\n");
		com_putblock(&com, (uint8_t*)&data, sizeof(distox_t));
		
		Sleep(1000);

		// Acknowlege receive:
		com_getchar(&com, &ch);
		
		printf("Acknowlege byte: %02x\r\n", ch);
		
		if (seq != (ch & 0x80))
		{
			// seq check fail
		}
		
		if (0x55 != (ch & 0x7F))
		{
			// no ack!
		}
		
		printf("Press 'q' for exit, other key for continue...\r\n");
		ch = getch();
		if (ch == 'q' ||
			ch == 'Q')
		{
			break;
		}
	}
	
	return 0;
}
