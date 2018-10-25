/*****************************************************************************
 *   config.h:  config file for usbhid_rom example for NXP LPC13xx Family
 *   Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2008.07.19  ver 1.00    Preliminary version, first Release
 *
******************************************************************************/

/*
Overview:
	This example shows how to use the on-chip USB driver in ROM to
	implement a simple HID (Human Interface Device) class USB peripheral.
	To run this example, you must attach a USB cable to the board. See
	the "Getting Started Guide" appendix for details.

User #defines:
   LED_PORT	I/O port driving an LED
   LED_BIT  I/O port bit driving an LED
   LED_ON   Value to set the bit to turn on the LED
   LED_OFF  Value to set the bit to turn off the LED
   LED_TOGGLE_TICKS
		    Number of timer ticks per flash. The timer is configured to generate
		    an interrupt 100 times a second or every 10mS.

How to use:
   Click the debug toolbar button.
   Click the go button.
   Plug the LPCXpresso's target side into a PC using a USB cable retrofit
   or a 3rd party base board.

   * You should be able to control the LED with LPC1343 HID Demonstration.exe
*/

#define USB_VENDOR_ID 0x1FC9	// Vendor ID
#define USB_PROD_ID   0x0007	// Product ID
#define USB_DEVICE    0x0100	// Device ID
