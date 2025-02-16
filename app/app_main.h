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
	u16 clayFontId;
	#endif
	
	sg_pass_action sokolPassAction;
	Shader main2dShader;
	Shader main3dShader;
	Shader pbrShader;
	
	VertBuffer squareBuffer;
	VertBuffer cubeBuffer;
	VertBuffer sphereBuffer;
	
	Texture testTexture;
	Texture albedoTexture;
	Texture normalTexture;
	Texture metallicTexture;
	Texture roughnessTexture;
	Texture occlusionTexture;
	
	Model3D testModel;
	
	Font testFont;
	Font debugFont;
	v2 textPos;
	Str8 text;
	TextMeasure textMeasure;
	TextLayout textLayout;
	
	v3 spherePos;
	r32 sphereRadius;
	
	v3 cameraPos;
	v3 cameraLookDir;
	
	v3 lightPos;
};

#endif //  _APP_MAIN_H
