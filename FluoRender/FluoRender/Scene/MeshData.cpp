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
#include <GL/glew.h>
#include <MeshData.h>
#include <MeshRenderer.h>
#include <VertexArray.h>
#include <compatibility.h>
#include <glm.h>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <unordered_map>

MeshData::MeshData() :
	m_data(nullptr, glmDelete),
	m_center(0.0, 0.0, 0.0),
	m_disp(true),
	m_draw_bounds(false),
	m_light(true),
	m_flat_shading(false),
	m_mat_amb(0.3, 0.3, 0.3),
	m_mat_diff(1.0, 0.0, 0.0),
	m_mat_spec(0.2, 0.2, 0.2),
	m_mat_shine(30.0),
	m_mat_alpha(1.0),
	m_shadow_enable(true),
	m_shadow_intensity(0.6),
	m_enable_limit(false),
	m_limit(50),
	m_cpu_dirty(true),
	m_gpu_dirty(true)
{
	type = 3;//mesh

	m_trans[0] = 0.0;
	m_trans[1] = 0.0;
	m_trans[2] = 0.0;
	m_rot[0] = 0.0;
	m_rot[1] = 0.0;
	m_rot[2] = 0.0;
	m_scale[0] = 1.0;
	m_scale[1] = 1.0;
	m_scale[2] = 1.0;

	double hue, sat, val;
	hue = double(std::rand()%360);
	sat = 1.0;
	val = 1.0;
	fluo::Color color(fluo::HSVColor(hue, sat, val));
	m_mat_diff = color;

	m_legend = true;

	m_mr = std::make_unique<flvr::MeshRenderer>();
}

MeshData::~MeshData()
{
}

//set viewport
void MeshData::SetViewport(GLint vp[4])
{
	if (m_mr)
		m_mr->set_viewport(vp);
}

int MeshData::Load(GLMmodel* mesh)
{
	if (!mesh) return 0;

	m_data_path = L"";
	m_name = L"New Mesh";

	m_data = GLMmodelPtr(mesh, glmDelete);

	//if (!m_data->normals)
	//{
	//	if (!m_data->facetnorms)
	//		glmFacetNormals(m_data.get());
	//	glmVertexNormals(m_data.get(), 89.0);
	//}

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	m_data->materials[0].name = NULL;
	m_data->materials[0].ambient[0] = m_mat_amb.r();
	m_data->materials[0].ambient[1] = m_mat_amb.g();
	m_data->materials[0].ambient[2] = m_mat_amb.b();
	m_data->materials[0].ambient[3] = m_mat_alpha;
	m_data->materials[0].diffuse[0] = m_mat_diff.r();
	m_data->materials[0].diffuse[1] = m_mat_diff.g();
	m_data->materials[0].diffuse[2] = m_mat_diff.b();
	m_data->materials[0].diffuse[3] = m_mat_alpha;
	m_data->materials[0].specular[0] = m_mat_spec.r();
	m_data->materials[0].specular[1] = m_mat_spec.g();
	m_data->materials[0].specular[2] = m_mat_spec.b();
	m_data->materials[0].specular[3] = m_mat_alpha;
	m_data->materials[0].shininess = m_mat_shine;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = GL_FALSE;
	m_data->materials[0].textureID = 0;

	//bounds
	GLfloat fbounds[6];
	glmBoundingBox(m_data.get(), fbounds);
	fluo::BBox bounds;
	fluo::Point pmin(fbounds[0], fbounds[2], fbounds[4]);
	fluo::Point pmax(fbounds[1], fbounds[3], fbounds[5]);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;
	m_center = fluo::Point(
		(m_bounds.Min().x()+m_bounds.Max().x())*0.5,
		(m_bounds.Min().y()+m_bounds.Max().y())*0.5,
		(m_bounds.Min().z()+m_bounds.Max().z())*0.5);

	SetFlatShading(m_data->numnormals == 0);

	SubmitData();

	return 1;
}

