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
#include <MeshData.h>
#include <MeshRenderer.h>
#include <VertexArray.h>
#include <base_mesh_reader.h>
#include <Global.h>
#include <FramebufferStateTracker.h>
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
	m_shading(true),
	m_flat_shading(false),
	m_vertex_color(false),
	m_alpha(0.7),
	m_fog(true),
	m_time(0),
	m_shading_strength(1.0),
	m_shading_shine(1.0),
	m_shadow_enable(true),
	m_shadow_intensity(0.6),
	m_cpu_dirty(true),
	m_gpu_dirty(true)
{
	type = 3;//mesh

	double hue, sat, val;
	hue = double(std::rand()%360);
	sat = 1.0;
	val = 1.0;
	m_color = fluo::Color(fluo::HSVColor(hue, sat, val));

	m_legend = true;

	m_mr = std::make_unique<flvr::MeshRenderer>();
	m_mr->set_shading(m_shading);
	m_mr->set_flat_shading(m_flat_shading);
	m_mr->set_color(m_color);
	m_mr->set_alpha(static_cast<float>(m_alpha));
	m_mr->set_shading_strength(m_shading_strength);
	m_mr->set_shading_shine(m_shading_shine);
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

	BuildMesh();

	return 1;
}

void MeshData::Save(const std::wstring& filename)
{
	if (m_gpu_dirty)
		ReturnData();

	if (m_data)
	{
		std::string str = ws2s(filename);
		glmWriteOBJ(m_data.get(), str.c_str(), GLM_SMOOTH | GLM_VERTC);
		m_data_path = filename;
	}
}

void MeshData::SetReader(const std::shared_ptr<BaseMeshReader>& reader)
{
	m_reader = reader;
}

