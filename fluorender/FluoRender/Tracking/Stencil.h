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
#ifndef FL_Stencil_h
#define FL_Stencil_h

#include <BBox.h>
#include "boost/unordered_map.hpp"

namespace FL
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
			if (box.min() == box.max())
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
			box.extend(FLIVR::Point(i, j, k));
		}

		//pointer to the entire data
		void* data;
		unsigned int id;
		size_t nx;
		size_t ny;
		size_t nz;
		size_t bits;
		float scale;
		FLIVR::BBox box;
	};

	typedef boost::unordered_map<unsigned int, Stencil> StencilList;
	typedef boost::unordered_map<unsigned int, Stencil>::iterator StencilListIter;

	//functions
	inline float operator*(const Stencil& s1, const Stencil& s2)
	{
		float result = 0.0f;

		//sum of products in the overlap region
		//with two stencils aligned at center
		size_t minx = size_t(s1.box.min().x() + 0.5);
		size_t maxx = size_t(s1.box.max().x() + 0.5);
		size_t miny = size_t(s1.box.min().y() + 0.5);
		size_t maxy = size_t(s1.box.max().y() + 0.5);
		size_t minz = size_t(s1.box.min().z() + 0.5);
		size_t maxz = size_t(s1.box.max().z() + 0.5);
		//s2
		size_t minx2 = size_t(s2.box.min().x() + 0.5);
		size_t miny2 = size_t(s2.box.min().y() + 0.5);
		size_t minz2 = size_t(s2.box.min().z() + 0.5);

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
		size_t minx = size_t(s1.box.min().x() + 0.5);
		size_t maxx = size_t(s1.box.max().x() + 0.5);
		size_t miny = size_t(s1.box.min().y() + 0.5);
		size_t maxy = size_t(s1.box.max().y() + 0.5);
		size_t minz = size_t(s1.box.min().z() + 0.5);
		size_t maxz = size_t(s1.box.max().z() + 0.5);
		//s2
		size_t minx2 = size_t(s2.box.min().x() + 0.5);
		size_t miny2 = size_t(s2.box.min().y() + 0.5);
		size_t minz2 = size_t(s2.box.min().z() + 0.5);

		float v1;
		float v1max = 0;
		float v2max = 0;
		FLIVR::Point p1max, p2max;
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
				p1max = FLIVR::Point(i, j, k);
			}
			//if (v2 > v2max)
			//{
			//	v2max = v2;
			//	p2max = FLIVR::Point(i2, j2, k2);
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
		Stencil& s2, const FLIVR::Vector &ext,
		FLIVR::Point &center, float &prob)
	{
		//std::ofstream file;
		//file.open("E:/Data/Holly/test/test.txt");

		FLIVR::BBox range = s1.box;
		range.extend_ani(ext);
		range.clamp(FLIVR::BBox(FLIVR::Point(0, 0, 0),
			FLIVR::Point(s2.nx, s2.ny, s2.nz)));
		FLIVR::Vector vmax = range.max() - s1.box.size();

		size_t minx = size_t(range.min().x() + 0.5);
		size_t miny = size_t(range.min().y() + 0.5);
		size_t minz = size_t(range.min().z() + 0.5);
		size_t maxx = size_t(vmax.x() + 0.5);
		size_t maxy = size_t(vmax.y() + 0.5);
		size_t maxz = size_t(vmax.z() + 0.5);

		size_t total = (maxx - minx + 1) *
			(maxy - miny + 1) * (maxz - minz + 1);
		float p;
		float minp = total;
		float sump = 0;
		FLIVR::Point s2min(minx, miny, minz);
		s2.box = FLIVR::BBox(s2min, s2min);
		FLIVR::BBox s2temp = s2.box;
		size_t i, j, k, ti, tj, tk;
		for (k = minz, tk = 0; k <= maxz; ++k, ++tk)
		for (j = miny, tj = 0; j <= maxy; ++j, ++tj)
		for (i = minx, ti = 0; i <= maxx; ++i, ++ti)
		{
			s2.box = s2temp;
			s2.box.translate(FLIVR::Vector(ti, tj, tk));
			p = s1 * s2;
			//p = similar(s1, s2);

			//file << p;
			//if (i < maxx)
			//	file << "\t";
			//else
			//	file << "\n";

			sump += p;
			if (p < minp)
			{
				minp = p;
				center = s2.box.min();
			}
		}
		prob = minp * total / sump;
		//center += s1.box.size() / 2;
		//center is actually the corner
		s2.box = FLIVR::BBox(center,
			FLIVR::Point(center + s1.box.size()));
		s2.id = s1.id;

		//file.close();
		return true;
	}

	inline void label_stencil(const Stencil& s1,
		const Stencil& s2, void* label1, void* label2)
	{
		size_t minx = size_t(s1.box.min().x() + 0.5);
		size_t maxx = size_t(s1.box.max().x() + 0.5);
		size_t miny = size_t(s1.box.min().y() + 0.5);
		size_t maxy = size_t(s1.box.max().y() + 0.5);
		size_t minz = size_t(s1.box.min().z() + 0.5);
		size_t maxz = size_t(s1.box.max().z() + 0.5);
		//s2
		size_t minx2 = size_t(s2.box.min().x() + 0.5);
		size_t miny2 = size_t(s2.box.min().y() + 0.5);
		size_t minz2 = size_t(s2.box.min().z() + 0.5);

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

}//namespace FL

#endif//FL_Stencil_h
