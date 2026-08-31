// Casket.cpp: implementation of the CCasket class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#include "Casket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCasket::CCasket()
{
	m_rot1 = 0.0f;
	m_rot2 = 0.0f;
	m_cnt1 = 0.0f;
	m_cnt2 = 0.0f;
}

CCasket::~CCasket()
{

}

//////////////////////////////////////////////////////////////////////
// Compile The Object Into A List
//////////////////////////////////////////////////////////////////////
GLvoid CCasket::CompileList(GLvoid)
{
	m_listCasket = glGenLists(2);									// Generate 1 List
	glNewList(m_listCasket,GL_COMPILE);
	int i;
	int j;

	glBegin (GL_TRIANGLES);
	for(i=0;i<sizeof(face_indiciesCasket)/sizeof(face_indiciesCasket[0]);i++)
	{
		for(j=0;j<3;j++)
		{
			int vi=face_indiciesCasket[i][j];
			int ni=face_indiciesCasket[i][j+3];
			int ti=face_indiciesCasket[i][j+6];
			glNormal3f (normalsCasket[ni][0],normalsCasket[ni][1],normalsCasket[ni][2]);
			glTexCoord2f(texturesCasket[ti][0],texturesCasket[ti][1]);
			glVertex3f (verticiesCasket[vi][0],verticiesCasket[vi][1],verticiesCasket[vi][2]);         
		}
	}
	glEnd ();
	glEndList();

	m_listCasketLid = m_listCasket + 1;

	glNewList(m_listCasketLid,GL_COMPILE);

	glBegin (GL_TRIANGLES);
	for(i=0;i<sizeof(face_indiciesCasketLid)/sizeof(face_indiciesCasketLid[0]);i++)
	{
		for(j=0;j<3;j++)
		{
			int vi=face_indiciesCasketLid[i][j];
			int ni=face_indiciesCasketLid[i][j+3];
			int ti=face_indiciesCasketLid[i][j+6];
			glNormal3f (normalsCasketLid[ni][0],normalsCasketLid[ni][1],normalsCasketLid[ni][2]);
			glTexCoord2f(texturesCasketLid[ti][0],texturesCasketLid[ti][1]);
			glVertex3f (verticiesCasketLid[vi][0],verticiesCasketLid[vi][1],verticiesCasketLid[vi][2]);         
		}
	}
	glEnd ();
	glEndList();
}

//////////////////////////////////////////////////////////////////////
// Render The Object
//////////////////////////////////////////////////////////////////////
GLvoid CCasket::Render(GLvoid)
{
	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	glDisable(GL_BLEND);											// Disable Blending

	// Move And Rotate The Casket/Lid
	glTranslatef(static_cast<float>(1.0f-sin(m_cnt1)),1.4f*static_cast<float>(cos(m_cnt1)),-3.5f);
	glRotatef(m_rot1++,0.0f,1.0f,0.0f);								// Rotate On The Y Axis
	glRotatef(m_rot2++,1.0f,0.0f,0.0f);								// Rotate On The X Axis
	glRotatef(m_rot2*2.0f,0.0f,0.0f,1.0f);							// Rotate On The Z Axis

	glCallList(m_listCasket);										// Draw The Casket
	
	// Line Up The Casket Lid With The Casket
	glTranslatef(0.0f,-0.08f,0.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);

	// To Do: Make Door Open & Shut..... Make It A Little Open Right Now
	glTranslatef(-0.14f, 0.0f, 0.0f);
	glRotatef(40.0f,0.0f,0.0f,1.0f);
	glCallList(m_listCasketLid);									// Draw The Casket Lid
	
	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix

	//--------------------------------------------------------------//
	// Render Casket #2; Casket Moves Into & Out Of The Scene		//
	//--------------------------------------------------------------//

	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	// Move And Rotate The Casket/Lid
	glTranslatef(static_cast<float>(-0.5f-sin(m_cnt1)),-0.4f*static_cast<float>(cos(m_cnt1)),4.0f/float(cos(m_cnt1+5.0f)));
	glRotatef(-m_rot1++,0.0f,1.0f,0.0f);								// Rotate On The Y Axis
	glRotatef(m_rot2++*-1.2f,1.0f,0.0f,0.0f);							// Rotate On The X Axis
	glRotatef(m_rot2,0.0f,0.0f,1.0f);									// Rotate On The Z Axis

	glCallList(m_listCasket);										// Draw The Casket
	
	// Line Up The Casket Lid With The Casket
	glTranslatef(0.0f,-0.08f,0.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);

	// To Do: Make Door Open & Shut..... Make It A Little Open Right Now
	glTranslatef(-0.14f, 0.0f, 0.0f);
	glRotatef(40.0f,0.0f,0.0f,1.0f);
	glCallList(m_listCasketLid);									// Draw The Casket Lid
	
	glEnable(GL_BLEND);												// Enable Blending
	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix

	m_cnt1+=0.005f;													// Increase The First Counter
	m_cnt2+=0.005f;													// Increase The First Counter
}
