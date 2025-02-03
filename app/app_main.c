/*
File:   app_main.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** Contains the dll entry point and all exported functions that the platform
	** layer can lookup by name and call. Also includes all other source files
	** required for the application to compile.
*/

#include "build_config.h"
#include "defines.h"
#define PIG_CORE_IMPLEMENTATION BUILD_INTO_SINGLE_UNIT

#include "base/base_all.h"
#include "std/std_all.h"
#include "os/os_all.h"
#include "misc/misc_all.h"
#include "mem/mem_all.h"
#include "struct/struct_all.h"
#include "gfx/gfx_all.h"
#include "gfx/gfx_system_global.h"

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "platform_interface.h"
#include "app_main.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
static AppData* app = nullptr;

#if !BUILD_INTO_SINGLE_UNIT //NOTE: The platform layer already has these globals
static PlatformInfo* platformInfo = nullptr;
static PlatformApi* platform = nullptr;
static Arena* stdHeap = nullptr;
#endif

// +--------------------------------------------------------------+
// |                         Source Files                         |
// +--------------------------------------------------------------+
#include "app_sokol.c"

// +==============================+
// |           DllMain            |
// +==============================+
#if (TARGET_IS_WINDOWS && !BUILD_INTO_SINGLE_UNIT)
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason,    // reason for calling function
	LPVOID lpReserved)
{
	UNUSED(hinstDLL);
	UNUSED(lpReserved);
	switch(fdwReason)
	{ 
		case DLL_PROCESS_ATTACH: break;
		case DLL_PROCESS_DETACH: break;
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
		default: break;
	}
	//If we don't return TRUE here then the LoadLibraryA call will return a failure!
	return TRUE;
}
#endif //(TARGET_IS_WINDOWS && !BUILD_INTO_SINGLE_UNIT)

void UpdateDllGlobals(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr)
{
	#if !BUILD_INTO_SINGLE_UNIT
	platformInfo = inPlatformInfo;
	platform = inPlatformApi;
	stdHeap = inPlatformInfo->platformStdHeap;
	#else
	UNUSED(inPlatformApi);
	UNUSED(inPlatformInfo);
	#endif
	app = (AppData*)memoryPntr;
}

Texture LoadTexture(Arena* arena, const char* path)
{
	ScratchBegin1(scratch, arena);
	Slice fileContents = Slice_Empty;
	bool readFileResult = OsReadFile(FilePathLit(path), scratch, false, &fileContents);
	Assert(readFileResult);
	ImageData imageData = ZEROED;
	Result parseResult = TryParseImageFile(fileContents, scratch, &imageData);
	Assert(parseResult == Result_Success);
	Texture result = InitTexture(arena, GetFileNamePart(FilePathLit(path), true), imageData.size, imageData.pixels, TextureFlag_IsRepeating);
	ScratchEnd(scratch);
	return result;
}

// +==============================+
// |           AppInit            |
// +==============================+
// void* AppInit(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi)
EXPORT_FUNC(AppInit) APP_INIT_DEF(AppInit)
{
	#if !BUILD_INTO_SINGLE_UNIT
	InitScratchArenasVirtual(Gigabytes(4));
	#endif
	ScratchBegin(scratch);
	ScratchBegin1(scratch2, scratch);
	ScratchBegin2(scratch3, scratch, scratch2);
	
	AppData* appData = AllocType(AppData, inPlatformInfo->platformStdHeap);
	ClearPointer(appData);
	UpdateDllGlobals(inPlatformInfo, inPlatformApi, (void*)appData);
	
	InitSokol();
	
	// sphereBuffer; TODO:
	
	app->albedoTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Base_Color.png");
	app->normalTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Normal_OpenGL.png");
	app->metallicTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Metallic.png");
	app->roughnessTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Roughness.png");
	app->occlusionTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Mixed_AO.png");
	// app->occlusionTexture = LoadTexture(stdHeap, "test_texture.png");
	
	app->spherePos = V3_Zero;
	app->sphereRadius = 0.5f;
	
	app->cameraPos = NewV3(3, 0.5f, 2);
	app->cameraLookDir = Normalize(Sub(app->spherePos, app->cameraPos));
	
	app->lightPos = NewV3(-0.5f, 0.8f, 3);
	
	app->initialized = true;
	ScratchEnd(scratch);
	ScratchEnd(scratch2);
	ScratchEnd(scratch3);
	return (void*)app;
}

