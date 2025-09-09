/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <ConvVolMeshSw.h>
#include <DataManager.h>
#include <RenderView.h>
#include <Texture.h>
#include <VolumeRenderer.h>
#include <MCTable.h>
#include <Utils.h>
#include <compatibility.h>
#include <glm.h>
#include <nrrd.h>
#include <vector>

using namespace flrd;

ConvVolMeshSw::ConvVolMeshSw() :
	m_area(0),
	BaseConvVolMesh()
{
}

ConvVolMeshSw::~ConvVolMeshSw()
{
}

void ConvVolMeshSw::Convert()
{
	auto vd = m_volume.lock();
	if (!vd)
		return;
	m_mesh = std::make_shared<MeshData>();
	std::wstring name = vd->GetName();
	m_mesh->SetName(name + L"_MESH");
	GLMmodel* model = new GLMmodel;
	if (!model)
	{
		SetProgress(0, "FluoRender failed to allocate memory for mesh.");
		return;
	}
	model->pathname = 0;
	model->mtllibname = 0;
	model->numvertices = 0;
	model->vertices = 0;
	model->numnormals = 0;
	model->normals = 0;
	model->numtexcoords = 0;
	model->texcoords = 0;
	model->numfacetnorms = 0;
	model->facetnorms = 0;
	model->numlines = 0;
	model->lines = 0;
	model->numtriangles = 0;
	model->triangles = 0;
	model->nummaterials = 0;
	model->materials = 0;
	model->numgroups = 0;
	model->groups = 0;
	model->position[0] = 0.0f;
	model->position[1] = 0.0f;
	model->position[2] = 0.0f;
	model->hastexture = false;

	vd->GetSpacings(m_spcx, m_spcy, m_spcz);

	SetProgress(0, "FluoRender is converting volume to mesh. Please wait.");
	if (m_vertex_merge)
		SetRange(0, 80);
	else
		SetRange(0, 100);
	//start converting
	if (!Compute((void*)model))
	{
		SetProgress(0, "");
		return;
	}

	SetRange(0, 100);
	if (m_vertex_merge)
	{
		SetProgress(80, "FluoRender is welding vertices. Please wait.");
		glmWeld(model, static_cast<GLfloat>(0.001 * fluo::Min(m_spcx, m_spcy, m_spcz)));
	}
	float scale[3] = { 1.0f, 1.0f, 1.0f };
	glmArea(model, scale, &m_area);

	m_info = std::to_string(m_area);

	m_mesh->Load(model);

	SetProgress(0, "");
}

void ConvVolMeshSw::Update(bool create_mesh)
{
	//no update for software version
}

