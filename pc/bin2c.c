#include <stdio.h>
#include "file.h"

int main(int argc, char **argv)
{
	uint8_t *buffer;
	uint32_t len;
	int ret;
	int i;

	if (argc != 3)
		return 1;
	
	// Загружаем образ программы:
	ret = f2b(argv[1], &buffer, &len);
	if (ret)
	{
		return 2;
	}

	printf("#include <stdint.h>\n");
	printf("uint32_t %s_len = %d;\n", argv[2], len);
	printf("uint8_t %s_bin[] = {", argv[2]);
	for (i=0; i<len; i++)
	{
		if (i%32 == 0)
			printf("\n\t");
		printf("0x%02x, ", buffer[i]);
		
	}
	printf("\n};\n");
	
	return 0;
}
