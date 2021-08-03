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
#ifndef FL_Stencil_h
#define FL_Stencil_h

#include "exmax1.h"
#include <Types/BBox.h>
#include <unordered_map>

namespace flrd
{
	struct Stencil
	{
		Stencil() :
		data(0), scale(1.0f) {}
		
		bool valid() const
		{
			if (!data)
				return false;
			if (!nx || !ny || !nz)
				return false;
			if (box.Min() == box.Max())
				return false;
			return true;
		}
		bool valid(size_t i, size_t j, size_t k) const
		{
			if (i >= nx || j >= ny || k >= nz)
				return false;
			else
				return true;
		}
		void extend(size_t i, size_t j, size_t k)
		{
			box.extend(fluo::Point(i, j, k));
		}

		//pointer to the entire data
		void* data;
		unsigned int id;
		size_t nx;
		size_t ny;
		size_t nz;
		size_t bits;
		float scale;
		fluo::BBox box;
	};

	typedef std::unordered_map<unsigned int, Stencil> StencilList;
	typedef std::unordered_map<unsigned int, Stencil>::iterator StencilListIter;

	//functions
	inline float operator*(const Stencil& s1, const Stencil& s2)
	{
		float result = 0.0f;

		//sum of products in the overlap region
		//with two stencils aligned at center
		size_t minx = size_t(s1.box.Min().x() + 0.5);
		size_t maxx = size_t(s1.box.Max().x() + 0.5);
		size_t miny = size_t(s1.box.Min().y() + 0.5);
		size_t maxy = size_t(s1.box.Max().y() + 0.5);
		size_t minz = size_t(s1.box.Min().z() + 0.5);
		size_t maxz = size_t(s1.box.Max().z() + 0.5);
		//s2
		size_t minx2 = size_t(s2.box.Min().x() + 0.5);
		size_t miny2 = size_t(s2.box.Min().y() + 0.5);
		size_t minz2 = size_t(s2.box.Min().z() + 0.5);

		float v1, v2, d;
		size_t index;
		size_t i, i2, j, j2, k, k2;
		for (i = minx, i2 = minx2; i <= maxx; ++i, ++i2)
		for (j = miny, j2 = miny2; j <= maxy; ++j, ++j2)
		for (k = minz, k2 = minz2; k <= maxz; ++k, ++k2)
		{
			if (!s1.valid(i, j, k) ||
				!s2.valid(i2, j2, k2))
				continue;
			//get v1
			index = s1.nx*s1.ny*k + s1.nx*j + i;
			if (s1.bits == 8)
				v1 = ((unsigned char*)(s1.data))[index] / 255.0f;
			else
				v1 = ((unsigned short*)(s1.data))[index] * s1.scale / 65535.0f;
			//get v2
			index = s2.nx*s2.ny*k2 + s2.nx*j2 + i2;
			if (s2.bits == 8)
				v2 = ((unsigned char*)(s2.data))[index] / 255.0f;
			else
				v2 = ((unsigned short*)(s2.data))[index] * s2.scale / 65535.0f;
			//get d
			d = fabs(v1 - v2);
			result += d;
		}
		return result;
	}

	inline float similar(const Stencil& s1, const Stencil& s2)
	{
		float result = 0.0f;

		//sum of products in the overlap region
		//with two stencils aligned at center
		size_t minx = size_t(s1.box.Min().x() + 0.5);
		size_t maxx = size_t(s1.box.Max().x() + 0.5);
		size_t miny = size_t(s1.box.Min().y() + 0.5);
		size_t maxy = size_t(s1.box.Max().y() + 0.5);
		size_t minz = size_t(s1.box.Min().z() + 0.5);
		size_t maxz = size_t(s1.box.Max().z() + 0.5);
		//s2
		size_t minx2 = size_t(s2.box.Min().x() + 0.5);
		size_t miny2 = size_t(s2.box.Min().y() + 0.5);
		size_t minz2 = size_t(s2.box.Min().z() + 0.5);

		float v1;
		float v1max = 0;
		float v2max = 0;
		fluo::Point p1max, p2max;
		size_t index;
		size_t i, i2, j, j2, k, k2;
		for (i = minx, i2 = minx2; i <= maxx; ++i, ++i2)
		for (j = miny, j2 = miny2; j <= maxy; ++j, ++j2)
		for (k = minz, k2 = minz2; k <= maxz; ++k, ++k2)
		{
			if (!s1.valid(i, j, k) ||
				!s2.valid(i2, j2, k2))
				continue;
			//get v1
			index = s1.nx*s1.ny*k + s1.nx*j + i;
			if (s1.bits == 8)
				v1 = ((unsigned char*)(s1.data))[index] / 255.0f;
			else
				v1 = ((unsigned short*)(s1.data))[index] * s1.scale / 65535.0f;
			//get v2
			//index = s2.nx*s2.ny*k2 + s2.nx*j2 + i2;
			//if (s2.bits == 8)
			//	v2 = ((unsigned char*)(s2.data))[index] / 255.0f;
			//else
			//	v2 = ((unsigned char*)(s2.data))[index] * s2.scale / 65535.0f;
			//get d
			if (v1 > v1max)
			{
				v1max = v1;
				p1max = fluo::Point(i, j, k);
			}
			//if (v2 > v2max)
			//{
			//	v2max = v2;
			//	p2max = flvr::Point(i2, j2, k2);
			//}
		}
		i2 = minx2 + (maxx - minx) / 2;
		j2 = miny2 + (maxy - miny) / 2;
		k2 = minz2 + (maxz - minz) / 2;
		index = s2.nx*s2.ny*k2 + s2.nx*j2 + i2;
		if (s2.bits == 8)
			v2max = ((unsigned char*)(s2.data))[index] / 255.0f;
		else
			v2max = ((unsigned short*)(s2.data))[index] * s2.scale / 65535.0f;
		result = fabs(v1max - v2max);
		return result;
	}

