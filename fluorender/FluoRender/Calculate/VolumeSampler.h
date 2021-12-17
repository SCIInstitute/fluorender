/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef _VOLUMESAMPLER_H_
#define _VOLUMESAMPLER_H_

#include "DataManager.h"
#include <Types/Vector.h>
#include <Types/Quaternion.h>

namespace flrd
{
	enum SampDataType
	{
		SDT_All = 0,
		SDT_Data,
		SDT_Mask,
		SDT_Label,
	};
	class VolumeSampler
	{
	public:
		VolumeSampler();
		~VolumeSampler();

		void SetInput(VolumeData *data);
		VolumeData* GetInput();
		VolumeData* GetResult();
		void SetSize(int nx, int ny, int nz);
		void SetFilter(int type);
		void SetFilterSize(int fx, int fy, int fz);
		void SetCrop(bool crop);
		void SetClipRotation(fluo::Quaternion &q);
		void Resize(SampDataType type, bool replace);
		double Sample(double x, double y, double z);
		unsigned int SampleInt(double x, double y, double z);

	private:
		VolumeData *m_input;	//input
		VolumeData *m_result;	//result
		void* m_raw_input;		//
		void* m_raw_result;		//

		//input size
		int m_nx_in;
		int m_ny_in;
		int m_nz_in;
		//new size (resize)
		int m_nx;
		int m_ny;
		int m_nz;
		//input bits
		int m_bits;
		//crop
		bool m_crop;
		//crop size (of new size)
		int m_ox;
		int m_oy;
		int m_oz;
		int m_lx;
		int m_ly;
		int m_lz;
		fluo::Quaternion m_q_cl;//rotation

		int m_filter;	//sampler type
						//0:nearest neighbor;
						//1:bilinear;
						//2:trilinear;
						//3:box;
		//filter size
		int m_fx;
		int m_fy;
		int m_fz;

		int m_border;	//border handling
						//0:set to 0;
						//1:clamp to border
						//2:mirror

	private:
		Nrrd* GetNrrd(VolumeData* vd, SampDataType type);
		void* GetRaw(VolumeData* vd, SampDataType type);
		double SampleNearestNeighbor(double x, double y, double z);
		double SampleBiLinear(double x, double y, double z);
		double SampleTriLinear(double x, double y, double z);
		double SampleBox(double x, double y, double z);
		int rotate_scale(fluo::Vector &vsize_in, fluo::Vector &vspc_in,
			fluo::Vector &vsize, fluo::Vector &vspc);
		int consv_volume(fluo::Vector &vec, fluo::Vector &vec_in);
		bool ijk(int &i, int &j, int &k);
		void xyz2ijk(double x, double y, double z,
			int &i, int &j, int &k);
		void xyz2ijkt(double x, double y, double z,
			int &i, int &j, int &k,
			double &tx, double &ty, double &tz);
		//interpolation linear
		//t - normalized factor [0, 1]
		double lerp(double t, double q0, double q1)
		{
			return (1.0 - t) * q0 + t * q1;
		}
		double bilerp(double tx, double ty,
			double q00, double q01, double q10, double q11)
		{
			double r1 = lerp(tx, q00, q10);
			double r2 = lerp(tx, q01, q11);
			return lerp(ty, r1, r2);
		}
		double trilerp(double tx, double ty, double tz,
			double q000, double q001, double q010, double q011,
			double q100, double q101, double q110, double q111)
		{
			double x00 = lerp(tx, q000, q100);
			double x10 = lerp(tx, q010, q110);
			double x01 = lerp(tx, q001, q101);
			double x11 = lerp(tx, q011, q111);
			double r0 = lerp(ty, x00, x10);
			double r1 = lerp(ty, x01, x11);
			return lerp(tz, r0, r1);
		}
	};
}
#endif//_VOLUMESAMPLER_H_