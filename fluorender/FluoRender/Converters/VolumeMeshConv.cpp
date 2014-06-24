#include "VolumeMeshConv.h"
#include "MCTable.h"
#include "../FLIVR/Utils.h"
#include "../compatibility.h"

double VolumeMeshConv::m_sw = 0.0;

VolumeMeshConv::VolumeMeshConv() :
	m_volume(0),
	m_mask(0),
	m_iso(0.5),
	m_downsample(0),
	m_downsample_z(0),
	m_vol_max(255.0),
	m_nx(0), m_ny(0), m_nz(0),
	m_spcx(1.0), m_spcy(1.0), m_spcz(1.0),
	m_use_transfer(false),
	m_gamma(1.0),
	m_lo_thresh(0.0),
	m_hi_thresh(1.0),
	m_offset(1.0),
	m_gm_thresh(0.0),
	m_use_mask(false)
{
	m_mesh = new GLMmodel;
	m_mesh->pathname = 0;
	m_mesh->mtllibname = 0;
	m_mesh->numvertices = 0;
	m_mesh->vertices = 0;
	m_mesh->numnormals = 0;
	m_mesh->normals = 0;
	m_mesh->numtexcoords = 0;
	m_mesh->texcoords = 0;
	m_mesh->numfacetnorms = 0;
	m_mesh->facetnorms = 0;
	m_mesh->numtriangles = 0;
	m_mesh->triangles = 0;
	m_mesh->nummaterials = 0;
	m_mesh->materials = 0;
	m_mesh->numgroups = 0;
	m_mesh->groups = 0;
	m_mesh->position[0] = 0.0f;
	m_mesh->position[1] = 0.0f;
	m_mesh->position[2] = 0.0f;
	m_mesh->hastexture = false;
}

VolumeMeshConv::~VolumeMeshConv()
{
}

void VolumeMeshConv::SetVolume(Nrrd* volume)
{
	m_volume = volume;
}

Nrrd* VolumeMeshConv::GetVolume()
{
	return m_volume;
}

GLMmodel* VolumeMeshConv::GetMesh()
{
	return m_mesh;
}

void VolumeMeshConv::SetVolumeSpacings(double x, double y, double z)
{
	m_spcx = x;
	m_spcy = y;
	m_spcz = z;
}

void VolumeMeshConv::SetVolumeUseTrans(bool use)
{
	m_use_transfer = use;
}

void VolumeMeshConv::SetVolumeTransfer(double gamma, double lo_thresh,
	double hi_thresh, double offset, double gm_thresh)
{
	m_gamma = gamma;
	m_lo_thresh = lo_thresh;
	m_hi_thresh = hi_thresh;
	m_offset = offset;
	m_gm_thresh = gm_thresh;
}

void VolumeMeshConv::SetVolumeUseMask(bool use)
{
	m_use_mask = use;
}

void VolumeMeshConv::SetVolumeMask(Nrrd* mask)
{
	m_mask = mask;
}

void VolumeMeshConv::SetMaxValue(double mv)
{
	m_vol_max = mv;
}

void VolumeMeshConv::SetIsoValue(double iso)
{
	m_iso = iso;
}

void VolumeMeshConv::SetDownsample(int downsample)
{
	m_downsample = downsample;
}

void VolumeMeshConv::SetDownsampleZ(int downsample)
{
	m_downsample_z = downsample;
}

