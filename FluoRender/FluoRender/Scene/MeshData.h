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
#ifndef _MESH_DATA_H_
#define _MESH_DATA_H_

#include <TreeLayer.h>
#include <BBox.h>
#include <glm/glm.hpp>

#ifndef __glew_h__
typedef unsigned int GLuint;
typedef int GLint;
typedef float GLfloat;
#endif // !__glew_h__

#define MESH_COLOR_AMB	1
#define MESH_COLOR_DIFF	2
#define MESH_COLOR_SPEC	3
#define MESH_FLOAT_SHN	4
#define MESH_FLOAT_ALPHA	5

struct _GLMmodel;
typedef struct _GLMmodel GLMmodel;
namespace flvr
{
	class MeshRenderer;
}
using GLMmodelPtr = std::unique_ptr<GLMmodel, void(*)(GLMmodel*)>;
class MeshData : public TreeLayer
{
public:
	MeshData();
	virtual ~MeshData();

	//set viewport
	void SetViewport(GLint vp[4]);

	std::wstring GetPath();
	fluo::BBox GetBounds();
	GLMmodel* GetMesh();
	void SetDisp(bool disp);
	void ToggleDisp();
	bool GetDisp();
	void SetDrawBounds(bool draw);
	void ToggleDrawBounds();
	bool GetDrawBounds();

	//data management
	int Load(const std::wstring &filename);
	int Load(GLMmodel* mesh);
	void Save(const std::wstring &filename);

	//data synchronization
	void SubmitData();//upload data to GPU
	void ReturnData();//download data from GPU
	void AddEmptyData();//create empty data for gpu generated mesh
	void ClearData();

	void SetCpuDirty() { m_cpu_dirty = true; }
	void SetGpuDirty() { m_gpu_dirty = true; }

	//allocate vbo
	GLuint AddCoordVBO(int vertex_size);
	GLuint ConvertIndexed(size_t vsize);//convert to vbo and index list
	void UpdateCoordVBO(const std::vector<float>& vbo, const std::vector<int>& idx);
	GLuint GetVBO();
	void UpdateNormalVBO(const std::vector<float>& vbo);
	void DeleteNormalVBO();

	void SetVertexNum(unsigned int num);
	unsigned int GetVertexNum();
	void SetTriangleNum(unsigned int num);
	unsigned int GetTriangleNum();

	//MR
	flvr::MeshRenderer* GetMR();

	//draw
	void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat);
	void Draw(int peel);
	void DrawBounds();
	void DrawInt(unsigned int name);

	//lighting
	void SetLighting(bool bVal);
	bool GetLighting();
	void SetFlatShading(bool bval);
	bool GetFlatShading();
	void SetFog(bool bVal, double fog_intensity, double fog_start, double fog_end);
	bool GetFog();
	void SetMaterial(fluo::Color& amb, fluo::Color& diff, fluo::Color& spec,
		double shine = 30.0, double alpha = 1.0);
	void SetColor(fluo::Color &color, int type);
	fluo::Color GetColor();
	void SetFloat(double &value, int type);
	void GetMaterial(fluo::Color& amb, fluo::Color& diff, fluo::Color& spec,
		double& shine, double& alpha);
	bool IsTransp();
	//shadow
	void SetShadowEnable(bool bVal);
	bool GetShadowEnable();
	void SetShadowIntensity(double val);
	double GetShadowIntensity();

	void SetTranslation(double x, double y, double z);
	void GetTranslation(double &x, double &y, double &z);
	void SetTranslation(const fluo::Vector& val);
	fluo::Vector GetTranslation();
	void SetRotation(double x, double y, double z);
	void GetRotation(double &x, double &y, double &z);
	void SetRotation(const fluo::Vector& val);
	fluo::Vector GetRotation();
	void SetScaling(double x, double y, double z);
	void GetScaling(double &x, double &y, double &z);
	void SetScaling(const fluo::Vector& val);
	fluo::Vector GetScaling();

	//randomize color
	void RandomizeColor();

	//shown in legend
	void SetLegend(bool val);
	bool GetLegend();

	//size limiter
	void SetLimit(bool bVal);
	bool GetLimit();
	void SetLimitNumer(int val);
	int GetLimitNumber();

private:
	std::wstring m_data_path;
	GLMmodelPtr m_data;
	std::unique_ptr<flvr::MeshRenderer> m_mr;
	fluo::BBox m_bounds;
	fluo::Point m_center;

	//sync flags
	bool m_cpu_dirty;//call SubmitData to update gpu data
	bool m_gpu_dirty;//call ReturnData to update cpu data

	bool m_disp;
	bool m_draw_bounds;

	//lighting
	bool m_light;
	bool m_flat_shading;
	bool m_fog;
	fluo::Color m_mat_amb;
	fluo::Color m_mat_diff;
	fluo::Color m_mat_spec;
	double m_mat_shine;
	double m_mat_alpha;
	//shadow
	bool m_shadow_enable;
	double m_shadow_intensity;
	//size limiter
	bool m_enable_limit;
	int m_limit;

	double m_trans[3];
	double m_rot[3];
	double m_scale[3];

	//legend
	bool m_legend;
};

#endif//_MESH_DATA_H_