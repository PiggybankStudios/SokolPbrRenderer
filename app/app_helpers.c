/*
File:   app_helpers.c
Author: Taylor Robbins
Date:   02\05\2025
Description: 
	** Contains various helper functions that don't have a home elsewhere
*/

#if FP3D_SCENE_ENABLED
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

void DrawObb3(obb3 boundingBox, Color32 color)
{
	mat4 worldMat = Mat4_Identity;
	TransformMat4(&worldMat, MakeTranslateMat4(FillV3(-0.5f)));
	TransformMat4(&worldMat, MakeScaleMat4(boundingBox.Size));
	TransformMat4(&worldMat, ToMat4FromQuat(boundingBox.Rotation));
	TransformMat4(&worldMat, MakeTranslateMat4(boundingBox.Center));
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

void DrawModel(Model3D* model, v3 position, v3 scale, quat rotation)
{
	mat4 baseWorldMat = Mat4_Identity;
	TransformMat4(&baseWorldMat, ToMat4FromQuat(rotation)); //TODO: Order of rotation and scaling??
	TransformMat4(&baseWorldMat, MakeScaleMat4(scale));
	TransformMat4(&baseWorldMat, MakeTranslateMat4(position));
	VarArrayLoop(&model->data.parts, pIndex)
	{
		VarArrayLoopGet(ModelDataPart, part, &model->data.parts, pIndex);
		
		if (part->materialIndex < model->data.materials.length)
		{
			ModelDataMaterial* material = VarArrayGetHard(ModelDataMaterial, &model->data.materials, part->materialIndex);
			if (material->albedoTextureIndex < model->data.textures.length) { BindTextureAtIndex(VarArrayGetHard(Texture, &model->textures, material->albedoTextureIndex), 0); }
			else { BindTextureAtIndex(&gfx.pixelTexture, 0); }
			if (material->normalTextureIndex < model->data.textures.length) { BindTextureAtIndex(VarArrayGetHard(Texture, &model->textures, material->normalTextureIndex), 1); }
			else { BindTextureAtIndex(&gfx.pixelTexture, 1); }
			if (material->metallicRoughnessTextureIndex < model->data.textures.length) { BindTextureAtIndex(VarArrayGetHard(Texture, &model->textures, material->metallicRoughnessTextureIndex), 2); }
			else { BindTextureAtIndex(&gfx.pixelTexture, 2); }
			if (material->metallicRoughnessTextureIndex < model->data.textures.length) { BindTextureAtIndex(VarArrayGetHard(Texture, &model->textures, material->metallicRoughnessTextureIndex), 3); }
			else { BindTextureAtIndex(&gfx.pixelTexture, 3); }
			if (material->ambientOcclusionTextureIndex < model->data.textures.length) { BindTextureAtIndex(VarArrayGetHard(Texture, &model->textures, material->ambientOcclusionTextureIndex), 4); }
			else { BindTextureAtIndex(&gfx.pixelTexture, 4); }
			SetTintColorRaw(material->albedoFactor);
		}
		else
		{
			BindTextureAtIndex(&gfx.pixelTexture, 0);
			BindTextureAtIndex(&gfx.pixelTexture, 1);
			BindTextureAtIndex(&gfx.pixelTexture, 2);
			BindTextureAtIndex(&gfx.pixelTexture, 3);
			BindTextureAtIndex(&gfx.pixelTexture, 4);
			SetTintColor(MonokaiPurple);
		}
		
		VertBuffer* partVertBuffer = VarArrayGetHard(VertBuffer, &model->vertBuffers, pIndex);
		mat4 partWorldMatrix = Mat4_Identity;
		TransformMat4(&partWorldMatrix, ToMat4FromQuat(part->transform.rotation)); //TODO: Order of rotation and scaling??
		TransformMat4(&partWorldMatrix, MakeScaleMat4(part->transform.scale));
		TransformMat4(&partWorldMatrix, MakeTranslateMat4(part->transform.position));
		SetWorldMat(Mul(baseWorldMat, partWorldMatrix));
		BindVertBuffer(partVertBuffer);
		DrawVertices();
	}
}
#endif //FP3D_SCENE_ENABLED
