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
#ifndef _TRAINER_H_
#define _TRAINER_H_

#include <string>

namespace flrd
{
	class Trainer
	{
	public:
		Trainer();
		~Trainer();

		virtual void set_trained_rec_num(size_t val)
		{
			m_trained_rec_num = val;
			m_valid = m_trained_rec_num > 0;
		}
		virtual size_t get_trained_rec_num() { return m_trained_rec_num; }

		virtual void add(float*, float*) = 0;

		virtual void train() = 0;
		virtual float* infer(float*) = 0;

		virtual double get_rate() = 0;
		bool is_valid() { return m_valid; }

		virtual void set_model_file(const std::string& file) { m_model_file = file; }
		virtual std::string get_model_file() { return m_model_file; }

	protected:
		bool m_valid;
		size_t m_trained_rec_num;
		std::string m_model_file;
	};
}
#endif//_TRAINER_H_