void MeshData::SubmitData()
{
	if (!m_data || !m_mr)
		return;

	m_mr->set_data(m_data.get());
	bool bnormal = m_data->normals;
	bool btexcoord = m_data->texcoords;
	bool bcolor = m_data->colors;

	// Map composite (v, vt, vn, vc) to unique vertex index
	struct VertexKey {
		unsigned int v, vt, vn, vc;
		bool operator==(const VertexKey& other) const {
			return v == other.v && vt == other.vt && vn == other.vn && vc == other.vc;
		}
	};
	struct VertexKeyHash {
		size_t operator()(const VertexKey& k) const {
			return std::hash<unsigned int>()(k.v) ^ std::hash<unsigned int>()(k.vt << 1) ^ std::hash<unsigned int>()(k.vn << 2) ^ std::hash<unsigned int>()(k.vc << 3);
		}
	};

	std::unordered_map<VertexKey, unsigned int, VertexKeyHash> vertex_map;
	std::vector<float> pos_buffer;
	std::vector<float> norm_buffer;
	std::vector<float> tex_buffer;
	std::vector<float> color_buffer;
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
					bnormal ? tri->nindices[j] : 0,
					bcolor ? tri->cindices[j] : 0
				};

				auto it = vertex_map.find(key);
				if (it != vertex_map.end())
				{
					index_buffer.push_back(it->second);
				}
				else
				{
					unsigned int new_index = static_cast<unsigned int>(pos_buffer.size() / 3);
					vertex_map[key] = new_index;
					index_buffer.push_back(new_index);

					// Position
					pos_buffer.push_back(m_data->vertices[3 * key.v]);
					pos_buffer.push_back(m_data->vertices[3 * key.v + 1]);
					pos_buffer.push_back(m_data->vertices[3 * key.v + 2]);

					// Normal
					if (bnormal)
					{
						norm_buffer.push_back(m_data->normals[3 * key.vn]);
						norm_buffer.push_back(m_data->normals[3 * key.vn + 1]);
						norm_buffer.push_back(m_data->normals[3 * key.vn + 2]);
					}

					// Texcoord
					if (btexcoord)
					{
						tex_buffer.push_back(m_data->texcoords[2 * key.vt]);
						tex_buffer.push_back(m_data->texcoords[2 * key.vt + 1]);
					}

					// Color
					if (bcolor)
					{
						color_buffer.push_back(m_data->colors[4 * key.vc]);
						color_buffer.push_back(m_data->colors[4 * key.vc + 1]);
						color_buffer.push_back(m_data->colors[4 * key.vc + 2]);
						color_buffer.push_back(m_data->colors[4 * key.vc + 3]);
					}
				}
			}
		}
		group = group->next;
	}

	auto va_model = m_mr->GetOrCreateVertexArray(true, true);
	assert(va_model);

	// Add and upload buffers
	va_model->add_buffer(flvr::VABufferType::VABuf_Coord);
	va_model->buffer_data(flvr::VABufferType::VABuf_Coord, sizeof(float) * pos_buffer.size(), &pos_buffer[0], flvr::BufferUsage::StaticDraw);
	va_model->attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Coord), 3, flvr::VertexAttribType::Float, false, 0, (const void*)0); // Position

	if (bnormal)
	{
		va_model->add_buffer(flvr::VABufferType::VABuf_Normal);
		va_model->buffer_data(flvr::VABufferType::VABuf_Normal, sizeof(float) * norm_buffer.size(), &norm_buffer[0], flvr::BufferUsage::StaticDraw);
		va_model->attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Normal), 3, flvr::VertexAttribType::Float, false, 0, (const void*)0); // Normal
	}

	if (btexcoord)
	{
		va_model->add_buffer(flvr::VABufferType::VABuf_Tex);
		va_model->buffer_data(flvr::VABufferType::VABuf_Tex, sizeof(float) * tex_buffer.size(), &tex_buffer[0], flvr::BufferUsage::StaticDraw);
		va_model->attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Tex), 2, flvr::VertexAttribType::Float, false, 0, (const void*)0); // Texcoord
	}

	if (bcolor)
	{
		va_model->add_buffer(flvr::VABufferType::VABuf_Color);
		va_model->buffer_data(flvr::VABufferType::VABuf_Color, sizeof(float) * color_buffer.size(), &color_buffer[0], flvr::BufferUsage::StaticDraw);
		va_model->attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Color), 4, flvr::VertexAttribType::Float, false, 0, (const void*)0); // Color
	}

	// Index buffer
	va_model->add_buffer(flvr::VABufferType::VABuf_Index);
	va_model->buffer_data(flvr::VABufferType::VABuf_Index, sizeof(unsigned int) * index_buffer.size(), &index_buffer[0], flvr::BufferUsage::StaticDraw);

	m_gpu_dirty = false;
}

