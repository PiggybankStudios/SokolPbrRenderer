/*
File:   platform_main.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _PLATFORM_MAIN_H
#define _PLATFORM_MAIN_H

typedef struct PlatformData PlatformData;
struct PlatformData
{
	Arena stdHeap;
	
	AppApi appApi;
	#if !BUILD_INTO_SINGLE_UNIT
	OsDll appDll;
	#endif
	void* appMemoryPntr;
	
	AppInput appInputs[2];
	AppInput* currentAppInput;
};

#endif //  _PLATFORM_MAIN_H
