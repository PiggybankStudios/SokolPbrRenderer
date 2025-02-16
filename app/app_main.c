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
#include "file_fmt/file_fmt_all.h"
#include "ui/ui_all.h"
#include "gfx/gfx_all.h"
#include "gfx/gfx_system_global.h"

// #define TEST_FONT_NAME "Georgia"
// #define TEST_FONT_NAME "Fira Code"
// #define TEST_FONT_NAME "Consolas"
// #define TEST_FONT_NAME "noto mono"
#define TEST_FONT_NAME "Arial"
#define TEST_FONT_STYLE FontStyleFlag_Bold
#define TEST_FONT_START_SIZE 36.0f

// +--------------------------------------------------------------+
// |                         Header Files                         |
// +--------------------------------------------------------------+
#include "platform_interface.h"
#include "app_main.h"
#include "app_shaders.h"

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
#include "app_helpers.c"

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

ImageData LoadImageData(Arena* arena, const char* path)
{
	ScratchBegin1(scratch, arena);
	Slice fileContents = Slice_Empty;
	bool readFileResult = OsReadFile(FilePathLit(path), scratch, false, &fileContents);
	Assert(readFileResult);
	ImageData imageData = ZEROED;
	Result parseResult = TryParseImageFile(fileContents, arena, &imageData);
	Assert(parseResult == Result_Success);
	ScratchEnd(scratch);
	return imageData;
}
Texture LoadTexture(Arena* arena, const char* path)
{
	ScratchBegin1(scratch, arena);
	ImageData imageData = LoadImageData(scratch, path);
	Texture result = InitTexture(arena, GetFileNamePart(FilePathLit(path), true), imageData.size, imageData.pixels, TextureFlag_IsRepeating);
	ScratchEnd(scratch);
	return result;
}

void LoadWindowIcon()
{
	ScratchBegin(scratch);
	ImageData iconImageDatas[6];
	#if 1
	iconImageDatas[0] = LoadImageData(scratch, "resources/image/icon_16.png");
	iconImageDatas[1] = LoadImageData(scratch, "resources/image/icon_24.png");
	iconImageDatas[2] = LoadImageData(scratch, "resources/image/icon_32.png");
	iconImageDatas[3] = LoadImageData(scratch, "resources/image/icon_64.png");
	iconImageDatas[4] = LoadImageData(scratch, "resources/image/icon_120.png");
	iconImageDatas[5] = LoadImageData(scratch, "resources/image/icon_256.png");
	#else
	iconImageDatas[0] = LoadImageData(scratch, "icon_16.png");
	iconImageDatas[1] = LoadImageData(scratch, "icon_24.png");
	iconImageDatas[2] = LoadImageData(scratch, "icon_32.png");
	iconImageDatas[3] = LoadImageData(scratch, "icon_64.png");
	iconImageDatas[4] = LoadImageData(scratch, "icon_120.png");
	iconImageDatas[5] = LoadImageData(scratch, "icon_256.png");
	#endif
	platform->SetWindowIcon(ArrayCount(iconImageDatas), &iconImageDatas[0]);
	ScratchEnd(scratch);
}

