#pragma once

#if defined (LINUX)
#include "../include/adl_sdk.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <stdlib.h>	
#include <string.h>	//memeset
#include <unistd.h>	//sleep

#else
#include <windows.h>
#include <tchar.h>
#include "..\adl\adl_sdk.h"
#endif

#include <stdio.h>

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int(*ADL_DISPLAY_COLORCAPS_GET) (int, int, int *, int *);
typedef int(*ADL_DISPLAY_COLOR_GET) (int, int, int, int *, int *, int *, int *, int *);
typedef int(*ADL_DISPLAY_COLOR_SET) (int, int, int, int);
typedef int(*ADL_DISPLAY_DISPLAYINFO_GET) (int, int *, ADLDisplayInfo **, int);

class amd_saturation_controller
{
private:
#if defined (LINUX)
	void *hDLL;		// Handle to .so library
#else
	HINSTANCE hDLL;		// Handle to DLL
#endif

	ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create;
	ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy;
	ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get;
	ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get;
	ADL_DISPLAY_COLORCAPS_GET        ADL_Display_ColorCaps_Get;
	ADL_DISPLAY_COLOR_GET            ADL_Display_Color_Get;
	ADL_DISPLAY_COLOR_SET            ADL_Display_Color_Set;
	ADL_DISPLAY_DISPLAYINFO_GET      ADL_Display_DisplayInfo_Get;

	LPAdapterInfo     lpAdapterInfo = NULL;
	LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
	int  i, j;
	int  iNumberAdapters;
	int  iAdapterIndex;
	int  iDisplayIndex;
	int  iNumDisplays;
	int  iColorCaps, iValidBits;
	int  iCurrent, iDefault, iMin, iMax, iStep;

	// Memory allocation function
	static void* __stdcall ADL_Main_Memory_Alloc(int iSize)
	{
		void* lpBuffer = malloc(iSize);
		return lpBuffer;
	}

	// Optional Memory de-allocation function
	static void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
	{
		if (NULL != *lpBuffer)
		{
			free(*lpBuffer);
			*lpBuffer = NULL;
		}
	}

#if defined (LINUX)
	// equivalent functions in linux
	void * GetProcAddress(void * pLibrary, const char * name)
	{
		return dlsym(pLibrary, name);
	}

	void Sleep(int time)
	{
		usleep(time * 1000);
	}

#endif
public:
	amd_saturation_controller()
	{

#if defined (LINUX)
		hDLL = dlopen("libatiadlxx.so", RTLD_LAZY | RTLD_GLOBAL);
#else
		hDLL = LoadLibrary(L"atiadlxx.dll");
		if (hDLL == NULL)
			// A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
			// Try to load the 32 bit library (atiadlxy.dll) instead
			hDLL = LoadLibrary(L"atiadlxy.dll");
#endif

		if (NULL == hDLL)
		{
			printf("ADL library not found!\n");
			return;
		}

		ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
		ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
		ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
		ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
		ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hDLL, "ADL_Display_DisplayInfo_Get");
		ADL_Display_ColorCaps_Get = (ADL_DISPLAY_COLORCAPS_GET)GetProcAddress(hDLL, "ADL_Display_ColorCaps_Get");
		ADL_Display_Color_Get = (ADL_DISPLAY_COLOR_GET)GetProcAddress(hDLL, "ADL_Display_Color_Get");
		ADL_Display_Color_Set = (ADL_DISPLAY_COLOR_SET)GetProcAddress(hDLL, "ADL_Display_Color_Set");
		if (NULL == ADL_Main_Control_Create ||
			NULL == ADL_Main_Control_Destroy ||
			NULL == ADL_Adapter_NumberOfAdapters_Get ||
			NULL == ADL_Adapter_AdapterInfo_Get ||
			NULL == ADL_Display_DisplayInfo_Get ||
			NULL == ADL_Display_ColorCaps_Get ||
			NULL == ADL_Display_Color_Get ||
			NULL == ADL_Display_Color_Set)
		{
			printf("ADL's API is missing!\n");
			return;
		}

		// Initialize ADL. The second parameter is 1, which means:
		// retrieve adapter information only for adapters that are physically present and enabled in the system
		if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
		{
			printf("ADL Initialization Error!\n");
			return;
		}

