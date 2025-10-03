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
#ifndef StencilCompare_h
#define StencilCompare_h

#include <Vector.h>
#include <Point.h>
#include <memory>

namespace flvr
{
	class KernelProgram;
	class Argument;
}
namespace flrd
{
	class Stencil;
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
		//float Similar();
		void Label();
		void Lookup();

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
		std::weak_ptr<flvr::Argument> m_img1;//filtered img
		std::weak_ptr<flvr::Argument> m_img2;
		std::weak_ptr<flvr::Argument> m_mask1;//mask buffer for img1

		float Similar(const std::string& name);
	};
}
#endif // !StencilCompare_h