void VolumeMeshConv::Convert()
{
	if (!m_volume || m_volume->dim!=3)
		return;
	if (m_mesh)
		glmClear(m_mesh);
	if (m_downsample <= 0)
		m_downsample = 1;
	if (m_downsample_z <= 0)
		m_downsample_z = 1;

	//triangle list
	vector<MCTriangle> tri_list;

	//add default group
	GLMgroup* group = new GLMgroup;
	group->name = STRDUP("default");
	group->material = 0;
	group->numtriangles = 0;
	group->triangles = 0;
	group->next = 0;
	m_mesh->groups = group;
	m_mesh->numgroups = 1;

	//get volume info
	m_nx = int(m_volume->axis[0].size);
	m_ny = int(m_volume->axis[1].size);
	m_nz = int(m_volume->axis[2].size);

	//marching cubes
	//parse the volume data
	//it has a 1 voxel border, in case the values touch the border
	int i, j, k;
	for (i=-1; i<m_nx+m_downsample; i+=m_downsample)
	for (j=-1; j<m_ny+m_downsample; j+=m_downsample)
	for (k=-1; k<m_nz+m_downsample_z; k+=m_downsample_z)
	{
		//current cube is (i, j, k)
		//27 neighbors
		double neighbors[3][3][3];
		int ii, jj, kk;
		for (ii=0; ii<3; ii++)
		for (jj=0; jj<3; jj++)
		for (kk=0; kk<3; kk++)
			neighbors[ii][jj][kk] =
			GetValue(i+(ii-1)*m_downsample,
			j+(jj-1)*m_downsample, k+(kk-1)*m_downsample_z);
		//8 vertices
		double verts[8];
		verts[0] = GetMaxNeighbor(neighbors, 0, 0, 0);
		verts[1] = GetMaxNeighbor(neighbors, 1, 0, 0);
		verts[2] = GetMaxNeighbor(neighbors, 1, 1, 0);
		verts[3] = GetMaxNeighbor(neighbors, 0, 1, 0);
		verts[4] = GetMaxNeighbor(neighbors, 0, 0, 1);
		verts[5] = GetMaxNeighbor(neighbors, 1, 0, 1);
		verts[6] = GetMaxNeighbor(neighbors, 1, 1, 1);
		verts[7] = GetMaxNeighbor(neighbors, 0, 1, 1);

		//calculate cube index
		int cubeindex = 0;
		for (int n=0; n<8; n++)
			if (verts[n] <= m_iso)
				cubeindex |= (1<<n);

		//check if it's completely inside or outside
		if (!edgeTable[cubeindex])
			continue;

		//get intersection vertices on edge
		Vector intverts[12];
		if(edgeTable[cubeindex] & 1) intverts[0] = Intersect(verts, 0, 1, i, j, k);
		if(edgeTable[cubeindex] & 2) intverts[1] = Intersect(verts, 1, 2, i, j, k);
		if(edgeTable[cubeindex] & 4) intverts[2] = Intersect(verts, 2, 3, i, j, k);
		if(edgeTable[cubeindex] & 8) intverts[3] = Intersect(verts, 3, 0, i, j, k);
		if(edgeTable[cubeindex] & 16) intverts[4] = Intersect(verts, 4, 5, i, j, k);
		if(edgeTable[cubeindex] & 32) intverts[5] = Intersect(verts, 5, 6, i, j, k);
		if(edgeTable[cubeindex] & 64) intverts[6] = Intersect(verts, 6, 7, i, j, k);
		if(edgeTable[cubeindex] & 128) intverts[7] = Intersect(verts, 7, 4, i, j, k);
		if(edgeTable[cubeindex] & 256) intverts[8] = Intersect(verts, 0, 4, i, j, k);
		if(edgeTable[cubeindex] & 512) intverts[9] = Intersect(verts, 1, 5, i, j, k);
		if(edgeTable[cubeindex] & 1024) intverts[10] = Intersect(verts, 2, 6, i, j, k);
		if(edgeTable[cubeindex] & 2048) intverts[11] = Intersect(verts, 3, 7, i, j, k);

		//build triangles
		for (int n=0; triTable[cubeindex][n] != -1; n+=3)
		{
			MCTriangle tri;
			tri.p[0] = intverts[triTable[cubeindex][n+2]];
			tri.p[1] = intverts[triTable[cubeindex][n+1]];
			tri.p[2] = intverts[triTable[cubeindex][n]];
			tri_list.push_back(tri);
		}
	}

	int numtriangles = int(tri_list.size());
	m_mesh->numvertices = numtriangles*3;
	m_mesh->numtriangles = numtriangles;
	m_mesh->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (m_mesh->numvertices + 1));
	m_mesh->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
		m_mesh->numtriangles);
	group->triangles = (GLuint*)malloc(sizeof(GLuint) * numtriangles);

	GLfloat* vertices = m_mesh->vertices;
	int numvertices = 1;
	numtriangles = 0;
	for (int n=0; n<(int)tri_list.size(); n++)
	{
		MCTriangle tri = tri_list[n];

		//copy data
		//vertex 1
		vertices[3*numvertices + 0] = GLfloat(tri.p[0].x());
		vertices[3*numvertices + 1] = GLfloat(tri.p[0].y());
		vertices[3*numvertices + 2] = GLfloat(tri.p[0].z());
		//triangle
		m_mesh->triangles[numtriangles].vindices[0] = numvertices;
		numvertices++;
		//vertex 2
		vertices[3*numvertices + 0] = GLfloat(tri.p[1].x());
		vertices[3*numvertices + 1] = GLfloat(tri.p[1].y());
		vertices[3*numvertices + 2] = GLfloat(tri.p[1].z());
		//triangle
		m_mesh->triangles[numtriangles].vindices[1] = numvertices;
		numvertices++;
		//vertex 3
		vertices[3*numvertices + 0] = GLfloat(tri.p[2].x());
		vertices[3*numvertices + 1] = GLfloat(tri.p[2].y());
		vertices[3*numvertices + 2] = GLfloat(tri.p[2].z());
		//triangle
		m_mesh->triangles[numtriangles].vindices[2] = numvertices;
		numvertices++;

		//group
		group->triangles[group->numtriangles++] = (GLint)numtriangles;
		numtriangles++;
	}
	
}