void MeshData::ReturnData()
{
	if (!m_data || !m_mr)
		return;

	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;

	bool bnormal = m_data->numnormals;
	bool btexcoord = m_data->numtexcoords;
	bool bcolor = m_data->numcolors;
	bool indexed = va_model->is_indexed();

	size_t num_vertices = m_data->numvertices;
	size_t num_triangles = m_data->numtriangles;
	size_t index_count = num_triangles * 3;
	if (num_triangles == 0)
		return;

	// Map position buffer
	std::vector<float> verts(3 * num_vertices);
	void* mapped_ptr = va_model->map_buffer(flvr::VABufferType::VABuf_Coord, flvr::BufferAccess::ReadOnly);
	if (!mapped_ptr)
		return;
	memcpy(verts.data(), mapped_ptr, verts.size() * sizeof(float));
	va_model->unmap_buffer(flvr::VABufferType::VABuf_Coord);

	// Map normal buffer
	std::vector<float> norms;
	if (bnormal)
	{
		norms.resize(3 * num_vertices);
		void* norm_ptr = va_model->map_buffer(flvr::VABufferType::VABuf_Normal, flvr::BufferAccess::ReadOnly);
		if (!norm_ptr)
			return;
		memcpy(norms.data(), norm_ptr, norms.size() * sizeof(float));
		va_model->unmap_buffer(flvr::VABufferType::VABuf_Normal);
	}

	// Map texcoord buffer
	std::vector<float> texs;
	if (btexcoord)
	{
		texs.resize(2 * num_vertices);
		void* tex_ptr = va_model->map_buffer(flvr::VABufferType::VABuf_Tex, flvr::BufferAccess::ReadOnly);
		if (!tex_ptr)
			return;
		memcpy(texs.data(), tex_ptr, texs.size() * sizeof(float));
		va_model->unmap_buffer(flvr::VABufferType::VABuf_Tex);
	}

	// Map color buffer
	std::vector<float> colors;
	if (bcolor)
	{
		colors.resize(4 * num_vertices);
		void* color_ptr = va_model->map_buffer(flvr::VABufferType::VABuf_Color, flvr::BufferAccess::ReadOnly);
		if (!color_ptr)
			return;
		memcpy(colors.data(), color_ptr, colors.size() * sizeof(float));
		va_model->unmap_buffer(flvr::VABufferType::VABuf_Color);
	}

	// Map index buffer
	std::vector<GLuint> indices;
	if (indexed)
	{
		void* index_ptr = va_model->map_buffer(flvr::VABufferType::VABuf_Index, flvr::BufferAccess::ReadOnly);
		if (!index_ptr)
			return;
		indices.resize(index_count);
		memcpy(indices.data(), index_ptr, sizeof(GLuint) * index_count);
		va_model->unmap_buffer(flvr::VABufferType::VABuf_Index);
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
	if (bcolor && !m_data->colors)
		m_data->colors = (GLfloat*)malloc(sizeof(GLfloat) * 4 * (num_vertices + 1));

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
	for (size_t i = 1; i <= num_vertices; ++i)
	{
		glm::vec3 v(verts[3 * (i - 1)], verts[3 * (i - 1) + 1], verts[3 * (i - 1) + 2]);
		v = glm::vec3(M * glm::vec4(v, 1.0f));
		m_data->vertices[3 * i + 0] = v.x;
		m_data->vertices[3 * i + 1] = v.y;
		m_data->vertices[3 * i + 2] = v.z;

		if (bnormal)
		{
			glm::vec3 n(norms[3 * (i - 1)], norms[3 * (i - 1) + 1], norms[3 * (i - 1) + 2]);
			n = glm::normalize(N * n);
			m_data->normals[3 * i + 0] = n.x;
			m_data->normals[3 * i + 1] = n.y;
			m_data->normals[3 * i + 2] = n.z;
		}

		if (btexcoord)
		{
			m_data->texcoords[2 * i + 0] = texs[2 * (i - 1)];
			m_data->texcoords[2 * i + 1] = texs[2 * (i - 1) + 1];
		}

		if (bcolor)
		{
			m_data->colors[4 * i + 0] = colors[4 * (i - 1)];
			m_data->colors[4 * i + 1] = colors[4 * (i - 1) + 1];
			m_data->colors[4 * i + 2] = colors[4 * (i - 1) + 2];
			m_data->colors[4 * i + 3] = colors[4 * (i - 1) + 3];
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
			for (int j = 0; j < 3; ++j)
			{
				GLuint idx = indices[i * 3 + j] + 1;

				tri.vindices[j] = idx;
				if (m_data->texcoords)
					tri.tindices[j] = idx;
				if (m_data->normals)
					tri.nindices[j] = idx;
				if (m_data->colors)
					tri.cindices[j] = idx;
			}
		}
		else
		{
			// Fallback for non-indexed mode (rare in modern OpenGL)
			for (int j = 0; j < 3; ++j)
			{
				GLuint raw_index = (GLuint)(i * 3 + j + 1);
				tri.vindices[j] = raw_index;
				if (m_data->texcoords)
					tri.tindices[j] = raw_index;
				if (m_data->normals)
					tri.nindices[j] = raw_index;
				if (m_data->colors)
					tri.cindices[j] = raw_index;
			}
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
	model->numcolors = 0;
	model->colors = 0;
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
	m_data->materials[0].ambient[0] = 0.0;
	m_data->materials[0].ambient[1] = 0.0;
	m_data->materials[0].ambient[2] = 0.0;
	m_data->materials[0].ambient[3] = 0.0;
	m_data->materials[0].diffuse[0] = 0.0;
	m_data->materials[0].diffuse[1] = 0.0;
	m_data->materials[0].diffuse[2] = 0.0;
	m_data->materials[0].diffuse[3] = 0.0;
	m_data->materials[0].specular[0] = 0.0;
	m_data->materials[0].specular[1] = 0.0;
	m_data->materials[0].specular[2] = 0.0;
	m_data->materials[0].specular[3] = 0.0;
	m_data->materials[0].shininess = 0.0;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = false;
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
	std::vector<float> verts(vertex_size * 3);
	size_t vbo_size = sizeof(float) * verts.size();
	auto va_model = m_mr->GetOrCreateVertexArray(true, false);
	assert(va_model);
	va_model->buffer_data(
		flvr::VABufferType::VABuf_Coord, vbo_size,
		&verts[0], flvr::BufferUsage::DynamicDraw);
	va_model->attrib_pointer(
		static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Coord), 3, flvr::VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	GLuint vbo_id = static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Coord));
	return vbo_id;
}

GLuint MeshData::AddIndexVBO(size_t vsize)
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	va_model->add_index_buffer();
	//assume coord buffer is a list of triangles
	std::vector<unsigned int> indices(vsize);
	for (size_t i = 0; i < vsize; ++i)
		indices[i] = static_cast<unsigned int>(i);
	size_t vbo_size = sizeof(unsigned int) * indices.size();
	va_model->buffer_data(flvr::VABufferType::VABuf_Index, vbo_size,
		&indices[0], flvr::BufferUsage::DynamicDraw);
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Index));
}

