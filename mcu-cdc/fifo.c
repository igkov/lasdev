#if defined( CORTEX )
#include "lpc13xx.h"
#endif
#include "fifo.h"
#include "type.h"

#if defined( CORTEX )
	#define INIT_MUTEX(...)
	#define DEINIT_MUTEX(...)
	#define BEGIN_PUT(...)		{__disable_irq();}
	#define END_PUT(...)		{__enable_irq();}
	#define BEGIN_GET(...)		{__disable_irq();}
	#define END_GET(...)		{__enable_irq();}
	#define BEGIN_CHK(...)		{__disable_irq();}
	#define END_CHK(...)		{__enable_irq();}
#else
	#define INIT_MUTEX(...)
	#define DEINIT_MUTEX(...)
	#define BEGIN_PUT(...)
	#define END_PUT(...)
	#define BEGIN_GET(...)
	#define END_GET(...)
	#define BEGIN_CHK(...)
	#define END_CHK(...)
#endif

int fifo_init(fifo_t *fifo, uint8_t *buf, int len) {
	fifo->head = 0;
	fifo->tail = 0;
	fifo->len = len;
	
	INIT_MUTEX(fifo);

	if (buf == NULL) {
		return 1;
	} else {
		fifo->buf = buf;
		return 0;
	}
}

int fifo_put(fifo_t *fifo, uint8_t c) {
	int next;
	
	BEGIN_PUT(fifo);
	
	// check if FIFO has room
	next = (fifo->head + 1) % fifo->len;
	if (next == fifo->tail) {
		// full
		END_PUT(fifo);
		return 1;
	}

	fifo->buf[fifo->head] = c;
	fifo->head = next;

	END_PUT(fifo);
	
	return 0;
}

int fifo_putn(fifo_t *fifo, const uint8_t *dataaddr, int len) {
#if 0
	uint8_t *data = (uint8_t *)dataaddr;
	while(len--) {
		if (fifo_put(fifo, *data++)) {
			return 1;
		}
	}
	return 0;
#else
	uint8_t *data = (uint8_t *)dataaddr;

	BEGIN_PUT(fifo);
	while(len--) {
		int next;
		// check if FIFO has room
		next = (fifo->head + 1) % fifo->len;
		if (next == fifo->tail) {
			// full
			END_PUT(fifo);
			return 1;
		}
		fifo->buf[fifo->head] = *data;
		data++;
		fifo->head = next;
	}
	END_PUT(fifo);
	return 0;
#endif
}

int fifo_puts(fifo_t *fifo, const uint8_t *datastr) {
	uint8_t *data = (uint8_t *)datastr;
	BEGIN_PUT(fifo);
	while(*data) {
		int next;
		// check if FIFO has room
		next = (fifo->head + 1) % fifo->len;
		if (next == fifo->tail) {
			// full
			END_PUT(fifo);
			return 1;
		}
		fifo->buf[fifo->head] = *data;
		data++;
		fifo->head = next;
	}
	END_PUT(fifo);
	return 0;
} 

int fifo_get(fifo_t *fifo, uint8_t *pc) {
	int next;
	BEGIN_GET(fifo);
	// check if FIFO has data
	if (fifo->head == fifo->tail) {
		// empty
		END_GET(fifo);
		return 1;
	}
	next = (fifo->tail + 1) % fifo->len;
	*pc = fifo->buf[fifo->tail];
	fifo->tail = next;
	END_GET(fifo);
	return 0;
}

int fifo_getn(fifo_t *fifo, const uint8_t *dataaddr, int len) {
#if 0
	int i;
	uint8_t *data = (uint8_t *)dataaddr;
	for (i = 0; i < len; i++) {
		if (fifo_get(fifo, data++)) {
			break;
		}
	}
	return i;
#else
	int i;
	uint8_t *data = (uint8_t *)dataaddr;
	BEGIN_GET(fifo);
	for (i = 0; i < len; i++) {
		int next;
		// check if FIFO has data
		if (fifo->head == fifo->tail) {
			// empty
			END_GET(fifo);
			break;
		}
		next = (fifo->tail + 1) % fifo->len;
		*data = fifo->buf[fifo->tail];
		data++;
		fifo->tail = next;
	}
	END_GET(fifo);
	return i;
#endif
}

int fifo_avail_free(fifo_t *fifo) {
	int ret;
	BEGIN_CHK(fifo);
	ret = fifo->len - (fifo->len + fifo->head - fifo->tail) % fifo->len - 1;
	END_CHK(fifo);
	return ret;
}

int fifo_avail_data(fifo_t *fifo) {
	int ret;
	BEGIN_CHK(fifo);
	ret = (fifo->len + fifo->head - fifo->tail) % fifo->len;
	END_CHK(fifo);
	return ret;
}

int fifo_flush(fifo_t *fifo) {
	fifo->tail = 0;
	fifo->head = 0;
	return 0;
}

int fifo_free(fifo_t *fifo) {
	fifo->head = 0;
	fifo->tail = 0;
	fifo->len = 0;
	fifo->buf = NULL;
	DEINIT_MUTEX(fifo);
	return 0;
}
