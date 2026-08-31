// GLWnd.cpp: implementation of the GLWnd class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#include "GLWnd.h"

// PlaySound()/SND_* are declared here. The original code called PlaySound()
// without ever including this -- VC6 apparently let that slide, but a
// modern compiler (correctly) won't.
#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

// Embedded texture data (see TextureImage::LoadTGAFromMemory() below).
// Only Init() needs these, so they're included here rather than in the
// header to avoid dragging several megabytes of array literals into
// every file that includes GLWnd.h.
#include "pumpkin_tga_data.h"
#include "interface_tga_data.h"
#include "face_tga_data.h"
#include "casket_tga_data.h"
#include "particle_tga_data.h"
#include "lighting_tga_data.h"
#include "ghosts_tga_data.h"
#include "fog_tga_data.h"
#include "witch_right_tga_data.h"
#include "witch_left_tga_data.h"
#include "skull_3ds_data.h"
#include "skull_tga_data.h"
#include "lighting_wav_data.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GLWnd::GLWnd()
{
	m_x = 0;														// Window X Starting Position
	m_y = 0;														// Window Y Starting Position
	m_width = 640;													// Window Width
	m_height = 480;													// Window Height
	m_fovy = 45.0f;													// Field Of View Angle, In Degrees, In The y-Direction
	m_aspect = (GLfloat)m_width / (GLfloat)m_height;				// Aspect Ratio That Determines The Field Of View in The x-Direction
	m_zNear = 0.1f;													// Distance From The Viewer To The Near Clipping Plane (Always Positive)
	m_zFar = 200.0f;												// Distance From The Viewer To The Far Clipping Plane (Always Positive)
	quadratic=gluNewQuadric();										// Create A Pointer To The Quadric Object (Return 0 If No Memory)
	quadratic2=gluNewQuadric();										// Create A Pointer To The Quadric Object (Return 0 If No Memory)

	LightAmbient[0]=	1.0f;										// Ambient Lighting Value
	LightAmbient[1]=	1.0f;										// Ambient Lighting Value
	LightAmbient[2]=	1.0f;										// Ambient Lighting Value
	LightAmbient[3]=	1.0f;										// Ambient Lighting Value
																
	LightDiffuse[0]=	1.0f;										// Diffuse Lighting Value
	LightDiffuse[1]=	1.0f;										// Diffuse Lighting Value
	LightDiffuse[2]=	1.0f;										// Diffuse Lighting Value
	LightDiffuse[3]=	1.0f;										// Diffuse Lighting Value
																
	LightSpecular[0]=	1.0f;										// Specular Lighting Value
	LightSpecular[1]=	1.0f;										// Specular Lighting Value
	LightSpecular[2]=	1.0f;										// Specular Lighting Value
	LightSpecular[3]=	1.0f;										// Specular Lighting Value
																
	LightPosition[0]=	0.0f;										// Light Position
	LightPosition[1]=	0.0f;										// Light Position
	LightPosition[2]=	2.0f;										// Light Position
	LightPosition[3]=	1.0f;										// Light Position

	slowdown=2.0f;													// Slow Down Particles
	xspeed=0.0f;													// Base X Speed (To Allow Keyboard Direction Of Tail)
	yspeed=0.0f;													// Base Y Speed (To Allow Keyboard Direction Of Tail)
	zoom=-20.0f;													// Used To Zoom Out

	m_bgRot=0.0f;													// Match The Original Function-Local Statics' Initial Values
	m_stripRot=0.0f;
	m_lightFlash=90.0f;
	m_lightBlink=0;
	m_ghostCnt1=0.0f;
	m_ghostCnt2=1.0f;

	m_witchX=-9.0f;													// Start Just Off The Left Edge
	m_witchBob=0.0f;
	m_witchFlyingRight=true;

	m_playAudio=true;												// WinMain turns this off for non-primary monitors

}

GLWnd::~GLWnd()
{

}