void RasterizeFontAtSize(Font* font, Str8 fontName, r32 fontSize, u8 fontStyleFlags)
{
	ClearFontAtlases(font);
	
	// UNUSED(fontName);
	// AttachTtfFileToFont(font, OsReadBinFileScratch(FilePathLit("consolab.ttf")), fontStyleFlags);
	Result attachResult = AttachOsTtfFileToFont(font, fontName, fontSize, fontStyleFlags);
	Assert(attachResult == Result_Success);
	// OsWriteBinFile(FilePathLit("Default.ttf"), font->ttfFile);
	
	FontCharRange charRanges[] = {
		FontCharRange_ASCII,
		FontCharRange_LatinExt,
	};
	Result bakeResult = BakeFontAtlas(font, fontSize, fontStyleFlags, NewV2i(256, 256), ArrayCount(charRanges), &charRanges[0]);
	if (bakeResult == Result_NotEnoughSpace)
	{
		bakeResult = BakeFontAtlas(font, fontSize, fontStyleFlags, NewV2i(512, 512), ArrayCount(charRanges), &charRanges[0]);
		if (bakeResult == Result_NotEnoughSpace)
		{
			bakeResult = BakeFontAtlas(font, fontSize, fontStyleFlags, NewV2i(1024, 1024), ArrayCount(charRanges), &charRanges[0]);
		}
	}
	Assert(bakeResult == Result_Success);
	
	FillFontKerningTable(font);
	#if 0
	PrintLine_D("Kerning table has %llu entries", (u64)font->kerningTable.numEntries);
	for (uxx eIndex = 0; eIndex < font->kerningTable.numEntries; eIndex++)
	{
		FontKerningTableEntry* entry = &font->kerningTable.entries[eIndex];
		FontGlyph* leftGlyph = nullptr;
		FontGlyph* rightGlyph = nullptr;
		VarArrayLoop(&font->atlases, aIndex)
		{
			VarArrayLoopGet(FontAtlas, atlas, &font->atlases, aIndex);
			VarArrayLoop(&atlas->glyphs, gIndex)
			{
				VarArrayLoopGet(FontGlyph, glyph, &atlas->glyphs, gIndex);
				if (glyph->ttfGlyphIndex >= 0)
				{
					if ((uxx)glyph->ttfGlyphIndex == entry->leftTtfGlyphIndex) { leftGlyph = glyph; }
					if ((uxx)glyph->ttfGlyphIndex == entry->rightTtfGlyphIndex) { rightGlyph = glyph; }
					if (leftGlyph != nullptr && rightGlyph != nullptr) { break; }
				}
			}
			if (leftGlyph != nullptr && rightGlyph != nullptr) { break; }
		}
		if (leftGlyph != nullptr && rightGlyph != nullptr)
		{
			PrintLine_D("%c%c (0x%X/0x%X) = %f",
				(char)leftGlyph->codepoint, (char)rightGlyph->codepoint,
				leftGlyph->codepoint, rightGlyph->codepoint,
				entry->value
			);
		}
		else
		{
			PrintLine_D("%u/%u = %f", entry->leftTtfGlyphIndex, entry->rightTtfGlyphIndex, entry->value);
		}
	}
	#endif
	
	RemoveAttachedTtfFile(font);
}

