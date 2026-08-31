// Skull.cpp: implementation of the CSkull class.
//////////////////////////////////////////////////////////////////////

#include "Skull.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////
// .3ds Chunk IDs -- Only The Handful This Loader Actually Needs.
// Everything else is skipped over using each chunk's own declared
// length, which is how a minimal reader like this can safely ignore
// chunk types it doesn't understand.
//////////////////////////////////////////////////////////////////////
#define CHUNK_MAIN3DS			0x4D4D								// Whole File
#define CHUNK_EDIT3DS			0x3D3D								// 3D Editor Chunk
#define CHUNK_EDIT_OBJECT		0x4000								// Object Block (Name-Terminated String Follows)
#define CHUNK_OBJ_TRIMESH		0x4100								// Triangle Mesh
#define CHUNK_TRI_VERTEXL		0x4110								// Vertex List
#define CHUNK_TRI_MAPPINGCOORS	0x4140								// Texture (u,v) Coordinate List
#define CHUNK_TRI_FACEL1		0x4120								// Face (Triangle Index) List

//////////////////////////////////////////////////////////////////////
// Little-Endian Reads From An Arbitrary Byte Offset
//
// Done a byte at a time rather than by casting/dereferencing a pointer
// so this works regardless of the buffer's alignment.
//////////////////////////////////////////////////////////////////////
static inline unsigned short Read16(const unsigned char *p)
{
	return (unsigned short)(p[0] | (p[1] << 8));
}