// +==============================+
// |          AppUpdate           |
// +==============================+
// bool AppUpdate(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr)
EXPORT_FUNC(AppUpdate) APP_UPDATE_DEF(AppUpdate)
{
	ScratchBegin(scratch);
	ScratchBegin1(scratch2, scratch);
	ScratchBegin2(scratch3, scratch, scratch2);
	bool shouldContinueRunning = true;
	UpdateDllGlobals(inPlatformInfo, inPlatformApi, memoryPntr);
	IncrementU64By(app->frameIndex, 16);
	
	v2 windowSize = ToV2Fromi(platform->GetWindowSize());
	
	r32 angle = OscillateBy(app->frameIndex, 0, TwoPi32, 5000, 0);
	r32 height = OscillateBy(app->frameIndex, -1.0f, 1.0f, 13000, 0);
	app->cameraPos = Add(app->spherePos, NewV3(CosR32(angle) * 4.5f, height, SinR32(angle) * 4.5f));
	app->cameraLookDir = Normalize(Sub(app->spherePos, app->cameraPos));
	
	BeginFrame(platform->GetSokolSwapchain(), MonokaiBack, 1.0f);
	{
		BindShader(&app->main3dShader);
		BindTexture(&app->occlusionTexture);
		SetSourceRec(NewV4(0, 0, (r32)app->occlusionTexture.Width, (r32)app->occlusionTexture.Height));
		
		#if 0
		mat4 projMat = Mat4_Identity;
		TransformMat4(&projMat, MakeScaleXYZMat4(1.0f/(windowSize.Width/2.0f), 1.0f/(windowSize.Height/2.0f), 1.0f));
		TransformMat4(&projMat, MakeTranslateXYZMat4(-1.0f, -1.0f, 0.0f));
		TransformMat4(&projMat, MakeScaleYMat4(-1.0f));
		SetProjectionMat(projMat);
		SetViewMat(Mat4_Identity);
		SetWorldMat(Mat4_Identity);
		// SetUniformByNameV2(StrLit("main2d_texture0_size"), ToV2Fromi(app->gradientTexture.size));
		#endif
		
		#if defined(SOKOL_GLCORE)
		mat4 projMat = MakePerspectiveMat4Gl(ToRadians32(30), windowSize.Width/windowSize.Height, 0.05f, 100);
		#else
		mat4 projMat = MakePerspectiveMat4Dx(ToRadians32(30), windowSize.Width/windowSize.Height, 0.05f, 100);
		#endif
		SetProjectionMat(projMat);
		mat4 viewMat = MakeLookAtMat4(app->cameraPos, Add(app->cameraPos, app->cameraLookDir), V3_Up);
		SetViewMat(viewMat);
		
		#if 0
		v2 tileSize = NewV2(0.1f, 0.1f); //ToV2Fromi(app->gradientTexture.size); //NewV2(48, 27);
		i32 numColumns = 100; //CeilR32i(windowSize.Width / tileSize.Width);
		i32 numRows = 100; //CeilR32i(windowSize.Height / tileSize.Height);
		for (i32 yIndex = 0; yIndex < numRows; yIndex++)
		{
			for (i32 xIndex = 0; xIndex < numColumns; xIndex++)
			{
				DrawRectangle(NewV2(tileSize.Width * xIndex, tileSize.Height * yIndex), tileSize, White);
			}
		}
		#endif
		
		// DrawBox(NewBoxV(Sub(app->spherePos, FillV3(app->sphereRadius)), FillV3(app->sphereRadius*2)), White);
		DrawSphere(NewSphereV(app->spherePos, app->sphereRadius), White);
		DrawBox(NewBoxV(Add(Sub(app->spherePos, FillV3(app->sphereRadius)), NewV3(2.0f*1, 0, 0)), FillV3(app->sphereRadius*2)), White);
	}
	EndFrame();
	
	ScratchEnd(scratch);
	ScratchEnd(scratch2);
	ScratchEnd(scratch3);
	return shouldContinueRunning;
}

// +==============================+
// |          AppGetApi           |
// +==============================+
// AppApi AppGetApi()
EXPORT_FUNC(AppGetApi) APP_GET_API_DEF(AppGetApi)
{
	AppApi result = ZEROED;
	result.AppInit = AppInit;
	result.AppUpdate = AppUpdate;
	return result;
}
