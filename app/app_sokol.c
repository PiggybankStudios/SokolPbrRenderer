/*
File:   app_sokol.c
Author: Taylor Robbins
Date:   02\02\2025
Description: 
	** Holds functions that help us intialize and interact with Sokol (sokol_sapp and sokol_gfx)
*/

#include "main2d_shader.glsl.h"

void InitSokol()
{
	NotNull(app);
	ScratchBegin(scratch);
	
	InitGfxSystem(stdHeap, &gfx);
	
	Vertex2D squareVertices[] = {
		{ .X=0.0f, .Y=0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		
		{ .X=1.0f, .Y=1.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
	};
	app->squareBuffer = InitVertBuffer2D(stdHeap, StrLit("square"), VertBufferUsage_Static, ArrayCount(squareVertices), &squareVertices[0], false);
	Assert(app->squareBuffer.error == Result_Success);
	
	v2i gradientSize = NewV2i(64, 64);
	Color32* gradientPixels = AllocArray(Color32, scratch, (uxx)(gradientSize.Width * gradientSize.Height));
	for (i32 pixelY = 0; pixelY < gradientSize.Height; pixelY++)
	{
		for (i32 pixelX = 0; pixelX < gradientSize.Width; pixelX++)
		{
			Color32* pixel = &gradientPixels[INDEX_FROM_COORD2D(pixelX, pixelY, gradientSize.Width, gradientSize.Height)];
			pixel->r = ClampCastI32ToU8(RoundR32i(LerpR32(0, 255.0f, (r32)pixelX / (r32)gradientSize.Width)));
			pixel->g = ClampCastI32ToU8(RoundR32i(LerpR32(0, 255.0f, (r32)pixelY / (r32)gradientSize.Height)));
			pixel->b = pixel->r/2 + pixel->g/2;
			pixel->a = 255;
		}
	}
	
	app->gradientTexture = InitTexture(stdHeap, StrLit("gradient"), gradientSize, gradientPixels, TextureFlag_IsRepeating);
	Assert(app->gradientTexture.error == Result_Success);
	
	InitCompiledShader(&app->main2dShader, stdHeap, main2d); Assert(app->main2dShader.error == Result_Success);
	
	ScratchEnd(scratch);
}

void DrawRectangle(Shader* shader, v2 topLeft, v2 size, Color32 color)
{
	NotNull(shader);
	
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleXYZMat4(size.Width, size.Height, 1.0f));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(topLeft.X, topLeft.Y, 0.0f));
	SetWorldMat(worldMat);
	SetTintColor(color);
	
	BindVertBuffer(&app->squareBuffer);
	DrawVertices();
}
