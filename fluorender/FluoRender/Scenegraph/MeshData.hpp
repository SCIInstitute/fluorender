/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#ifndef MESHDATA_HPP
#define MESHDATA_HPP

#include <Node.hpp>
#include <Names.hpp>
#include <glm/glm.hpp>

struct _GLMmodel;
typedef _GLMmodel GLMmodel;
namespace flvr
{
	class MeshRenderer;
}
namespace fluo
{
	class MeshFactory;
	class MeshData;
	typedef std::vector<MeshData*> MeshList;

	class MeshData : public Node
	{
	public:
		MeshData();
		MeshData(const MeshData& data, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new MeshData(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const MeshData*>(obj) != NULL;
		}

		virtual const char* className() const { return "MeshData"; }

		virtual MeshData* asMeshData() { return this; }
		virtual const MeshData* asMeshData() const { return this; }

		//load
		int LoadData(GLMmodel* mesh);
		int LoadData(const std::wstring &filename);

		//save
		void SaveData(const std::string &filename);

		//mesh renderer
		flvr::MeshRenderer* GetRenderer();

		//draw
		void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat);
		void Draw(int peel);
		void DrawBounds();
		void DrawInt(unsigned int name);

		friend class MeshFactory;

	protected:
		virtual ~MeshData();

	private:
		GLMmodel* m_data;
		flvr::MeshRenderer* m_mr;

	private:
		void Initialize();

		void OnViewportChanged(Event& event);
		void OnLightEnableChanged(Event& event);
		void OnDepthAttenChanged(Event& event);
		void OnMaterialChanged(Event& event);
		void OnBoundsChanged(Event& event);
		void OnRandomizeColor(Event& event);

	};
}

#endif//_MESHDATA_H_
