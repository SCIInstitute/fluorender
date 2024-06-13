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
#ifndef FL_Stencil_h
#define FL_Stencil_h

#ifdef _DEBUG
#include <Debug.h>
#endif
#include <ExGauss.h>
#include <BBox.h>
#include <Point.h>
#include <Vector.h>
#include <Transform.h>
#include <Neighbor.h>
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
		bool valid(const fluo::Point &p, fluo::Point &tfp) const
		{
			tf.project(p, tfp);
			if (tfp.intx() >= nx || tfp.inty() >= ny || tfp.intz() >= nz)
				return false;
			else
				return true;
		}
		void extend(const fluo::Point &p)
		{
			box.extend(p);
		}
		inline fluo::Point center() const
		{
			fluo::Point p = box.center();
			fluo::Point tfp;
			tf.project(p, tfp);
			return tfp;
		}

		float get(const fluo::Point &p) const
		{
			if (!data)
				return 0.0f;
			fluo::Point tfp;
			if (!valid(p, tfp))
				return 0.0f;
			unsigned long long index =
				(unsigned long long)nx*ny*tfp.intz() +
				(unsigned long long)nx*tfp.inty() +
				(unsigned long long)tfp.intx();
			if (bits == 8)
				return ((unsigned char*)data)[index] / 255.0f;
			else
				return ((unsigned short*)data)[index] * scale / 65535.0f;
		}
		float getfilter(const fluo::Point &p) const
		{
			if (!data)
				return 0.0f;
			fluo::Point tfp;
			if (!valid(p, tfp))
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
					val = get(p + fluo::Vector(ii, jj, 0));
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
				return get(p);
			}
			return 0.0f;
		}
		float getmask(const fluo::Point &p) const
		{
			if (!mask)
				return 0.0f;
			fluo::Point tfp;
			if (!valid(p, tfp))
				return 0.0f;
			unsigned long long index =
				(unsigned long long)nx*ny*tfp.intz() +
				(unsigned long long)nx*tfp.inty() +
				(unsigned long long)tfp.intx();
			return ((unsigned char*)mask)[index] / 255.0f;
		}
		unsigned int getlabel(const fluo::Point &p) const
		{
			if (!label)
				return 0;
			fluo::Point tfp;
			if (!valid(p, tfp))
				return 0;
			unsigned long long index =
				(unsigned long long)nx*ny*tfp.intz() +
				(unsigned long long)nx*tfp.inty() +
				(unsigned long long)tfp.intx();
			return ((unsigned int*)label)[index];
		}
		void setlabel(const fluo::Point &p, unsigned int l)
		{
			if (!label)
				return;
			fluo::Point tfp;
			if (!valid(p, tfp))
				return;
			unsigned long long index =
				(unsigned long long)nx*ny*tfp.intz() +
				(unsigned long long)nx*tfp.inty() +
				(unsigned long long)tfp.intx();
			((unsigned int*)label)[index] = l;
		}
		unsigned int lookuplabel(const fluo::Point &p, Stencil &s)
		{
			fluo::Point tfp;
			s.tf.unproject(p, tfp);
			int x = tfp.intx(), y = tfp.inty(), z = tfp.intz();
			if (x < 0 || x >= nx ||
				y < 0 || y >= ny ||
				z < 0 || z >= nz)
				return 0;
			unsigned long long index =
				(unsigned long long)nx*ny*z +
				(unsigned long long)nx*y +
				(unsigned long long)x;
			return ((unsigned int*)label)[index];
		}

		//transformation
		void load_identity()
		{
			tf.load_identity();
		}
		void translate(const fluo::Vector &v)
		{
			tf.post_translate(v);
		}
		void rotate(const fluo::Vector &v, const fluo::Vector &cp)
		{
			//rotate around center cp
			tf.post_translate(cp);
			if (v.x() != 0.0) tf.post_rotate(fluo::d2r(v.x()), fluo::Vector(1, 0, 0));
			if (v.y() != 0.0) tf.post_rotate(fluo::d2r(v.y()), fluo::Vector(0, 1, 0));
			if (v.z() != 0.0) tf.post_rotate(fluo::d2r(v.z()), fluo::Vector(0, 0, 1));
			tf.post_translate(-cp);
		}

		//pointer to the entire data
		void* data;
		void* mask;
		void* label;
		unsigned int id;
		size_t nx;
		size_t ny;
		size_t nz;
		size_t bits;
		float scale;
		float max_int;
		fluo::BBox box;
		fluo::Transform tf;
		size_t fsize;//filter size (box)
	};

	typedef std::unordered_map<unsigned int, Stencil> StencilList;
	typedef std::unordered_map<unsigned int, Stencil>::iterator StencilListIter;

	//functions
	//inline float operator*(const Stencil& s1, const Stencil& s2)
	//{
	//	float result = 0.0f;

	//	float v1, v2, d;
	//	fluo::Range nb(s1.box);
	//	for (fluo::Point i = nb.begin(); i != nb.end(); i = ++nb)
	//	{
	//		v1 = s1.getfilter(i);
	//		v2 = s2.getfilter(i);
	//		d = std::abs(v1 - v2);
	//		result += d;
	//	}
	//	return result;
	//}


	//inline bool match_stencils(const Stencil& s1, Stencil& s2,
	//	const fluo::Vector &ext, const fluo::Vector &off,
	//	fluo::Point &center, float &prob, int iter, float eps,
	//	int sim)
	//{
	//	fluo::BBox range = s1.box;
	//	range.extend_ani(ext);
	//	//range.clamp(fluo::BBox(fluo::Point(0, 0, 0),
	//	//	fluo::Point(s2.nx, s2.ny, s2.nz)));
	//	fluo::Vector vmax = range.Max() - s1.box.size();

	//	int minx = int(range.Min().x() + 0.5);
	//	int miny = int(range.Min().y() + 0.5);
	//	int minz = int(range.Min().z() + 0.5);
	//	int maxx = int(vmax.x() + 0.5);
	//	int maxy = int(vmax.y() + 0.5);
	//	int maxz = int(vmax.z() + 0.5);

	//	//size_t total = (maxx - minx + 1) *
	//	//	(maxy - miny + 1) * (maxz - minz + 1);
	//	float p;
	//	fluo::Point s2min(minx, miny, minz);
	//	s2.box = fluo::BBox(s2min, s2min);
	//	fluo::BBox s2temp = s2.box;
	//	ExGauss eg(maxx - minx + 1, maxy - miny + 1, maxz - minz + 1);
	//	int i, j, k, ti, tj, tk;
	//	for (k = minz, tk = 0; k <= maxz; ++k, ++tk)
	//	for (j = miny, tj = 0; j <= maxy; ++j, ++tj)
	//	for (i = minx, ti = 0; i <= maxx; ++i, ++ti)
	//	{
	//		s2.box = s2temp;
	//		s2.translate(fluo::Vector(ti, tj, tk) + off);
	//		//s2.box.translate(off);
	//		//p = s1 * s2;
	//		p = similar(s1, s2, sim);
	//		eg.SetData(ti, tj, tk, p);
	//	}

	//	eg.SetIter(iter, eps);
	//	eg.Execute();
	//	center = eg.GetCenter();
	//	center = fluo::Point(center + off + s2min);
	//	//center is actually the corner
	//	s2.box = fluo::BBox(center,
	//		fluo::Point(center + s1.box.size()));
	//	s2.id = s1.id;
	//	prob = eg.GetProb();

	//	return true;
	//}

}//namespace flrd

#endif//FL_Stencil_h