//////////////////////////////////////////////////////////////////////
// Resize And Initialize The GL Window
//////////////////////////////////////////////////////////////////////
GLvoid GLWnd::Resize(GLsizei width, GLsizei height = NULL)
{
	if (!(height == NULL))											// If NULL, Take The Defaults For The Class
	{
		if (height == 0)											// Prevent A Divide By Zero By
			height = 1;												// Making Height Equal One
	
		m_width  = width;											// Save The New Width Of The Window
		m_height = height;											// Save The New Height Of The Window
		m_aspect = (GLfloat)m_width / (GLfloat)m_height;			// Calculate The Aspect Ratio Of The Window
	}
	glViewport(m_x, m_y, m_width, m_height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glLoadIdentity();												// Reset The Projection Matrix
																	
	gluPerspective(m_fovy, m_aspect, m_zNear, m_zFar);				// Set The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();												// Reset The Modelview Matrix
}

//////////////////////////////////////////////////////////////////////
// Setup Initial OpenGL Scene Here
//////////////////////////////////////////////////////////////////////
BOOL GLWnd::Init(GLvoid)
{
	// Load Our Images -- from the embedded byte arrays baked into the
	// .scr (see the *_DATA symbols, one per asset header) rather than
	// files under Data\, so the screensaver ships as a single file.
	if (!(m_texPumpkin.LoadTGAFromMemory(PUMPKIN_TGA_DATA, PUMPKIN_TGA_DATA_LEN)) ||
		!(m_texOverlay.LoadTGAFromMemory(INTERFACE_TGA_DATA, INTERFACE_TGA_DATA_LEN)) ||
		!(m_texFace.LoadTGAFromMemory(FACE_TGA_DATA, FACE_TGA_DATA_LEN)) ||
		!(m_texCasket.LoadTGAFromMemory(CASKET_TGA_DATA, CASKET_TGA_DATA_LEN)) ||
		!(m_texParticle.LoadTGAFromMemory(PARTICLE_TGA_DATA, PARTICLE_TGA_DATA_LEN)) ||
		!(m_texLighting.LoadTGAFromMemory(LIGHTING_TGA_DATA, LIGHTING_TGA_DATA_LEN)) ||
		!(m_texGhosts.LoadTGAFromMemory(GHOSTS_TGA_DATA, GHOSTS_TGA_DATA_LEN)) ||
		!(m_texFog.LoadTGAFromMemory(FOG_TGA_DATA, FOG_TGA_DATA_LEN)) ||
		!(m_texWitchRight.LoadTGAFromMemory(WITCH_RIGHT_TGA_DATA, WITCH_RIGHT_TGA_DATA_LEN)) ||
		!(m_texWitchLeft.LoadTGAFromMemory(WITCH_LEFT_TGA_DATA, WITCH_LEFT_TGA_DATA_LEN)) ||
		!(m_texSkull.LoadTGAFromMemory(SKULL_TGA_DATA, SKULL_TGA_DATA_LEN)))
	{
			return FALSE;											// Error Loading Graphics, Return FALSE
	}

	if (!m_objSkull.LoadFromMemory(SKULL_3DS_DATA, SKULL_3DS_DATA_LEN))
	{
			return FALSE;											// Error Parsing The Skull Model, Return FALSE
	}

	m_objCasket.CompileList();
	m_objPumpkin.CompileList();
	m_objSkull.CompileList();

	glShadeModel(GL_SMOOTH);										// Enable Smooth Shading
	glClearColor(0.0f, 0.1f, 0.0f, 0.0f);							// Set Background Color
	glClearDepth(1.0f);												// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);										// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);											// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				// Really Nice Perspective Calculations
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);							// Really Nice Point Smoothing

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);								// Blending Function For Translucency Based On Source Alpha Value
	glEnable(GL_TEXTURE_2D);										// Enable Texture Mapping

	gluQuadricNormals(quadratic, GLU_SMOOTH);						// Create Smooth Normals 
	gluQuadricTexture(quadratic, GL_TRUE);							// Create Texture Coords 
	gluQuadricDrawStyle(quadratic, GLU_FILL);						// Render With Polygon Primitives

	gluQuadricNormals(quadratic2, GLU_SMOOTH);						// Create Smooth Normals 
	gluQuadricTexture(quadratic2, GL_TRUE);							// Create Texture Coords 
	gluQuadricDrawStyle(quadratic2, GLU_FILL);						// Render With Polygon Primitives

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);					// Setup Ambient Lighting
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);					// Setup Diffuse Lighting
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);				// Setup Specular Lighting
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);				// Position The Light
	glEnable(GL_LIGHT1);											// Enable Light Two


	///////////////////////////
	for (loop=0;loop<MAX_PARTICLES;loop++)							// Initials All The Textures
	{															
		m_Particle[loop].active=true;								// Make All The Particles Active
		m_Particle[loop].life=1.0f;									// Give All The Particles Full Life
																
		m_Particle[loop].fade=float(rand()%100)/1000.0f+0.003f;		// Random Fade Speed

		m_Particle[loop].r=colors[(loop+1)/(MAX_PARTICLES/12)][0];	// Select Red Rainbow Color
		m_Particle[loop].g=colors[(loop+1)/(MAX_PARTICLES/12)][1];	// Select Green Rainbow Color
		m_Particle[loop].b=colors[(loop+1)/(MAX_PARTICLES/12)][2];	// Select Blue Rainbow Color

		m_Particle[loop].xi=float((rand()%50)-25.0f)*50.0f;			// Random Speed On X Axis
		m_Particle[loop].yi=float((rand()%50)-25.0f)*50.0f;			// Random Speed On Y Axis
		m_Particle[loop].zi=float((rand()%50)-25.0f)*50.0f;			// Random Speed On Z Axis

		m_Particle[loop].xg=0.0f;									// Set Horizontal Pull To Zero
		m_Particle[loop].yg=-0.8f;									// Set Vertical Pull Downward
		m_Particle[loop].zg=0.0f;									// Set Pull On Z Axis To Zero
	}
	///////////////////////////

	return TRUE;													// Everything Went OK
}