		// Obtain the number of adapters for the system
		if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters))
		{
			printf("Cannot get the number of adapters!\n");
			return;
		}

		if (0 < iNumberAdapters)
		{
			lpAdapterInfo = (LPAdapterInfo)malloc(sizeof (AdapterInfo)* iNumberAdapters);
			memset(lpAdapterInfo, '\0', sizeof (AdapterInfo)* iNumberAdapters);

			// Get the AdapterInfo structure for all adapters in the system
			ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof (AdapterInfo)* iNumberAdapters);
		}
	}

	~amd_saturation_controller()
	{
		ADL_Main_Memory_Free((void **)&lpAdapterInfo);
		ADL_Main_Memory_Free((void **)&lpAdlDisplayInfo);
		ADL_Main_Control_Destroy();

#if defined (LINUX)
		dlclose(hDLL);
#else
		FreeLibrary(hDLL);
#endif
	}

	int get_setting(int logical_display_id, int setting)
	{
		// Repeat for all available adapters in the system
		for (i = 0; i < iNumberAdapters; i++)
		{
			iAdapterIndex = lpAdapterInfo[i].iAdapterIndex;
			ADL_Main_Memory_Free((void **)&lpAdlDisplayInfo);
			if (ADL_OK != ADL_Display_DisplayInfo_Get(lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
				continue;

			for (j = 0; j < iNumDisplays; j++)
			{
				// For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
				if ((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED) !=
					(ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED	&
					lpAdlDisplayInfo[j].iDisplayInfoValue))
					continue;   // Skip the not connected or not mapped displays

				// Is the display mapped to this adapter? This test is too restrictive and may not be needed.
				if (iAdapterIndex != lpAdlDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex)
					continue;

				if (lpAdlDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex != logical_display_id)
					continue;

				iDisplayIndex = lpAdlDisplayInfo[j].displayID.iDisplayLogicalIndex;

				ADL_Display_ColorCaps_Get(iAdapterIndex, iDisplayIndex, &iColorCaps, &iValidBits);

				// Use only the valid bits from iColorCaps
				iColorCaps &= iValidBits;

				// Check if the display supports this particular capability
				if (setting & iColorCaps)
				{
					// Get the Current display Brightness, Default value, Min, Max and Step
					if (ADL_OK == ADL_Display_Color_Get(iAdapterIndex, iDisplayIndex, setting,
						&iCurrent, &iDefault, &iMin, &iMax, &iStep))
					{
						return iCurrent;
					}
				}
			}
		}
		return -1;
	}

	int set_setting(int value, int logical_display_id, int setting)
	{
		// Repeat for all available adapters in the system
		for (i = 0; i < iNumberAdapters; i++)
		{
			iAdapterIndex = lpAdapterInfo[i].iAdapterIndex;
			ADL_Main_Memory_Free((void **)&lpAdlDisplayInfo);
			if (ADL_OK != ADL_Display_DisplayInfo_Get(lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
				continue;

			for (j = 0; j < iNumDisplays; j++)
			{
				// For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
				if ((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED) !=
					(ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED	&
					lpAdlDisplayInfo[j].iDisplayInfoValue))
					continue;   // Skip the not connected or not mapped displays

				// Is the display mapped to this adapter? This test is too restrictive and may not be needed.
				if (iAdapterIndex != lpAdlDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex)
					continue;

				if (lpAdlDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex != logical_display_id)
					continue;

				iDisplayIndex = lpAdlDisplayInfo[j].displayID.iDisplayLogicalIndex;

				ADL_Display_ColorCaps_Get(iAdapterIndex, iDisplayIndex, &iColorCaps, &iValidBits);

				// Use only the valid bits from iColorCaps
				iColorCaps &= iValidBits;

				// Check if the display supports this particular capability
				if (setting & iColorCaps)
				{
					// Get the Current display Brightness, Default value, Min, Max and Step
					if (ADL_OK == ADL_Display_Color_Get(iAdapterIndex, iDisplayIndex, setting,
						&iCurrent, &iDefault, &iMin, &iMax, &iStep))
					{
						if (iCurrent != value)
						{
							ADL_Display_Color_Set(iAdapterIndex, iDisplayIndex, setting, value);
							std::cout << "Changed saturation successfully to " << value << std::endl;
							return value;
						}
						// Set half of the Min brightness for .5 sec
					}
				}
			}
		}
		return -1;
	}
};
