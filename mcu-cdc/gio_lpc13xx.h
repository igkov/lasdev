#ifndef _GIO_LPC13XX_H_
#define _GIO_LPC13XX_H_

void gio_init(void);									// �������������.
void gio_out(unsigned char bPort, unsigned char bPin);	// ��������� ����, ��� �����.
void gio_in(unsigned char bPort, unsigned char bPin);	// ��������� ����, ��� ����.
void gio_set0(unsigned char bPort, unsigned char bPin);	// ��������� ���. 0 �� ����.
void gio_set1(unsigned char bPort, unsigned char bPin);	// ��������� ���. 1 �� ����.
int  gio_get(unsigned char bPort, unsigned char bPin);	// �������� �������� �� ����.

#endif //_GIO_LPC13XX_H_