int MeshData::Load(const std::wstring &filename)
{
	m_data_path = filename;
	std::filesystem::path p(filename);
	m_name = p.filename().wstring();

	std::string str_fn = ws2s(filename);
	bool no_fail = true;
	GLMmodel* new_model = glmReadOBJ(str_fn.c_str(), &no_fail);
	if (!new_model)
		return 0;
	m_data.reset(new_model);

	//if (!m_data->normals && m_data->numtriangles)
	//{
	//	if (!m_data->facetnorms)
	//		glmFacetNormals(m_data.get());
	//	glmVertexNormals(m_data.get(), 89.0);
	//}

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	m_data->materials[0].name = NULL;
	m_data->materials[0].ambient[0] = m_mat_amb.r();
	m_data->materials[0].ambient[1] = m_mat_amb.g();
	m_data->materials[0].ambient[2] = m_mat_amb.b();
	m_data->materials[0].ambient[3] = m_mat_alpha;
	m_data->materials[0].diffuse[0] = m_mat_diff.r();
	m_data->materials[0].diffuse[1] = m_mat_diff.g();
	m_data->materials[0].diffuse[2] = m_mat_diff.b();
	m_data->materials[0].diffuse[3] = m_mat_alpha;
	m_data->materials[0].specular[0] = m_mat_spec.r();
	m_data->materials[0].specular[1] = m_mat_spec.g();
	m_data->materials[0].specular[2] = m_mat_spec.b();
	m_data->materials[0].specular[3] = m_mat_alpha;
	m_data->materials[0].shininess = m_mat_shine;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = GL_FALSE;
	m_data->materials[0].textureID = 0;

	//bounds
	GLfloat fbounds[6];
	glmBoundingBox(m_data.get(), fbounds);
	fluo::BBox bounds;
	fluo::Point pmin(fbounds[0], fbounds[2], fbounds[4]);
	fluo::Point pmax(fbounds[1], fbounds[3], fbounds[5]);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;
	m_center = fluo::Point(
		(m_bounds.Min().x()+m_bounds.Max().x())*0.5,
		(m_bounds.Min().y()+m_bounds.Max().y())*0.5,
		(m_bounds.Min().z()+m_bounds.Max().z())*0.5);

	SetFlatShading(m_data->numnormals == 0);

	SubmitData();

	return 1;
}

void MeshData::Save(const std::wstring& filename)
{
	if (m_gpu_dirty)
		ReturnData();

	if (m_data)
	{
		std::string str = ws2s(filename);
		glmWriteOBJ(m_data.get(), str.c_str(), GLM_SMOOTH);
		m_data_path = filename;
	}
}

