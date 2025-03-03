﻿/*
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
#ifndef _DNNTRAINER_H_
#define _DNNTRAINER_H_

#include <Trainer.h>
#include <dlib/matrix.h>
#include <Numbers.h>

namespace flrd
{
	class DnnTrainer : public Trainer
	{
	public :
		DnnTrainer();
		~DnnTrainer();

		virtual void add(float*, float*) = 0;
		virtual void train() = 0;
		virtual float* infer(float*) = 0;
		virtual double get_rate() = 0;
		virtual void set_model_file(const std::wstring& file) = 0;

	protected:
		std::vector<dlib::matrix<float>> m_input;
		std::vector<dlib::matrix<float>> m_output;

		dlib::matrix<float> m_result;
	};
}
#endif