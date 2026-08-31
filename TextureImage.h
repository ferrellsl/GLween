// TextureImage.h: interface for the CTextureImage class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#if !defined(_TEXTUREIMAGE_H_INCLUDED_)
#define _TEXTUREIMAGE_H_INCLUDED_

#ifdef WIN32														// Compiling on a windows based machine
	#include <windows.h>											// Header File For Windows
#endif

#include <gl\gl.h>													// Header File For The OpenGL32 Library
#include <gl\glu.h>													// Header File For The GLu32 Library
// glaux.h was only ever #included here, never actually used (no AUX_*
// symbols anywhere in this codebase) -- Windows/Visual Studio dropped it
// years ago, so it's simply gone now.

#include <stdio.h>													// Header File For Standard I/O Operations

class CTextureImage
{
public:
	GLubyte	*imageData;												// Image Data (Up To 32 Bits)
	GLuint	bpp;													// Image Color Depth In Bits Per Pixel.
	GLuint	width;													// Image Width
	GLuint	height;													// Image Height
	GLuint	texID;													// Texture ID Used To Select A Texture

public:
	CTextureImage();												// Class Constructor
	virtual ~CTextureImage();										// Class Destructor

	BOOL LoadTGA(char *filename);									// Load A TGA Image File
	BOOL LoadTGAFromMemory(const unsigned char *bytes, unsigned int len);	// Load A TGA Already Sitting In Memory

};

#endif // !defined(_TEXTUREIMAGE_H_INCLUDED_)