void MeshData::SubmitData()
{
	if (!m_data || !m_mr)
		return;

	m_mr->set_data(m_data.get());
	bool bnormal = m_data->normals;
	bool btexcoord = m_data->texcoords;

	// Map composite (v, vt, vn) to unique vertex index
	struct VertexKey {
		unsigned int v, vt, vn;
		bool operator==(const VertexKey& other) const {
			return v == other.v && vt == other.vt && vn == other.vn;
		}
	};
	struct VertexKeyHash {
		size_t operator()(const VertexKey& k) const {
			return std::hash<unsigned int>()(k.v) ^ std::hash<unsigned int>()(k.vt << 1) ^ std::hash<unsigned int>()(k.vn << 2);
		}
	};

	std::unordered_map<VertexKey, unsigned int, VertexKeyHash> vertex_map;
	std::vector<float> vertex_buffer;
	std::vector<unsigned int> index_buffer;

	GLMgroup* group = m_data->groups;
	while (group)
	{
		for (size_t i = 0; i < group->numtriangles; ++i)
		{
			GLMtriangle* tri = &m_data->triangles[group->triangles[i]];
			for (size_t j = 0; j < 3; ++j)
			{
				VertexKey key = {
					tri->vindices[j],
					btexcoord ? tri->tindices[j] : 0,
					bnormal ? tri->nindices[j] : 0
				};

				auto it = vertex_map.find(key);
				if (it != vertex_map.end())
				{
					index_buffer.push_back(it->second);
				}
				else
				{
					unsigned int new_index = static_cast<unsigned int>(vertex_buffer.size() / (3 + (bnormal ? 3 : 0) + (btexcoord ? 2 : 0)));
					vertex_map[key] = new_index;
					index_buffer.push_back(new_index);

					// Position
					vertex_buffer.push_back(m_data->vertices[3 * key.v]);
					vertex_buffer.push_back(m_data->vertices[3 * key.v + 1]);
					vertex_buffer.push_back(m_data->vertices[3 * key.v + 2]);

					// Normal
					if (bnormal)
					{
						vertex_buffer.push_back(m_data->normals[3 * key.vn]);
						vertex_buffer.push_back(m_data->normals[3 * key.vn + 1]);
						vertex_buffer.push_back(m_data->normals[3 * key.vn + 2]);
					}

					// Texcoord
					if (btexcoord)
					{
						vertex_buffer.push_back(m_data->texcoords[2 * key.vt]);
						vertex_buffer.push_back(m_data->texcoords[2 * key.vt + 1]);
					}
				}
			}
		}
		group = group->next;
	}

	flvr::VertexArray* va_model = m_mr->GetOrCreateVertexArray(true, true);
	if (!va_model)
		return;

	// Upload vertex buffer
	va_model->buffer_data(
		flvr::VABuf_Coord,
		sizeof(float) * vertex_buffer.size(),
		&vertex_buffer[0],
		GL_STATIC_DRAW);

	// Upload index buffer
	va_model->buffer_data(
		flvr::VABuf_Index,
		sizeof(unsigned int) * index_buffer.size(),
		&index_buffer[0],
		GL_STATIC_DRAW);

	// Set attribute pointers
	GLsizei stride = sizeof(float) * (3 + (bnormal ? 3 : 0) + (btexcoord ? 2 : 0));
	va_model->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0); // Position

	if (bnormal)
		va_model->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(sizeof(float) * 3)); // Normal

	if (btexcoord)
	{
		GLuint tex_slot = bnormal ? 2 : 1;
		GLsizei tex_offset = sizeof(float) * (bnormal ? 6 : 3);
		va_model->attrib_pointer(tex_slot, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(size_t)tex_offset); // Texcoord
	}

	m_gpu_dirty = false;
}