//////////////////////////////////////////////////////////////////////
// Render The OpenGL Scene
// Returns TRUE if Everything Went Ok
//////////////////////////////////////////////////////////////////////
BOOL GLWnd::Render(GLvoid)
{
	//////////////////////////////////////////////////////////////////

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear Screen And Depth Buffer
	glLoadIdentity();												// Reset The Current Modelview Matrix
	//////////////////////////////////////////////////////////////////

	
	glEnable(GL_LIGHTING);											// Enable Lighting Effects

	glBindTexture(GL_TEXTURE_2D, m_texCasket.texID);				// Select The Texture
	m_objCasket.Render();

	glBindTexture(GL_TEXTURE_2D, m_texSkull.texID);				// Select The Texture
	m_objSkull.Render();

	DrawBackground();												// Draw The Background Scene
	RenderGhosts();													// Render Ghost
	RenderWitch();													// Fly The Witch Across The Screen

	RenderParticles();

	DrawPumpkinStrip();												// Draw The Pumpkin Strip
	glBindTexture(GL_TEXTURE_2D, m_texFace.texID);					// Select The Texture
	m_objPumpkin.Render();

	RenderLighting();												// Render Lighting Flashes

	glDisable(GL_LIGHTING);											// Disable Lighting Effects

	RenderParticles();

	glBindTexture(GL_TEXTURE_2D, m_texOverlay.texID);				// Select The Texture
	Interface.Render();												// Draw Our Interface

	return TRUE;													// Rendering Went OK
}

//////////////////////////////////////////////////////////////////////
// Draw The Background Scene
//////////////////////////////////////////////////////////////////////
GLvoid GLWnd::DrawBackground(GLvoid)
{
	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	glDisable(GL_LIGHTING);											// Disable Lighting Effects

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);				// Blending Function For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);												// Enable Blending

	glTranslatef(0.0f,-100.0f,0.0f);								// Center The Cylinder
	glRotatef(-90.0f,1.0f,0.0f,0.0f);
	glRotatef(m_bgRot++,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D, m_texFog.texID);					// Select The Texture
	gluCylinder(quadratic2,200.0f,200.0f,200.0f,20,5);				// A Cylinder With A Radius Of 0.5 And A Height Of 2
	
	glDisable(GL_BLEND);

	glEnable(GL_LIGHTING);											// Enable Lighting Effects

	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix
}