	inline bool match_stencils(const Stencil& s1,
		Stencil& s2, const fluo::Vector &ext,
		fluo::Point &center, float &prob)
	{
//#ifdef _DEBUG
//		std::ofstream ofs;
//		ofs.open("E:/Data/Test/pattern_tracking/test.data",
//			std::ios::out | std::ios::binary);
//#endif
		fluo::BBox range = s1.box;
		range.extend_ani(ext);
		range.clamp(fluo::BBox(fluo::Point(0, 0, 0),
			fluo::Point(s2.nx, s2.ny, s2.nz)));
		fluo::Vector vmax = range.Max() - s1.box.size();

		size_t minx = size_t(range.Min().x() + 0.5);
		size_t miny = size_t(range.Min().y() + 0.5);
		size_t minz = size_t(range.Min().z() + 0.5);
		size_t maxx = size_t(vmax.x() + 0.5);
		size_t maxy = size_t(vmax.y() + 0.5);
		size_t maxz = size_t(vmax.z() + 0.5);

		size_t total = (maxx - minx + 1) *
			(maxy - miny + 1) * (maxz - minz + 1);
		float p;
		fluo::Point s2min(minx, miny, minz);
		s2.box = fluo::BBox(s2min, s2min);
		fluo::BBox s2temp = s2.box;
		ExMax1 em1;
		size_t i, j, k, ti, tj, tk;
		for (k = minz, tk = 0; k <= maxz; ++k, ++tk)
		for (j = miny, tj = 0; j <= maxy; ++j, ++tj)
		for (i = minx, ti = 0; i <= maxx; ++i, ++ti)
		{
			s2.box = s2temp;
			s2.box.translate(fluo::Vector(ti, tj, tk));
			p = s1 * s2;
			//p = similar(s1, s2);

			EmVec pnt = {
				static_cast<double>(s2.box.Min().x()),
				static_cast<double>(s2.box.Min().y()),
				static_cast<double>(s2.box.Min().z()) };
			em1.AddClusterPoint(
				pnt, p);
//#ifdef _DEBUG
//			ofs.write((char*)(&p), sizeof(float));
//#endif
		}

		em1.Execute();
		center = em1.GetCenter();
		//center is actually the corner
		s2.box = fluo::BBox(center,
			fluo::Point(center + s1.box.size()));
		s2.id = s1.id;
		prob = em1.GetProb();

//#ifdef _DEBUG
//		ofs.close();
//#endif
		return true;
	}

	inline void label_stencil(const Stencil& s1,
		const Stencil& s2, void* label1, void* label2)
	{
		size_t minx = size_t(s1.box.Min().x() + 0.5);
		size_t maxx = size_t(s1.box.Max().x() + 0.5);
		size_t miny = size_t(s1.box.Min().y() + 0.5);
		size_t maxy = size_t(s1.box.Max().y() + 0.5);
		size_t minz = size_t(s1.box.Min().z() + 0.5);
		size_t maxz = size_t(s1.box.Max().z() + 0.5);
		//s2
		size_t minx2 = size_t(s2.box.Min().x() + 0.5);
		size_t miny2 = size_t(s2.box.Min().y() + 0.5);
		size_t minz2 = size_t(s2.box.Min().z() + 0.5);

		unsigned int l;
		size_t index;
		size_t i, i2, j, j2, k, k2;
		for (i = minx, i2 = minx2; i <= maxx; ++i, ++i2)
		for (j = miny, j2 = miny2; j <= maxy; ++j, ++j2)
		for (k = minz, k2 = minz2; k <= maxz; ++k, ++k2)
		{
			if (!s1.valid(i, j, k) ||
				!s2.valid(i2, j2, k2))
				continue;
			//get v1
			index = s1.nx*s1.ny*k + s1.nx*j + i;
			l = ((unsigned int*)label1)[index];
			if (l == s1.id)
			{
				//get v2
				index = s2.nx*s2.ny*k2 + s2.nx*j2 + i2;
				((unsigned int*)label2)[index] = l;
			}
		}
	}

}//namespace flrd

#endif//FL_Stencil_h
