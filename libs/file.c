#include <stdio.h>
#include <stdlib.h>

#include "file.h"

int b2f(const char *pszFileName, uint8_t *pbFileBuffer, uint32_t dwSize)
{
	FILE *hFile;
	
	//if (fopen_s(&hFile, pszFileName, "wb+") != 0)
	//	return 1;

	if ((hFile = fopen(pszFileName, "wb+")) == 0)
		return 1;
	
	fseek(hFile, 0, SEEK_SET);
	fwrite(pbFileBuffer, 1, dwSize, hFile);

	fclose(hFile);
	
	return 0;
}

int f2b(const char *pszFileName, uint8_t **ppbFileBuffer, uint32_t *pdwSize)
{
	FILE *hFile;
	uint32_t dwSize = 0;

	*pdwSize = 0x00;
	//if (fopen_s(&hFile, pszFileName, "rb") != 0)
	//	return 1;

	if ((hFile = fopen(pszFileName, "rb")) == 0)
		return 1;

	*ppbFileBuffer = (uint8_t*)malloc(1024*sizeof(uint8_t));
	do
	{
		if (ppbFileBuffer == NULL)
			return 2;
		dwSize = fread(&((*ppbFileBuffer)[*pdwSize]), 1, 1024, hFile);
		*pdwSize += dwSize;
		*ppbFileBuffer = (uint8_t*)realloc(*ppbFileBuffer, *pdwSize + 1024);
	}
	while (dwSize == 1024);

	fclose(hFile);
	return 0;
}