void MeshData::ReturnData()
{
	if (!m_data || !m_mr)
		return;

	flvr::VertexArray* va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;

	bool bnormal = m_data->normals;
	bool btexcoord = m_data->texcoords;
	bool indexed = va_model->is_indexed();

	GLsizei stride = sizeof(float) * (3 + (bnormal ? 3 : 0) + (btexcoord ? 2 : 0));

	// Determine vertex and triangle counts
	size_t num_vertices = m_data->numvertices;
	size_t num_triangles = m_data->numtriangles;
	size_t index_count = num_triangles * 3;
	if (num_triangles == 0)
		return;

	// Map vertex buffer
	std::vector<float> verts(stride / sizeof(float) * num_vertices);
	void* mapped_ptr = va_model->map_buffer(flvr::VABuf_Coord, GL_READ_ONLY);
	if (!mapped_ptr)
		return;
	memcpy(verts.data(), mapped_ptr, verts.size() * sizeof(float));
	va_model->unmap_buffer(flvr::VABuf_Coord);

	// Map index buffer if needed
	std::vector<GLuint> indices;
	if (indexed)
	{
		void* index_ptr = va_model->map_buffer(flvr::VABuf_Index, GL_READ_ONLY);
		if (!index_ptr)
			return;
		indices.resize(index_count);
		memcpy(indices.data(), index_ptr, sizeof(GLuint) * index_count);
		va_model->unmap_buffer(flvr::VABuf_Index);
	}

	// Allocate vertex arrays with 1-based indexing
	if (m_data->vertices)
		free(m_data->vertices);
	m_data->vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (num_vertices + 1));
	m_data->numvertices = (GLuint)num_vertices;

	if (bnormal && !m_data->normals)
		m_data->normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (num_vertices + 1));
	if (btexcoord && !m_data->texcoords)
		m_data->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (num_vertices + 1));

	// Transform setup
	glm::vec3 center(m_center.x(), m_center.y(), m_center.z());
	glm::vec3 trans(m_trans[0], m_trans[1], m_trans[2]);
	glm::vec3 rot(glm::radians(m_rot[0]), glm::radians(m_rot[1]), glm::radians(m_rot[2]));
	glm::vec3 scale(m_scale[0], m_scale[1], m_scale[2]);

	glm::mat4 M = glm::mat4(1.0f);
	M = glm::translate(M, trans + center);
	M = glm::rotate(M, rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
	M = glm::rotate(M, rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate(M, rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
	M = glm::scale(M, scale);
	M = glm::translate(M, -center);
	glm::mat3 N = glm::transpose(glm::inverse(glm::mat3(M)));

	// Copy vertex data
	size_t offset = 0;
	for (size_t i = 1; i <= num_vertices; ++i)
	{
		glm::vec3 v(verts[offset], verts[offset + 1], verts[offset + 2]);
		v = glm::vec3(M * glm::vec4(v, 1.0f));
		m_data->vertices[3 * i + 0] = v.x;
		m_data->vertices[3 * i + 1] = v.y;
		m_data->vertices[3 * i + 2] = v.z;
		offset += 3;

		if (bnormal)
		{
			glm::vec3 n(verts[offset], verts[offset + 1], verts[offset + 2]);
			n = glm::normalize(N * n);
			m_data->normals[3 * i + 0] = n.x;
			m_data->normals[3 * i + 1] = n.y;
			m_data->normals[3 * i + 2] = n.z;
			offset += 3;
		}

		if (btexcoord)
		{
			m_data->texcoords[2 * i + 0] = verts[offset++];
			m_data->texcoords[2 * i + 1] = verts[offset++];
		}
	}

	// Allocate triangle array
	if (m_data->triangles)
		free(m_data->triangles);
	m_data->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) * num_triangles);
	m_data->numtriangles = (GLuint)num_triangles;

	for (size_t i = 0; i < num_triangles; ++i)
	{
		GLMtriangle& tri = m_data->triangles[i];
		if (indexed)
		{
			tri.vindices[0] = indices[i * 3 + 0] + 1;
			tri.vindices[1] = indices[i * 3 + 1] + 1;
			tri.vindices[2] = indices[i * 3 + 2] + 1;
		}
		else
		{
			tri.vindices[0] = (GLuint)(i * 3 + 1);
			tri.vindices[1] = (GLuint)(i * 3 + 2);
			tri.vindices[2] = (GLuint)(i * 3 + 3);
		}
		tri.findex = 0;
	}

	// Rebuild group triangle lists
	size_t tri_offset = 0;
	GLMgroup* group = m_data->groups;
	while (group)
	{
		group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
		for (size_t i = 0; i < group->numtriangles; ++i)
		{
			group->triangles[i] = (GLuint)(tri_offset + i);
		}
		tri_offset += group->numtriangles;
		group = group->next;
	}

	m_cpu_dirty = false;
}

void MeshData::AddEmptyData()
{
	GLMmodel* model = new GLMmodel;
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

	//add default group
	GLMgroup* group = new GLMgroup;
	group->name = STRDUP("default");
	group->material = 0;
	group->numtriangles = 0;
	group->triangles = 0;
	group->next = 0;
	model->groups = group;
	model->numgroups = 1;

	m_data = GLMmodelPtr(model, glmDelete);

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	m_data->materials[0].name = NULL;
	m_data->materials[0].ambient[0] = m_mat_amb.r();
	m_data->materials[0].ambient[1] = m_mat_amb.g();
	m_data->materials[0].ambient[2] = m_mat_amb.b();
	m_data->materials[0].ambient[3] = m_mat_alpha;
	m_data->materials[0].diffuse[0] = m_mat_diff.r();
	m_data->materials[0].diffuse[1] = m_mat_diff.g();
	m_data->materials[0].diffuse[2] = m_mat_diff.b();
	m_data->materials[0].diffuse[3] = m_mat_alpha;
	m_data->materials[0].specular[0] = m_mat_spec.r();
	m_data->materials[0].specular[1] = m_mat_spec.g();
	m_data->materials[0].specular[2] = m_mat_spec.b();
	m_data->materials[0].specular[3] = m_mat_alpha;
	m_data->materials[0].shininess = m_mat_shine;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = GL_FALSE;
	m_data->materials[0].textureID = 0;

	m_mr->set_data(m_data.get());
}

