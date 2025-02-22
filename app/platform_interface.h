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
	Arena* platformStdHeapAllowFreeWithoutSize;
};

typedef struct AppInput AppInput;
struct AppInput
{
	u64 programTime; //num ms since start of program
	u64 frameIndex;
	
	KeyboardState keyboard;
	MouseState mouse;
	//TODO: Add ControllerStates
	
	sapp_mouse_cursor cursorType;
	bool isFullscreen;
	bool isFullscreenChanged;
	bool isMinimized;
	bool isMinimizedChanged;
	bool isFocused;
	bool isFocusedChanged;
	v2i screenSize;
	bool screenSizeChanged;
	// v2i windowSize; //TODO: Can we somehow ask sokol_sapp for the window size (include title bar and border)?
};

// +--------------------------------------------------------------+
// |                         Platform API                         |
// +--------------------------------------------------------------+
#define GET_SOKOL_SWAPCHAIN_DEF(functionName) sg_swapchain functionName()
typedef GET_SOKOL_SWAPCHAIN_DEF(GetSokolSwapchain_f);

#define SET_MOUSE_LOCKED_DEF(functionName) void functionName(bool isMouseLocked)
typedef SET_MOUSE_LOCKED_DEF(SetMouseLocked_f);

#define SET_MOUSE_CURSOR_TYPE_DEF(functionName) void functionName(sapp_mouse_cursor cursorType)
typedef SET_MOUSE_CURSOR_TYPE_DEF(SetMouseCursorType_f);

#define SET_WINDOW_TITLE_DEF(functionName) void functionName(Str8 windowTitle)
typedef SET_WINDOW_TITLE_DEF(SetWindowTitle_f);

#define SET_WINDOW_ICON_DEF(functionName) void functionName(uxx numIconSizes, const ImageData* iconSizes)
typedef SET_WINDOW_ICON_DEF(SetWindowIcon_f);

#define GET_NATIVE_WINDOW_HANDLE_DEF(functionName) const void* functionName()
typedef GET_NATIVE_WINDOW_HANDLE_DEF(GetNativeWindowHandle_f);

typedef struct PlatformApi PlatformApi;
struct PlatformApi
{
	GetSokolSwapchain_f* GetSokolSwapchain;
	SetMouseLocked_f* SetMouseLocked;
	SetMouseCursorType_f* SetMouseCursorType;
	SetWindowTitle_f* SetWindowTitle;
	SetWindowIcon_f* SetWindowIcon;
	GetNativeWindowHandle_f* GetNativeWindowHandle;
};

// +--------------------------------------------------------------+
// |                       App DLL Exports                        |
// +--------------------------------------------------------------+
#define APP_INIT_DEF(functionName) void* functionName(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi)
typedef APP_INIT_DEF(AppInit_f);

#define APP_UPDATE_DEF(functionName) bool functionName(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr, AppInput* appInput)
typedef APP_UPDATE_DEF(AppUpdate_f);

#define APP_CLOSING_DEF(functionName) void functionName(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr)
typedef APP_CLOSING_DEF(AppClosing_f);

typedef struct AppApi AppApi;
struct AppApi
{
	AppInit_f* AppInit;
	AppUpdate_f* AppUpdate;
	AppClosing_f* AppClosing;
};

#define APP_GET_API_DEF(functionName) AppApi functionName()
typedef APP_GET_API_DEF(AppGetApi_f);

#endif //  _PLATFORM_INTERFACE_H