//////////////////////////////////////////////////////////////////////
// Draw The Pumpkin Strip
// Cheesy little wave effect on the pumpkin image by making less slices
// so the cylinder isn't so smooth.
//////////////////////////////////////////////////////////////////////
GLvoid GLWnd::DrawPumpkinStrip(GLvoid)
{
	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	glDisable(GL_LIGHTING);											// Disable Lighting Effects

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);				// Blending Function For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);												// Turn Blending On
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);			// Set The Texture Generation Mode For S
	glEnable(GL_TEXTURE_GEN_S);										// Enable Texture Coord Generation For S

	glTranslatef(0.0f,-0.45f,-1.5f);								// Center The Cylinder In The Scene
	glRotatef(-75.0f,1.0f,0.0f,0.0f);								// Rotate Cylinder In The Scene
	glRotatef(m_stripRot++,0.0f,0.0f,1.0f);							// Rotate Cylinder To Make Cheesy Wave Effect
	glBindTexture(GL_TEXTURE_2D, m_texPumpkin.texID);				// Select The Texture

	gluCylinder(quadratic,5.0f,5.0f,0.8f,20,5);						// Draw The Cylinder

	glDisable(GL_TEXTURE_GEN_S);									// Disable Texture Coord Generation For S
	glDisable(GL_BLEND);

	glEnable(GL_LIGHTING);											// Enable Lighting Effects

	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix
}

//////////////////////////////////////////////////////////////////////
// Draw The Pumpkin Strip
// Cheesy little wave effect on the pumpkin image by making less slices
// so the cylinder isn't so smooth.
//////////////////////////////////////////////////////////////////////
GLvoid GLWnd::RenderParticles(GLvoid)
{
	GLuint	loop;					// Misc Loop Variable

	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);				// Blend For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, m_texParticle.texID);					// Select The Texture
	
	for (loop=0;loop<MAX_PARTICLES;loop++)					// Loop Through All The Particles
	{

		if (m_Particle[loop].active)					// If The Particle Is Active
		{
			float x=m_Particle[loop].x;				// Grab Our Particle X Position
			float y=m_Particle[loop].y;				// Grab Our Particle Y Position
			float z=m_Particle[loop].z+zoom;				// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(m_Particle[loop].r,m_Particle[loop].g,m_Particle[loop].b,m_Particle[loop].life);

			glBegin(GL_TRIANGLE_STRIP);				// Build Quad From A Triangle Strip

				glTexCoord2d(1,1); glVertex3f(x+0.1f,y+0.1f,z); // Top Right
				glTexCoord2d(0,1); glVertex3f(x-0.1f,y+0.1f,z); // Top Left
				glTexCoord2d(1,0); glVertex3f(x+0.1f,y-0.1f,z); // Bottom Right
				glTexCoord2d(0,0); glVertex3f(x-0.1f,y-0.1f,z); // Bottom Left

			glEnd();						// Done Building Triangle Strip

			m_Particle[loop].x+=m_Particle[loop].xi/(slowdown*1000);	// Move On The X Axis By X Speed
			m_Particle[loop].y+=m_Particle[loop].yi/(slowdown*1000);	// Move On The Y Axis By Y Speed
			m_Particle[loop].z+=m_Particle[loop].zi/(slowdown*1000);	// Move On The Z Axis By Z Speed

			m_Particle[loop].xi+=m_Particle[loop].xg;				// Take Pull On X Axis Into Account
			m_Particle[loop].yi+=m_Particle[loop].yg;				// Take Pull On Y Axis Into Account
			m_Particle[loop].zi+=m_Particle[loop].zg;				// Take Pull On Z Axis Into Account

			m_Particle[loop].life-=m_Particle[loop].fade;			// Reduce Particles Life By 'Fade'

			if (m_Particle[loop].life<0.0f)							// If Particle Is Burned Out
			{

				m_Particle[loop].life=1.0f;							// Give It New Life
				m_Particle[loop].fade=float(rand()%100)/1000.0f+0.003f;	// Random Fade Value

				m_Particle[loop].x=0.0f+float((rand()%50)-26.0f)*0.4f;	// 
				m_Particle[loop].y=0.0f+float((rand()%50)-26.0f)*0.4f;	// 
				m_Particle[loop].z=0.0f+float((rand()%50)-26.0f)*0.4f;	// 

				m_Particle[loop].xi=xspeed+float((rand()%60)-32.0f);	// X Axis Speed And Direction
				m_Particle[loop].yi=yspeed+float((rand()%60)-30.0f);	// Y Axis Speed And Direction
				m_Particle[loop].zi=float((rand()%60)-30.0f);			// Z Axis Speed And Direction

				m_Particle[loop].r=colors[(loop+1)/(MAX_PARTICLES/12)][0];			// Select Red From Color Table
				m_Particle[loop].g=colors[(loop+1)/(MAX_PARTICLES/12)][1];			// Select Green From Color Table
				m_Particle[loop].b=colors[(loop+1)/(MAX_PARTICLES/12)][2];			// Select Blue From Color Table
			}

		}
    }

	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix
}

