/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <VolumeBaker.h>
#include <VolumeData.h>
#include <Texture.h>
#include <stdexcept>

using namespace flrd;

VolumeBaker::VolumeBaker() :
	m_raw_input(0),
	m_raw_result(0),
	m_bits(0)
{

}

VolumeBaker::~VolumeBaker()
{
	//if (m_result)
	//	delete m_result;
}

void VolumeBaker::SetInput(const std::shared_ptr<VolumeData>& data)
{
	m_input = data;
}

std::shared_ptr<VolumeData> VolumeBaker::GetInput()
{
	return m_input.lock();
}

std::shared_ptr<VolumeData> VolumeBaker::GetResult()
{
	return m_result;
}

void VolumeBaker::Bake(bool replace)
{
	auto input = m_input.lock();
	if (!input)
		return;
	m_raw_input = input->GetVolume(false);
	if (!m_raw_input)
		return;

	//input size
	auto size = m_raw_input->GetSize();
	m_size = fluo::Vector(
		size[0],
		size[1],
		size[2]);
	m_bits = m_raw_input->GetBitsPerElement();

	//output raw
	m_raw_result = std::make_shared<fluo::RawData>(size, m_raw_input->GetFormat());
	if (!m_raw_result)
		return;
	//pointer to raw data
	void* raw_ptr = m_raw_result->GetDataVoid();

	//transfer function
	unsigned long long index;
	for (int i = 0; i < m_size.intx(); i++)
	for (int j = 0; j < m_size.inty(); j++)
	for (int k = 0; k < m_size.intz(); k++)
	{
		index = (unsigned long long)m_size.get_size_xy() * k +
			(unsigned long long)m_size.intx() * j + i;
		fluo::Vector p(i, j, k);
		p /= m_size;
		double new_value = input->GetTransferedValue(fluo::Point(p));
		if (m_bits == 8)
			((unsigned char*)raw_ptr)[index] = uint8_t(new_value*255.0);
		else if (m_bits == 16)
			((unsigned short*)raw_ptr)[index] = uint16_t(new_value*65535.0);
	}

	auto spc = input->GetSpacing();

	if (replace)
	{
		input->Replace(m_raw_result, true);
	}
	else
	{
		if (!m_result)
		{
			m_result = std::make_shared<VolumeData>();
			std::wstring name, path;
			m_result->Load(m_raw_result, name, path);
		}
		else
			m_result->Replace(m_raw_result, false);
		m_result->SetSpacing(spc);
		m_result->SetBaseSpacing(spc);
	}
}

