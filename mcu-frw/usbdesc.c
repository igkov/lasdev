/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbdesc.h
 * Purpose: USB Descriptors Definitions
 * Version: V1.20
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC microcontroller devices only. Nothing else 
 *      gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "lpc13xx.h"
#include "usb.h"
#include "usbdesc.h"
#include "config.h"

/* USB String Descriptor (optional) */
const uint8_t USB_StringDescriptor[] = {
  /* Index 0x00: LANGID Codes */
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(USB_VENDOR_ID), /* US English */    /* wLANGID */
  /* Index 0x04: Manufacturer */
  2+11*2,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'T',0,'E',0,'S',0,'T',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0,
  /* Index 0x20: Product */
  2+10*2,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'I',0,'2',0,'C',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0,
#if 0
  /* Index 0x48: Serial Number */
  0x1A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'D',0,'E',0,'M',0,'O',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,
  /* Index 0x62: Interface 0, Alternate Setting 0 */
  0x0E,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'H',0,'I',0,'D',0,' ',0,' ',0,' ',0,
#endif
  0,
};