GLvoid GLWnd::RenderLighting(GLvoid)
{
	if ((m_lightFlash > 100.0f) && (m_lightFlash < 101.0f))
	{
		if (m_playAudio)												// Only The Primary Monitor's Instance Plays The Sound
		{
			// SND_MEMORY: play directly from the embedded byte array
			// instead of a Data\Lighting.wav file on disk.
			PlaySound((LPCSTR)LIGHTING_WAV_DATA, NULL, SND_MEMORY|SND_ASYNC);
		}
		glPushMatrix();													// Copy The Current Matrix
		glLoadIdentity();												// Reset The Matrix
		//////////////////////////////////////////////////////////////////
		glDisable(GL_LIGHTING);											// Disable Lighting Effects
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);				// Blend For Translucency Based On Source Alpha Value
		glEnable(GL_BLEND);

		glBindTexture(GL_TEXTURE_2D, m_texLighting.texID);				// Select The Texture

		glTranslatef( 0.0f, 0.0f, -1.0f);
		glBegin(GL_QUADS);												// Draw The Interface Shell
			glTexCoord2d(0, 1); glVertex2f( -0.5f,  0.5f);				// Top Side
			glTexCoord2d(1, 1); glVertex2f(  0.5f,  0.5f);			// Right Side
			glTexCoord2d(1, 0); glVertex2f(  0.5f, -0.5f);			// Bottom Side
			glTexCoord2d(0, 0); glVertex2f( -0.5f, -0.5f);			// Left Side
		glEnd();

		glDisable(GL_BLEND);											// Disable Blending
		glEnable(GL_LIGHTING);											// Enable Lighting Effects
		//////////////////////////////////////////////////////////////////
		glPopMatrix();													// Pop The Matrix

		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);					// Set Background Color
		if (m_lightBlink > 2)
		{
			glClearColor(0.0f, 0.1f, 0.0f, 0.0f);					// Set Background Color
		}
		else if (m_lightBlink > 5)
		{
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);					// Set Background Color
		}
		m_lightBlink += 1;
	}
	else if (m_lightFlash > 101.0f)
	{
		glClearColor(0.0f, 0.1f, 0.0f, 0.0f);				// Set Background Color
		m_lightFlash = 0.0f;
		m_lightBlink = 0;
	}

	m_lightFlash += 0.2f;
}
GLvoid GLWnd::RenderGhosts(GLvoid)
{
		glPushMatrix();													// Copy The Current Matrix
		glLoadIdentity();												// Reset The Matrix
		//////////////////////////////////////////////////////////////////
		glDisable(GL_LIGHTING);											// Disable Lighting Effects
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);				// Blend For Translucency Based On Source Alpha Value
		glEnable(GL_BLEND);

		glBindTexture(GL_TEXTURE_2D, m_texGhosts.texID);				// Select The Texture

		glTranslatef(10.0f*float(sin(m_ghostCnt1)),2.0f*float(cos(m_ghostCnt2)),-10.0f);
		glBegin(GL_QUADS);												// Draw The Interface Shell
			glTexCoord2d(0, 1); glVertex2f( -0.5f,  0.5f);				// Top Side
			glTexCoord2d(1, 1); glVertex2f(  0.5f,  0.5f);				// Right Side
			glTexCoord2d(1, 0); glVertex2f(  0.5f, -0.5f);				// Bottom Side
			glTexCoord2d(0, 0); glVertex2f( -0.5f, -0.5f);				// Left Side
		glEnd();

		glDisable(GL_BLEND);											// Disable Blending
		glEnable(GL_LIGHTING);											// Enable Lighting Effects
		//////////////////////////////////////////////////////////////////
		glPopMatrix();													// Pop The Matrix
	m_ghostCnt1-=0.005f;
	m_ghostCnt2+=0.005f;
}