GLuint MeshData::GetIndexVBO()
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Index));
}

void MeshData::UpdateCoordVBO(const std::vector<float>& vbo_data, const std::vector<int>& index_data)
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	if (vbo_data.size() > 0)
	{
		size_t vbo_size = sizeof(float) * vbo_data.size();
		va_model->buffer_data(
			flvr::VABufferType::VABuf_Coord, vbo_size,
			&vbo_data[0], flvr::BufferUsage::DynamicDraw);
	}
	if (index_data.size() > 0)
	{
		size_t ibo_size = sizeof(unsigned int) * index_data.size();
		va_model->buffer_data(
			flvr::VABufferType::VABuf_Index, ibo_size,
			&index_data[0], flvr::BufferUsage::DynamicDraw);
	}
}

GLuint MeshData::GetCoordVBO()
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Coord));
}

void MeshData::UpdateNormalVBO(const std::vector<float>& vbo)
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	if (vbo.size() > 0)
	{
		va_model->add_buffer(flvr::VABufferType::VABuf_Normal);
		size_t vbo_size = sizeof(float) * vbo.size();
		va_model->buffer_data(
			flvr::VABufferType::VABuf_Normal, vbo_size,
			&vbo[0], flvr::BufferUsage::DynamicDraw);
		va_model->attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Normal), 3, flvr::VertexAttribType::Float, false, 0, (const void*)0);
		SetFlatShading(false);
		if (m_data)
			m_data->numnormals = static_cast<GLuint>(vbo.size() / 3);
	}
}

void MeshData::DeleteNormalVBO()
{
	SetFlatShading(true);
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	va_model->delete_buffer(flvr::VABufferType::VABuf_Normal);
	va_model->remove_attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Normal));
	if (m_data)
	{
		m_data->numnormals = 0;
		free(m_data->normals);
	}
}

GLuint MeshData::GetNormalVBO()
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Normal));
}

GLuint MeshData::AddColorVBO(int vertex_size)
{
	std::vector<float> verts(vertex_size * 4);
	size_t vbo_size = sizeof(float) * verts.size();
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	va_model->add_buffer(flvr::VABufferType::VABuf_Color);
	va_model->buffer_data(
		flvr::VABufferType::VABuf_Color, vbo_size,
		&verts[0], flvr::BufferUsage::DynamicDraw);
	va_model->attrib_pointer(
		static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Color), 4, flvr::VertexAttribType::Float, false, 0, (const void*)0);
	GLuint vbo_id = static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Color));
	if (m_data)
		m_data->numcolors = static_cast<GLuint>(vertex_size);
	SetVertexColor(true);
	return vbo_id;
}

