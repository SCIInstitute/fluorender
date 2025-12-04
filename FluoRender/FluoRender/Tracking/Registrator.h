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
#ifndef REGISTRATOR_H
#define REGISTRATOR_H

#include <Vector.h>
#include <Point.h>
#include <Transform.h>
#include <BBox.h>

class VolumeData;
namespace flrd
{
	class Registrator
	{
	public:
		Registrator();
		~Registrator();

		void SetUseMask(bool bval)
		{
			m_use_mask = bval;
		}
		void SetExtension(const fluo::Vector& ext1,
			const fluo::Vector& ext2)
		{
			m_extt = ext1;
			m_exta = ext2;
		}
		void SetMaxIter(const int iter)
		{
			m_iter = iter;
		}
		void SetConvNum(const int num)
		{
			m_conv_num = num;
		}
		void SetMethod(const int method)
		{
			m_method = method;
		}
		void SetFilterSize(const int fsize)
		{
			m_fsize = fsize;
		}
		void SetVolumeData(VolumeData* vd)
		{
			m_vd = vd;
		}

		bool Run(size_t f1, size_t f2,
			int mode, size_t start);

		void SetTranslate(const fluo::Vector &p)
		{
			m_translate = p;
		}
		fluo::Vector GetTranslate()
		{
			return m_translate;
		}
		fluo::Vector GetTranslateVol();
		fluo::Point GetCenter()
		{
			return m_center;
		}
		fluo::Point GetCenterVol();
		void SetCenter(const fluo::Point &p)
		{
			m_center = p;
		}
		fluo::Vector GetEuler()
		{
			return m_euler;
		}
		void SetEuler(const fluo::Vector &e)
		{
			m_euler = e;
		}
		fluo::Transform GetTransform()
		{
			return m_tf;
		}
		void SetTransform(const fluo::Transform &tf)
		{
			m_tf = tf;
		}

	private:
		bool m_use_mask;//only match img in the mask
		fluo::Vector m_extt;//transform extension
		fluo::Vector m_exta;//rotation extension
		fluo::Vector m_offt;//offset for transformation
		fluo::Vector m_offa;//offset for angle
		int m_iter;//max iteration number
		int m_conv_num;//max convergence for step size shrink
		int m_method;//compare method
		int m_fsize;//filter size
		VolumeData *m_vd;

		//output
		fluo::Vector m_translate;//translate
		fluo::Point m_center;//center of rotation
		fluo::Vector m_euler;
		fluo::Transform m_tf;

	private:
		fluo::BBox GetExtent(void* mask, const fluo::Vector& res);
	};
}
#endif//REGISTRATOR_H