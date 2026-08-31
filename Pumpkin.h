// Pumpkin.h: interface for the CPumpkin class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#if !defined(_PUMPKIN_H_INCLUDED_)
#define _PUMPKIN_H_INCLUDED_

#ifdef WIN32														// Compiling on a windows based machine
	#include <windows.h>											// Header File For Windows
	#include <math.h>												// Header File For Windows Math Library
#endif

#include <gl\gl.h>													// Header File For The OpenGL32 Library
#include <gl\glu.h>													// Header File For The GLu32 Library
// glaux.h was only ever #included here, never actually used (no AUX_*
// symbols anywhere in this codebase) -- Windows/Visual Studio dropped it
// years ago, so it's simply gone now.

// 25 Verticies
// 25 Texture Coordinates
// 1 Normals
// 32 Triangles

static BYTE face_indicies[32][9] = {
// Object: Plane01
	{5,0,6 ,0,0,0 ,0,1,2 }, {1,6,0 ,0,0,0 ,3,2,1 }, {6,1,7 ,0,0,0 ,2,3,4 },
	{2,7,1 ,0,0,0 ,5,4,3 }, {7,2,8 ,0,0,0 ,4,5,6 }, {3,8,2 ,0,0,0 ,7,6,5 },
	{8,3,9 ,0,0,0 ,6,7,8 }, {4,9,3 ,0,0,0 ,9,8,7 }, {10,5,11 ,0,0,0 ,10,0,11 },
	{6,11,5 ,0,0,0 ,2,11,0 }, {11,6,12 ,0,0,0 ,11,2,12 }, {7,12,6 ,0,0,0 ,4,12,2 },
	{12,7,13 ,0,0,0 ,12,4,13 }, {8,13,7 ,0,0,0 ,6,13,4 }, {13,8,14 ,0,0,0 ,13,6,14 },
	{9,14,8 ,0,0,0 ,8,14,6 }, {15,10,16 ,0,0,0 ,15,10,16 }, {11,16,10 ,0,0,0 ,11,16,10 },
	{16,11,17 ,0,0,0 ,16,11,17 }, {12,17,11 ,0,0,0 ,12,17,11 }, {17,12,18 ,0,0,0 ,17,12,18 },
	{13,18,12 ,0,0,0 ,13,18,12 }, {18,13,19 ,0,0,0 ,18,13,19 }, {14,19,13 ,0,0,0 ,14,19,13 },
	{20,15,21 ,0,0,0 ,20,15,21 }, {16,21,15 ,0,0,0 ,16,21,15 }, {21,16,22 ,0,0,0 ,21,16,22 },
	{17,22,16 ,0,0,0 ,17,22,16 }, {22,17,23 ,0,0,0 ,22,17,23 }, {18,23,17 ,0,0,0 ,18,23,17 },
	{23,18,24 ,0,0,0 ,23,18,24 }, {19,24,18 ,0,0,0 ,19,24,18 }
};
static GLfloat verticies [25][3] = {
{-0.5f,-0.5f,0.0f},{-0.25f,-0.5f,0.0f},{0.0f,-0.5f,0.0f},
{0.25f,-0.5f,0.0f},{0.5f,-0.5f,0.0f},{-0.5f,-0.25f,0.0f},
{-0.25f,-0.25f,0.0f},{0.0f,-0.25f,0.0f},{0.25f,-0.25f,0.0f},
{0.5f,-0.25f,0.0f},{-0.5f,0.0f,0.0f},{-0.25f,0.0f,0.0f},
{0.0f,0.0f,0.0f},{0.25f,0.0f,0.0f},{0.5f,0.0f,0.0f},
{-0.5f,0.25f,0.0f},{-0.25f,0.25f,0.0f},{0.0f,0.25f,0.0f},
{0.25f,0.25f,0.0f},{0.5f,0.25f,0.0f},{-0.5f,0.5f,0.0f},
{-0.25f,0.5f,0.0f},{0.0f,0.5f,0.0f},{0.25f,0.5f,0.0f},
{0.5f,0.5f,0.0f}
};
static GLfloat normals [1][3] = {
{0.0f,0.0f,1.0f}
};
static GLfloat textures [25][2] = {
{0.0f,0.25f},{0.0f,0.0f},{0.25f,0.25f},
{0.25f,0.0f},{0.5f,0.25f},{0.5f,0.0f},
{0.75f,0.25f},{0.75f,0.0f},{1.0f,0.25f},
{1.0f,0.0f},{0.0f,0.5f},{0.25f,0.5f},
{0.5f,0.5f},{0.75f,0.5f},{1.0f,0.5f},
{0.0f,0.75f},{0.25f,0.75f},{0.5f,0.75f},
{0.75f,0.75f},{1.0f,0.75f},{0.0f,1.0f},
{0.25f,1.0f},{0.5f,1.0f},{0.75f,1.0f},
{1.0f,1.0f}
};

class CPumpkin
{
private:
	GLuint m_listPumpkin;

	// See the comment on CCasket's equivalent members: these used to be
	// function-local statics in Render(), which broke once more than one
	// CPumpkin exists (one per monitor in the screensaver).
	GLfloat m_rot1, m_rot2;
	GLfloat m_cnt1, m_cnt2;

public:
	GLvoid CompileList(GLvoid);										// Compile The Object Into A List
	GLvoid Render(GLvoid);
	CPumpkin();														// Class Constructor
	virtual ~CPumpkin();											// Class Destructor

};

#endif // !defined(_PUMPKIN_H_INCLUDED_)
