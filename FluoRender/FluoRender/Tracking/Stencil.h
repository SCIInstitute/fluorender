/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#ifdef _DEBUG
#include <Debug.hpp>
#endif
#include <ExGauss.h>
#include <Types/BBox.h>
#include <unordered_map>

namespace flrd
{
	struct Stencil
	{
		Stencil() :
		data(0), scale(1.0f), fsize(1) {}
		
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
		float get(size_t i, size_t j, size_t k) const
		{
			if (!data)
				return 0.0f;
			if (!valid(i, j, k))
				return 0.0f;
			unsigned long long index =
				(unsigned long long)nx*ny*k +
				(unsigned long long)nx*j +
				(unsigned long long)i;
			if (bits == 8)
				return ((unsigned char*)data)[index] / 255.0f;
			else
				return ((unsigned short*)data)[index] * scale / 65535.0f;
		}
		float getfilter(size_t i, size_t j, size_t k) const
		{
			if (!data)
				return 0.0f;
			if (!valid(i, j, k))
				return 0.0f;
			if (fsize > 1)
			{
				size_t count = 0;
				float sum = 0;
				float val;
				int lb = fsize / 2 + fsize % 2 - 1;
				int ub = fsize / 2;
				for (int ii = -lb; ii <= ub; ++ii)
				for (int jj = -lb; jj <= ub; ++jj)
				//for (int kk = -lb; kk <= ub; ++kk)
				{
					val = get(i + ii, j + jj, k);
					if (val > 0.0)
					{
						sum += val;
						count++;
					}
				}
				if (count)
					return sum / count;
			}
			else
			{
				return get(i, j, k);
			}
			return 0.0f;
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
		size_t fsize;//filter size (box)
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
			//get v1
			v1 = s1.getfilter(i, j, k);
			//get v2
			v2 = s2.getfilter(i2, j2, k2);
			//get d
			d = fabs(v1 - v2);
			result += d;
		}
		return result;
	}

	inline float similar(const Stencil& s1, const Stencil& s2, int sim)
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

		float v1, v2, d1, d2, w;
		size_t index;
		size_t i, i2, j, j2, k, k2;
		if (sim == 0)
		{
			//dot product
			for (i = minx, i2 = minx2; i <= maxx; ++i, ++i2)
			for (j = miny, j2 = miny2; j <= maxy; ++j, ++j2)
			for (k = minz, k2 = minz2; k <= maxz; ++k, ++k2)
			{
				//get v1
				v1 = s1.getfilter(i, j, k);
				//get v2
				v2 = s2.getfilter(i2, j2, k2);
				//get d weighted
				//d1 = v1 - v2;
				//d2 = 1.0 - std::min(v1, v2);
				w = v1 * v2;
				result += w;
			}
		}
		else if (sim == 1)
		{
			//diff squared
			for (i = minx, i2 = minx2; i <= maxx; ++i, ++i2)
			for (j = miny, j2 = miny2; j <= maxy; ++j, ++j2)
			for (k = minz, k2 = minz2; k <= maxz; ++k, ++k2)
			{
				//get v1
				v1 = s1.getfilter(i, j, k);
				//get v2
				v2 = s2.getfilter(i2, j2, k2);
				//get d weighted
				d1 = v1 - v2;
				//d2 = 1.0 - std::min(v1, v2);
				w = 1.0 - d1 * d1;
				result += w;
			}
		}
		return result;
	}

	inline bool match_stencils(const Stencil& s1, Stencil& s2,
		const fluo::Vector &ext, const fluo::Vector &off,
		fluo::Point &center, float &prob, int iter, float eps,
		int sim)
	{
		fluo::BBox range = s1.box;
		range.extend_ani(ext);
		//range.clamp(fluo::BBox(fluo::Point(0, 0, 0),
		//	fluo::Point(s2.nx, s2.ny, s2.nz)));
		fluo::Vector vmax = range.Max() - s1.box.size();

		int minx = int(range.Min().x() + 0.5);
		int miny = int(range.Min().y() + 0.5);
		int minz = int(range.Min().z() + 0.5);
		int maxx = int(vmax.x() + 0.5);
		int maxy = int(vmax.y() + 0.5);
		int maxz = int(vmax.z() + 0.5);

		//size_t total = (maxx - minx + 1) *
		//	(maxy - miny + 1) * (maxz - minz + 1);
		float p;
		fluo::Point s2min(minx, miny, minz);
		s2.box = fluo::BBox(s2min, s2min);
		fluo::BBox s2temp = s2.box;
		ExGauss eg(maxx - minx + 1, maxy - miny + 1, maxz - minz + 1);
		int i, j, k, ti, tj, tk;
		for (k = minz, tk = 0; k <= maxz; ++k, ++tk)
		for (j = miny, tj = 0; j <= maxy; ++j, ++tj)
		for (i = minx, ti = 0; i <= maxx; ++i, ++ti)
		{
			s2.box = s2temp;
			s2.box.translate(fluo::Vector(ti, tj, tk));
			s2.box.translate(off);
			//p = s1 * s2;
			p = similar(s1, s2, sim);
			eg.SetData(ti, tj, tk, p);
		}

		eg.SetIter(iter, eps);
		eg.Execute();
		center = eg.GetCenter();
		center = fluo::Point(center + off + s2min);
		//center is actually the corner
		s2.box = fluo::BBox(center,
			fluo::Point(center + s1.box.size()));
		s2.id = s1.id;
		prob = eg.GetProb();

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
