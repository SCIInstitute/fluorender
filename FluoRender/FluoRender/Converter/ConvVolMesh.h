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
#ifndef _CONV_VOL_MESH_H_
#define _CONV_VOL_MESH_H_

#include <BaseConvVolMesh.h>
#include <memory>

class VolumeData;
class MeshData;
namespace flvr
{
	class TextureBrick;
	class Argument;
}
namespace flrd
{
	//convert volume data to mesh
	class ConvVolMesh : public BaseConvVolMesh
	{
	public:
		ConvVolMesh();
		~ConvVolMesh();

		//high-level wrapper
		virtual void Convert() override;
		virtual void Update(bool create_mesh) override;
		virtual void MergeVertices(bool avg_normals) override;

		//simplify and smooth
		virtual void Simplify() override;
		virtual void Smooth() override;

		virtual bool GetAutoUpdate() override;
		virtual bool GetAutoThreshold() override;

	private:
		bool m_use_sel = false;//internal flag based on validity of mask
		bool m_merged = false;

	private:
		std::string GetKernelStrMarchingCubes(bool mask, bool tf);
		std::string GetKernelStrSmoothNormals();

		bool GetInfo(flvr::TextureBrick* b,
			long &bits, long &nx, long &ny, long &nz,
			long &ox, long &oy, long &oz);

		void MarchingCubes(VolumeData* vd, MeshData* md);

		void AverageNormals(std::weak_ptr<flvr::Argument> vbo, std::weak_ptr<flvr::Argument> ibo,
			int vertex_count, int idx_count);

		void PrefixSum(const std::vector<int>& unique_flags,
			std::vector<int>& remap_table,
			std::vector<int>& prefix_sum,
			std::vector<int>& remap_to_compact,
			bool flatten_remap_table);
	};

}

#endif//_CONV_VOL_MESH_H_