bool ConvVolMeshSw::Compute(void* m)
{
	auto vd = m_volume.lock();
	if (!vd)
		return false;
	if (!m_mesh)
		return false;
	GLMmodel* model = static_cast<GLMmodel*>(m);
	if (!model)
		return false;

	VolumeData* vol_data = vd.get();
	Nrrd* nrrd_data = nullptr;
	if (m_use_mask)
		nrrd_data = vd->GetMask(true);
	else
		nrrd_data = vd->GetVolume(false);
	if (!nrrd_data)
	{
		SetProgress(0, "FluoRender failed to get volume data.");
		return false;
	}

	if (nrrd_data->dim!=3)
		return false;
	if (m_downsample <= 0)
		m_downsample = 1;
	if (m_downsample_z <= 0)
		m_downsample_z = 1;

	//triangle list
	std::vector<MCTriangle> tri_list;

	//add default group
	GLMgroup* group = new GLMgroup;
	group->name = STRDUP("default");
	group->material = 0;
	group->numtriangles = 0;
	group->triangles = 0;
	group->next = 0;
	model->groups = group;
	model->numgroups = 1;

	//get volume info
	int nx = int(nrrd_data->axis[0].size);
	int ny = int(nrrd_data->axis[1].size);
	int nz = int(nrrd_data->axis[2].size);

	int v1, v2, range;
	v1 = GetMin();
	v2 = GetMax();
	range = GetRange();
	SetRange(v1, v1 + range / 2);

	//marching cubes
	//parse the volume data
	//it has a 1 voxel border, in case the values touch the border
	int i, j, k;
	for (k = -1; k < nz + m_downsample_z; k += m_downsample_z)
	{
		SetProgress(100 * (k + 1) / (nz + m_downsample_z + 1),
			"FluoRender is converting volume to mesh. Please wait.");
		for (j = -1; j < ny + m_downsample; j += m_downsample)
		for (i = -1; i < nx + m_downsample; i += m_downsample)
		{
			//current cube is (i, j, k)
			//27 neighbors
			double neighbors[3][3][3];
			int ii, jj, kk;
			for (ii = 0; ii < 3; ii++)
				for (jj = 0; jj < 3; jj++)
					for (kk = 0; kk < 3; kk++)
						neighbors[ii][jj][kk] =
						GetValue(vol_data, i + (ii - 1) * m_downsample,
							j + (jj - 1) * m_downsample, k + (kk - 1) * m_downsample_z);
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
			for (int n = 0; n < 8; n++)
				if (verts[n] <= m_iso)
					cubeindex |= (1 << n);

			//check if it's completely inside or outside
			if (!edgeTable[cubeindex])
				continue;

			//get intersection vertices on edge
			fluo::Vector intverts[12];
			if (edgeTable[cubeindex] & 1) intverts[0] = Intersect(verts, 0, 1, i, j, k);
			if (edgeTable[cubeindex] & 2) intverts[1] = Intersect(verts, 1, 2, i, j, k);
			if (edgeTable[cubeindex] & 4) intverts[2] = Intersect(verts, 2, 3, i, j, k);
			if (edgeTable[cubeindex] & 8) intverts[3] = Intersect(verts, 3, 0, i, j, k);
			if (edgeTable[cubeindex] & 16) intverts[4] = Intersect(verts, 4, 5, i, j, k);
			if (edgeTable[cubeindex] & 32) intverts[5] = Intersect(verts, 5, 6, i, j, k);
			if (edgeTable[cubeindex] & 64) intverts[6] = Intersect(verts, 6, 7, i, j, k);
			if (edgeTable[cubeindex] & 128) intverts[7] = Intersect(verts, 7, 4, i, j, k);
			if (edgeTable[cubeindex] & 256) intverts[8] = Intersect(verts, 0, 4, i, j, k);
			if (edgeTable[cubeindex] & 512) intverts[9] = Intersect(verts, 1, 5, i, j, k);
			if (edgeTable[cubeindex] & 1024) intverts[10] = Intersect(verts, 2, 6, i, j, k);
			if (edgeTable[cubeindex] & 2048) intverts[11] = Intersect(verts, 3, 7, i, j, k);

			//build triangles
			for (int n = 0; triTable[cubeindex][n] != -1; n += 3)
			{
				MCTriangle tri;
				tri.p[0] = intverts[triTable[cubeindex][n + 2]];
				tri.p[1] = intverts[triTable[cubeindex][n + 1]];
				tri.p[2] = intverts[triTable[cubeindex][n]];
				tri_list.push_back(tri);
			}
		}
	}

	SetRange(v1 + range / 2, v2);

	int numtriangles = int(tri_list.size());
	model->numvertices = numtriangles*3;
	model->numtriangles = numtriangles;
	model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numvertices + 1));
	model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
		model->numtriangles);
	group->triangles = (GLuint*)malloc(sizeof(GLuint) * numtriangles);

	GLfloat* vertices = model->vertices;
	int numvertices = 1;
	numtriangles = 0;
	size_t tri_list_size = tri_list.size();
	for (size_t n = 0; n < tri_list_size; ++n)
	{
		SetProgress(static_cast<int>(100 * n / tri_list_size),
			"FluoRender is converting volume to mesh. Please wait.");

		MCTriangle tri = tri_list[n];

		//copy data
		//vertex 1
		vertices[3*numvertices + 0] = GLfloat(tri.p[0].x());
		vertices[3*numvertices + 1] = GLfloat(tri.p[0].y());
		vertices[3*numvertices + 2] = GLfloat(tri.p[0].z());
		//triangle
		model->triangles[numtriangles].vindices[0] = numvertices;
		numvertices++;
		//vertex 2
		vertices[3*numvertices + 0] = GLfloat(tri.p[1].x());
		vertices[3*numvertices + 1] = GLfloat(tri.p[1].y());
		vertices[3*numvertices + 2] = GLfloat(tri.p[1].z());
		//triangle
		model->triangles[numtriangles].vindices[1] = numvertices;
		numvertices++;
		//vertex 3
		vertices[3*numvertices + 0] = GLfloat(tri.p[2].x());
		vertices[3*numvertices + 1] = GLfloat(tri.p[2].y());
		vertices[3*numvertices + 2] = GLfloat(tri.p[2].z());
		//triangle
		model->triangles[numtriangles].vindices[2] = numvertices;
		numvertices++;

		//group
		group->triangles[group->numtriangles++] = (GLint)numtriangles;
		numtriangles++;
	}
	
	SetRange(v1, v2);

	return true;
}

double ConvVolMeshSw::GetValue(VolumeData* vd, int x, int y, int z)
{
	if (!vd)
		return 0.0;
	if (m_use_mask)
	{
		double v = vd->GetMaskValue(x, y, z);
		if (v > 0.0)
		{
			if (m_use_transfer)
				return vd->GetTransferedValue(x, y, z);
			else
				return vd->GetOriginalValue(x, y, z);
		}
	}
	else
	{
		if (m_use_transfer)
			return vd->GetTransferedValue(x, y, z);
		else
			return vd->GetOriginalValue(x, y, z);
	}
	return 0.0;
}

double ConvVolMeshSw::GetMaxNeighbor(double neighbors[3][3][3],
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

	return std::max(v1, std::max(v2, std::max(v3, std::max(v4, std::max(v5, std::max(v6, std::max(v7, v8)))))));
}

fluo::Vector ConvVolMeshSw::Intersect(double verts[8], int v1, int v2,
	int x, int y, int z)
{
	double val1 = verts[v1];
	double val2 = verts[v2];
	fluo::Vector p1(cubeTable[v1][0], cubeTable[v1][1], cubeTable[v1][2]);
	fluo::Vector p2(cubeTable[v2][0], cubeTable[v2][1], cubeTable[v2][2]);

	fluo::Vector p;
	if (val1 != val2)
		p = p1 + (p2 - p1) * (m_iso - val1) / (val2 - val1);
	else
		p = p1;

	p = fluo::Vector(x*m_spcx, y*m_spcy, z*m_spcz) +
		fluo::Vector(p.x()*m_spcx*m_downsample, p.y()*m_spcy*m_downsample, p.z()*m_spcz*m_downsample_z);

	return p;
}
