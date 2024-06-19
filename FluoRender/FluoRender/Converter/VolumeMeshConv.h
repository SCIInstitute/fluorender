/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _VOLUME_MESH_CONV_H_
#define _VOLUME_MESH_CONV_H_

#include <vector>
#include <glm.h>
#include <Vector.h>
#include <nrrd.h>

namespace flrd
{
	//convert volume data to mesh
	class VolumeMeshConv
	{
	public:
		VolumeMeshConv();
		~VolumeMeshConv();

		//high-level wrapper
		void Compute();
		void Convert();

		void SetVolumeUseTrans(bool val) { m_use_transfer = val; }
		bool GetVolumeUseTrans() { return m_use_transfer; }
		void SetVolumeUseMask(bool val) { m_use_mask = val; }
		bool GetVolumeUseMask() { return m_use_mask; }
		void SetIsoValue(double val) { m_iso = val; }
		double GetIsoValue() { return m_iso; }
		void SetDownsample(int val) { m_downsample = val; }
		int GetDownsample() { return m_downsample; }
		void SetDownsampleZ(int val) { m_downsample_z = val; }
		int GetDownsampleZ() { return m_downsample_z; }
		void SetWeldVertices(bool val) { m_weld = val; }
		bool GetWeldVertices() { return m_weld; }
		float GetArea() { return m_area; }

	private:
		typedef struct
		{
			fluo::Vector p[3];
		} MCTriangle;
		Nrrd* m_volume;
		Nrrd* m_mask;
		GLMmodel* m_mesh;
		float m_area;
	
		//iso value
		double m_iso;
		//downsampling
		int m_downsample;
		//downsampling Z
		int m_downsample_z;
		//volume max value
		double m_vol_max;
		//grid info
		int m_nx, m_ny, m_nz;
		double m_spcx, m_spcy, m_spcz;
		//volume info
		bool m_use_transfer;
		double m_gamma, m_lo_thresh, m_hi_thresh, m_sw, m_saturation, m_boundary;
		bool m_use_mask;
		bool m_weld;

	private:
		double GetValue(int x, int y, int z);
		double GetMaxNeighbor(double neighbors[3][3][3],
			int xx, int yy, int zz);
		fluo::Vector Intersect(double verts[8], int v1, int v2,
			int x, int y, int z);
	};

}

#endif//_VOLUME_MESH_CONV_H_
