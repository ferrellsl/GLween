// Skull.h: interface for the CSkull class.
//
// Loads a (single-mesh) Autodesk 3D Studio (.3ds) model plus its texture,
// both from in-memory buffers (see the embedded *_DATA arrays baked into
// the .scr) rather than files on disk, and renders it moving around the
// scene the same way CCasket does -- a slow sin/cos wander plus a
// continuous rotation.
//
// This is a minimal, single-material .3ds reader -- it understands just
// enough of the chunk format (walking every chunk by its declared length
// and only paying attention to the few chunk IDs that carry vertices/
// faces/texture coordinates) to load a simple prop like a skull. It DOES
// merge multiple objects within the file (e.g. a skull cap and a
// separate jaw object, which is how these models are commonly built) --
// each object's own trimesh is parsed and appended to one combined
// vertex/face list. It does not handle multiple materials or local
// coordinate systems -- if a fancier model is ever dropped in here, this
// loader would need extending.
//////////////////////////////////////////////////////////////////////

#if !defined(_SKULL_H_INCLUDED_)
#define _SKULL_H_INCLUDED_

#ifdef WIN32														// Compiling on a windows based machine
	#include <windows.h>											// Header File For Windows
	#include <math.h>												// Header File For Windows Math Library
#endif

#include <gl\gl.h>													// Header File For The OpenGL32 Library
#include <gl\glu.h>													// Header File For The GLu32 Library

#include <vector>

class CSkull
{
private:
	std::vector<GLfloat>			m_vertices;						// x,y,z Per Vertex
	std::vector<GLfloat>			m_uvs;								// u,v Per Vertex
	std::vector<unsigned short>	m_faces;							// 3 Vertex Indices Per Triangle

	GLuint	m_listSkull;											// Compiled Display List

	// Movement state -- same style as CCasket's m_rot1/m_rot2/m_cnt1/m_cnt2
	// (ordinary per-instance members, not statics -- see the comment in
	// GLWnd.h about why that matters for a multi-monitor screensaver).
	GLfloat	m_rot1;													// Slow Spin
	GLfloat	m_rot2;													// Gentle Secondary Tilt
	GLfloat	m_cnt1;													// Position-Wander Counter

	GLvoid ParseChunk(const unsigned char *data, unsigned int startOffset, unsigned int endOffset);
	GLvoid ParseTriMesh(const unsigned char *data, unsigned int startOffset, unsigned int endOffset);	// One Object's Mesh -- Merged Into m_vertices/m_uvs/m_faces
	GLvoid NormalizeModel(GLfloat targetSize);							// Center On Origin & Scale To A Reasonable Size

public:
	CSkull();															// Class Constructor
	virtual ~CSkull();												// Class Destructor

	BOOL LoadFromMemory(const unsigned char *data, unsigned int len);	// Parse The .3ds Data
	GLvoid CompileList(GLvoid);										// Compile The Object Into A List
	GLvoid Render(GLvoid);											// Render & Move The Object
};

#endif // !defined(_SKULL_H_INCLUDED_)
