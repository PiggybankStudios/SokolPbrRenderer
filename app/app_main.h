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
	Texture gradientTexture;
	VertBuffer squareBuffer;
};

#endif //  _APP_MAIN_H
