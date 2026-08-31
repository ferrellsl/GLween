// Particle.h: interface for the CParticle class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#if !defined(_PARTICLE_H_INCLUDED_)
#define _PARTICLE_H_INCLUDED_

#ifdef WIN32														// Compiling on a windows based machine
	#include <windows.h>											// Header File For Windows
#endif

#include <gl\gl.h>													// Header File For The OpenGL32 Library
#include <gl\glu.h>													// Header File For The GLu32 Library
// glaux.h was only ever #included here, never actually used (no AUX_*
// symbols anywhere in this codebase) -- Windows/Visual Studio dropped it
// years ago, so it's simply gone now.

class CParticle  
{
public:
	bool active;													// Active (Yes/No)
	GLfloat life;													// Particle Life
	GLfloat fade;													// Fade Speed
																
	GLfloat r;														// Red Value
	GLfloat g;														// Green Value
	GLfloat b;														// Blue Value
																
	GLfloat	x;														// X Position
	GLfloat	y;														// Y Position
	GLfloat	z;														// Z Position
																
	GLfloat	xi;														// X Direction
	GLfloat	yi;														// Y Direction
	GLfloat	zi;														// Z Direction
																
	GLfloat	xg;														// X Gravity
	GLfloat	yg;														// Y Gravity
	GLfloat	zg;														// Z Gravity

	CParticle();
	virtual ~CParticle();

};

#endif // !defined(_PARTICLE_H_INCLUDED_)
