/*
File:   bullet_c_api.h
Author: Taylor Robbins
Date:   02\11\2025
*/

#ifndef _BULLET_C_API_H
#define _BULLET_C_API_H

void* InitBulletPhysics(Arena* arena);
void FreeBulletPhysics(Arena* arena, void* world);
void* TestAllocatingClasses(Arena* arena);
void TestFreeingClasses(Arena* arena, void* pntr);

#endif //  _BULLET_C_API_H
