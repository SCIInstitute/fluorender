/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <nrrd.h>
#ifdef STATIC_COMPILE
#define nrrdWrap nrrdWrap_va
#define nrrdAxisInfoSet nrrdAxisInfoSet_va
#endif

namespace FL
{
	class VolumeSampler
	{
	public:
		VolumeSampler();
		~VolumeSampler();

		void SetVolume(Nrrd *data);
		Nrrd* GetVolume();
		Nrrd* GetResult();
		void SetSize(int nx, int ny, int nz);
		void SetType(int type);
		void SetFilterSize(int fx, int fy, int fz);
		void Resize();
		double Sample(double x, double y, double z);

	private:
		Nrrd *m_vd_r;	//result volume data
		Nrrd *m_vd;	//volume data A

		//new size
		int m_nx;
		int m_ny;
		int m_nz;
		//old size
		int m_nx_in;
		int m_ny_in;
		int m_nz_in;
		//bits
		int m_bits;
		int m_bits_in;

		int m_type;	//sampler type
					//0:nearest neighbor;
					//1:linear;
					//2:box;
		//filter size
		int m_fx;
		int m_fy;
		int m_fz;

		int m_border;	//border handling
					//0:set to 0;
					//1:clamp to border
					//2:mirror

	private:
		bool ijk(int &i, int &j, int &k);
		void xyz2ijk(double x, double y, double z,
			int &i, int &j, int &k);
		double SampleNearestNeighbor(double x, double y, double z);
		double SampleLinear(double x, double y, double z);
		double SampleBox(double x, double y, double z);
	};
}
#endif//_VOLUMESAMPLER_H_