/*
File:   platform_interface.h
Author: Taylor Robbins
Date:   02\02\2025
Description:
	** This file is included by both the application and the platform layer and
	** constitutes all the types and defines they use to communicate between each other
*/

#ifndef _PLATFORM_INTERFACE_H
#define _PLATFORM_INTERFACE_H

typedef struct PlatformInfo PlatformInfo;
struct PlatformInfo
{
	Arena* platformStdHeap;
};

// +--------------------------------------------------------------+
// |                         Platform API                         |
// +--------------------------------------------------------------+
#define GET_SOKOL_SWAPCHAIN_DEF(functionName) sg_swapchain functionName()
typedef GET_SOKOL_SWAPCHAIN_DEF(GetSokolSwapchain_f);

#define GET_WINDOW_SIZE_DEF(functionName) v2i functionName()
typedef GET_WINDOW_SIZE_DEF(GetWindowSize_f);

typedef struct PlatformApi PlatformApi;
struct PlatformApi
{
	GetSokolSwapchain_f* GetSokolSwapchain;
	GetWindowSize_f* GetWindowSize;
};

// +--------------------------------------------------------------+
// |                       App DLL Exports                        |
// +--------------------------------------------------------------+
#define APP_INIT_DEF(functionName) void* functionName(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi)
typedef APP_INIT_DEF(AppInit_f);

#define APP_UPDATE_DEF(functionName) bool functionName(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr)
typedef APP_UPDATE_DEF(AppUpdate_f);

typedef struct AppApi AppApi;
struct AppApi
{
	AppInit_f* AppInit;
	AppUpdate_f* AppUpdate;
};

#define APP_GET_API_DEF(functionName) AppApi functionName()
typedef APP_GET_API_DEF(AppGetApi_f);

#endif //  _PLATFORM_INTERFACE_H
