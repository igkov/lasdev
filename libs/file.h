#ifndef __FILE_H__
#define __FILE_H__

#include <stdint.h>

/* Buffer to File */
int b2f(const char *pszFileName, uint8_t *pbFileBuffer, uint32_t dwSize);

/* File to buffer. */
int f2b(const char *pszFileName, uint8_t **ppbFileBuffer, uint32_t *pdwSize);

#endif // __FILE_H__
