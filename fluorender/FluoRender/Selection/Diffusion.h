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
#ifndef FL_Diffusion_h
#define FL_Diffusion_h

#include <vector>
#include "DataManager.h"
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>

using namespace std;

namespace FL
{
	class Diffusion
	{
	public:
		Diffusion(VolumeData* vd);
		~Diffusion();

		void Init(Point& ip, double ini_thresh);
		void Grow(int iter, double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate);

	private:
		VolumeData *m_vd;

	private:
		bool CheckBricks();
		void GetMask(size_t brick_num, TextureBrick* b, void** val);
		void ReleaseMask(void* val, size_t brick_num, TextureBrick* b);
	};

}
#endif//FL_Diffusion_h