double VolumeMeshConv::GetValue(int x, int y, int z)
{
	if (!m_volume || !m_volume->data)
		return 0.0;
	if (x<0 || x>=m_nx ||
		y<0 || y>=m_ny ||
		z<0 || z>=m_nz)
		return 0.0;

	double value = 0.0;
	int index = m_nx*m_ny*z + m_nx*y + x;
	if (m_volume->type == nrrdTypeUChar)
	{
		value = ((unsigned char*)m_volume->data)[index];
		value /= 255.0;
		if (m_use_transfer)
		{
			double gm = 0.0;
			if (x>0 && x<m_nx-1 &&
				y>0 && y<m_ny-1 &&
				z>0 && z<m_nz-1)
			{
				double v1 = ((unsigned char*)(m_volume->data))[m_nx*m_ny*z + m_nx*y + (x-1)];
				double v2 = ((unsigned char*)(m_volume->data))[m_nx*m_ny*z + m_nx*y + (x+1)];
				double v3 = ((unsigned char*)(m_volume->data))[m_nx*m_ny*z + m_nx*(y-1) + x];
				double v4 = ((unsigned char*)(m_volume->data))[m_nx*m_ny*z + m_nx*(y+1) + x];
				double v5 = ((unsigned char*)(m_volume->data))[m_nx*m_ny*(z-1) + m_nx*y + x];
				double v6 = ((unsigned char*)(m_volume->data))[m_nx*m_ny*(z+1) + m_nx*y + x];
				double normal_x, normal_y, normal_z;
				normal_x = (v2 - v1) / 255.0;
				normal_y = (v4 - v3) / 255.0;
				normal_z = (v6 - v5) / 255.0;
				gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
			}
			if (value<m_lo_thresh-m_sw ||
				value>m_hi_thresh+m_sw ||
				gm<m_gm_thresh)
				value = 0.0;
			else
			{
				double gamma = 1.0 / m_gamma;
				value = (value<m_lo_thresh?
					(m_sw-m_lo_thresh+value)/m_sw:
					(value>m_hi_thresh?
					(m_sw-value+m_hi_thresh)/m_sw:1.0))
					*value;
				value = pow(Clamp(value/m_offset,
					gamma<1.0?-(gamma-1.0)*0.00001:0.0,
					gamma>1.0?0.9999:1.0), gamma);
			}
		}
	}
	else if (m_volume->type == nrrdTypeUShort)
	{
		value = ((unsigned short*)m_volume->data)[index];
		value /= m_vol_max;
		if (m_use_transfer)
		{
			double gm = 0.0;
			if (x>0 && x<m_nx-1 &&
				y>0 && y<m_ny-1 &&
				z>0 && z<m_nz-1)
			{
				double v1 = ((unsigned short*)(m_volume->data))[m_nx*m_ny*z + m_nx*y + (x-1)];
				double v2 = ((unsigned short*)(m_volume->data))[m_nx*m_ny*z + m_nx*y + (x+1)];
				double v3 = ((unsigned short*)(m_volume->data))[m_nx*m_ny*z + m_nx*(y-1) + x];
				double v4 = ((unsigned short*)(m_volume->data))[m_nx*m_ny*z + m_nx*(y+1) + x];
				double v5 = ((unsigned short*)(m_volume->data))[m_nx*m_ny*(z-1) + m_nx*y + x];
				double v6 = ((unsigned short*)(m_volume->data))[m_nx*m_ny*(z+1) + m_nx*y + x];
				double normal_x, normal_y, normal_z;
				normal_x = (v2 - v1) / m_vol_max;
				normal_y = (v4 - v3) / m_vol_max;
				normal_z = (v6 - v5) / m_vol_max;
				gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
			}
			if (value<m_lo_thresh-m_sw ||
				value>m_hi_thresh+m_sw ||
				gm<m_gm_thresh)
				value = 0.0;
			else
			{
				double gamma = 1.0 / m_gamma;
				value = (value<m_lo_thresh?
					(m_sw-m_lo_thresh+value)/m_sw:
					(value>m_hi_thresh?
					(m_sw-value+m_hi_thresh)/m_sw:1.0))
					*value;
				value = pow(Clamp(value/m_offset,
					gamma<1.0?-(gamma-1.0)*0.00001:0.0,
					gamma>1.0?0.9999:1.0), gamma);
			}
		}
	}

	if (m_use_mask && m_mask)
	{
		double mask_value = ((unsigned char*)m_mask->data)[index];
		mask_value /= 255.0;
		value *= mask_value;
	}

	return value;
}