void MeshData::ClearData()
{
	GLMmodel* model = m_mr->get_data();
	glmClearGeometry(model);
	//add default group
	GLMgroup* group = new GLMgroup;
	group->name = STRDUP("default");
	group->material = 0;
	group->numtriangles = 0;
	group->triangles = 0;
	group->next = 0;
	model->groups = group;
	model->numgroups = 1;
}

GLuint MeshData::AddCoordVBO(int vertex_size)
{
	std::vector<float> verts(vertex_size * 45);
	size_t vbo_size = sizeof(float) * verts.size();
	flvr::VertexArray* va_model = m_mr->GetOrCreateVertexArray(true, false);
	va_model->buffer_data(
		flvr::VABuf_Coord, vbo_size,
		&verts[0], GL_DYNAMIC_DRAW);
	va_model->attrib_pointer(
		0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
	GLuint vbo_id = static_cast<GLuint>(va_model->id_buffer(flvr::VABuf_Coord));
	return vbo_id;
}

GLuint MeshData::ConvertIndexed(size_t vsize)
{
	flvr::VertexArray* va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	va_model->add_index_buffer();
	//assume coord buffer is a list of triangles
	std::vector<unsigned int> indices(vsize);
	for (size_t i = 0; i < vsize; ++i)
		indices[i] = static_cast<unsigned int>(i);
	size_t vbo_size = sizeof(unsigned int) * indices.size();
	va_model->buffer_data(flvr::VABuf_Index, vbo_size,
		&indices[0], GL_DYNAMIC_DRAW);
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABuf_Index));
}

void MeshData::UpdateCoordVBO(const std::vector<float>& vbo_data, const std::vector<int>& index_data)
{
	flvr::VertexArray* va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	if (vbo_data.size() > 0)
	{
		size_t vbo_size = sizeof(float) * vbo_data.size();
		va_model->buffer_data(
			flvr::VABuf_Coord, vbo_size,
			&vbo_data[0], GL_DYNAMIC_DRAW);
	}
	if (index_data.size() > 0)
	{
		size_t ibo_size = sizeof(unsigned int) * index_data.size();
		va_model->buffer_data(
			flvr::VABuf_Index, ibo_size,
			&index_data[0], GL_DYNAMIC_DRAW);
	}
}

GLuint MeshData::GetVBO()
{
	flvr::VertexArray* va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABuf_Coord));
}

void MeshData::UpdateNormalVBO(const std::vector<float>& vbo)
{
	flvr::VertexArray* va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	if (vbo.size() > 0)
	{
		va_model->add_buffer(flvr::VABuf_Normal);
		size_t vbo_size = sizeof(float) * vbo.size();
		va_model->buffer_data(
			flvr::VABuf_Normal, vbo_size,
			&vbo[0], GL_DYNAMIC_DRAW);
		va_model->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
		SetFlatShading(false);
	}
}

void MeshData::DeleteNormalVBO()
{
	SetFlatShading(true);
	flvr::VertexArray* va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	va_model->delete_buffer(flvr::VABuf_Normal);
}

void MeshData::SetVertexNum(unsigned int num)
{
	if (!m_data)
		return;
	m_data->numvertices = static_cast<GLuint>(num);
}

unsigned int MeshData::GetVertexNum()
{
	if (!m_data)
		return 0;
	return static_cast<unsigned int>(m_data->numvertices);
}

void MeshData::SetTriangleNum(unsigned int num)
{
	m_data->numtriangles = static_cast<GLuint>(num);
	GLMgroup* group = m_data->groups;
	if (group)
	{
		group->numtriangles = static_cast<GLuint>(num);
	}
}

unsigned int MeshData::GetTriangleNum()
{
	if (!m_data)
		return 0;
	return static_cast<unsigned int>(m_data->numtriangles);
}