static inline unsigned int Read32(const unsigned char *p)
{
	return (unsigned int)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

static inline float ReadFloat(const unsigned char *p)
{
	unsigned int bits = Read32(p);
	float f;
	memcpy(&f, &bits, sizeof(f));
	return f;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkull::CSkull()
{
	m_listSkull = 0;
	m_rot1 = 0.0f;
	m_rot2 = 0.0f;
	m_cnt1 = 0.0f;
}

CSkull::~CSkull()
{

}

//////////////////////////////////////////////////////////////////////
// Walk The Chunk Tree Between startOffset And endOffset, Pulling Out
// Vertices/Faces/Texture Coordinates As They're Found. A Model Can Be
// Built From Several Separate Objects (E.g. A Skull Cap And A Separate
// Jaw) -- Each OBJ_TRIMESH Found Is Handed Off To ParseTriMesh() Below,
// Which Merges It Into The Combined Model Rather Than Keeping Only The
// First One.
//////////////////////////////////////////////////////////////////////
GLvoid CSkull::ParseChunk(const unsigned char *data, unsigned int startOffset, unsigned int endOffset)
{
	unsigned int pos = startOffset;

	while (pos + 6 <= endOffset)
	{
		unsigned short chunkId  = Read16(data + pos);
		unsigned int   chunkLen = Read32(data + pos + 2);

		// A Corrupt Or Truncated Chunk -- Bail Out Rather Than Loop Forever
		if (chunkLen < 6 || pos + chunkLen > endOffset)
			break;

		unsigned int childStart = pos + 6;
		unsigned int childEnd   = pos + chunkLen;

		switch (chunkId)
		{
			case CHUNK_MAIN3DS:
			case CHUNK_EDIT3DS:
				ParseChunk(data, childStart, childEnd);				// Just A Container -- Recurse Into It
				break;

			case CHUNK_EDIT_OBJECT:
			{
				// An Object Block Starts With A Null-Terminated Name
				unsigned int namePos = childStart;
				while (namePos < childEnd && data[namePos] != 0)
					namePos++;
				namePos++;												// Skip The Null Terminator
				if (namePos <= childEnd)
					ParseChunk(data, namePos, childEnd);
				break;
			}

			case CHUNK_OBJ_TRIMESH:
				ParseTriMesh(data, childStart, childEnd);				// One Object's Geometry -- Merge It In
				break;

			default:
				break;												// Unneeded Chunk -- Skip It (pos += chunkLen Below)
		}

		pos += chunkLen;
	}
}

//////////////////////////////////////////////////////////////////////
// Parse One Object's Trimesh Chunk (Vertices/UVs/Faces, Local To This
// Object) And Append It To The Combined m_vertices/m_uvs/m_faces --
// Face Indices Are Rebased By However Many Vertices Are Already In The
// Combined Model So Everything Still Points At The Right Place.
//////////////////////////////////////////////////////////////////////
GLvoid CSkull::ParseTriMesh(const unsigned char *data, unsigned int startOffset, unsigned int endOffset)
{
	std::vector<GLfloat> localVerts;
	std::vector<GLfloat> localUVs;
	std::vector<unsigned short> localFaces;

	unsigned int pos = startOffset;

	while (pos + 6 <= endOffset)
	{
		unsigned short chunkId  = Read16(data + pos);
		unsigned int   chunkLen = Read32(data + pos + 2);

		if (chunkLen < 6 || pos + chunkLen > endOffset)
			break;

		unsigned int childStart = pos + 6;
		unsigned int childEnd   = pos + chunkLen;

		switch (chunkId)
		{
			case CHUNK_TRI_VERTEXL:
				if (childStart + 2 <= childEnd)
				{
					unsigned short count = Read16(data + childStart);
					unsigned int p = childStart + 2;
					for (unsigned short i = 0; i < count && p + 12 <= childEnd; i++)
					{
						float x = ReadFloat(data + p);
						float y = ReadFloat(data + p + 4);
						float z = ReadFloat(data + p + 8);
						// .3ds Is Z-Up; This Scene (Like The Rest Of GLween)
						// Is Y-Up, So Swap Y/Z On The Way In.
						localVerts.push_back(x);
						localVerts.push_back(z);
						localVerts.push_back(-y);
						p += 12;
					}
				}
				break;

			case CHUNK_TRI_MAPPINGCOORS:
				if (childStart + 2 <= childEnd)
				{
					unsigned short count = Read16(data + childStart);
					unsigned int p = childStart + 2;
					for (unsigned short i = 0; i < count && p + 8 <= childEnd; i++)
					{
						// The TGA This Texture Was Converted From Is Stored
						// Top-Down, But .3ds Mapping Coordinates Assume A
						// Bottom-Up Image -- Flip V So The Texture Lands
						// Right-Side Up On The Model.
						localUVs.push_back(ReadFloat(data + p));
						localUVs.push_back(1.0f - ReadFloat(data + p + 4));
						p += 8;
					}
				}
				break;

			case CHUNK_TRI_FACEL1:
				if (childStart + 2 <= childEnd)
				{
					unsigned short count = Read16(data + childStart);
					unsigned int p = childStart + 2;
					for (unsigned short i = 0; i < count && p + 8 <= childEnd; i++)
					{
						localFaces.push_back(Read16(data + p));
						localFaces.push_back(Read16(data + p + 2));
						localFaces.push_back(Read16(data + p + 4));
						// p+6 Is A Per-Face Edge-Visibility Flag -- Not Needed Here
						p += 8;
					}
				}
				// Any TRI_MATERIAL (0x4130) Sub-Chunk Nested In Here Is Just
				// Skipped, Like Any Other Chunk We Don't Care About -- We
				// Already Know Which Texture To Use From GLWnd.
				break;

			default:
				break;												// Unneeded Chunk -- Skip It (pos += chunkLen Below)
		}

		pos += chunkLen;
	}

	if (localVerts.empty() || localFaces.empty())
		return;														// Nothing Usable In This Object

	// Rebase This Object's (Locally-Numbered) Face Indices By However Many
	// Vertices Are Already In The Combined Model, Then Append Everything.
	unsigned int vertexOffset = (unsigned int)(m_vertices.size() / 3);

	for (size_t i = 0; i < localFaces.size(); i++)
		m_faces.push_back((unsigned short)(localFaces[i] + vertexOffset));

	for (size_t i = 0; i < localVerts.size(); i++)
		m_vertices.push_back(localVerts[i]);

	// Keep m_uvs In Step With m_vertices One-For-One Even If This Object
	// Had No Mapping Coordinates Of Its Own.
	size_t localVertCount = localVerts.size() / 3;
	for (size_t i = 0; i < localVertCount; i++)
	{
		if (i < localUVs.size()/2)
		{
			m_uvs.push_back(localUVs[i*2]);
			m_uvs.push_back(localUVs[i*2+1]);
		}
		else
		{
			m_uvs.push_back(0.0f);
			m_uvs.push_back(0.0f);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Recenter The Model On Its Own Origin And Scale It So Its Largest
// Dimension Is targetSize -- Otherwise Whatever Arbitrary Scale/Offset
// The Model Was Exported At Would Determine How It Looks (And Spins)
// In The Scene.
//////////////////////////////////////////////////////////////////////
GLvoid CSkull::NormalizeModel(GLfloat targetSize)
{
	if (m_vertices.empty())
		return;

	GLfloat minX = m_vertices[0], maxX = m_vertices[0];
	GLfloat minY = m_vertices[1], maxY = m_vertices[1];
	GLfloat minZ = m_vertices[2], maxZ = m_vertices[2];

	for (size_t i = 0; i < m_vertices.size(); i += 3)
	{
		GLfloat x = m_vertices[i], y = m_vertices[i+1], z = m_vertices[i+2];
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
		if (z < minZ) minZ = z;
		if (z > maxZ) maxZ = z;
	}

	GLfloat cx = (minX+maxX)*0.5f, cy = (minY+maxY)*0.5f, cz = (minZ+maxZ)*0.5f;

	GLfloat extent = maxX-minX;
	if (maxY-minY > extent) extent = maxY-minY;
	if (maxZ-minZ > extent) extent = maxZ-minZ;
	if (extent < 0.0001f) extent = 1.0f;

	GLfloat scale = targetSize / extent;

	for (size_t i = 0; i < m_vertices.size(); i += 3)
	{
		m_vertices[i]   = (m_vertices[i]   - cx) * scale;
		m_vertices[i+1] = (m_vertices[i+1] - cy) * scale;
		m_vertices[i+2] = (m_vertices[i+2] - cz) * scale;
	}
}

//////////////////////////////////////////////////////////////////////
// Parse A .3ds File Sitting In Memory
//////////////////////////////////////////////////////////////////////
BOOL CSkull::LoadFromMemory(const unsigned char *data, unsigned int len)
{
	m_vertices.clear();
	m_uvs.clear();
	m_faces.clear();

	if (!data || len < 6 || Read16(data) != CHUNK_MAIN3DS)
		return FALSE;

	ParseChunk(data, 0, len);

	if (m_vertices.empty() || m_faces.empty())
		return FALSE;

	// A Comfortable Size Given The Rest Of The Scene's Scale (The Casket,
	// For Comparison, Spans Roughly 1-2 Units) -- Tweak If The Skull Looks
	// Too Big/Small Next To Everything Else.
	NormalizeModel(1.25f);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Compile The Object Into A List
//////////////////////////////////////////////////////////////////////
GLvoid CSkull::CompileList(GLvoid)
{
	m_listSkull = glGenLists(1);
	glNewList(m_listSkull, GL_COMPILE);

	glBegin(GL_TRIANGLES);
	size_t faceCount = m_faces.size() / 3;
	size_t vertexCount = m_vertices.size() / 3;
	size_t uvCount = m_uvs.size() / 2;

	for (size_t f = 0; f < faceCount; f++)
	{
		unsigned short idx[3] =
		{
			m_faces[f*3+0],
			m_faces[f*3+1],
			m_faces[f*3+2],
		};

		if (idx[0] >= vertexCount || idx[1] >= vertexCount || idx[2] >= vertexCount)
			continue;												// Corrupt/Out-Of-Range Face -- Skip It

		GLfloat *a = &m_vertices[idx[0]*3];
		GLfloat *b = &m_vertices[idx[1]*3];
		GLfloat *c = &m_vertices[idx[2]*3];

		// .3ds Doesn't Store Per-Vertex Normals (Just Smoothing Groups),
		// So This Computes One Flat Normal Per Triangle -- The Skull Will
		// Look A Little Faceted Rather Than Perfectly Smooth, Which Is
		// Fine For A Prop Like This.
		GLfloat ux=b[0]-a[0], uy=b[1]-a[1], uz=b[2]-a[2];
		GLfloat vx=c[0]-a[0], vy=c[1]-a[1], vz=c[2]-a[2];
		GLfloat nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
		GLfloat nlen = (GLfloat)sqrt(nx*nx+ny*ny+nz*nz);
		if (nlen > 0.00001f)
		{
			nx/=nlen; ny/=nlen; nz/=nlen;
		}
		glNormal3f(nx, ny, nz);

		GLfloat *verts[3] = { a, b, c };
		for (int k = 0; k < 3; k++)
		{
			if (idx[k] < uvCount)
				glTexCoord2f(m_uvs[idx[k]*2], m_uvs[idx[k]*2+1]);
			else
				glTexCoord2f(0.0f, 0.0f);

			glVertex3f(verts[k][0], verts[k][1], verts[k][2]);
		}
	}
	glEnd();

	glEndList();
}

//////////////////////////////////////////////////////////////////////
// Render The Object -- Moves And Spins The Same Way CCasket Does: A
// Slow sin/cos Wander Around The Scene Plus A Continuous Rotation.
//////////////////////////////////////////////////////////////////////
GLvoid CSkull::Render(GLvoid)
{
	glPushMatrix();													// Copy The Current Matrix
	glLoadIdentity();												// Reset The Matrix
	//////////////////////////////////////////////////////////////////

	glDisable(GL_BLEND);											// Disable Blending

	// Wander Slowly Around The Scene
	glTranslatef(3.0f*static_cast<float>(sin(m_cnt1)), 1.5f*static_cast<float>(cos(m_cnt1*1.3f)), -8.0f+2.0f*static_cast<float>(cos(m_cnt1*0.6f)));

	// Spin Slowly
	glRotatef(m_rot1, 0.0f, 1.0f, 0.0f);								// Rotate On The Y Axis
	glRotatef(m_rot2, 1.0f, 0.0f, 0.0f);								// Gentle Tilt On The X Axis

	glCallList(m_listSkull);										// Draw The Skull

	//////////////////////////////////////////////////////////////////
	glPopMatrix();													// Pop The Matrix

	m_cnt1 += 0.004f;												// Position-Wander Counter (Slow -- ~50s Per Loop At 30fps)
	m_rot1 += 0.75f;												// Spin (~16s Per Revolution At 30fps)
	m_rot2 += 0.25f;												// Secondary Tilt, A Bit Slower Than The Main Spin
}
