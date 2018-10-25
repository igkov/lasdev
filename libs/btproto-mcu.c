/* Структура посылки: */

typedef struct
{
	uint8_t type;
	uint8_t size;
	uint8_t crc8;
	uint8_t res;
	uint8_t data[];
} proto_t, *pproto_t;

#define COMM_MAX_SIZE 64

#define COMM_E_OK        0x00
#define COMM_E_SIZE      0x01
#define COMM_E_OFFSET    0x02
#define COMM_E_CRC       0x03
#define COMM_E_BAD_TYPE  0x04
#define COMM_E_HW_ERR    0x05
#define COMM_E_PASS      0x06
#define COMM_E_PARAM     0x07

uint8_t comm[COMM_MAX_SIZE];
//uint8_t buffer[528];

void bt_proto(void)
{
	while (1)
	{
		// Цикл обработки команд:
		uint8_t offset = 0xFF;
		uint8_t b;
		pproto_t hdr = (pproto_t)comm;
		
		while (1)
		{
			b = uart_getchar();
			switch(b)
			{
			case ':':
				// сброс протокола.
				memset(comm, 0, COMM_MAX_SIZE);
				//if (offset != 0xFF)
				//	goto comm_next;
				offset = 0;
			case '\n':
				break;
			case '\r':
				if (offset == 0xFF)
					break;
				if (offset & 0x01)
				{
					// error, offset
					b = COMM_E_OFFSET;
					goto comm_error;
				}
				// делим на 2, чтобы получить реальную длину команды:
				offset /= 2;
				// проверка длины:
				if (hdr->size != offset - sizeof(proto_t))
				{
					// error, size
					b = COMM_E_SIZE;
					goto comm_error;
				}
				// проверка CRC:
				if (hdr->size != 0 &&
					hdr->crc8 != crc8(hdr->data, hdr->size))
				{
					// error, size
					b = COMM_E_CRC;
					goto comm_error;
				}
				// переход на обработку команды
				goto comm_work;
			default:
				if (offset == 0xFF)
					break;
				// char2hex
				if (b <= '9' && b >= '0')
					b -= '0';
				else if (b <= 'f' && b >= 'a')
					b -= 'a' - 10;
				else if (b <= 'F' && b >= 'A')
					b -= 'A' - 10;
				else break; // ignore
				
				if (offset/2 == COMM_MAX_SIZE)
				{
					b = COMM_E_SIZE;
					goto comm_error;
				}
				else
				{
					comm[offset/2] |= b << (offset & 0x01 ? 0 : 4);
					offset++;
				}
			}
		}
		continue;
comm_work:
		BLINK();
		// Обработка команды:
		switch (hdr->type)
		{
		case 0xFF:
			// End Bluetooth session:
			goto comm_end;
		case 0x00:
			// Test command:
			{
				char ack_data[] = "Hello!\r\n";
				#define ACK_SIZE sizeof(ack_data)
				hdr->type = COMM_E_OK;
				hdr->size = ACK_SIZE;
				hdr->crc8 = crc8((uint8_t*)ack_data, ACK_SIZE);
				hdr->res  = 0;
				memcpy(hdr->data, ack_data, ACK_SIZE);
				goto comm_ack;
			}
		// Здесь обработка других команд
		// ....
		default:
			b = COMM_E_BAD_TYPE;
			goto comm_error;
		}
		goto comm_next;
comm_error:
		// Ошибка (код в b):
		hdr->type = b;
		hdr->size = 0;
		hdr->crc8 = 0xFF;
		hdr->res  = 0;
comm_ack:
		uart_putchar(':');
		for (offset = 0; offset < (hdr->size + sizeof(proto_t)); offset++)
		{
			b = comm[offset];
			if ((b & 0xF0) < 0xA0)
				uart_putchar('0' + (b>>4));
			else
				uart_putchar('A' - 10 + (b>>4));
			if ((b & 0x0F) < 0x0A)
				uart_putchar('0' + (b&0x0F));
			else
				uart_putchar('A' - 10 + (b&0x0F));
		}
		uart_putchar('\r');
		uart_putchar('\n');
comm_next:
		// Переход на след. команду:
		continue;
	}
comm_end:
	// Выход из протокола.
	return;
}
