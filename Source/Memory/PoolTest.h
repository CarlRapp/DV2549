#ifndef POOLTEST_H
#define POOLTEST_H

#include <atomic>
#include <string>
#include <SDL/SDL.h>
#include <vector>

#include "Memory/MemoryWrapper.h"

struct ExampleClassA
{
	float x = 1,	y = 1,	z = 1;
	float vx = 1,	vy = 1,	vz = 1;

	float	m11 = 1, m12 = 1, m13 = 1, m14 = 1,
			m21 = 1, m22 = 1, m23 = 1, m24 = 1,
			m31 = 1, m32 = 1, m33 = 1, m34 = 1,
			m41 = 1, m42 = 1, m43 = 1, m44 = 1;

	unsigned int a = 1;
	unsigned int b = 1;
	unsigned int c = 1;
};

struct ExampleClassB
{
	float x = 1, y = 1, z = 1;
	float vx = 1, vy = 1, vz = 1;

	float	m11 = 1, m12 = 1, m13 = 1, m14 = 1,
			m21 = 1, m22 = 1, m23 = 1, m24 = 1,
			m31 = 1, m32 = 1, m33 = 1, m34 = 1,
			m41 = 1, m42 = 1, m43 = 1, m44 = 1;

	unsigned int a = 1;
	unsigned int b = 1;
	unsigned int c = 1;

	float vertex[5000];
};


void RunTest()
{
	Uint32 measure1 = SDL_GetTicks();

	Memory::MemoryWrapper mem = Memory::MemoryWrapper::GetInstance();
	//game supports 50 simultaneous particle systems of 50 particles each
	mem.CreatePool(50, 50 * sizeof(ExampleClassA));
	//game supports up to 25 different models at a time
	mem.CreatePool(25, sizeof(ExampleClassB));

	Uint32 measure2 = SDL_GetTicks();

	ExampleClassA* a1 = NEW_ARRAY(ExampleClassA,50, mem);

	Uint32 measure3 = SDL_GetTicks();

	ExampleClassA* a2 = NEW_ARRAY(ExampleClassA, 50, mem);
	ExampleClassA* a3 = NEW_ARRAY(ExampleClassA, 50, mem);
	ExampleClassA* a4 = NEW_ARRAY(ExampleClassA, 50, mem);
	ExampleClassA* a5 = NEW_ARRAY(ExampleClassA, 50, mem);
	ExampleClassA* a6 = NEW_ARRAY(ExampleClassA, 50, mem);

	Uint32 measure4 = SDL_GetTicks();

	ExampleClassB* m1 = NEW(ExampleClassB, mem);

	Uint32 measure5 = SDL_GetTicks();

	ExampleClassB* m2 = NEW(ExampleClassB, mem);
	ExampleClassB* m3 = NEW(ExampleClassB, mem);
	ExampleClassB* m4 = NEW(ExampleClassB, mem);
	ExampleClassB* m5 = NEW(ExampleClassB, mem);
	ExampleClassB* m6 = NEW(ExampleClassB, mem);
	ExampleClassB* m7 = NEW(ExampleClassB, mem);
	ExampleClassB* m8 = NEW(ExampleClassB, mem);

	Uint32 measure6 = SDL_GetTicks();

	DELETE_ARRAY(ExampleClassA, a1, 50, mem);

	Uint32 measure7 = SDL_GetTicks();

	DELETE_ARRAY(ExampleClassA, a2, 50, mem);
	DELETE_ARRAY(ExampleClassA, a3, 50, mem);
	DELETE_ARRAY(ExampleClassA, a4, 50, mem);
	DELETE_ARRAY(ExampleClassA, a5, 50, mem);
	DELETE_ARRAY(ExampleClassA, a6, 50, mem);

	Uint32 measure8 = SDL_GetTicks();

	DELETE(ExampleClassB, m1, mem);

	Uint32 measure9 = SDL_GetTicks();

	DELETE(ExampleClassB, m2, mem);
	DELETE(ExampleClassB, m3, mem);
	DELETE(ExampleClassB, m4, mem);
	DELETE(ExampleClassB, m5, mem);
	DELETE(ExampleClassB, m6, mem);
	DELETE(ExampleClassB, m7, mem);
	DELETE(ExampleClassB, m8, mem);

	Uint32 measure10 = SDL_GetTicks();
	
	Uint32 msrCreatePools	= (measure2 - measure1);
	Uint32 msrOneArray		= (measure3 - measure2);
	Uint32 msrAllArray		= (measure4 - measure3)		+ msrOneArray;
	Uint32 msrOneNew		= (measure5 - measure4);
	Uint32 msrAllNew		= (measure6 - measure5)		+ msrOneNew;
	Uint32 msrOneDelArray	= (measure7 - measure6);
	Uint32 msrAllDelArray	= (measure8 - measure7)		+ msrOneDelArray;
	Uint32 msrOneDel		= (measure9 - measure8);
	Uint32 msrAllDel		= (measure10 - measure9)	+ msrOneDel;
	Uint32 total			= (measure10 - measure1);

	printf("\
\n_Pool time measure_\n\
\n\
Item A: %dB\n\
Item B: %dB\n\
\n\
Create Pools:		%dms\n\
New One Array:		%dms\n\
New All Array:		%dms\n\
New One:		%dms\n\
New All:		%dms\n\
One Delete Array:	%dms\n\
All Delete Array:	%dms\n\
One Delete:		%dms\n\
All Delete:		%dms\n\
Total:			%dms\n\
",
(int)sizeof(ExampleClassA),
(int)sizeof(ExampleClassB),
msrCreatePools,
msrOneArray,
msrAllArray,
msrOneNew,
msrAllNew,
msrOneDelArray,
msrAllDelArray,
msrOneDel,
msrAllDel,
total
);

	mem.PrintPoolsPoolLevel();


}

#endif
