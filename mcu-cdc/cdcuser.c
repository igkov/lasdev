/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    cdcuser.c
 *      Purpose: USB Communication Device Class User module 
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC family microcontroller devices only. Nothing 
 *      else gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "type.h"

#include "lpc13xx.h"

#include "usb.h"
#include "usbhw_LPC13xx.h"
#include "usbcfg_LPC13xx.h"
#include "usbcore.h"
#include "cdc.h"
#include "cdcuser.h"
#include "fifo.h"
#include "uart.h"

#include "gio_LPC13xx.h"

unsigned char BulkBufIn  [USB_CDC_BUFSIZE];            // Buffer to store USB IN  packet
unsigned char BulkBufOut [USB_CDC_BUFSIZE];            // Buffer to store USB OUT packet
unsigned char NotificationBuf[10];

CDC_LINE_CODING CDC_LineCoding  = { 115200, 0, 0, 8 };
unsigned short  CDC_SerialState = 0x0000;
volatile int CDC_DepInEmpty  = 1;                   // Data IN EP is empty

fifo_t fifo_in;
uint8_t data_in[256] __align(4);
fifo_t fifo_out;
uint8_t data_out[1024] __align(4);

/*----------------------------------------------------------------------------
  CDC Initialisation
  Initializes the data structures and serial port
  Parameters:   None 
  Return Value: None
 *---------------------------------------------------------------------------*/
void CDC_Init (void)
{
  CDC_DepInEmpty  = 1;
  CDC_SerialState = CDC_GetSerialState();

  fifo_init(&fifo_in, data_in, sizeof(data_in));
  fifo_init(&fifo_out, data_out, sizeof(data_out));
}


