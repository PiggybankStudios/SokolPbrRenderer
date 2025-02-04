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
#include "input/input_all.h"
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
static AppInput* appIn = nullptr;

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

void UpdateDllGlobals(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr, AppInput* appInput)
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
	appIn = appInput;
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
	UpdateDllGlobals(inPlatformInfo, inPlatformApi, (void*)appData, nullptr);
	
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
// bool AppUpdate(PlatformInfo* inPlatformInfo, PlatformApi* inPlatformApi, void* memoryPntr, AppInput* appInput)
EXPORT_FUNC(AppUpdate) APP_UPDATE_DEF(AppUpdate)
{
	ScratchBegin(scratch);
	ScratchBegin1(scratch2, scratch);
	ScratchBegin2(scratch3, scratch, scratch2);
	bool shouldContinueRunning = true;
	UpdateDllGlobals(inPlatformInfo, inPlatformApi, memoryPntr, appInput);
	
	if (IsMouseBtnPressed(&appIn->mouse, MouseBtn_Left) && appIn->mouse.isOverWindow && !appIn->mouse.isLocked)
	{
		platform->SetMouseLocked(true);
	}
	if (IsKeyboardKeyPressed(&appIn->keyboard, Key_Escape) && appIn->mouse.isLocked)
	{
		platform->SetMouseLocked(false);
	}
	
	if (appIn->mouse.isLocked)
	{
		r32 cameraHoriRot = AtanR32(app->cameraLookDir.Z, app->cameraLookDir.X);
		r32 cameraVertRot = AtanR32(app->cameraLookDir.Y, Length(NewV2(app->cameraLookDir.X, app->cameraLookDir.Z)));
		cameraHoriRot = AngleFixR32(cameraHoriRot - appIn->mouse.lockedPosDelta.X / 500.0f);
		cameraVertRot = ClampR32(cameraVertRot - appIn->mouse.lockedPosDelta.Y / 500.0f, -HalfPi32+0.05f, HalfPi32-0.05f);
		r32 horizontalRadius = CosR32(cameraVertRot);
		app->cameraLookDir = NewV3(CosR32(cameraHoriRot) * horizontalRadius, SinR32(cameraVertRot), SinR32(cameraHoriRot) * horizontalRadius);
	}
	#if 0
	else
	{
		r32 angle = OscillateBy(appIn->programTime, 0, TwoPi32, 5000, 0);
		r32 height = OscillateBy(appIn->programTime, -1.0f, 1.0f, 13000, 0);
		app->cameraPos = Add(app->spherePos, NewV3(CosR32(angle) * 4.5f, height, SinR32(angle) * 4.5f));
		app->cameraLookDir = Normalize(Sub(app->spherePos, app->cameraPos));
	}
	#endif
	
	v3 horizontalForwardVec = Normalize(NewV3(app->cameraLookDir.X, 0.0f, app->cameraLookDir.Z));
	v3 horizontalRightVec = Normalize(NewV3(app->cameraLookDir.Z, 0.0f, -app->cameraLookDir.X));
	const r32 moveSpeed = IsKeyboardKeyDown(&appIn->keyboard, Key_Shift) ? 0.08f : 0.02f;
	if (IsKeyboardKeyDown(&appIn->keyboard, Key_W)) { app->cameraPos = Add(app->cameraPos, Mul(horizontalForwardVec, moveSpeed)); }
	if (IsKeyboardKeyDown(&appIn->keyboard, Key_A)) { app->cameraPos = Add(app->cameraPos, Mul(horizontalRightVec, -moveSpeed)); }
	if (IsKeyboardKeyDown(&appIn->keyboard, Key_S)) { app->cameraPos = Add(app->cameraPos, Mul(horizontalForwardVec, -moveSpeed)); }
	if (IsKeyboardKeyDown(&appIn->keyboard, Key_D)) { app->cameraPos = Add(app->cameraPos, Mul(horizontalRightVec, moveSpeed)); }
	if (IsKeyboardKeyDown(&appIn->keyboard, Key_E)) { app->cameraPos = Add(app->cameraPos, Mul(V3_Up, moveSpeed)); }
	if (IsKeyboardKeyDown(&appIn->keyboard, Key_Q)) { app->cameraPos = Add(app->cameraPos, Mul(V3_Down, moveSpeed)); }
	
	BeginFrame(platform->GetSokolSwapchain(), MonokaiBack, 1.0f);
	{
		// BindShader(&app->main3dShader);
		BindShader(&app->pbrShader);
		BindTextureAtIndex(&app->albedoTexture, 0);
		BindTextureAtIndex(&app->normalTexture, 1);
		BindTextureAtIndex(&app->metallicTexture, 2);
		BindTextureAtIndex(&app->roughnessTexture, 3);
		BindTextureAtIndex(&app->occlusionTexture, 4);
		SetSourceRec(NewV4(0, 0, (r32)app->occlusionTexture.Width, (r32)app->occlusionTexture.Height));
		SetShaderUniformByNameV4(&app->pbrShader, StrLit("lightPos"), ToV4From3(app->lightPos, 1.0f));
		SetShaderUniformByNameV4(&app->pbrShader, StrLit("cameraPos"), ToV4From3(app->cameraPos, 1.0f));
		
		#if defined(SOKOL_GLCORE)
		mat4 projMat = MakePerspectiveMat4Gl(ToRadians32(45), (r32)appIn->screenSize.Width/(r32)appIn->screenSize.Height, 0.05f, 25);
		#else
		mat4 projMat = MakePerspectiveMat4Dx(ToRadians32(45), (r32)appIn->screenSize.Width/(r32)appIn->screenSize.Height, 0.05f, 25);
		#endif
		SetProjectionMat(projMat);
		mat4 viewMat = MakeLookAtMat4(app->cameraPos, Add(app->cameraPos, app->cameraLookDir), V3_Up);
		SetViewMat(viewMat);
		
		// DrawBox(NewBoxV(Sub(app->spherePos, FillV3(app->sphereRadius)), FillV3(app->sphereRadius*2)), White);
		DrawSphere(NewSphereV(app->spherePos, app->sphereRadius), White);
		DrawBox(NewBoxV(Add(Sub(app->spherePos, FillV3(app->sphereRadius)), NewV3(2.0f*1, 0, 0)), FillV3(app->sphereRadius*2)), White);
		DrawBox(NewBoxV(Sub(app->lightPos, FillV3(0.05f)), FillV3(0.1f)), White);
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
