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
	u64 frameIndex;
	
	sg_pass_action sokolPassAction;
	Shader main2dShader;
	Shader main3dShader;
	
	VertBuffer squareBuffer;
	VertBuffer cubeBuffer;
	VertBuffer sphereBuffer;
	
	Texture gradientTexture;
	Texture albedoTexture;
	Texture normalTexture;
	Texture metallicTexture;
	Texture roughnessTexture;
	Texture occlusionTexture;
	
	v3 spherePos;
	r32 sphereRadius;
	
	v3 cameraPos;
	v3 cameraLookDir;
	
	v3 lightPos;
};

#endif //  _APP_MAIN_H