/*----------------------------------------------------------------------------
  CDC SendEncapsulatedCommand Request Callback
  Called automatically on CDC SEND_ENCAPSULATED_COMMAND Request
  Parameters:   None                          (global SetupPacket and EP0Buf)
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_SendEncapsulatedCommand (void)
{
  /* ... add code to handle request */
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC GetEncapsulatedResponse Request Callback
  Called automatically on CDC Get_ENCAPSULATED_RESPONSE Request
  Parameters:   None                          (global SetupPacket and EP0Buf)
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_GetEncapsulatedResponse (void)
{
  /* ... add code to handle request */
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC SetCommFeature Request Callback
  Called automatically on CDC Set_COMM_FATURE Request
  Parameters:   FeatureSelector
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_SetCommFeature (unsigned short wFeatureSelector)
{
  /* ... add code to handle request */
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC GetCommFeature Request Callback
  Called automatically on CDC Get_COMM_FATURE Request
  Parameters:   FeatureSelector
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_GetCommFeature (unsigned short wFeatureSelector)
{
  /* ... add code to handle request */
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC ClearCommFeature Request Callback
  Called automatically on CDC CLEAR_COMM_FATURE Request
  Parameters:   FeatureSelector
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_ClearCommFeature (unsigned short wFeatureSelector) {
  /* ... add code to handle request */
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC SetLineCoding Request Callback
  Called automatically on CDC SET_LINE_CODING Request
  Parameters:   none                    (global SetupPacket and EP0Buf)
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_SetLineCoding (void)
{
  CDC_LineCoding.dwDTERate   =   (EP0Buf[0] <<  0)
                               | (EP0Buf[1] <<  8)
                               | (EP0Buf[2] << 16)
                               | (EP0Buf[3] << 24); 
  CDC_LineCoding.bCharFormat =  EP0Buf[4];
  CDC_LineCoding.bParityType =  EP0Buf[5];
  CDC_LineCoding.bDataBits   =  EP0Buf[6];

  // set uart speed
  if (LPC_SYSCON->SYSAHBCLKCTRL & (1<<12)) {
    //uart_init_speed(CDC_LineCoding.dwDTERate, SystemCoreClock);
	uart_init(CDC_LineCoding.dwDTERate, SystemCoreClock);
  }

  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC GetLineCoding Request Callback
  Called automatically on CDC GET_LINE_CODING Request
  Parameters:   None                         (global SetupPacket and EP0Buf)
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_GetLineCoding (void)
{
  EP0Buf[0] = (CDC_LineCoding.dwDTERate >>  0) & 0xFF;
  EP0Buf[1] = (CDC_LineCoding.dwDTERate >>  8) & 0xFF;
  EP0Buf[2] = (CDC_LineCoding.dwDTERate >> 16) & 0xFF;
  EP0Buf[3] = (CDC_LineCoding.dwDTERate >> 24) & 0xFF;
  EP0Buf[4] =  CDC_LineCoding.bCharFormat;
  EP0Buf[5] =  CDC_LineCoding.bParityType;
  EP0Buf[6] =  CDC_LineCoding.bDataBits;

  /*
  	ƒанна€ установка - небольшой хак.
	¬ отсутсвии коннекта к устройству, возникает ситукци€, 
	когда данные не уход€т и цепочка отправки рушитс€.
	Ётот сброс позвол€ет нормально продолжить работу с устройтсвом.
   */
  CDC_DepInEmpty = 1;

  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC SetControlLineState Request Callback
  Called automatically on CDC SET_CONTROL_LINE_STATE Request
  Parameters:   ControlSignalBitmap 
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_SetControlLineState (unsigned short wControlSignalBitmap) {
  /* ... add code to handle request */
  // Bit 0 - DTR
  // Bit 1 - RST
  if (wControlSignalBitmap & 0x0001) {
     gio_out(0,1);
	 gio_set0(0,1);
  } else {
     gio_out(0,1);
	 gio_set1(0,1);
  }
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC SendBreak Request Callback
  Called automatically on CDC Set_COMM_FATURE Request
  Parameters:   0xFFFF  start of Break 
                0x0000  stop  of Break
                0x####  Duration of Break
  Return Value: TRUE - Success, FALSE - Error
 *---------------------------------------------------------------------------*/
uint32_t CDC_SendBreak (unsigned short wDurationOfBreak)
{
  /* ... add code to handle request */
  return (TRUE);
}


/*----------------------------------------------------------------------------
  CDC_BulkIn call on DataIn Request
  Parameters:   none
  Return Value: none
 *---------------------------------------------------------------------------*/
void CDC_BulkIn(void) {
	CDC_DepInEmpty = 1;
}

extern fifo_t fifo_in;

/*----------------------------------------------------------------------------
  CDC_BulkOut call on DataOut Request
  Parameters:   none
  Return Value: none
 *---------------------------------------------------------------------------*/
void CDC_BulkOut(void)
{
  int numBytesRead;
  // get data from USB into intermediate buffer
  numBytesRead = USB_ReadEP(CDC_DEP_OUT, &BulkBufOut[0]);
  fifo_putn(&fifo_in, BulkBufOut, numBytesRead);
}

/*----------------------------------------------------------------------------
  Get the SERIAL_STATE as defined in usbcdc11.pdf, 6.3.5, Table 69.
  Parameters:   none
  Return Value: SerialState as defined in usbcdc11.pdf
 *---------------------------------------------------------------------------*/
unsigned short CDC_GetSerialState (void)
{
  CDC_SerialState = 0;
  return (CDC_SerialState);
}

/*----------------------------------------------------------------------------
  Send the SERIAL_STATE notification as defined in usbcdc11.pdf, 6.3.5.
 *---------------------------------------------------------------------------*/
void CDC_NotificationIn (void)
{
  NotificationBuf[0] = 0xA1;                           // bmRequestType
  NotificationBuf[1] = CDC_NOTIFICATION_SERIAL_STATE;  // bNotification (SERIAL_STATE)
  NotificationBuf[2] = 0x00;                           // wValue
  NotificationBuf[3] = 0x00;
  NotificationBuf[4] = 0x00;                           // wIndex (Interface #, LSB first)
  NotificationBuf[5] = 0x00;
  NotificationBuf[6] = 0x02;                           // wLength (Data length = 2 bytes, LSB first)
  NotificationBuf[7] = 0x00; 
  NotificationBuf[8] = (CDC_SerialState >>  0) & 0xFF; // UART State Bitmap (16bits, LSB first)
  NotificationBuf[9] = (CDC_SerialState >>  8) & 0xFF;

  USB_WriteEP (CDC_CEP_IN, &NotificationBuf[0], 10);   // send notification
}