//MR
flvr::MeshRenderer* MeshData::GetMR()
{
	return m_mr.get();
}

void MeshData::SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat)
{
	if (m_mr)
	{
		glm::mat4 mv_temp;
		mv_temp = glm::translate(
			mv_mat, glm::vec3(
		m_trans[0]+m_center.x(),
		m_trans[1]+m_center.y(),
		m_trans[2]+m_center.z()));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(m_rot[0])),
			glm::vec3(1.0, 0.0, 0.0));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(m_rot[1])),
			glm::vec3(0.0, 1.0, 0.0));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(m_rot[2])),
			glm::vec3(0.0, 0.0, 1.0));
		mv_temp = glm::scale(mv_temp,
			glm::vec3(float(m_scale[0]), float(m_scale[1]), float(m_scale[2])));
		mv_temp = glm::translate(mv_temp,
			glm::vec3(-m_center.x(), -m_center.y(), -m_center.z()));

		m_mr->SetMatrices(mv_temp, proj_mat);
	}
}

void MeshData::Draw(int peel)
{
	if (!m_mr)
		return;

	glDisable(GL_CULL_FACE);
	m_mr->set_depth_peel(peel);
	m_mr->draw();
	if (m_draw_bounds && (peel==4 || peel==5))
		DrawBounds();
	glEnable(GL_CULL_FACE);
}

void MeshData::DrawBounds()
{
	if (!m_mr)
		return;

	m_mr->draw_wireframe();
}

void MeshData::DrawInt(unsigned int name)
{
	if (!m_mr)
		return;

	m_mr->draw_integer(name);
}

//lighting
void MeshData::SetLighting(bool bVal)
{
	m_light = bVal;
	if (m_mr) m_mr->set_lighting(m_light);
}

bool MeshData::GetLighting()
{
	return m_light;
}

void MeshData::SetFlatShading(bool bval)
{
	m_flat_shading = bval;
	if (m_mr) m_mr->set_flat_shading(bval);
}

bool MeshData::GetFlatShading()
{
	return m_flat_shading;
}

//fog
void MeshData::SetFog(bool bVal,
	double fog_intensity, double fog_start, double fog_end)
{
	m_fog = bVal;
	if (m_mr) m_mr->set_fog(m_fog, fog_intensity, fog_start, fog_end);
}

bool MeshData::GetFog()
{
	return m_fog;
}

void MeshData::SetMaterial(fluo::Color& amb, fluo::Color& diff, fluo::Color& spec,
	double shine, double alpha)
{
	m_mat_amb = amb;
	m_mat_diff = diff;
	m_mat_spec = spec;
	m_mat_shine = shine;
	m_mat_alpha = alpha;

	if (m_data && m_data->materials)
	{
		for (int i=0; i<(int)m_data->nummaterials; i++)
		{
			if (i==0)
			{
				m_data->materials[i].ambient[0] = m_mat_amb.r();
				m_data->materials[i].ambient[1] = m_mat_amb.g();
				m_data->materials[i].ambient[2] = m_mat_amb.b();
				m_data->materials[i].diffuse[0] = m_mat_diff.r();
				m_data->materials[i].diffuse[1] = m_mat_diff.g();
				m_data->materials[i].diffuse[2] = m_mat_diff.b();
				m_data->materials[i].specular[0] = m_mat_spec.r();
				m_data->materials[i].specular[1] = m_mat_spec.g();
				m_data->materials[i].specular[2] = m_mat_spec.b();
				m_data->materials[i].shininess = m_mat_shine;
			}
			m_data->materials[i].specular[3] = m_mat_alpha;
			m_data->materials[i].ambient[3] = m_mat_alpha;
			m_data->materials[i].diffuse[3] = m_mat_alpha;
		}
	}
}

