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
#include <DnnTrainer.h>

using namespace flrd;

DnnTrainer::DnnTrainer() :
	Trainer(),
	m_trainer(m_net)
{
	m_trainer.set_learning_rate(0.1);
}

DnnTrainer::~DnnTrainer()
{

}

void DnnTrainer::add(float* ii, float* oi)
{

}

void DnnTrainer::train()
{
	//train all
	size_t bs = m_trainer.get_mini_batch_size();
	if (m_input.size() < bs ||
		m_output.size() < bs)
		return;

	while (m_trainer.get_learning_rate() >= 1e-2)
		m_trainer.train_one_step(m_input, m_output);

	m_input.clear();
	m_output.clear();
}

float* DnnTrainer::infer()
{
	//m_trainer->get_net();

	//std::vector<float> ii = input->getStdData();
	//auto output = m_net(ii);
	return 0;
}