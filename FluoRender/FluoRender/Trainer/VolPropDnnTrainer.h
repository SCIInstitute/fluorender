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
#ifndef _VOLPROPDNNTRAINER_H_
#define _VOLPROPDNNTRAINER_H_

#include <DnnTrainer.h>

namespace flrd
{
	//adjust net definition to change behaviors
	using net_type_vp =
		dlib::loss_mean_squared_multioutput<
		dlib::fc<gno_vp_output_size,
		dlib::input<dlib::matrix<float>>>>;
		//dlib::relu<dlib::fc<gno_vp_input_size,
		//dlib::input<dlib::matrix<float>>
		//>>>>;

	class VolPropDnnTrainer : public DnnTrainer
	{
	public :
		VolPropDnnTrainer();
		~VolPropDnnTrainer();

		virtual void add(float*, float*);
		virtual void train();
		virtual float* infer(float*);
		virtual double get_rate();
		virtual void set_model_file(const std::string& file);

	protected:
		net_type_vp m_net;
		dlib::dnn_trainer<net_type_vp> m_trainer;
	};
}
#endif