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
#ifndef _VOLUME_MESH_CONV_H_
#define _VOLUME_MESH_CONV_H_

#include <vector>
#include "../FLIVR/glm.h"
#include "../FLIVR/Vector.h"
#include "nrrd.h"

using namespace std;
using namespace FLIVR;

//convert volume data to mesh
class VolumeMeshConv
{
public:
	VolumeMeshConv();
	~VolumeMeshConv();

	void SetVolume(Nrrd* volume);
	Nrrd* GetVolume();
	GLMmodel* GetMesh();
	void SetVolumeSpacings(double x, double y, double z);
	void SetVolumeUseTrans(bool use);
	void SetVolumeTransfer(double gamma, double lo_thresh,
		double hi_thresh, double offset, double gm_thresh);
	void SetVolumeUseMask(bool use);
	void SetVolumeMask(Nrrd* mask);

	void SetMaxValue(double mv);
	void SetIsoValue(double iso);
	void SetDownsample(int downsample);
	void SetDownsampleZ(int downsample);
	void Convert();

	//soft threshold
	static void SetSoftThreshold(double val)
	{ m_sw = val; }

private:
	typedef struct
	{
		Vector p[3];
	} MCTriangle;
	Nrrd* m_volume;
	Nrrd* m_mask;
	GLMmodel* m_mesh;
	
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
	double m_gamma, m_lo_thresh, m_hi_thresh, m_offset, m_gm_thresh;
	bool m_use_mask;

	//soft threshold
	static double m_sw;

private:
	double GetValue(int x, int y, int z);
	double GetMaxNeighbor(double neighbors[3][3][3],
		int xx, int yy, int zz);
	Vector Intersect(double verts[8], int v1, int v2,
		int x, int y, int z);
};

#endif//_VOLUME_MESH_CONV_H_