void MeshData::SetColor(fluo::Color &color, int type)
{
	switch (type)
	{
	case MESH_COLOR_AMB:
		m_mat_amb = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].ambient[0] = m_mat_amb.r();
			m_data->materials[0].ambient[1] = m_mat_amb.g();
			m_data->materials[0].ambient[2] = m_mat_amb.b();
		}
		break;
	case MESH_COLOR_DIFF:
		m_mat_diff = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].diffuse[0] = m_mat_diff.r();
			m_data->materials[0].diffuse[1] = m_mat_diff.g();
			m_data->materials[0].diffuse[2] = m_mat_diff.b();
		}
		break;
	case MESH_COLOR_SPEC:
		m_mat_spec = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].specular[0] = m_mat_spec.r();
			m_data->materials[0].specular[1] = m_mat_spec.g();
			m_data->materials[0].specular[2] = m_mat_spec.b();
		}
		break;
	}
}

fluo::Color MeshData::GetColor()
{
	return m_mat_amb;
}

void MeshData::SetFloat(double &value, int type)
{
	switch (type)
	{
	case MESH_FLOAT_SHN:
		m_mat_shine = value;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].shininess = m_mat_shine;
		}
		break;
	case MESH_FLOAT_ALPHA:
		m_mat_alpha = value;
		if (m_data && m_data->materials)
		{
			for (int i=0; i<(int)m_data->nummaterials; i++)
			{
				m_data->materials[i].ambient[3] = m_mat_alpha;
				m_data->materials[i].diffuse[3] = m_mat_alpha;
				m_data->materials[i].specular[3] = m_mat_alpha;
			}
		}
		if (m_mr) m_mr->set_alpha(value);
		break;
	}

}

void MeshData::GetMaterial(fluo::Color& amb, fluo::Color& diff, fluo::Color& spec,
	double& shine, double& alpha)
{
	amb = m_mat_amb;
	diff = m_mat_diff;
	spec = m_mat_spec;
	shine = m_mat_shine;
	alpha = m_mat_alpha;
}

bool MeshData::IsTransp()
{
	if (m_mat_alpha>=1.0)
		return false;
	else
		return true;
}

//shadow
void MeshData::SetShadowEnable(bool bVal)
{
	m_shadow_enable= bVal;
}

bool MeshData::GetShadowEnable()
{
	return m_shadow_enable;
}

void MeshData::SetShadowIntensity(double val)
{
	m_shadow_intensity = val;
}

double MeshData::GetShadowIntensity()
{
	return m_shadow_intensity;
}

std::wstring MeshData::GetPath()
{
	return m_data_path;
}

fluo::BBox MeshData::GetBounds()
{
	fluo::BBox bounds;
	fluo::Point p[8];
	p[0] = fluo::Point(m_bounds.Min().x(), m_bounds.Min().y(), m_bounds.Min().z());
	p[1] = fluo::Point(m_bounds.Min().x(), m_bounds.Min().y(), m_bounds.Max().z());
	p[2] = fluo::Point(m_bounds.Min().x(), m_bounds.Max().y(), m_bounds.Min().z());
	p[3] = fluo::Point(m_bounds.Min().x(), m_bounds.Max().y(), m_bounds.Max().z());
	p[4] = fluo::Point(m_bounds.Max().x(), m_bounds.Min().y(), m_bounds.Min().z());
	p[5] = fluo::Point(m_bounds.Max().x(), m_bounds.Min().y(), m_bounds.Max().z());
	p[6] = fluo::Point(m_bounds.Max().x(), m_bounds.Max().y(), m_bounds.Min().z());
	p[7] = fluo::Point(m_bounds.Max().x(), m_bounds.Max().y(), m_bounds.Max().z());
	double s, c;
	fluo::Point temp;
	for (int i=0 ; i<8 ; i++)
	{
		p[i] = fluo::Point(p[i].x()*m_scale[0], p[i].y()*m_scale[1], p[i].z()*m_scale[2]);
		s = sin(d2r(m_rot[2]));
		c = cos(d2r(m_rot[2]));
		temp = fluo::Point(c*p[i].x()-s*p[i].y(), s*p[i].x()+c*p[i].y(), p[i].z());
		p[i] = temp;
		s = sin(d2r(m_rot[1]));
		c = cos(d2r(m_rot[1]));
		temp = fluo::Point(c*p[i].x()+s*p[i].z(), p[i].y(), -s*p[i].x()+c*p[i].z());
		p[i] = temp;
		s = sin(d2r(m_rot[0]));
		c = cos(d2r(m_rot[0]));
		temp = fluo::Point(p[i].x(), c*p[i].y()-s*p[i].z(), s*p[i].y()+c*p[i].z());
		p[i] = fluo::Point(temp.x()+m_trans[0], temp.y()+m_trans[1], temp.z()+m_trans[2]);
		bounds.extend(p[i]);
	}
	return bounds;
}

