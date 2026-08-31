// Interface.cpp: implementation of the CInterface class.
//
// Code created by: Jim Strong © 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#include "Interface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterface::CInterface()
{
	m_left = 0;														// Window Left Starting Position
	m_right = 640;													// Window Right Starting Position
	m_bottom = 480;													// Window Bottom
	m_top = 0;														// Window Top
	// Setting m_zNear Clipping Plane To This Number Seems To Ensure
	//  That This Interface Never Get Overwriten By The 3D Objects When
	// Then Come At The User.
//	m_zNear = -0.00001f;											// Distance From The Viewer To The Near Clipping Plane (Negative If Plane Behind Viewer)
//	m_zFar = 1.0f;													// Distance From The Viewer To The Far Clipping Plane (Negative If Plane Behind Viewer)
	m_zNear = -1.0f;											// Distance From The Viewer To The Near Clipping Plane (Negative If Plane Behind Viewer)
	m_zFar = 1.0f;													// Distance From The Viewer To The Far Clipping Plane (Negative If Plane Behind Viewer)

	m_Visible = TRUE;												// Show Interface By Default
}

CInterface::~CInterface()
{

}

//////////////////////////////////////////////////////////////////////
// Render The Interface
// Returns TRUE if Everything Went Ok
//////////////////////////////////////////////////////////////////////
BOOL CInterface::Render(GLvoid)
{
	if (!m_Visible)													// Return If Interface Is Invisible
		return TRUE;

	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////
	glMatrixMode(GL_PROJECTION);									// Switch To The Projection Matrix
	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Identity
	glOrtho(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);		// Setup Orthographic View
	/////////////////////////////////////////////////////////////////

	glDisable(GL_LIGHTING);											// Disable Lighting Effects
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);				// Blending Function For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);												// Enable Blending

	glBegin(GL_QUADS);												// Draw The Interface Shell
		glTexCoord2d(0, 1); glVertex2d( m_left, m_top);				// Top Side
		glTexCoord2d(1, 1); glVertex2d( m_right, m_top);			// Right Side
		glTexCoord2d(1, 0); glVertex2d( m_right, m_bottom);			// Bottom Side
		glTexCoord2d(0, 0); glVertex2d( m_left, m_bottom);			// Left Side
	glEnd();

	glDisable(GL_BLEND);											// Disable Blending
	glEnable(GL_LIGHTING);											// Enable Lighting Effects

	/////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop Our Matrix Back
	glMatrixMode(GL_MODELVIEW);										// Set Matrix Back To Modelview
	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix

	glFinish();														// Don't Return Until All Called OpenGL Functions Are Complete
																	// Stops The Flickering Effect

	return TRUE;													// Rendering Went Ok
}
