/*
File:   app_main.h
Author: Taylor Robbins
Date:   02\02\2025
*/

#ifndef _APP_MAIN_H
#define _APP_MAIN_H

typedef struct Model3D Model3D;
struct Model3D
{
	ModelData data;
	VarArray vertBuffers; //VertBuffer
	VarArray textures; //Texture
};

typedef struct AppData AppData;
struct AppData
{
	bool initialized;
	
	#if BUILD_WITH_CLAY
	ClayUIRenderer clay;
	u16 clayFont;
	#endif
	
	#if BUILD_WITH_IMGUI
	ImguiUI imgui;
	#endif
	
	sg_pass_action sokolPassAction;
	Shader main2dShader;
	#if FP3D_SCENE_ENABLED
	Shader main3dShader;
	Shader pbrShader;
	#endif
	
	VertBuffer squareBuffer;
	#if FP3D_SCENE_ENABLED
	VertBuffer cubeBuffer;
	VertBuffer sphereBuffer;
	#endif
	
	Texture testSprite;
	Texture testTexturePink;
	Texture testTextureBlue;
	
	#if FP3D_SCENE_ENABLED
	Texture albedoTexture;
	Texture normalTexture;
	Texture metallicTexture;
	Texture roughnessTexture;
	Texture occlusionTexture;
	Model3D testModel;
	#endif //FP3D_SCENE_ENABLED
	
	Font testFont;
	Font debugFont;
	v2 textPos;
	Str8 text;
	TextMeasure textMeasure;
	TextLayout textLayout;
	
	#if FP3D_SCENE_ENABLED
	v3 spherePos;
	r32 sphereRadius;
	
	v3 cameraPos;
	v3 cameraLookDir;
	
	v3 lightPos;
	#endif
	
	bool topbarDebugMenuOpen;
	bool topbarCameraMenuOpen;
	bool scissorTestEnabled;
	bool borderThicknessTestEnabled;
	bool roundedRecTestEnabled;
	bool roundedBorderTestEnabled;
	r32 roundedBorderThickness;
	bool circleTestEnabled;
	bool ringTestEnabled;
	bool horizontalGuidesEnabled;
	bool verticalGuidesEnabled;
	r32 circlePieceAngleOffset;
	r32 ringPieceAngleOffset;
	r32 ringThickness;
};

#endif //  _APP_MAIN_H