void MeshData::DeleteColorVBO()
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return;
	va_model->delete_buffer(flvr::VABufferType::VABuf_Color);
	va_model->remove_attrib_pointer(static_cast<GLuint>(flvr::VAAttribIndex::VAAttrib_Color));
	if (m_data)
	{
		m_data->numcolors = 0;
		free(m_data->colors);
	}
}

GLuint MeshData::GetColorVBO()
{
	auto va_model = m_mr->GetVertexArray();
	if (!va_model)
		return 0;
	return static_cast<GLuint>(va_model->id_buffer(flvr::VABufferType::VABuf_Color));
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

	m_mr->set_depth_peel(peel);
	m_mr->draw();
	if (m_draw_bounds && (peel==4 || peel==5))
		DrawBounds();
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
void MeshData::SetShading(bool bVal)
{
	m_shading = bVal;
	if (m_mr)
		m_mr->set_shading(m_shading);
}

bool MeshData::GetShading()
{
	return m_shading;
}

void MeshData::SetFlatShading(bool bval)
{
	m_flat_shading = bval;
	if (m_mr)
		m_mr->set_flat_shading(bval);
}

bool MeshData::GetFlatShading()
{
	return m_flat_shading;
}

void MeshData::SetVertexColor(bool val)
{
	m_vertex_color = val;
	if (m_mr)
		m_mr->set_vertex_color(val);
}

bool MeshData::GetVertexColor()
{
	return m_vertex_color;
}

//fog
void MeshData::SetFog(bool bVal,
	double fog_intensity, double fog_start, double fog_end)
{
	m_fog = bVal;
	if (m_mr)
		m_mr->set_fog(m_fog, fog_intensity, fog_start, fog_end);
}

void MeshData::SetFogColor(const fluo::Color &color)
{
	if (m_mr)
		m_mr->set_fog_color(color);
}

bool MeshData::GetFog()
{
	return m_fog;
}

void MeshData::SetColor(const fluo::Color &color)
{
	m_color = color;
	if (m_mr)
		m_mr->set_color(color);
}

fluo::Color MeshData::GetColor()
{
	return m_color;
}

void MeshData::SetAlpha(double alpha)
{
	m_alpha = alpha;
	if (m_mr)
		m_mr->set_alpha(alpha);
}

double MeshData::GetAlpha()
{
	return m_alpha;
}

void MeshData::SetShadingStrength(double val)
{
	m_shading_strength = val;
	if (m_mr)
		m_mr->set_shading_strength(val);
}

double MeshData::GetShadingStrength()
{
	return m_shading_strength;
}

void MeshData::SetShadingShine(double val)
{
	m_shading_shine = val;
	if (m_mr)
		m_mr->set_shading_shine(val);
}

double MeshData::GetShadingShine()
{
	return m_shading_shine;
}

bool MeshData::GetTransparent()
{
	if (m_alpha < 1.0)
		return true;
	return false;
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

fluo::BBox MeshData::GetBounds()
{
	if (!m_bounds.valid())
	{
		if (m_gpu_dirty)
			ReturnData();
		UpdateBounds();
	}

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

void MeshData::SetClippingBox(const fluo::ClippingBox& box)
{
	TreeLayer::SetClippingBox(box);
	m_mr->set_clipping_box(m_clipping_box);
}

fluo::ClippingBox& MeshData::GetClippingBox()
{
	if (!m_bounds.valid())
	{
		if (m_gpu_dirty)
			ReturnData();
		UpdateBounds();
	}
	return m_clipping_box;
}

void MeshData::SetClipValue(fluo::ClipPlane i, int val)
{
	TreeLayer::SetClipValue(i, val);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetClipValues(fluo::ClipPlane i, int val1, int val2)
{
	TreeLayer::SetClipValues(i, val1, val2);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetClipValues(const std::array<int, 6>& vals)
{
	TreeLayer::SetClipValues(vals);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::ResetClipValues()
{
	TreeLayer::ResetClipValues();
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::ResetClipValues(fluo::ClipPlane i)
{
	TreeLayer::ResetClipValues(i);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetClipRotation(int i, double val)
{
	TreeLayer::SetClipRotation(i, val);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetClipRotation(const fluo::Vector& euler)
{
	TreeLayer::SetClipRotation(euler);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetClipRotation(const fluo::Quaternion& q)
{
	TreeLayer::SetClipRotation(q);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetLink(fluo::ClipPlane i, bool link)
{
	TreeLayer::SetLink(i, link);
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::ResetLink()
{
	TreeLayer::ResetLink();
	m_mr->set_clipping_box(m_clipping_box);
}

void MeshData::SetLinkedDist(fluo::ClipPlane i, int val)
{
	TreeLayer::SetLinkedDist(i, val);
	m_mr->set_clipping_box(m_clipping_box);
}

//randomize color
void MeshData::RandomizeColor()
{
	double hue = (double)std::rand()/(RAND_MAX) * 360.0;
	fluo::Color color(fluo::HSVColor(hue, 1.0, 1.0));
	SetColor(color);
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

void MeshData::BuildMesh()
{
	if (!m_data->normals && m_data->numtriangles)
	{
		if (!m_data->facetnorms)
			glmFacetNormals(m_data.get());
		glmVertexNormals(m_data.get(), 89.0);
	}

	if (m_data->hastexture && !m_data->numtexcoords)
	{
		glmLinearTexture(m_data.get());
	}

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	m_data->materials[0].name = NULL;
	m_data->materials[0].ambient[0] = 0.0;
	m_data->materials[0].ambient[1] = 0.0;
	m_data->materials[0].ambient[2] = 0.0;
	m_data->materials[0].ambient[3] = 0.0;
	m_data->materials[0].diffuse[0] = 0.0;
	m_data->materials[0].diffuse[1] = 0.0;
	m_data->materials[0].diffuse[2] = 0.0;
	m_data->materials[0].diffuse[3] = 0.0;
	m_data->materials[0].specular[0] = 0.0;
	m_data->materials[0].specular[1] = 0.0;
	m_data->materials[0].specular[2] = 0.0;
	m_data->materials[0].specular[3] = 0.0;
	m_data->materials[0].shininess = 0.0;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = false;
	m_data->materials[0].textureID = 0;

	UpdateBounds();

	SetFlatShading(m_data->numnormals == 0);
	SetVertexColor(m_data->numcolors > 0);

	if (m_data->numcolors)
	{
		if (m_data->numcolors > 0)
		{
			//get first color
			float r = m_data->colors[4];
			float g = m_data->colors[5];
			float b = m_data->colors[6];

			if (r < 1.0f || g < 1.0f || b < 1.0f)
				SetColor(fluo::Color(1.0));
		}
	}

	SubmitData();
}

void MeshData::UpdateBounds()
{
	//bounds
	GLfloat fbounds[6];
	glmBoundingBox(m_data.get(), fbounds);
	fluo::BBox bounds;
	fluo::Point pmin(fbounds[0], fbounds[2], fbounds[4]);
	fluo::Point pmax(fbounds[1], fbounds[3], fbounds[5]);
	bounds.extend(pmin);
	bounds.extend(pmax);
	bounds.expand_to_int();
	m_bounds = bounds;
	m_center = fluo::Point(
		(m_bounds.Min().x()+m_bounds.Max().x())*0.5,
		(m_bounds.Min().y()+m_bounds.Max().y())*0.5,
		(m_bounds.Min().z()+m_bounds.Max().z())*0.5);

	m_clipping_box.SetBBoxes(m_bounds, m_bounds);
	m_mr->set_clipping_box(m_clipping_box);
}