//////////////////////////////////////////////////////////////////////
// Fly The Witch Back And Forth Across The Screen
//
// She flies from one edge to the other, using whichever of the two
// textures actually faces the direction she's travelling, then turns
// around (and swaps texture) once she's flown a bit past the visible
// edge of the screen. WITCH_SPEED controls how fast she crosses --
// tweak it if she still looks too fast/slow for your taste.
//////////////////////////////////////////////////////////////////////
GLvoid GLWnd::RenderWitch(GLvoid)
{
	static const GLfloat WITCH_SPEED = 0.04f;		// Units Per Frame
	static const GLfloat WITCH_Z = -8.0f;			// How Far Back She Flies
	static const GLfloat WITCH_SIZE = 1.0f;		// Half-Width/Height Of Her Quad

	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////
	glDisable(GL_LIGHTING);											// Disable Lighting Effects
	// GL_ONE (Not GL_SRC_ALPHA) Because The Embedded Witch Textures Are
	// Premultiplied -- Their RGB Is Already Scaled By Alpha, Which Avoids
	// The White Edge-Fringing That Linear-Filtering A Non-Premultiplied
	// Cutout Sprite Produces (Especially Noticeable As She Moves).
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Face The Texture That Actually Matches Her Current Direction
	glBindTexture(GL_TEXTURE_2D,
		m_witchFlyingRight ? m_texWitchRight.texID : m_texWitchLeft.texID);

	// How Wide Is The Visible Screen At Her Depth? (Same Perspective
	// Math gluPerspective() Used To Set Up The Projection In Resize())
	GLfloat halfHeight = (GLfloat)(fabs(WITCH_Z) * tan((m_fovy*0.5) * PI / 180.0));
	GLfloat halfWidth  = halfHeight * (GLfloat)m_aspect;
	GLfloat edge       = halfWidth + WITCH_SIZE;					// A Bit Past The Edge Before Turning Around

	GLfloat y = 0.1f + 0.3f*(GLfloat)sin(m_witchBob);				// Gentle Up/Down Bobbing -- Lower, Below The Border Overlay

	glTranslatef(m_witchX, y, WITCH_Z);
	glBegin(GL_QUADS);												// Draw The Witch As A Textured Quad
		glTexCoord2d(0, 1); glVertex2f(-WITCH_SIZE,  WITCH_SIZE);		// Top Left
		glTexCoord2d(1, 1); glVertex2f( WITCH_SIZE,  WITCH_SIZE);		// Top Right
		glTexCoord2d(1, 0); glVertex2f( WITCH_SIZE, -WITCH_SIZE);		// Bottom Right
		glTexCoord2d(0, 0); glVertex2f(-WITCH_SIZE, -WITCH_SIZE);		// Bottom Left
	glEnd();

	glDisable(GL_BLEND);											// Disable Blending
	glEnable(GL_LIGHTING);											// Enable Lighting Effects
	//////////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix

	if (m_witchFlyingRight)
	{
		m_witchX += WITCH_SPEED;
		if (m_witchX > edge)
			m_witchFlyingRight = false;							// Reached The Right Side -- Turn Around
	}
	else
	{
		m_witchX -= WITCH_SPEED;
		if (m_witchX < -edge)
			m_witchFlyingRight = true;								// Reached The Left Side -- Turn Around
	}

	m_witchBob += 0.05f;
}
