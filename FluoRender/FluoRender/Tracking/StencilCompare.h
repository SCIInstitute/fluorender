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
#ifndef StencilCompare_h
#define StencilCompare_h

#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <Stencil.h>

namespace flrd
{
	class StencilCompare
	{
	public:
		StencilCompare(Stencil* s1, Stencil* s2,
			const fluo::Vector& ext1, const fluo::Vector& ext2,
			const fluo::Vector& off1, const fluo::Vector& off2,
			const int iter, const int conv_num, const int method,
			const bool use_mask);
		~StencilCompare();

		void Prepare(const std::string& cmp_name);
		void Clean();
		bool Compare();

		//inline float Similar()
		//{
		//	float result = 0.0f;

		//	float v1, v2, d1, d2, w;
		//	fluo::Range nb(m_s1->box);
		//	if (m_method == 0)
		//	{
		//		//dot product
		//		for (fluo::Point i = nb.begin(); i != nb.end(); i = ++nb)
		//		{
		//			//get v1
		//			v1 = m_s1->getfilter(i);
		//			//get v2
		//			v2 = m_s2->getfilter(i);
		//			//get d weighted
		//			//d1 = v1 - v2;
		//			//d2 = 1.0 - std::min(v1, v2);
		//			w = v1 * v2;
		//			result += w;
		//		}
		//	}
		//	else if (m_method == 1)
		//	{
		//		//diff squared
		//		for (fluo::Point i = nb.begin(); i != nb.end(); i = ++nb)
		//		{
		//			//get v1
		//			v1 = m_s1->getfilter(i);
		//			//get v2
		//			v2 = m_s2->getfilter(i);
		//			//get d weighted
		//			d1 = v1 - v2;
		//			//d2 = 1.0 - std::min(v1, v2);
		//			w = 1.0 - d1 * d1;
		//			result += w;
		//		}
		//	}
		//	return result;
		//}

		inline void Label()
		{
			fluo::Range nb(m_s1->box);

			unsigned int l;
			fluo::Point tfp1, tfp2;
			for (fluo::Point i = nb.begin(); i != nb.end(); i = ++nb)
			{
				if (!m_s1->valid(i, tfp1) || !m_s2->valid(i, tfp2))
					continue;
				//get v1
				l = m_s1->getlabel(i);
				//set s2
				if (l == m_s1->id)
					m_s2->setlabel(i, l);
			}
		}

		inline void Lookup()
		{
			fluo::Range all2(fluo::Point(),
				fluo::Vector(m_s2->nx - 1, m_s2->ny - 1, m_s2->nz - 1));
			for (fluo::Point i = all2.begin(); i != all2.end(); i = ++all2)
			{
				unsigned int l = m_s1->lookuplabel(i, *m_s2);
				if (l == m_s1->id)
				{
					unsigned long long index =
						(unsigned long long)m_s2->nx*m_s2->ny*i.intz() +
						(unsigned long long)m_s2->nx*i.inty() +
						(unsigned long long)i.intx();
					((unsigned int*)m_s2->label)[index] = l;
				}
			}
		}

		fluo::Point GetTranslate()
		{
			return m_translate;
		}
		fluo::Point GetCenter()
		{
			return m_center;
		}
		fluo::Point GetEuler()
		{
			return m_euler;
		}

	private:
		bool m_use_mask;

		Stencil* m_s1;
		Stencil* m_s2;

		fluo::Vector m_ext1;//initial sample neighborhood
		fluo::Vector m_ext2;
		fluo::Vector m_off1;//interframe offsets
		fluo::Vector m_off2;
		int m_iter;//iteration limit
		int m_conv_num;//max convergence number for step size shrink
		int m_method;//0-dot product; 1-diff squared
		int m_fsize;//filer size

		//rigid transform
		fluo::Point m_translate;
		fluo::Point m_center;//center of rotation
		fluo::Point m_euler;//rotation

		flvr::KernelProgram* m_prog;
		flvr::Argument m_img1;//filtered img
		flvr::Argument m_img2;
		flvr::Argument m_mask1;//mask buffer for img1

		float Similar(const std::string& name);
	};
}
#endif // !StencilCompare_h
