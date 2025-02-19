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
#include <VolPropDnnTrainer.h>

using namespace flrd;

#define VP_TRAIN_STEPS 10000

VolPropDnnTrainer::VolPropDnnTrainer() :
	DnnTrainer(),
	m_trainer(m_net)
{
	m_trainer.set_learning_rate(0.1);
	m_trainer.set_mini_batch_size(16);
}

VolPropDnnTrainer::~VolPropDnnTrainer()
{

}

void VolPropDnnTrainer::add(float* in, float* out)
{
	dlib::matrix<float> tii(gno_vp_input_size, 1);
	dlib::matrix<float> toi(gno_vp_output_size, 1);

	for (int i = 0; i < gno_vp_input_size; ++i)
		tii(i, 0) = in[i];
	for (int i = 0; i < gno_vp_output_size; ++i)
		toi(i, 0) = out[i];

	m_input.push_back(tii);
	m_output.push_back(toi);

	size_t bs = m_trainer.get_mini_batch_size();
	if (m_input.size() >= bs)
		train();
}

void VolPropDnnTrainer::train()
{
	//train all
	size_t bs = m_trainer.get_mini_batch_size();
	if (m_input.size() < bs ||
		m_output.size() < bs)
		return;

	for (int i = 0; i < VP_TRAIN_STEPS; ++i)
		m_trainer.train_one_step(m_input, m_output);

	m_input.clear();
	m_output.clear();

	m_trainer.get_net();

	m_valid = true;
	m_trained_rec_num += bs;
}

float* VolPropDnnTrainer::infer(float* in)
{
	if (!m_valid)
		return 0;

	dlib::matrix<float> tii(gno_vp_input_size, 1);
	for (int i = 0; i < gno_vp_input_size; ++i)
		tii(i, 0) = in[i];

	m_result = m_net(tii);

	return &m_result(0, 0);
}

double VolPropDnnTrainer::get_rate()
{
	if (!m_valid)
		return 1;

	return m_trainer.get_learning_rate();
}

void VolPropDnnTrainer::set_model_file(const std::string& file)
{
	Trainer::set_model_file(file);
	m_trainer.set_synchronization_file(file, std::chrono::minutes(5));
}