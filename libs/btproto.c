/*
	btproto.c - Реализация простого протокола взаимодействия.
	
	igorkov / 2015 / fsp@igorkov.org
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btproto.h"
#include "crc.h"

static pcom_struct_t pcom = NULL;

static uint8_t buffer[1+sizeof(proto_t)*2+MAX_PROTO_SIZE*2+2+1];
static int out_offset = 0;
static int proto_putchar(uint8_t ch)
{
	if (out_offset == sizeof(buffer))
		return 1;
	buffer[out_offset] = ch;
	out_offset++;
	return 0;
}
static int proto_putcharH(uint8_t b)
{
	int ret;
	char c = (b & 0xF0) >> 4;
	if (c > 0x09)
		ret = proto_putchar('A' + c - 10);
	else
		ret = proto_putchar('0' + c);
	c = (b & 0x0F);
	if (c > 0x09)
		ret = proto_putchar('A' + c - 10);
	else
		ret = proto_putchar('0' + c);
	return ret;
}
static void proto_flush(void)
{
	memset(buffer, 0, sizeof(buffer));
	out_offset = 0;
}
static int proto_send(void)
{
	int ret;
	if (out_offset == 0)
		return 1;
	ret = com_write(pcom, buffer, out_offset);
	return ret;
}
static int proto_get(int size)
{
	int ret;
	ret = com_read(pcom, buffer, size);
	return ret;
}

/*
	Архитектура запрос-ответ.
	Хост посылает команды
 */
int packet_send(uint8_t code, uint8_t *data, int len)
{
	proto_t hdr;
	int i;
	int ret;
	
	if (pcom == NULL)
		return 1;

	if (len > MAX_PROTO_SIZE)
		return 1;
	
	hdr.type = code;
	hdr.size = len;
	if (len)
		hdr.crc8 = crc8(data, len);
	else
		hdr.crc8 = 0;
	
	proto_flush();

	proto_putchar(':');
	for (i = 0; i < sizeof(proto_t); i++)
		proto_putcharH(((uint8_t*)&hdr)[i]);
	for (i = 0; i < len; i++)
		proto_putcharH(data[i]);
	proto_putchar('\r');
	proto_putchar('\n');
	
	ret = proto_send();
	return ret;
}

int packet_receive(uint8_t *type, uint8_t *data, int *len)
{
	proto_t hdr;
	int i;
	int ret;
	
	int start = -1;
	int size  = 0;
	int bsize = 1+sizeof(proto_t)*2+1;
	int timeout = 100;

	if (pcom == NULL)
		return 1;
		
	//printf("packet_receive() start!\r\n");
	
	proto_flush();
	if (*len)
		memset(data, 0, *len);
	
	while (1)
	{
		ret = proto_get(bsize);
		//printf("proto_get(%d):%d;", bsize, ret);
		//buffer[ret] = 0x00;
		//printf("packet_receive() get %db, data: %s.\r\n", ret, buffer);
		if (ret == 0)
		{
			timeout--;
			if (timeout)
				continue;
			else
				goto error_timeout;
		}
		for (i = 0; i < ret; i++)
		{
			switch (buffer[i])
			{
			case ':':
				// start:
				size = 0;
				start = i;
				break;
			case '\r':
			case '\n':
				// stop:
				if (start >= 0)
					goto received;
			default:
				// data
				if (start >= 0)
				{
					uint8_t c = buffer[i];
					if (c >= '0' && c <= '9')
						c = (c - '0');
					else if (c >= 'A' && c <= 'F')
						c = (c - 'A' + 10);
					else if (c >= 'a' && c <= 'f')
						c = (c - 'a' + 10);
					else
						break;
					
					if (size/2 < sizeof(proto_t))
					{
						if (size & 0x01)
							((uint8_t*)&hdr)[size/2] |= (c);
						else
							((uint8_t*)&hdr)[size/2]  = (c<<4);
					}
					else if (size/2 < MAX_PROTO_SIZE)
					{
						if (*len > size/2 - sizeof(proto_t))
						{
							if (size & 0x01)
								data[size/2 - sizeof(proto_t)] |= (c);
							else
								data[size/2 - sizeof(proto_t)]  = (c<<4);
						}
					}
					size++;
				}
			}
		}
		if (start >= 0)
		{
			if (size >= 4)
				bsize = sizeof(proto_t)*2 + 1 + hdr.size*2 - size;
			else
				bsize = sizeof(proto_t)*2 + 1 - size;
		}
	}

received:
	// 1. Len check:
	if (size < sizeof(proto_t)*2)
		goto error_packet;
	if (hdr.size > *len)
		goto error_ovf;
	// 2. Copy data:
	*type = hdr.type;
	*len = hdr.size;
	// 3. Other:
	if (size/2 != (hdr.size + sizeof(proto_t)))
		goto error_size;
	if (hdr.size)
		if (hdr.crc8 != crc8(data, hdr.size))
			goto error_crc;
	// 4. Ok:
	return 0;
error_size:
	return -1;
error_crc:
	return -2;
error_packet:
	return -3;
error_timeout:
	return -4;
error_ovf:
	return -5;
}

void proto_init(pcom_struct_t com)
{
	pcom = com;
	out_offset = 0;
	memset(buffer, 0, sizeof(buffer));
}