Model3D LoadModel(FilePath filePath)
{
	Model3D result = ZEROED;
	Result loadResult = TryLoadGltfFile(filePath, stdHeap, &result.data);
	if (loadResult != Result_Success)
	{
		PrintLine_E("Failed to load/parse gltf file at \"%.*s\": %s", StrPrint(filePath), GetResultStr(loadResult));
		return result;
	}
	InitVarArrayWithInitial(Texture, &result.textures, stdHeap, result.data.textures.length);
	VarArrayLoop(&result.data.textures, tIndex)
	{
		VarArrayLoopGet(ModelDataTexture, texture, &result.data.textures, tIndex);
		Texture* newTexture = VarArrayAdd(Texture, &result.textures);
		NotNull(newTexture);
		*newTexture = InitTexture(stdHeap, texture->name, texture->imageData.size, texture->imageData.pixels, TextureFlag_IsRepeating);
		Assert(newTexture->error == Result_Success);
	}
	InitVarArrayWithInitial(VertBuffer, &result.vertBuffers, stdHeap, result.data.parts.length);
	VarArrayLoop(&result.data.parts, pIndex)
	{
		VarArrayLoopGet(ModelDataPart, part, &result.data.parts, pIndex);
		VertBuffer* newVertBuffer = VarArrayAdd(VertBuffer, &result.vertBuffers);
		NotNull(newVertBuffer);
		*newVertBuffer = InitVertBuffer3D(stdHeap, part->name, VertBufferUsage_Static, part->vertices.length, (Vertex3D*)part->vertices.items, false);
		if (part->indices.length > 0) { AddIndicesToVertBufferEx(newVertBuffer, sizeof(i32), part->indices.length, (i32*)part->indices.items, false); }
		Assert(newVertBuffer->error == Result_Success);
	}
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
	
	platform->SetWindowTitle(StrLit("Sokol PBR"));
	LoadWindowIcon();
	
	#if 1
	GeneratedMesh cubeMesh = GenerateVertsForBox(scratch, NewBoxV(V3_Zero, V3_One), White);
	#else
	Color32 cubeSideColors[BOX_NUM_FACES] = { MonokaiWhite, MonokaiRed, MonokaiBlue, MonokaiOrange, MonokaiGreen, MonokaiYellow };
	GeneratedMesh cubeMesh = GenerateVertsForBoxEx(scratch, NewBoxV(V3_Zero, V3_One), &cubeSideColors[0]);
	#endif
	Vertex3D* cubeVertices = AllocArray(Vertex3D, scratch, cubeMesh.numIndices);
	for (uxx iIndex = 0; iIndex < cubeMesh.numIndices; iIndex++)
	{
		MyMemCopy(&cubeVertices[iIndex], &cubeMesh.vertices[cubeMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	app->cubeBuffer = InitVertBuffer3D(stdHeap, StrLit("cube"), VertBufferUsage_Static, cubeMesh.numIndices, cubeVertices, false);
	Assert(app->cubeBuffer.error == Result_Success);
	
	GeneratedMesh sphereMesh = GenerateVertsForSphere(scratch, NewSphereV(V3_Zero, 1.0f), 12, 20, White);
	Vertex3D* sphereVertices = AllocArray(Vertex3D, scratch, sphereMesh.numIndices);
	for (uxx iIndex = 0; iIndex < sphereMesh.numIndices; iIndex++)
	{
		MyMemCopy(&sphereVertices[iIndex], &sphereMesh.vertices[sphereMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	app->sphereBuffer = InitVertBuffer3D(stdHeap, StrLit("sphere"), VertBufferUsage_Static, sphereMesh.numIndices, sphereVertices, false);
	Assert(app->sphereBuffer.error == Result_Success);
	
	InitCompiledShader(&app->main2dShader, stdHeap, main2d); Assert(app->main2dShader.error == Result_Success);
	InitCompiledShader(&app->main3dShader, stdHeap, main3d); Assert(app->main3dShader.error == Result_Success);
	InitCompiledShader(&app->pbrShader, stdHeap, pbr); Assert(app->pbrShader.error == Result_Success);
	
	#if 0
	PrintLine_D("pbrShader has %llu image%s", app->pbrShader.numImages, Plural(app->pbrShader.numImages, "s"));
	for (uxx iIndex = 0; iIndex < app->pbrShader.numImages; iIndex++)
	{
		ShaderImage* image = &app->pbrShader.images[iIndex];
		PrintLine_D("Image[%llu]: \"%.*s\" index %llu", iIndex, StrPrint(image->name), image->index);
	}
	PrintLine_D("pbrShader has %llu sampler%s", app->pbrShader.numSamplers, Plural(app->pbrShader.numSamplers, "s"));
	for (uxx sIndex = 0; sIndex < app->pbrShader.numSamplers; sIndex++)
	{
		ShaderSampler* sampler = &app->pbrShader.samplers[sIndex];
		PrintLine_D("Sampler[%llu]: \"%.*s\" index %llu", sIndex, StrPrint(sampler->name), sampler->index);
	}
	#endif
	
	#if 1
	app->testTexture = LoadTexture(stdHeap, "resources/image/piggyblob.png");
	// app->albedoTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Base_Color.png");
	// app->normalTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Normal_OpenGL.png");
	// app->metallicTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Metallic.png");
	// app->roughnessTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Roughness.png");
	// app->occlusionTexture = LoadTexture(stdHeap, "resources/model/fire_hydrant/fire_hydrant_Mixed_AO.png");
	// app->testModel = LoadModel(FilePathLit("resources/model/fire_hydrant/fire_hydrant.gltf"));
	// app->testModel = LoadModel(FilePathLit("resources/model/fire_hydrant2/fire_hydrant_extern.gltf"));
	app->testModel = LoadModel(FilePathLit("resources/model/chest/chest.gltf"));
	#else
	app->testTexture = LoadTexture(stdHeap, "piggyblob.png");
	// app->albedoTexture = LoadTexture(stdHeap, "fire_hydrant_Base_Color.png");
	// app->normalTexture = LoadTexture(stdHeap, "fire_hydrant_Normal_OpenGL.png");
	// app->metallicTexture = LoadTexture(stdHeap, "fire_hydrant_Metallic.png");
	// app->roughnessTexture = LoadTexture(stdHeap, "fire_hydrant_Roughness.png");
	// app->occlusionTexture = LoadTexture(stdHeap, "fire_hydrant_Mixed_AO.png");
	app->testModel = LoadModel(FilePathLit("chest.gltf"));
	#endif
	// app->occlusionTexture = LoadTexture(stdHeap, "test_texture.png");
	
	app->testFont = InitFont(stdHeap, StrLit("testFont"));
	RasterizeFontAtSize(&app->testFont, StrLit(TEST_FONT_NAME), TEST_FONT_START_SIZE, TEST_FONT_STYLE);
	app->debugFont = InitFont(stdHeap, StrLit("debugFont"));
	RasterizeFontAtSize(&app->debugFont, StrLit("Consolas"), 18, FontStyleFlag_Bold);
	
	#if BUILD_WITH_CLAY
	InitClayUIRenderer(stdHeap, V2_Zero, &app->clay);
	app->clayFontId = AddClayUIRendererFont(&app->clay, &app->debugFont, GetDefaultFontSize(&app->debugFont), GetDefaultFontStyleFlags(&app->debugFont));
	#endif
	
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
	
	if (IsMouseBtnPressed(&appIn->mouse, MouseBtn_Right) && appIn->mouse.isOverWindow && !appIn->mouse.isLocked)
	{
		platform->SetMouseLocked(true);
	}
	if (IsKeyboardKeyPressed(&appIn->keyboard, Key_Escape) && appIn->mouse.isLocked)
	{
		platform->SetMouseLocked(false);
	}
	if (IsKeyboardKeyPressed(&appIn->keyboard, Key_Plus) && IsKeyboardKeyDown(&appIn->keyboard, Key_Control))
	{
		FontAtlas* lastAtlas = VarArrayGetLast(FontAtlas, &app->testFont.atlases);
		RasterizeFontAtSize(&app->testFont, StrLit(TEST_FONT_NAME), lastAtlas->fontSize + 2.0f, TEST_FONT_STYLE);
	}
	if (IsKeyboardKeyPressed(&appIn->keyboard, Key_Minus) && IsKeyboardKeyDown(&appIn->keyboard, Key_Control))
	{
		FontAtlas* lastAtlas = VarArrayGetLast(FontAtlas, &app->testFont.atlases);
		RasterizeFontAtSize(&app->testFont, StrLit(TEST_FONT_NAME), MaxR32(4.0f, lastAtlas->fontSize - 2.0f), TEST_FONT_STYLE);
	}
	
	if ((appIn->screenSizeChanged || IsEmptyStr(app->text)) && !appIn->isMinimized)
	{
		PrintLine_D("ScreenSize: %dx%d", appIn->screenSize.Width, appIn->screenSize.Height);
		app->textPos = Div(ToV2Fromi(appIn->screenSize), 2.0f);
		app->text = StrLit("Hello World!");
		FreeTextLayout(&app->textLayout);
		FontFlowState flowState = ZEROED;
		flowState.font = &app->testFont;
		flowState.position = app->textPos;
		flowState.text = app->text;
		flowState.fontSize = GetDefaultFontSize(&app->testFont);
		flowState.styleFlags = GetDefaultFontStyleFlags(&app->testFont);
		Result layoutResult = DoTextLayoutInArena(stdHeap, &flowState, &app->textLayout);
		Assert(layoutResult == Result_Success);
		app->textMeasure = MeasureText(&app->testFont, app->text);
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
	
	BeginFrame(platform->GetSokolSwapchain(), appIn->screenSize, PalBlueLighter, 1.0f);
	{
		// +==============================+
		// |         3D Rendering         |
		// +==============================+
		{
			// BindShader(&app->main3dShader);
			BindShader(&app->pbrShader);
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
			// DrawSphere(NewSphereV(app->spherePos, app->sphereRadius), White);
			// DrawBox(NewBoxV(Add(Sub(app->spherePos, FillV3(app->sphereRadius)), NewV3(2.0f*1, 0, 0)), FillV3(app->sphereRadius*2)), White);
			
			// DrawModel(&app->testModel, app->spherePos, FillV3(app->sphereRadius*2), Quat_Identity);
			for (uxx yIndex = 0; yIndex < 10; yIndex++)
			{
				for (uxx xIndex = 0; xIndex < 10; xIndex++)
				{
					RandomSeries random = ZEROED;
					InitRandomSeriesDefault(&random);
					SeedRandomSeriesU64(&random, (u64)(xIndex * 17 + yIndex * 117));
					r32 scale = GetRandR32Range(&random, 0.85f, 1.0f);
					r32 rotation = GetRandR32Range(&random, 0, TwoPi32);
					v3 modelPos = NewV3(xIndex * 1.5f, 0, yIndex * 1.5f);
					if (((xIndex + yIndex) % 2) == 0) { SetClipRec(NewReci(appIn->screenSize.Width/4, appIn->screenSize.Height/4, appIn->screenSize.Width/2, appIn->screenSize.Height/2)); }
					else { DisableClipRec(); }
					DrawModel(&app->testModel, modelPos, FillV3(scale), ToQuatFromAxis(V3_Up, rotation));
				}
			}
			DisableClipRec();
			
			BindTextureAtIndex(&gfx.pixelTexture, 0);
			BindTextureAtIndex(&gfx.pixelTexture, 1);
			BindTextureAtIndex(&gfx.pixelTexture, 2);
			BindTextureAtIndex(&gfx.pixelTexture, 3);
			BindTextureAtIndex(&gfx.pixelTexture, 4);
			DrawBox(NewBoxV(Sub(app->lightPos, FillV3(0.05f)), FillV3(0.1f)), White);
		}
		
		// +==============================+
		// |         2D Rendering         |
		// +==============================+
		{
			BindShader(&app->main2dShader);
			ClearDepthBuffer(1.0f);
			mat4 projMat = Mat4_Identity;
			TransformMat4(&projMat, MakeScaleXYZMat4(1.0f/((r32)appIn->screenSize.Width/2.0f), 1.0f/((r32)appIn->screenSize.Height/2.0f), 1.0f));
			TransformMat4(&projMat, MakeTranslateXYZMat4(-1.0f, -1.0f, 0.0f));
			TransformMat4(&projMat, MakeScaleYMat4(-1.0f));
			SetProjectionMat(projMat);
			SetViewMat(Mat4_Identity);
			SetDepth(1.0f);
			
			// DrawRectangle(NewRec(10, 10, 200, 80), MonokaiPurple);
			
			rec piggyblobRec = NewRec(0, (r32)appIn->screenSize.Height - (r32)app->testTexture.Height, (r32)app->testTexture.Width, (r32)app->testTexture.Height);
			DrawTexturedRectangle(piggyblobRec, White, &app->testTexture);
			
			#if BUILD_WITH_CLAY
			BeginClayUIRender(&app->clay.clay, ToV2Fromi(appIn->screenSize), 16.6f, appIn->mouse.position, IsMouseBtnDown(&appIn->mouse, MouseBtn_Left), appIn->mouse.scrollDelta);
			{
				Clay_Sizing layoutExpand = (Clay_Sizing){ .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
				CLAY({ .id = CLAY_ID("FullscreenContainer"),
					// .backgroundColor = ToClayColor(Transparent),
					.layout = {
						.layoutDirection = CLAY_TOP_TO_BOTTOM,
						.sizing = layoutExpand,
						.padding = CLAY_PADDING_ALL(16),
						.childGap = 16,
					} })
				{
					CLAY({ .id = CLAY_ID("HeaderBar"),
						.layout = {
							.sizing = {
								.height = CLAY_SIZING_FIXED(60),
								.width = CLAY_SIZING_GROW(0),
							},
							.padding = { 16, 16, 0, 0 },
							.childGap = 16,
							.childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
						},
						.backgroundColor = ToClayColor(MonokaiBack),
						.cornerRadius = CLAY_CORNER_RADIUS(8) })
					{
						CLAY({ .id = CLAY_ID("FileButton"),
							.layout = { .padding = { 16, 16, 8, 8 } },
							.backgroundColor = ToClayColor(MonokaiGray2),
							.cornerRadius = CLAY_CORNER_RADIUS(5) })
						{
							CLAY_TEXT(CLAY_STRING("File"), CLAY_TEXT_CONFIG({
								.fontId = app->clayFontId,
								.fontSize = 16,
								.textColor = ToClayColor(MonokaiWhite),
							}));
							
							bool fileMenuVisible = (Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileButton"))) || Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileMenu"))));
							if (fileMenuVisible)
							{
								CLAY({ .id = CLAY_ID("FileMenu"),
									.floating = {
										.attachTo = CLAY_ATTACH_TO_PARENT,
										.attachPoints = {
											.parent = CLAY_ATTACH_POINT_LEFT_BOTTOM,
										},
									},
									.layout = {
										.padding = { 0, 0, 8, 8 },
									} })
								{
									CLAY({
										.layout = {
											.layoutDirection = CLAY_TOP_TO_BOTTOM,
											.sizing = {
												.width = CLAY_SIZING_FIXED(200),
											},
										},
										.backgroundColor = ToClayColor(MonokaiBack),
										.cornerRadius = CLAY_CORNER_RADIUS(8) })
									{
										CLAY({.layout = { .padding = CLAY_PADDING_ALL(16) } })
										{
											CLAY_TEXT(CLAY_STRING("New"), CLAY_TEXT_CONFIG({
												.fontId = app->clayFontId,
												.fontSize = 16,
												.textColor = ToClayColor(MonokaiWhite),
											}));
										}
										CLAY({.layout = { .padding = CLAY_PADDING_ALL(16) } })
										{
											CLAY_TEXT(CLAY_STRING("Open"), CLAY_TEXT_CONFIG({
												.fontId = app->clayFontId,
												.fontSize = 16,
												.textColor = ToClayColor(MonokaiWhite),
											}));
										}
										CLAY({.layout = { .padding = CLAY_PADDING_ALL(16) } })
										{
											CLAY_TEXT(CLAY_STRING("Close"), CLAY_TEXT_CONFIG({
												.fontId = app->clayFontId,
												.fontSize = 16,
												.textColor = ToClayColor(MonokaiWhite),
											}));
										}
									}
								}
							}
						}
					}
				}
			}
			Clay_RenderCommandArray clayRenderCommands = EndClayUIRender(&app->clay.clay);
			RenderClayCommandArray(&app->clay, &gfx, &clayRenderCommands);
			#endif //BUILD_WITH_CLAY
			
			#if 0
			r32 atlasPosX = 0;
			VarArrayLoop(&app->testFont.atlases, aIndex)
			{
				VarArrayLoopGet(FontAtlas, atlas, &app->testFont.atlases, aIndex);
				rec atlasRec = NewRec(atlasPosX, 0, (r32)atlas->texture.Width, (r32)atlas->texture.Height);
				DrawTexturedRectangle(atlasRec, White, &atlas->texture);
				DrawRectangleOutline(atlasRec, 2.0f, White);
				atlasPosX += (r32)atlas->texture.Width;
			}
			
			if (!IsEmptyStr(app->text))
			{
				BindFont(&app->testFont);
				DrawText(app->text, app->textPos, White);
				// DrawRectangle(gfx.prevFontFlow.visualRec, NewColor(255, 128, 100, 128));
				// DrawRectangleOutline(gfx.prevFontFlow.visualRec, 2.0f, White);
				// for (uxx gIndex = 0; gIndex < app->textLayout.numGlyphs; gIndex++)
				// {
				// 	FontFlowGlyph* glyph = &app->textLayout.glyphs[gIndex];
				// 	DrawRectangleOutlineEx(glyph->drawRec, 1.0f, MonokaiRed, true);
				// }
			}
			#endif
		}
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
