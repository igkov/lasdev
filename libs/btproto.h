#ifndef __BTPROTO_H__
#define __BTPROTO_H__

#include <stdint.h>
#include "com.h"

#define MAX_PROTO_SIZE 128

typedef struct
{
	uint8_t type;
	uint8_t size;
	uint8_t crc8;
	uint8_t res;
	uint8_t data[];
} proto_t, *pproto_t;

int packet_receive(uint8_t *type, uint8_t *data, int *len);
int packet_send(uint8_t code, uint8_t *data, int len);
void proto_init(pcom_struct_t com);

#endif // __BTPROTO_H__
