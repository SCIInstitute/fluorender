/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <MeshData.hpp>
#include <FLIVR/MeshRenderer.h>
#include <FLIVR/glm.h>
#include <compatibility.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace fluo;

MeshData::MeshData() :
	m_data(0),
	m_mr(0)
{
}

MeshData::MeshData(const MeshData& data, const CopyOp& copyop, bool copy_values):
	Node(data, copyop, false),
	m_data(0),
	m_mr(0)
{
	if (data.m_mr)
		m_mr = new flvr::MeshRenderer(*data.m_mr);
	m_data = data.m_data;
	if (copy_values)
		copyValues(data, copyop);
}

MeshData::~MeshData()
{
	if (m_mr)
		delete m_mr;
	if (m_data)
		glmDelete(m_data);
}

void MeshData::Initialize()
{
	//OnViewportChanged();
	//OnLightEnableChanged();
	//OnDepthAttenChanged();
	//OnMaterialChanged();
}

void MeshData::OnViewportChanged(Event& event)
{
	if (!m_mr)
		return;

	Vector4i vp;
	getValue(gstViewport, vp);
	m_mr->set_viewport(vp.get());
}

void MeshData::OnLightEnableChanged(Event& event)
{
	if (!m_mr)
		return;

	bool light_enable;
	getValue(gstShadingEnable, light_enable);
	m_mr->set_lighting(light_enable);
}

void MeshData::OnDepthAttenChanged(Event& event)
{
	if (!m_mr)
		return;

	bool depth_atten;
	double da_int, da_start, da_end;
	getValue(gstDepthAtten, depth_atten);
	getValue(gstDaInt, da_int);
	getValue(gstDaStart, da_start);
	getValue(gstDaEnd, da_end);
	m_mr->set_fog(depth_atten, da_int, da_start, da_end);
}

void MeshData::OnMaterialChanged(Event& event)
{
	if (!m_data)
		return;

	Color color;
	double amb, diff, spec;
	double shine, alpha;
	getValue(gstColor, color);
	getValue(gstMatAmb, amb);
	getValue(gstMatDiff, diff);
	getValue(gstMatSpec, spec);
	getValue(gstMatShine, shine);
	getValue(gstAlpha, alpha);

	if (m_data && m_data->materials)
	{
		for (int i = 0; i<(int)m_data->nummaterials; i++)
		{
			if (i == 0)
			{
				m_data->materials[i].ambient[0] = amb * color.r();
				m_data->materials[i].ambient[1] = amb * color.g();
				m_data->materials[i].ambient[2] = amb * color.b();
				m_data->materials[i].diffuse[0] = diff * color.r();
				m_data->materials[i].diffuse[1] = diff * color.g();
				m_data->materials[i].diffuse[2] = diff * color.b();
				m_data->materials[i].specular[0] = spec * color.r();
				m_data->materials[i].specular[1] = spec * color.g();
				m_data->materials[i].specular[2] = spec * color.b();
				m_data->materials[i].shininess = shine;
			}
			m_data->materials[i].specular[3] = alpha;
			m_data->materials[i].ambient[3] = alpha;
			m_data->materials[i].diffuse[3] = alpha;
		}
	}
}

void MeshData::OnBoundsChanged(Event& event)
{
	BBox bounds;
	getValue(gstBounds, bounds);

	//res
	Vector diag = bounds.diagonal();
	updateValue(gstResX, long(diag.x()+0.5), event);
	updateValue(gstResY, long(diag.y()+0.5), event);
	updateValue(gstResZ, long(diag.z()+0.5), event);

	//transformed bounds
	Point p[8];
	p[0] = Point(bounds.Min().x(), bounds.Min().y(), bounds.Min().z());
	p[1] = Point(bounds.Min().x(), bounds.Min().y(), bounds.Max().z());
	p[2] = Point(bounds.Min().x(), bounds.Max().y(), bounds.Min().z());
	p[3] = Point(bounds.Min().x(), bounds.Max().y(), bounds.Max().z());
	p[4] = Point(bounds.Max().x(), bounds.Min().y(), bounds.Min().z());
	p[5] = Point(bounds.Max().x(), bounds.Min().y(), bounds.Max().z());
	p[6] = Point(bounds.Max().x(), bounds.Max().y(), bounds.Min().z());
	p[7] = Point(bounds.Max().x(), bounds.Max().y(), bounds.Max().z());

	double trans_x, trans_y, trans_z;
	double rot_x, rot_y, rot_z;
	double scale_x, scale_y, scale_z;
	getValue(gstTransX, trans_x);
	getValue(gstTransY, trans_y);
	getValue(gstTransZ, trans_z);
	getValue(gstRotX, rot_x);
	getValue(gstRotY, rot_y);
	getValue(gstRotZ, rot_z);
	getValue(gstScaleX, scale_x);
	getValue(gstScaleY, scale_y);
	getValue(gstScaleZ, scale_z);

	double s, c;
	Point temp;
	for (int i = 0; i<8; i++)
	{
		p[i] = Point(p[i].x()*scale_x, p[i].y()*scale_y, p[i].z()*scale_z);
		s = sin(d2r(rot_z));
		c = cos(d2r(rot_z));
		temp = Point(c*p[i].x() - s*p[i].y(), s*p[i].x() + c*p[i].y(), p[i].z());
		p[i] = temp;
		s = sin(d2r(rot_y));
		c = cos(d2r(rot_y));
		temp = Point(c*p[i].x() + s*p[i].z(), p[i].y(), -s*p[i].x() + c*p[i].z());
		p[i] = temp;
		s = sin(d2r(rot_x));
		c = cos(d2r(rot_x));
		temp = Point(p[i].x(), c*p[i].y() - s*p[i].z(), s*p[i].y() + c*p[i].z());
		p[i] = Point(temp.x() + trans_x, temp.y() + trans_y, temp.z() + trans_z);
		bounds.extend(p[i]);
	}
	updateValue(gstBoundsTf, bounds, event);
}

