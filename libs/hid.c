#include <stdio.h>

#include "hid.h"

#ifdef TEST
	#define DBG printf
#else
	#define DBG(...)
#endif

hid_context device = { NULL };

static void GetDeviceCapabilities(phid_context hid)
{
	PHIDP_PREPARSED_DATA PreparsedData;
	HidD_GetPreparsedData(hid->hDeviceHandle, &PreparsedData);
	HidP_GetCaps(PreparsedData, &hid->Capabilities);

	DBG("%s%X\n", "Usage Page:                      ", hid->Capabilities.UsagePage);
	DBG("%s%d\n", "Input Report Byte Length:        ", hid->Capabilities.InputReportByteLength);
	DBG("%s%d\n", "Output Report Byte Length:       ", hid->Capabilities.OutputReportByteLength);
	DBG("%s%d\n", "Feature Report Byte Length:      ", hid->Capabilities.FeatureReportByteLength);
	DBG("%s%d\n", "Number of Link Collection Nodes: ", hid->Capabilities.NumberLinkCollectionNodes);
	DBG("%s%d\n", "Number of Input Button Caps:     ", hid->Capabilities.NumberInputButtonCaps);
	DBG("%s%d\n", "Number of InputValue Caps:       ", hid->Capabilities.NumberInputValueCaps);
	DBG("%s%d\n", "Number of InputData Indices:     ", hid->Capabilities.NumberInputDataIndices);
	DBG("%s%d\n", "Number of Output Button Caps:    ", hid->Capabilities.NumberOutputButtonCaps);
	DBG("%s%d\n", "Number of Output Value Caps:     ", hid->Capabilities.NumberOutputValueCaps);
	DBG("%s%d\n", "Number of Output Data Indices:   ", hid->Capabilities.NumberOutputDataIndices);
	DBG("%s%d\n", "Number of Feature Button Caps:   ", hid->Capabilities.NumberFeatureButtonCaps);
	DBG("%s%d\n", "Number of Feature Value Caps:    ", hid->Capabilities.NumberFeatureValueCaps);
	DBG("%s%d\n", "Number of Feature Data Indices:  ", hid->Capabilities.NumberFeatureDataIndices);
}

HIDAPI_FUNC int hid_find(phid_context hid, unsigned short wVendorID, unsigned short wProductID, unsigned short wUsagePage)
{
	HIDD_ATTRIBUTES Attributes;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	BOOL bLastDevice = FALSE;
	int MemberIndex = 0;
	BOOL bMyDeviceDetected = FALSE; 
	LONG lResult;

	// 
	hid->Length = 0;
	hid->detailData = NULL;
	hid->hDeviceHandle=NULL;

	// 
	HidD_GetHidGuid(&hid->HidGuid);
	// 
	hid->hDevInfo = SetupDiGetClassDevs(&hid->HidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);

	devInfoData.cbSize = sizeof(devInfoData);
	MemberIndex = 0;
	bLastDevice = FALSE;

	do
	{
		bMyDeviceDetected = FALSE;
		lResult = SetupDiEnumDeviceInterfaces(hid->hDevInfo, 0, &hid->HidGuid, MemberIndex, &devInfoData);
		if (lResult != 0)
		{
			// 
			lResult = SetupDiGetDeviceInterfaceDetail(hid->hDevInfo, &devInfoData, NULL, 0, &hid->Length, NULL);
			// 
			hid->detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(hid->Length);
			// 
			hid->detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			// 
			lResult = SetupDiGetDeviceInterfaceDetail(hid->hDevInfo, &devInfoData, hid->detailData, hid->Length, &hid->Required, NULL);
			// 
			hid->hDeviceHandle = CreateFile(hid->detailData->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,	0, NULL);
			// 
			Attributes.Size = sizeof(Attributes);
			// 
			lResult = HidD_GetAttributes(hid->hDeviceHandle, &Attributes);
			// 
			bMyDeviceDetected = FALSE;
			// 
			if (Attributes.VendorID == wVendorID)
			{
				if (Attributes.ProductID == wProductID)
				{
					GetDeviceCapabilities(hid);
					if (hid->Capabilities.UsagePage == wUsagePage)
					{
						bMyDeviceDetected = TRUE;
					}
					else
						CloseHandle(hid->hDeviceHandle);
				}
				else
					CloseHandle(hid->hDeviceHandle);
			}
			else
				CloseHandle(hid->hDeviceHandle);
			free(hid->detailData);
		}
		else
		{
			bLastDevice = TRUE;
		}
		MemberIndex = MemberIndex + 1;
	}
	while ((bLastDevice == FALSE) && (bMyDeviceDetected == FALSE));

	// 
	SetupDiDestroyDeviceInfoList(hid->hDevInfo);

	return bMyDeviceDetected;
}

HIDAPI_FUNC void hid_getsize(phid_context hid, phid_size_info_t ps)
{
	if (hid->hDeviceHandle == NULL)
		return;

	// 
	GetDeviceCapabilities(hid);

	// 
	ps->r_in    = hid->Capabilities.InputReportByteLength-1;
	ps->r_out   = hid->Capabilities.OutputReportByteLength-1;
	ps->f_inout = hid->Capabilities.FeatureReportByteLength-1;
}

HIDAPI_FUNC int hid_read(phid_context hid, unsigned char *buffer, unsigned int len)
{
	unsigned char *pbHIDBuffer = (unsigned char*)malloc(hid->Capabilities.InputReportByteLength);
	int i;
	int iRet;

	if (hid->hDeviceHandle == NULL)
		return 0;

	if (ReadFile(hid->hDeviceHandle, pbHIDBuffer, hid->Capabilities.InputReportByteLength, &hid->dwNumberOfBytesRead,  NULL))
	{
		for (i = 1; i < hid->Capabilities.InputReportByteLength && i < len+1; i++)
		{
			buffer[i-1] = pbHIDBuffer[i];
		}
		iRet = 1;
	}
	else
		iRet = 0;
	
	free(pbHIDBuffer);
	
	return iRet;
}

HIDAPI_FUNC int hid_write(phid_context hid, unsigned char *buffer, unsigned int len)
{
	unsigned char *pbHIDBuffer = (unsigned char*)malloc(hid->Capabilities.OutputReportByteLength);
	int i;
	int iRet;

	if (hid->hDeviceHandle == NULL)
		return 1;

	pbHIDBuffer[0] = 0;	
	for (i = 1; i < hid->Capabilities.OutputReportByteLength && i < len+1; i++)
	{
		pbHIDBuffer[i] = buffer[i-1];
	}
	iRet = WriteFile(hid->hDeviceHandle, pbHIDBuffer, hid->Capabilities.OutputReportByteLength, &hid->dwNumberOfBytesRead,  NULL);
	free(pbHIDBuffer);
	return iRet;
	
}

HIDAPI_FUNC void hid_close(phid_context hid)
{
	if (hid->hDeviceHandle == NULL)
		return;
	CloseHandle(hid->hDeviceHandle);
}

