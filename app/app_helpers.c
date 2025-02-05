/*
File:   app_helpers.c
Author: Taylor Robbins
Date:   02\05\2025
Description: 
	** Contains various helper functions that don't have a home elsewhere
*/

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