void MeshData::OnRandomizeColor(Event& event)
{
	double hue = (double)rand() / (RAND_MAX) * 360.0;
	Color color(HSVColor(hue, 1.0, 1.0));
	updateValue(gstColor, color, event);
	updateValue(gstMatAmb, 0.3, event);
}

int MeshData::LoadData(GLMmodel* mesh)
{
	if (!mesh) return 0;

	//m_name = "New Mesh";

	if (m_data)
		delete m_data;
	m_data = mesh;

	if (!m_data->normals && m_data->numtriangles)
	{
		if (!m_data->facetnorms)
			glmFacetNormals(m_data);
		glmVertexNormals(m_data, 89.0);
	}

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	//m_data->materials[0].name = NULL;
	//m_data->materials[0].ambient[0] = m_mat_amb.r();
	//m_data->materials[0].ambient[1] = m_mat_amb.g();
	//m_data->materials[0].ambient[2] = m_mat_amb.b();
	//m_data->materials[0].ambient[3] = m_mat_alpha;
	//m_data->materials[0].diffuse[0] = m_mat_diff.r();
	//m_data->materials[0].diffuse[1] = m_mat_diff.g();
	//m_data->materials[0].diffuse[2] = m_mat_diff.b();
	//m_data->materials[0].diffuse[3] = m_mat_alpha;
	//m_data->materials[0].specular[0] = m_mat_spec.r();
	//m_data->materials[0].specular[1] = m_mat_spec.g();
	//m_data->materials[0].specular[2] = m_mat_spec.b();
	//m_data->materials[0].specular[3] = m_mat_alpha;
	//m_data->materials[0].shininess = m_mat_shine;
	//m_data->materials[0].emmissive[0] = 0.0;
	//m_data->materials[0].emmissive[1] = 0.0;
	//m_data->materials[0].emmissive[2] = 0.0;
	//m_data->materials[0].emmissive[3] = 0.0;
	//m_data->materials[0].havetexture = GL_FALSE;
	//m_data->materials[0].textureID = 0;

	//bounds
	GLfloat fbounds[6];
	glmBoundingBox(m_data, fbounds);
	BBox bounds;
	Point pmin(fbounds[0], fbounds[2], fbounds[4]);
	Point pmax(fbounds[1], fbounds[3], fbounds[5]);
	bounds.extend(pmin);
	bounds.extend(pmax);
	setValue(gstBounds, bounds);
	Point center((bounds.Min().x() + bounds.Max().x())*0.5,
		(bounds.Min().y() + bounds.Max().y())*0.5,
		(bounds.Min().z() + bounds.Max().z())*0.5);
	setValue(gstCenter, center);

	if (m_mr)
		delete m_mr;
	m_mr = new flvr::MeshRenderer(m_data);

	Initialize();

	return 1;
}

int MeshData::LoadData(const std::wstring &filename)
{
	bool no_fail = true;
	std::string str = ws2s(filename);
	GLMmodel* mesh = glmReadOBJ(str.c_str(), &no_fail);
	setValue(gstDataPath, filename);
	return LoadData(mesh);
}

void MeshData::SaveData(const std::string& filename)
{
	if (!m_data)
		return;
	glmWriteOBJ(m_data, filename.c_str(), GLM_SMOOTH);
}

flvr::MeshRenderer* MeshData::GetRenderer()
{
	return m_mr;
}

void MeshData::SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat)
{
	if (m_mr)
	{
		glm::mat4 mv_temp;
		Point center;
		getValue(gstCenter, center);
		double trans_x, trans_y, trans_z;
		getValue(gstTransX, trans_x);
		getValue(gstTransY, trans_y);
		getValue(gstTransZ, trans_z);
		mv_temp = glm::translate(
			mv_mat, glm::vec3(
				trans_x + center.x(),
				trans_y + center.y(),
				trans_z + center.z()));
		double rot_x, rot_y, rot_z;
		getValue(gstRotX, rot_x);
		getValue(gstRotY, rot_y);
		getValue(gstRotZ, rot_z);
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(rot_x)),
			glm::vec3(1.0, 0.0, 0.0));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(rot_y)),
			glm::vec3(0.0, 1.0, 0.0));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(rot_z)),
			glm::vec3(0.0, 0.0, 1.0));
		double scale_x, scale_y, scale_z;
		getValue(gstScaleX, scale_x);
		getValue(gstScaleY, scale_y);
		getValue(gstScaleZ, scale_z);
		mv_temp = glm::scale(mv_temp,
			glm::vec3(float(scale_x), float(scale_y), float(scale_z)));
		mv_temp = glm::translate(mv_temp,
			glm::vec3(-center.x(), -center.y(), -center.z()));

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
	bool draw_bounds;
	getValue(gstDrawBounds, draw_bounds);
	if (draw_bounds && (peel == 4 || peel == 5))
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

