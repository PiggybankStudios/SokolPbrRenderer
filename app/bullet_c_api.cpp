/*
File:   bullet_c_api.cpp
Author: Taylor Robbins
Date:   02\11\2025
Description: 
	** Exposes a C-friendly wrapper around all the Bullet Physics APIs that we want to use
*/

#include "build_config.h"
#define PIG_CORE_IMPLEMENTATION 0

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "std/std_includes.h"
START_EXTERN_C
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "base/base_debug_output.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
END_EXTERN_C

#include "third_party/bullet/btBulletDynamicsCommon.h"

START_EXTERN_C

#include "bullet_c_api.h"

typedef struct PhysicsWorld PhysicsWorld;
struct PhysicsWorld
{
	#if 1
	btDefaultCollisionConfiguration*     collisionConfig;
	btCollisionDispatcher*               dispatcher;
	btDbvtBroadphase*                    broadphase;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld*             dynamicsWorld;
	#else
	btDefaultCollisionConfiguration     collisionConfig;
	btCollisionDispatcher               dispatcher;
	btDbvtBroadphase                    broadphase;
	btSequentialImpulseConstraintSolver solver;
	btDiscreteDynamicsWorld             dynamicsWorld;
	#endif
};

void* InitBulletPhysics(Arena* arena)
{
	PhysicsWorld* result = AllocType(PhysicsWorld, arena);
	PrintLine_D("Hello from Bullet Physics v%d", btGetVersion());
	#if 0
	std::construct_at(&result->collisionConfig);
	std::construct_at(&result->dispatcher, &result->collisionConfig);
	std::construct_at(&result->broadphase);
	std::construct_at(&result->solver);
	std::construct_at(&result->dynamicsWorld, &result->dispatcher, &result->broadphase, &result->solver, &result->collisionConfig);
	result->dynamicsWorld.setGravity(btVector3(0, -10, 0));
	#elif 0
	AllocAndNew(result->collisionConfig, arena, btDefaultCollisionConfiguration);
	AllocAndNewWithArgs(result->dispatcher, arena, btCollisionDispatcher,
		result->collisionConfig
	);
	AllocAndNew(result->broadphase, arena, btDbvtBroadphase);
	AllocAndNew(result->solver, arena, btSequentialImpulseConstraintSolver);
	AllocAndNewWithArgs(result->dynamicsWorld, arena, btDiscreteDynamicsWorld,
		result->dispatcher,
		result->broadphase,
		result->solver,
		result->collisionConfig
	);
	result->dynamicsWorld->setGravity(btVector3(0, -10, 0));
	#else
	result->collisionConfig = new btDefaultCollisionConfiguration();
	result->dispatcher = new btCollisionDispatcher(result->collisionConfig);
	result->broadphase = new btDbvtBroadphase();
	// result->solver = new btSequentialImpulseConstraintSolver();
	// result->dynamicsWorld = new btDiscreteDynamicsWorld(
	// 	result->dispatcher,
	// 	result->broadphase,
	// 	result->solver,
	// 	result->collisionConfig
	// );
	// result->dynamicsWorld->setGravity(btVector3(0, -10, 0));
	#endif
	WriteLine_D("Leaving InitBulletPhysics...");
	return result;
}

void FreeBulletPhysics(Arena* arena, void* worldVoidPntr)
{
	PhysicsWorld* world = (PhysicsWorld*)worldVoidPntr;
	#if 0
	std::destroy_at(&world->dynamicsWorld);
	std::destroy_at(&world->solver);
	std::destroy_at(&world->broadphase);
	std::destroy_at(&world->dispatcher);
	std::destroy_at(&world->collisionConfig);
	#else
	// delete world->dynamicsWorld;
	// delete world->solver;
	delete world->broadphase;
	delete world->dispatcher;
	delete world->collisionConfig;
	#endif
	ClearPointer(world);
	FreeMem(arena, world, sizeof(PhysicsWorld));
}

class ClassA
{
public:
	ClassA(int newId)
	{
		id = newId;
		PrintLine_D("ClassA constructor %d!", id);
	}
	virtual ~ClassA()
	{
		PrintLine_D("ClassA destructor: %d!", id);
	}
	
	virtual int GetInteger() { return id * id; }
private:
	int id;
};

class ClassB : public ClassA
{
public:
	ClassB(float newId) : ClassA((int)newId)
	{
		floatId = newId;
		PrintLine_D("ClassB constructor: %f", floatId);
	}
	virtual ~ClassB()
	{
		PrintLine_D("ClassB destructor: %f", floatId);
	}
	
	virtual int GetInteger() { return (int)(floatId * floatId * floatId); }
private:
	float floatId;
};

void* TestAllocatingClasses(Arena* arena)
{
	ClassB* bPntr = AllocType(ClassB, arena);
	std::construct_at(bPntr, 3.1515926f);
	PrintLine_D("TestAllocatingClasses -> %p", bPntr);
	Write_D("\t{");
	for (uxx bIndex = 0; bIndex < sizeof(ClassB); bIndex++)
	{
		Print_D(" 0x%02X", ((u8*)bPntr)[bIndex]);
	}
	WriteLine_D(" }");
	PrintLine_D("GetInteger() -> %d", bPntr->GetInteger());
	return bPntr;
}

void TestFreeingClasses(Arena* arena, void* pntr)
{
	ClassB* bPntr = (ClassB*)pntr;
	PrintLine_D("TestFreeingClasses(%p)", bPntr);
	Write_D("\t{");
	for (uxx bIndex = 0; bIndex < sizeof(ClassB); bIndex++)
	{
		Print_D(" 0x%02X", ((u8*)bPntr)[bIndex]);
	}
	WriteLine_D(" }");
	PrintLine_D("GetInteger() -> %d", bPntr->GetInteger());
	std::destroy_at(bPntr);
	FreeMem(arena, bPntr, sizeof(ClassB));
}

END_EXTERN_C
