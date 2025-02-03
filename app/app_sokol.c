/*
File:   app_sokol.c
Author: Taylor Robbins
Date:   02\02\2025
Description: 
	** Holds functions that help us intialize and interact with Sokol (sokol_sapp and sokol_gfx)
*/

#include "main2d_shader.glsl.h"
#include "main3d_shader.glsl.h"
#include "pbr_shader.glsl.h"

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