#ifndef _HIDAPI_H_
#define _HIDAPI_H_

#include <windows.h>

#if defined( __MINGW64__ )
#include <hidsdi.h>

BOOLEAN __stdcall HidD_GetPreparsedData(
  HANDLE HidDeviceObject,
  PHIDP_PREPARSED_DATA *PreparsedData
);

void __stdcall HidD_GetHidGuid(
  LPGUID HidGuid
);

#elif defined( MINGW ) || defined( __MINGW32__ )
#include <ddk/hidsdi.h>
#endif

#include <setupapi.h>

#ifdef __DLL__ 
	#define HIDAPI_FUNC __declspec(dllexport)
#else 
	#define HIDAPI_FUNC 
#endif

#define REPORT_SIZE				0x40
#define TEST_CYCLES				1000

typedef struct
{
	unsigned short r_in;
	unsigned short r_out;
	unsigned short f_inout;
} hid_size_info_t, *phid_size_info_t;

typedef struct
{
	HANDLE hDeviceHandle;
	GUID HidGuid;
	ULONG Length;
	HANDLE hDevInfo;
	ULONG Required;
	HIDP_CAPS Capabilities;
	OVERLAPPED HIDOverlapped;
	DWORD dwNumberOfBytesRead;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;
	BOOL DeviceDetected;
} hid_context, *phid_context;

HIDAPI_FUNC int  hid_find(phid_context hid, unsigned short wVendorID, unsigned short wProductID, unsigned short wUsagePage);
HIDAPI_FUNC int  hid_read(phid_context hid, unsigned char * buffer, unsigned int len);
HIDAPI_FUNC int  hid_write(phid_context hid, unsigned char * buffer, unsigned int len);
HIDAPI_FUNC void hid_close(phid_context hid);
HIDAPI_FUNC void hid_getsize(phid_context hid, phid_size_info_t ps);

#endif //_HIDAPI_H_
