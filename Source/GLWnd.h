// GLWnd.h: interface for the GLWnd class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#if !defined(_GLWND_H__INCLUDED_)
#define _GLWND_H__INCLUDED_

#ifdef WIN32														// Compiling on a windows based machine
	#include <windows.h>											// Header File For Windows
	#include <math.h>												// Header File For Windows Math Library
#endif

#include <gl\gl.h>													// Header File For The OpenGL32 Library
#include <gl\glu.h>													// Header File For The GLu32 Library
// glaux.h was only ever #included here, never actually used (no AUX_*
// symbols anywhere in this codebase) -- Windows/Visual Studio dropped it
// years ago, so it's simply gone now.

#include "Interface.h"												// Header File For The Interface Object

#include "TextureImage.h"											// Header File For Loading Our Textures
#include "Casket.h"													// Added by ClassView
#include "Pumpkin.h"												// Added by ClassView
#include "Particle.h"												// Added by ClassView
#include "Skull.h"													// Header File For The Skull Object

#define PI 3.1415926535												// Define The Value Of PI
#define MAX_PARTICLES 500											// Maximum Number Of Particles

	static GLfloat colors[12][3]=									// Rainbow Of Colors
	{
		{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
		{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
		{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
	};

// (No longer an `extern HDC hDC` here -- it was unused even in the
// original code, and the screensaver version manages one HDC/HGLRC per
// monitor window rather than a single global pair.)

class GLWnd
{
private:
	GLsizei m_x;													// Window X Starting Position
	GLsizei m_y;													// Window Y Starting Position
	GLsizei m_width;												// Window Width
	GLsizei m_height;												// Window Height
	GLdouble m_fovy;												// Field Of View Angle, In Degrees, In The y-Direction
	GLdouble m_aspect;												// Aspect Ratio That Determines The Field Of View in The x-Direction
	GLdouble m_zNear;												// Distance From The Viewer To The Near Clipping Plane (Always Positive)
	GLdouble m_zFar;												// Distance From The Viewer To The Far Clipping Plane (Always Positive)

	GLfloat LightAmbient[4];										// Ambient Light
	GLfloat LightDiffuse[4];										// Diffuse Light
	GLfloat LightSpecular[4];										// Specular Light
	GLfloat LightPosition[4];										// Light Position

	GLUquadricObj *quadratic;										// Storage For Our Quadratic Objects
	GLUquadricObj *quadratic2;										// Storage For Our Quadratic Objects

	CTextureImage m_texOverlay;										// Texture
	CTextureImage m_texPumpkin;										// Texture
	CTextureImage m_texFog;											// Texture
	CTextureImage m_texCasket;										// Texture
	CTextureImage m_texFace;										// Texture
	CTextureImage m_texParticle;									// Texture
	CTextureImage m_texLighting;									// Texture
	CTextureImage m_texGhosts;										// Texture
	CTextureImage m_texWitchRight;									// Texture -- Witch, Facing Right
	CTextureImage m_texWitchLeft;									// Texture -- Witch, Facing Left
	CTextureImage m_texSkull;										// Texture -- Skull

	CCasket m_objCasket;											// Create Casket Object
	CPumpkin m_objPumpkin;											// Create Pumpkin Object
	CSkull m_objSkull;												// Create Skull Object

	CParticle m_Particle[MAX_PARTICLES];

	GLvoid DrawBackground(GLvoid);									// Draw The Background Scene
	GLvoid DrawPumpkinStrip(GLvoid);								// Draw The Background Scene
	GLvoid RenderParticles(GLvoid);									// Draw Particles
	GLvoid RenderLighting(GLvoid);									// Render Lighting Effects
	GLvoid RenderGhosts(GLvoid);									// Render Ghosts
	GLvoid RenderWitch(GLvoid);										// Fly The Witch Across The Screen

	bool	keys[256];												// Array Used For The Keyboard Routine
	GLuint	loop;													// Misc Loop Variable
	GLuint	col;													// Current Color Selection
	float	slowdown;												// Slow Down Particles
	float	xspeed;													// Base X Speed (To Allow Keyboard Direction Of Tail)
	float	yspeed;													// Base Y Speed (To Allow Keyboard Direction Of Tail)
	float	zoom;													// Used To Zoom Out

	// These used to be function-local `static` variables. That was fine
	// when exactly one GLWnd ever existed, but a multi-monitor screensaver
	// creates one GLWnd per monitor -- and a `static` inside a member
	// function is shared by ALL instances, not per-object. With several
	// instances calling Render() once per frame each, those shared statics
	// would have ticked several times per logical frame (animation running
	// N times too fast on an N-monitor setup). Moving them to ordinary
	// member variables makes each monitor's instance independently and
	// correctly paced.
	GLfloat	m_bgRot;												// DrawBackground() rotation
	GLfloat	m_stripRot;												// DrawPumpkinStrip() rotation
	GLfloat	m_lightFlash;											// RenderLighting() flash timer
	int		m_lightBlink;											// RenderLighting() blink counter
	GLfloat	m_ghostCnt1;											// RenderGhosts() position counter
	GLfloat	m_ghostCnt2;											// RenderGhosts() position counter

	GLfloat	m_witchX;												// RenderWitch() horizontal position
	GLfloat	m_witchBob;												// RenderWitch() up/down bobbing phase
	bool	m_witchFlyingRight;										// RenderWitch() current direction of travel

public:
	CInterface Interface;											// Create The Interace Object

	// Only the "primary" instance actually plays the thunder sound --
	// otherwise every monitor's independently-timed lightning strike
	// would fight over PlaySound() and cut each other off. Defaults to
	// true; WinMain turns it off for every monitor but the first.
	bool	m_playAudio;

public:
	GLWnd();														// Class Constructor
	virtual ~GLWnd();												// Class Destructor

	BOOL Init(GLvoid);												// Setup Initial OpenGL Scene Here
	GLvoid Resize(GLsizei width, GLsizei height);					// Resize And Initialize The GL Window
	BOOL Render(GLvoid);											// Render The OpenGL Scene

};

#endif // !defined(_GLWND_H__INCLUDED_)
