/*
File:   app_sokol.c
Author: Taylor Robbins
Date:   02\02\2025
Description: 
	** Holds functions that help us intialize and interact with Sokol (sokol_sapp and sokol_gfx)
*/

#include "main2d_shader.glsl.h"
#include "main3d_shader.glsl.h"

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
	
	Vertex3D cubeVertices[] = {
		//Back Face (Green)
		{ .X=0.0f, .Y=1.0f, .Z=0.0f,   .nX= 0.0f, .nY= 0.0f, .nZ=-1.0f,   .tX=0.0f, .tY=0.0f,   .R=0.2f, .G=1.0f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=0.0f,   .nX= 0.0f, .nY= 0.0f, .nZ=-1.0f,   .tX=1.0f, .tY=0.0f,   .R=0.2f, .G=1.0f, .B=0.2f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=0.0f,   .nX= 0.0f, .nY= 0.0f, .nZ=-1.0f,   .tX=0.0f, .tY=1.0f,   .R=0.2f, .G=1.0f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=0.0f,   .nX= 0.0f, .nY= 0.0f, .nZ=-1.0f,   .tX=1.0f, .tY=1.0f,   .R=0.2f, .G=1.0f, .B=0.2f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=0.0f,   .nX= 0.0f, .nY= 0.0f, .nZ=-1.0f,   .tX=0.0f, .tY=1.0f,   .R=0.2f, .G=1.0f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=0.0f,   .nX= 0.0f, .nY= 0.0f, .nZ=-1.0f,   .tX=1.0f, .tY=0.0f,   .R=0.2f, .G=1.0f, .B=0.2f, .A=1.0f },
		
		//Top Face (White)
		{ .X=0.0f, .Y=1.0f, .Z=1.0f,   .nX= 0.0f, .nY= 1.0f, .nZ= 0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=1.0f,   .nX= 0.0f, .nY= 1.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f, .Z=0.0f,   .nX= 0.0f, .nY= 1.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=0.0f,   .nX= 0.0f, .nY= 1.0f, .nZ= 0.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f, .Z=0.0f,   .nX= 0.0f, .nY= 1.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=1.0f,   .nX= 0.0f, .nY= 1.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=1.0f, .A=1.0f },
		
		//Right Face (Red)
		{ .X=1.0f, .Y=1.0f, .Z=0.0f,   .nX= 1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=0.2f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=1.0f,   .nX= 1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=0.2f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=0.0f,   .nX= 1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=0.2f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=1.0f,   .nX= 1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=0.2f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=0.0f,   .nX= 1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=0.2f, .B=0.2f, .A=1.0f },
		{ .X=1.0f, .Y=1.0f, .Z=1.0f,   .nX= 1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=0.2f, .B=0.2f, .A=1.0f },
		
		//Front Face (Blue)
		{ .X=1.0f, .Y=1.0f, .Z=1.0f,   .nX= 0.0f, .nY= 0.0f, .nZ= 1.0f,   .tX=0.0f, .tY=0.0f,   .R=0.2f, .G=0.2f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f, .Z=1.0f,   .nX= 0.0f, .nY= 0.0f, .nZ= 1.0f,   .tX=1.0f, .tY=0.0f,   .R=0.2f, .G=0.2f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=1.0f,   .nX= 0.0f, .nY= 0.0f, .nZ= 1.0f,   .tX=0.0f, .tY=1.0f,   .R=0.2f, .G=0.2f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=1.0f,   .nX= 0.0f, .nY= 0.0f, .nZ= 1.0f,   .tX=1.0f, .tY=1.0f,   .R=0.2f, .G=0.2f, .B=1.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=1.0f,   .nX= 0.0f, .nY= 0.0f, .nZ= 1.0f,   .tX=0.0f, .tY=1.0f,   .R=0.2f, .G=0.2f, .B=1.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f, .Z=1.0f,   .nX= 0.0f, .nY= 0.0f, .nZ= 1.0f,   .tX=1.0f, .tY=0.0f,   .R=0.2f, .G=0.2f, .B=1.0f, .A=1.0f },
		
		//Left Face (Orange)
		{ .X=0.0f, .Y=1.0f, .Z=1.0f,   .nX=-1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=0.6f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f, .Z=0.0f,   .nX=-1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=0.6f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=1.0f,   .nX=-1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=0.6f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=0.0f,   .nX=-1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=0.6f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=1.0f,   .nX=-1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=0.6f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=1.0f, .Z=0.0f,   .nX=-1.0f, .nY= 0.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=0.6f, .B=0.0f, .A=1.0f },
		
		//Bottom Face (Yellow)
		{ .X=0.0f, .Y=0.0f, .Z=0.0f,   .nX= 0.0f, .nY=-1.0f, .nZ= 0.0f,   .tX=0.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=0.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=0.0f,   .nX= 0.0f, .nY=-1.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=1.0f,   .nX= 0.0f, .nY=-1.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=0.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=1.0f,   .nX= 0.0f, .nY=-1.0f, .nZ= 0.0f,   .tX=1.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=0.0f, .A=1.0f },
		{ .X=0.0f, .Y=0.0f, .Z=1.0f,   .nX= 0.0f, .nY=-1.0f, .nZ= 0.0f,   .tX=0.0f, .tY=1.0f,   .R=1.0f, .G=1.0f, .B=0.0f, .A=1.0f },
		{ .X=1.0f, .Y=0.0f, .Z=0.0f,   .nX= 0.0f, .nY=-1.0f, .nZ= 0.0f,   .tX=1.0f, .tY=0.0f,   .R=1.0f, .G=1.0f, .B=0.0f, .A=1.0f },
	};
	app->cubeBuffer = InitVertBuffer3D(stdHeap, StrLit("cube"), VertBufferUsage_Static, ArrayCount(cubeVertices), &cubeVertices[0], false);
	Assert(app->cubeBuffer.error == Result_Success);
	
	GeneratedMesh sphereMesh = GenerateVertsForSphere(scratch, NewSphereV(V3_Zero, 1.0f), 12, 20, White);
	PrintLine_D("There are %llu indice%s and %llu vertice%s", sphereMesh.numIndices, Plural(sphereMesh.numIndices, "s"), sphereMesh.numVertices, Plural(sphereMesh.numVertices, "s"));
	Vertex3D* sphereVertices = AllocArray(Vertex3D, scratch, sphereMesh.numIndices);
	for (uxx iIndex = 0; iIndex < sphereMesh.numIndices; iIndex++)
	{
		// sphereVertices[iIndex] = sphereMesh.vertices[sphereMesh.indices[iIndex]];
		MyMemCopy(&sphereVertices[iIndex], &sphereMesh.vertices[sphereMesh.indices[iIndex]], sizeof(Vertex3D));
	}
	app->sphereBuffer = InitVertBuffer3D(stdHeap, StrLit("sphere"), VertBufferUsage_Static, sphereMesh.numIndices, sphereVertices, false);
	Assert(app->sphereBuffer.error == Result_Success);
	
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
	InitCompiledShader(&app->main3dShader, stdHeap, main3d); Assert(app->main3dShader.error == Result_Success);
	
	ScratchEnd(scratch);
}

void DrawRectangle(v2 topLeft, v2 size, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleXYZMat4(size.Width, size.Height, 1.0f));
	TransformMat4(&worldMat, MakeTranslateXYZMat4(topLeft.X, topLeft.Y, 0.0f));
	SetWorldMat(worldMat);
	SetTintColor(color);
	
	BindVertBuffer(&app->squareBuffer);
	DrawVertices();
}

void DrawBox(box boundingBox, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleMat4(boundingBox.Size));
	TransformMat4(&worldMat, MakeTranslateMat4(boundingBox.BottomLeftBack));
	SetWorldMat(worldMat);
	SetTintColor(color);
	
	BindVertBuffer(&app->cubeBuffer);
	DrawVertices();
}

void DrawSphere(Sphere sphere, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeScaleMat4(FillV3(sphere.Radius)));
	TransformMat4(&worldMat, MakeTranslateMat4(sphere.Center));
	SetWorldMat(worldMat);
	SetTintColor(color);
	
	BindVertBuffer(&app->sphereBuffer);
	DrawVertices();
}