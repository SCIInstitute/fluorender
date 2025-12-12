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
#include <Vector.h>
#include <glm/glm.hpp>

#ifndef __glew_h__
typedef unsigned int GLuint;
typedef int GLint;
#endif // !__glew_h__

struct _GLMmodel;
typedef struct _GLMmodel GLMmodel;
namespace flvr
{
	class MeshRenderer;
}
namespace fluo
{
	enum class ClipPlane : int;
	class Quaternion;
}
using GLMmodelPtr = std::unique_ptr<GLMmodel, void(*)(GLMmodel*)>;
class BaseMeshReader;
class MeshData : public TreeLayer
{
public:
	MeshData();
	virtual ~MeshData();

	//set viewport
	void SetViewport(GLint vp[4]);

	//path
	void SetPath(const std::wstring& path) { m_data_path = path; }
	std::wstring GetPath() { return m_data_path; }
	fluo::BBox GetBounds();
	GLMmodel* GetMesh();
	void SetDisp(bool disp);
	void ToggleDisp();
	bool GetDisp();
	void SetDrawBounds(bool draw);
	void ToggleDrawBounds();
	bool GetDrawBounds();

	//data management
	int Load(GLMmodel* mesh);
	void Save(const std::wstring &filename);
	void SetReader(const std::shared_ptr<BaseMeshReader>& reader);
	std::shared_ptr<BaseMeshReader> GetReader() { return m_reader.lock(); }

	//data synchronization
	void SetTransformation();
	void SubmitData();//upload data to GPU
	void ReturnData();//download data from GPU
	void AddEmptyData();//create empty data for gpu generated mesh
	void ClearData();

	void SetCpuDirty() { m_cpu_dirty = true; }
	void SetGpuDirty() { m_gpu_dirty = true; }

	//allocate vbo
	GLuint AddCoordVBO(int vertex_size);
	void UpdateCoordVBO(const std::vector<float>& vbo, const std::vector<int>& idx);
	GLuint GetCoordVBO();
	GLuint AddIndexVBO(size_t vsize);//convert to vbo and index list
	GLuint GetIndexVBO();
	void UpdateNormalVBO(const std::vector<float>& vbo);
	void DeleteNormalVBO();
	GLuint GetNormalVBO();
	GLuint AddColorVBO(int vertex_size);
	void DeleteColorVBO();
	GLuint GetColorVBO();

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
	void SetShading(bool bVal);
	bool GetShading();
	void SetFlatShading(bool bval);
	bool GetFlatShading();
	void SetVertexColor(bool val);
	bool GetVertexColor();
	void SetColor(const fluo::Color &color);
	virtual fluo::Color GetColor() override;
	void SetAlpha(double val);
	double GetAlpha();
	bool GetTransparent();
	void SetShadingStrength(double val);
	double GetShadingStrength();
	void SetShadingShine(double val);
	double GetShadingShine();
	void SetFog(bool bVal, double fog_intensity, double fog_start, double fog_end);
	void SetFogColor(const fluo::Color& color);
	bool GetFog();
	//shadow
	void SetShadowEnable(bool bVal);
	bool GetShadowEnable();
	void SetShadowIntensity(double val);
	double GetShadowIntensity();

	void SetTranslation(const fluo::Vector& val) { m_trans = val; SetTransformation(); }
	fluo::Vector GetTranslation() { return m_trans; }
	void SetRotation(const fluo::Vector& val) { m_rot = val; SetTransformation(); }
	fluo::Vector GetRotation() { return m_rot; }
	void SetScaling(const fluo::Vector& val) { m_scale = val; SetTransformation(); }
	fluo::Vector GetScaling() { return m_scale; }

	virtual void SetClippingBox(const fluo::ClippingBox& box) override;
	virtual fluo::ClippingBox& GetClippingBox() override;
	//clip size
	virtual void SetClipValue(fluo::ClipPlane i, int val) override;
	virtual void SetClipValues(fluo::ClipPlane i, int val1, int val2) override;
	virtual void SetClipValues(const std::array<int, 6>& vals) override;
	virtual void ResetClipValues() override;
	virtual void ResetClipValues(fluo::ClipPlane i) override;
	//clip rotation
	virtual void SetClipRotation(int i, double val) override;
	virtual void SetClipRotation(const fluo::Vector& euler) override;
	virtual void SetClipRotation(const fluo::Quaternion& q) override;
	//clip distance
	virtual void SetLink(fluo::ClipPlane i, bool link) override;
	virtual void ResetLink() override;
	virtual void SetLinkedDist(fluo::ClipPlane i, int val) override;

	//randomize color
	void RandomizeColor();

	//shown in legend
	void SetLegend(bool val);
	bool GetLegend();

	//time sequence
	void SetCurTime(int time) { m_time = time; }
	int GetCurTime() { return m_time; }

private:
	std::wstring m_data_path;
	GLMmodelPtr m_data;
	std::unique_ptr<flvr::MeshRenderer> m_mr;
	fluo::BBox m_bounds;
	fluo::Point m_center;
	//reader
	std::weak_ptr<BaseMeshReader> m_reader;
	int m_time;	//time index of the original file

	//sync flags
	bool m_cpu_dirty;//call SubmitData to update gpu data
	bool m_gpu_dirty;//call ReturnData to update cpu data

	bool m_disp;
	bool m_draw_bounds;

	//lighting
	bool m_shading;
	bool m_flat_shading;
	bool m_vertex_color;
	fluo::Color m_color;
	double m_alpha;
	double m_shading_strength;
	double m_shading_shine;
	//fog
	bool m_fog;
	//shadow
	bool m_shadow_enable;
	double m_shadow_intensity;

	fluo::Vector m_trans;
	fluo::Vector m_rot;
	fluo::Vector m_scale = fluo::Vector(1.0);

	//legend
	bool m_legend;

private:
	void BuildMesh();
	void UpdateBounds();
};

#endif//_MESH_DATA_H_