GLMmodel* MeshData::GetMesh()
{
	return m_data.get();
}

void MeshData::SetDisp(bool disp)
{
	m_disp = disp;
}

void MeshData::ToggleDisp()
{
	m_disp = !m_disp;
}

bool MeshData::GetDisp()
{
	return m_disp;
}

void MeshData::SetDrawBounds(bool draw)
{
	m_draw_bounds = draw;
}

void MeshData::ToggleDrawBounds()
{
	m_draw_bounds = !m_draw_bounds;
}

bool MeshData::GetDrawBounds()
{
	return m_draw_bounds;
}

void MeshData::SetTranslation(double x, double y, double z)
{
	m_trans[0] = x;
	m_trans[1] = y;
	m_trans[2] = z;
}

void MeshData::GetTranslation(double &x, double &y, double &z)
{
	x = m_trans[0];
	y = m_trans[1];
	z = m_trans[2];
}

void MeshData::SetTranslation(const fluo::Vector& val)
{
	m_trans[0] = val.x();
	m_trans[1] = val.y();
	m_trans[2] = val.z();
}

fluo::Vector MeshData::GetTranslation()
{
	return fluo::Vector(m_trans[0], m_trans[1], m_trans[2]);
}

void MeshData::SetRotation(double x, double y, double z)
{
	m_rot[0] = x;
	m_rot[1] = y;
	m_rot[2] = z;
}

void MeshData::GetRotation(double &x, double &y, double &z)
{
	x = m_rot[0];
	y = m_rot[1];
	z = m_rot[2];
}

void MeshData::SetRotation(const fluo::Vector& val)
{
	m_rot[0] = val.x();
	m_rot[1] = val.y();
	m_rot[2] = val.z();
}

fluo::Vector MeshData::GetRotation()
{
	return fluo::Vector(m_rot[0], m_rot[1], m_rot[2]);
}

void MeshData::SetScaling(double x, double y, double z)
{
	m_scale[0] = x;
	m_scale[1] = y;
	m_scale[2] = z;
}

void MeshData::GetScaling(double &x, double &y, double &z)
{
	x = m_scale[0];
	y = m_scale[1];
	z = m_scale[2];
}

void MeshData::SetScaling(const fluo::Vector& val)
{
	m_scale[0] = val.x();
	m_scale[1] = val.y();
	m_scale[2] = val.z();
}

fluo::Vector MeshData::GetScaling()
{
	return fluo::Vector(m_scale[0], m_scale[1], m_scale[2]);
}

//randomize color
void MeshData::RandomizeColor()
{
	double hue = (double)std::rand()/(RAND_MAX) * 360.0;
	fluo::Color color(fluo::HSVColor(hue, 1.0, 1.0));
	SetColor(color, MESH_COLOR_DIFF);
	fluo::Color amb = color * 0.3;
	SetColor(amb, MESH_COLOR_AMB);
}

//shown in legend
void MeshData::SetLegend(bool val)
{
	m_legend = val;
}

bool MeshData::GetLegend()
{
	return m_legend;
}

//size limiter
void MeshData::SetLimit(bool bVal)
{
	m_enable_limit = bVal;
	if (m_enable_limit)
		m_mr->set_limit(m_limit);
	else
		m_mr->set_limit(-1);
//	m_mr->update();
}

bool MeshData::GetLimit()
{
	return m_enable_limit;
}

void MeshData::SetLimitNumer(int val)
{
	m_limit = val;
	if (m_enable_limit)
	{
		m_mr->set_limit(val);
//		m_mr->update();
	}
}

int MeshData::GetLimitNumber()
{
	return m_limit;
}

