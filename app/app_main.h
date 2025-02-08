/*
File:   app_main.h
Author: Taylor Robbins
Date:   02\02\2025
*/

#ifndef _APP_MAIN_H
#define _APP_MAIN_H

typedef struct AppData AppData;
struct AppData
{
	bool initialized;
	
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
	
	Font testFont;
	
	v3 spherePos;
	r32 sphereRadius;
	
	v3 cameraPos;
	v3 cameraLookDir;
	
	v3 lightPos;
};

#endif //  _APP_MAIN_H