double VolumeMeshConv::GetMaxNeighbor(double neighbors[3][3][3],
	int xx, int yy, int zz)
{
	double v1 = neighbors[xx][yy][zz];
	double v2 = neighbors[xx+1][yy][zz];
	double v3 = neighbors[xx][yy+1][zz];
	double v4 = neighbors[xx+1][yy+1][zz];
	double v5 = neighbors[xx][yy][zz+1];
	double v6 = neighbors[xx+1][yy][zz+1];
	double v7 = neighbors[xx][yy+1][zz+1];
	double v8 = neighbors[xx+1][yy+1][zz+1];

	return Max(v1, Max(v2, Max(v3, Max(v4, Max(v5, Max(v6, Max(v7, v8)))))));
}

Vector VolumeMeshConv::Intersect(double verts[8], int v1, int v2,
	int x, int y, int z)
{
	double val1 = verts[v1];
	double val2 = verts[v2];
	Vector p1 = Vector(cubeTable[v1][0], cubeTable[v1][1], cubeTable[v1][2]);
	Vector p2 = Vector(cubeTable[v2][0], cubeTable[v2][1], cubeTable[v2][2]);

	Vector p;
	if (val1 != val2)
		p = p1 + (p2 - p1) * (m_iso - val1) / (val2 - val1);
	else
		p = p1;

	p = Vector(x*m_spcx, y*m_spcy, z*m_spcz) +
		Vector(p.x()*m_spcx*m_downsample, p.y()*m_spcy*m_downsample, p.z()*m_spcz*m_downsample_z);

	return p;
}
