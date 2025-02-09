/*
File:   platform_main.c
Author: Taylor Robbins
Date:   01\19\2025
Description: 
	** Holds the main entry point for the application and #includes all source files
	** that are needed for the platform layer to be compiled to an executable.
	** If BUILD_INTO_SINGLE_UNIT then this file #includes app_main.c as well.
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

#define SOKOL_APP_IMPL
#if TARGET_IS_LINUX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers" //warning: missing field 'revents' initializer [-Wmissing-field-initializers]
#endif
#include "third_party/sokol/sokol_app.h"
#if TARGET_IS_LINUX
#pragma clang diagnostic pop
#endif

#include "misc/misc_sokol_app_helpers.c"

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "platform_interface.h"
#include "platform_main.h"

#if BUILD_INTO_SINGLE_UNIT
EXPORT_FUNC(AppGetApi) APP_GET_API_DEF(AppGetApi);
#endif

// +--------------------------------------------------------------+
// |                       Platform Globals                       |
// +--------------------------------------------------------------+
PlatformData* platformData = nullptr;
//These globals are shared between app and platform when BUILD_INTO_SINGLE_UNIT
Arena* stdHeap = nullptr;
PlatformInfo* platformInfo = nullptr;
PlatformApi* platform = nullptr;

// +--------------------------------------------------------------+
// |                    Platform Source Files                     |
// +--------------------------------------------------------------+
#include "platform_api.c"

// +--------------------------------------------------------------+
// |                       Main Entry Point                       |
// +--------------------------------------------------------------+
void PlatSappInit(void)
{
	Arena stdHeapLocal = ZEROED;
	InitArenaStdHeap(&stdHeapLocal);
	platformData = AllocType(PlatformData, &stdHeapLocal);
	NotNull(platformData);
	ClearPointer(platformData);
	MyMemCopy(&platformData->stdHeap, &stdHeapLocal, sizeof(Arena));
	stdHeap = &platformData->stdHeap;
	InitScratchArenasVirtual(Gigabytes(4));
	
	ScratchBegin(loadScratch);
	
	InitKeyboardState(&platformData->appInputs[0].keyboard);
	InitKeyboardState(&platformData->appInputs[1].keyboard);
	InitMouseState(&platformData->appInputs[0].mouse);
	InitMouseState(&platformData->appInputs[1].mouse);
	platformData->currentAppInput = &platformData->appInputs[0];
	platformData->oldAppInput = &platformData->appInputs[1];
	
	platformInfo = AllocType(PlatformInfo, stdHeap);
	NotNull(platformInfo);
	ClearPointer(platformInfo);
	platformInfo->platformStdHeap = stdHeap;
	
	platform = AllocType(PlatformApi, stdHeap);
	NotNull(platform);
	ClearPointer(platform);
	platform->GetSokolSwapchain = Plat_GetSokolSwapchain;
	platform->SetMouseLocked = Plat_SetMouseLocked;
	
	#if BUILD_INTO_SINGLE_UNIT
	{
		WriteLine_N("Compiled as single unit!");
		platformData->appApi = AppGetApi();
	}
	#else
	{
		#if TARGET_IS_WINDOWS
		FilePath dllPath = StrLit(PROJECT_DLL_NAME_STR ".dll");
		#elif TARGET_IS_LINUX
		FilePath dllPath = StrLit("./" PROJECT_DLL_NAME_STR ".so");
		#else
		#error Current TARGET doesn't have an implementation for shared library suffix!
		#endif
		Result loadDllResult = OsLoadDll(dllPath, &platformData->appDll);
		if (loadDllResult != Result_Success) { PrintLine_E("Failed to load \"%.*s\": %s", StrPrint(dllPath), GetResultStr(loadDllResult)); }
		Assert(loadDllResult == Result_Success);
		
		AppGetApi_f* appGetApi = (AppGetApi_f*)OsFindDllFunc(&platformData->appDll, StrLit("AppGetApi"));
		NotNull(appGetApi);
		platformData->appApi = appGetApi();
		NotNull(platformData->appApi.AppInit);
		NotNull(platformData->appApi.AppUpdate);
	}
	#endif
	
	//TODO: Should we do an early call into app dll to get options?
	
	InitSokolGraphics((sg_desc){
		// .buffer_pool_size = ?; //int
		// .image_pool_size = ?; //int
		// .sampler_pool_size = ?; //int
		// .shader_pool_size = ?; //int
		// .pipeline_pool_size = ?; //int
		// .attachments_pool_size = ?; //int
		// .uniform_buffer_size = ?; //int
		// .max_commit_listeners = ?; //int
		// .disable_validation = ?; //bool    // disable validation layer even in debug mode, useful for tests
		// .d3d11_shader_debugging = ?; //bool    // if true, HLSL shaders are compiled with D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
		// .mtl_force_managed_storage_mode = ?; //bool // for debugging: use Metal managed storage mode for resources even with UMA
		// .mtl_use_command_buffer_with_retained_references = ?; //bool    // Metal: use a managed MTLCommandBuffer which ref-counts used resources
		// .wgpu_disable_bindgroups_cache = ?; //bool  // set to true to disable the WebGPU backend BindGroup cache
		// .wgpu_bindgroups_cache_size = ?; //int      // number of slots in the WebGPU bindgroup cache (must be 2^N)
		// .allocator = ?; //sg_allocator TODO: Fill this out!
		.environment = CreateSokolAppEnvironment(),
		.logger.func = SokolLogCallback,
		
	});
	
	InitGfxSystem(stdHeap, &gfx);
	#if DEBUG_BUILD
	gfx.prevFontFlow.numGlyphsAlloc = 256;
	gfx.prevFontFlow.glyphs = AllocArray(FontFlowGlyph, stdHeap, gfx.prevFontFlow.numGlyphsAlloc);
	NotNull(gfx.prevFontFlow.glyphs);
	#endif
	
	platformData->appMemoryPntr = platformData->appApi.AppInit(platformInfo, platform);
	NotNull(platformData->appMemoryPntr);
	
	ScratchEnd(loadScratch);
}

void PlatSappCleanup(void)
{
	ShutdownSokolGraphics();
}

void PlatSappEvent(const sapp_event* event)
{
	bool handledEvent = false;
	
	if (platformData->currentAppInput != nullptr)
	{
		handledEvent = HandleSokolKeyboardAndMouseEvents(
			event,
			platformData->currentAppInput->programTime, //TODO: Calculate a more accurate programTime to pass here!
			&platformData->currentAppInput->keyboard,
			&platformData->currentAppInput->mouse,
			sapp_mouse_locked()
		);
	}
	
	if (!handledEvent)
	{
		switch (event->type)
		{
			case SAPP_EVENTTYPE_CHAR:              /*WriteLine_D("Event: CHAR");*/          break;
			case SAPP_EVENTTYPE_TOUCHES_BEGAN:     WriteLine_D("Event: TOUCHES_BEGAN");     break;
			case SAPP_EVENTTYPE_TOUCHES_MOVED:     WriteLine_D("Event: TOUCHES_MOVED");     break;
			case SAPP_EVENTTYPE_TOUCHES_ENDED:     WriteLine_D("Event: TOUCHES_ENDED");     break;
			case SAPP_EVENTTYPE_TOUCHES_CANCELLED: WriteLine_D("Event: TOUCHES_CANCELLED"); break;
			case SAPP_EVENTTYPE_RESIZED:           /*PrintLine_D("Event: RESIZED %dx%d / %dx%d", event->window_width, event->window_height, event->framebuffer_width, event->framebuffer_height);*/ break;
			case SAPP_EVENTTYPE_ICONIFIED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isMinimized == false)
				{
					platformData->currentAppInput->isMinimized = true;
					platformData->currentAppInput->isMinimizedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_RESTORED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isMinimized == true)
				{
					platformData->currentAppInput->isMinimized = false;
					platformData->currentAppInput->isMinimizedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_FOCUSED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isFocused == false)
				{
					platformData->currentAppInput->isFocused = true;
					platformData->currentAppInput->isFocusedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_UNFOCUSED:
			{
				if (platformData->currentAppInput != nullptr && platformData->currentAppInput->isFocused == true)
				{
					platformData->currentAppInput->isFocused = false;
					platformData->currentAppInput->isFocusedChanged = true;
				}
			} break;
			case SAPP_EVENTTYPE_SUSPENDED:         WriteLine_D("Event: SUSPENDED");         break;
			case SAPP_EVENTTYPE_RESUMED:           WriteLine_D("Event: RESUMED");           break;
			case SAPP_EVENTTYPE_QUIT_REQUESTED:    WriteLine_D("Event: QUIT_REQUESTED");    break;
			case SAPP_EVENTTYPE_CLIPBOARD_PASTED:  WriteLine_D("Event: CLIPBOARD_PASTED");  break;
			case SAPP_EVENTTYPE_FILES_DROPPED:     WriteLine_D("Event: FILES_DROPPED");     break;
			default: PrintLine_D("Event: UNKNOWN(%d)", event->type); break;
		}
	}
}

