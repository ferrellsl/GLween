// Pumpkin.cpp: implementation of the CPumpkin class.
//
// Code created by: Jim Strong � 2000
//					jim@scn.net - http://jim.iwarp.com
//////////////////////////////////////////////////////////////////////

#include "Pumpkin.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPumpkin::CPumpkin()
{
	m_rot1 = 0.0f;
	m_rot2 = 0.0f;
	m_cnt1 = 0.0f;
	m_cnt2 = 0.0f;
}

CPumpkin::~CPumpkin()
{

}

//////////////////////////////////////////////////////////////////////
// Compile The Object Into A List
//////////////////////////////////////////////////////////////////////
GLvoid CPumpkin::CompileList(GLvoid)
{
	m_listPumpkin = glGenLists(2);									// Generate 1 List
	glNewList(m_listPumpkin,GL_COMPILE);
	int i;
	int j;

	glBegin (GL_TRIANGLES);
	for(i=0;i<sizeof(face_indicies)/sizeof(face_indicies[0]);i++)
	{
		for(j=0;j<3;j++)
		{
			int vi=face_indicies[i][j];
			int ni=face_indicies[i][j+3];
			int ti=face_indicies[i][j+6];
			glNormal3f (normals[ni][0],normals[ni][1],normals[ni][2]);
			glTexCoord2f(textures[ti][0],textures[ti][1]);
			glVertex3f (verticies[vi][0],verticies[vi][1],verticies[vi][2]);         
		}
	}
	glEnd ();
	glEndList();
}

//////////////////////////////////////////////////////////////////////
// Render The Object
//////////////////////////////////////////////////////////////////////
GLvoid CPumpkin::Render(GLvoid)
{
	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);				// Blend For Translucency Based On Source Alpha Value
	glEnable(GL_BLEND);

	glTranslatef(static_cast<float>(sin(m_cnt1)),1.4f*static_cast<float>(cos(m_cnt1)),-5.0f);

	glCallList(m_listPumpkin);

	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix

	m_cnt1+=0.005f;													// Increase The First Counter
	m_cnt2+=0.005f;													// Increase The First Counter

}
