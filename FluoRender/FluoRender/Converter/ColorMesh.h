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
#ifndef _COLOR_MESH_H_
#define _COLOR_MESH_H_

#include <Progress.h>
#include <memory>

class VolumeData;
class MeshData;
namespace flvr
{
	class TextureBrick;
}
namespace flrd
{
	//apply per vertex color to mesh based on components
	class ColorMesh : public Progress
	{
	public:
		ColorMesh() : Progress() {}
		~ColorMesh() {}

		void SetVolumeData(const std::shared_ptr<VolumeData>& vd) { m_volume = vd; }
		void SetMeshData(const std::shared_ptr<MeshData>& md) { m_mesh = md; }

		void SetUseSel(bool val) { m_use_sel = val; }
		void SetUseComp(bool val) { m_use_comp = val; }

		void Update();

	protected:
		std::weak_ptr<VolumeData> m_volume;
		std::weak_ptr<MeshData> m_mesh;

		bool m_use_sel = true;
		bool m_use_comp = false;

	protected:
		std::string GetKernelStrColorMesh(int mode);//0: vol; 1: mask; 2: label

		bool GetInfo(flvr::TextureBrick* b,
			long& bits, long& nx, long& ny, long& nz,
			long& ox, long& oy, long& oz);

	};
}

#endif//_COLOR_MESH_H_