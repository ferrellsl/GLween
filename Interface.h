// Interface.h: interface for the CInterface class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#if !defined(_INTERFACE_H_INCLUDED_)
#define _INTERFACE_H_INCLUDED_

#ifdef WIN32														// Compiling on a windows based machine
	#include <windows.h>											// Header File For Windows
#endif

#include <gl\gl.h>													// Header File For The OpenGL32 Library
#include <gl\glu.h>													// Header File For The GLu32 Library
// glaux.h was only ever #included here, never actually used (no AUX_*
// symbols anywhere in this codebase) -- Windows/Visual Studio dropped it
// years ago, so it's simply gone now.

#include "TextureImage.h"											// Header File For Loading Our Textures

class CInterface  
{
private:
	GLsizei m_left;													// Window Left Starting Position
	GLsizei m_right;												// Window Right Starting Position
	GLsizei m_bottom;												// Window Bottom Position
	GLsizei m_top;													// Window Top Position
	GLdouble m_zNear;												// Distance From The Viewer To The Near Clipping Plane (Always Positive)
	GLdouble m_zFar;												// Distance From The Viewer To The Far Clipping Plane (Always Positive)

public:
	BOOL m_Visible;													// Interface Visible Or Not

public:
	CInterface();													// Class Constructor
	virtual ~CInterface();											// Class Destructor
	
	BOOL Render(GLvoid);											// Render The OpenGL Scene
};

#endif // !defined(_INTERFACE_H_INCLUDED_)
