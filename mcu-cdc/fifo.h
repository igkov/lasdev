#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>

/* ��������� ������������� � �������, ��������� ������������ ������� �� ���������� �������. */
#define FIFO_SYNC

typedef struct 
{
	int		head;
	int		tail;
	int		len;
	uint8_t	*buf;
} fifo_t, *pfifo_t;

/*
	fifo_init()
	
	������������� ������� fifo � ������ buf � ������ len.
 */
int fifo_init(fifo_t *fifo, uint8_t *buf, int len);

/*
	fifo_put()
	
	�������� 1 ������� � �������.
 */
int fifo_put(fifo_t *fifo, uint8_t c);

/*
	fifo_putn()
	
	�������� n ��������� � �������.
 */
int fifo_putn(fifo_t *fifo, const uint8_t *dataaddr, int len);

/*
	fifo_puts()
	
	�������� ������ � �������.
 */
int fifo_puts(fifo_t *fifo, const uint8_t *datastr);

/*
	fifo_get()
	
	����� 1 ������� �� �������.
 */
int fifo_get(fifo_t *fifo, uint8_t *pc);

/*
	fifo_getn()
	
	����� n ��������� �� �������.
 */
int fifo_getn(fifo_t *fifo, const uint8_t *dataaddr, int len);

/*
	fifo_avail_free()
	
	���������� ��������� ����� � �������.
 */
int  fifo_avail_free(fifo_t *fifo);

/*
	fifo_avail_data()
	
	���������� ����� ��������� � �������.
 */
int  fifo_avail_data(fifo_t *fifo);


/*
	fifo_flush()
	
	������� �������.
 */
int fifo_flush(fifo_t *fifo);

/*
	fifo_free()
	
	�������� ��������������� ������� (� ������, ���� ����� ��� ������� 
	���������� � ����, ����������� ������.
 */
int fifo_free(fifo_t *fifo);

#endif // __FIFO_H__