void PlatSappFrame(void)
{
	//TODO: Check for dll changes, reload it!
	
	//Swap which appInput is being written to and pass the static version to the application
	AppInput* oldAppInput = platformData->currentAppInput;
	AppInput* newAppInput = (platformData->currentAppInput == &platformData->appInputs[0]) ? &platformData->appInputs[1] : &platformData->appInputs[0];
	
	v2i newScreenSize = NewV2i(sapp_width(), sapp_height());
	if (!AreEqual(newScreenSize, oldAppInput->screenSize)) { oldAppInput->screenSizeChanged = true; }
	oldAppInput->screenSize = newScreenSize;
	bool newIsFullScreen = sapp_is_fullscreen();
	if (oldAppInput->isFullscreen != newIsFullScreen) { oldAppInput->isFullscreenChanged = true; }
	oldAppInput->isFullscreen = newIsFullScreen;
	
	MyMemCopy(newAppInput, oldAppInput, sizeof(AppInput));
	newAppInput->screenSizeChanged = false;
	newAppInput->isFullscreenChanged = false;
	newAppInput->isMinimizedChanged = false;
	newAppInput->isFocusedChanged = false;
	RefreshKeyboardState(&newAppInput->keyboard);
	RefreshMouseState(&newAppInput->mouse, sapp_mouse_locked(), NewV2(sapp_widthf()/2.0f, sapp_heightf()/2.0f));
	IncrementU64(newAppInput->frameIndex);
	IncrementU64By(newAppInput->programTime, 16); //TODO: Replace this hardcoded increment!
	platformData->oldAppInput = oldAppInput;
	platformData->currentAppInput = newAppInput;
	
	bool shouldContinueRunning = platformData->appApi.AppUpdate(platformInfo, platform, platformData->appMemoryPntr, oldAppInput);
	if (!shouldContinueRunning) { sapp_quit(); }
}

sapp_desc sokol_main(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	return (sapp_desc){
		.init_cb = PlatSappInit,
		.frame_cb = PlatSappFrame,
		.cleanup_cb = PlatSappCleanup,
		.event_cb = PlatSappEvent,
		.width = 800,
		.height = 600,
		.window_title = "Loading...",
		.icon.sokol_default = true,
		.logger.func = SokolLogCallback,
	};
}

#if BUILD_INTO_SINGLE_UNIT
#include "app/app_main.c"
#endif
