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
#include <VolumeSampler.h>
#include <VolumeData.h>
#include <Vector.h>
#include <Plane.h>
#include <Texture.h>
#include <VolumeRenderer.h>
#include <stdexcept>

using namespace flrd;

VolumeSampler::VolumeSampler() :
	m_raw_input(0),
	m_raw_result(0),
	m_size_in(0.0),
	m_fix_size(false),
	m_size_out(0.0),
	m_bits(0),
	m_crop(false),
	m_neg_mask(false),
	m_crop_origin(0.0),
	m_crop_size(0.0),
	m_use_clipbox(false),
	m_filter(0),
	m_filter_size(0.0),
	m_border(0)
{
}

VolumeSampler::~VolumeSampler()
{
}

void VolumeSampler::SetInput(const std::shared_ptr<VolumeData>& data)
{
	m_input = data;
}

std::shared_ptr<VolumeData> VolumeSampler::GetInput()
{
	return m_input.lock();
}

std::shared_ptr<VolumeData> VolumeSampler::GetResult()
{
	return m_result;
}

void VolumeSampler::SetFixSize(bool bval)
{
	m_fix_size = bval;
}

void VolumeSampler::SetSize(const fluo::Vector& size)
{
	m_size_out = size;
}

void VolumeSampler::SetFilter(int type)
{
	m_filter = type;
}

void VolumeSampler::SetFilterSize(const fluo::Vector& size)
{
	m_filter_size = size;
}

void VolumeSampler::SetCrop(bool crop)
{
	m_crop = crop;
}

void VolumeSampler::SetNegMask(bool bval)
{
	m_neg_mask = bval;
}

void VolumeSampler::SetUseClipbox(bool bval)
{
	m_use_clipbox = bval;
}

void VolumeSampler::SetRotation(const fluo::Quaternion &q)
{
	m_q = q;
}

void VolumeSampler::SetCenter(const fluo::Point &p)
{
	m_center = p;
}

void VolumeSampler::SetTranslate(const fluo::Vector &t)
{
	m_trans = t;
}

void VolumeSampler::Resize(SampDataType type, bool replace)
{
	if (type == SDT_All)
	{
		Resize(SDT_Data, replace);
		Resize(SDT_Mask, replace);
		Resize(SDT_Label, replace);
		return;
	}

	auto input = m_input.lock();
	if (!input)
		return;
	m_raw_input = input->GetVolume(false);
	if (!m_raw_input)
		return;

	//input size
	auto raw_size = m_raw_input->GetSize();
	m_size_in = fluo::Vector(
		raw_size[0],
		raw_size[1],
		raw_size[2]);
	//bits
	m_bits = m_raw_input->GetBitsPerElement();

	//use input size if no resizing
	if (m_size_out.any_le_zero() || m_fix_size)
		m_size_out = m_size_in;
	//check rotation & translation
	auto& cb = input->GetClippingBox();
	fluo::Quaternion q_rot;
	if (m_use_clipbox)
		q_rot = cb.GetRotation();
	else
		q_rot = m_q;
	bool rot = !q_rot.IsIdentity();
	bool trans = m_trans != fluo::Vector();
	fluo::Vector size = m_size_out - fluo::Vector(0.5);
	fluo::Vector size_in = m_size_in - fluo::Vector(0.5);
	//spacing
	auto spc_in = input->GetSpacing();
	fluo::Vector spc;

	if (m_crop || rot)
	{
		if (!m_fix_size && rot &&
			m_size_out.all_non_zero())
		{
			size = cb.GetPlaneSizeIndex();
			m_size_out = size;
			m_size_out.normalize_at_least_one();
			size -= fluo::Vector(0.5);
			spc = cb.GetPlaneSizeWorld() / cb.GetPlaneSizeIndex();
		}

		//recalculate range
		m_crop_origin = cb.GetClipsUnit().Min() * m_size_out;
		m_crop_size = cb.GetClipSizeIndex();
	}
	else
	{
		m_crop_origin = fluo::Point(0.0);
		m_crop_size = m_size_out;
	}
	//normalized translation
	fluo::Point ntrans = fluo::Point (m_trans / m_size_out);
	fluo::Vector ncenter;
	if (m_center == fluo::Point())
		ncenter = fluo::Vector(0.5);
	else
		ncenter = fluo::Vector(m_center) / m_size_out;
	bool neg = m_neg_mask && (type == SDT_Mask || type == SDT_Label);
	if (neg)
	{
		//ntrans = -ntrans;
		//q_cl = -q_cl;
		trans = false;
		rot = false;
	}

	if (spc.is_zero())
		spc = spc_in * m_size_in / m_size_out;

	//output raw
	int lx, ly, lz;
	lx = m_crop_size.intx();
	ly = m_crop_size.inty();
	lz = m_crop_size.intz();
	fluo::RawData::Size3 size_out = { (size_t)lx, (size_t)ly, (size_t)lz };
	m_raw_result = std::make_shared<fluo::RawData>(size_out, m_raw_input->GetFormat());
	if (!m_raw_result)
		return;
	//pointer to raw data
	void* raw_ptr = m_raw_result->GetDataVoid();

	unsigned long long index;
	int i, j, k;
	double value;
	fluo::Point xyz;
	fluo::Vector vec;
	fluo::Vector spcsize, spcsize_in;
	if (rot)
	{
		spcsize_in = spc_in * size_in;
		spcsize = spc * size;
	}
	for (k = 0; k < lz; ++k)
	for (j = 0; j < ly; ++j)
	for (i = 0; i < lx; ++i)
	{
		index = (unsigned long long)lx*(unsigned long long)ly*
			(unsigned long long)k + (unsigned long long)lx*
			(unsigned long long)j + (unsigned long long)i;
		xyz = fluo::Point(fluo::Vector(m_crop_origin + fluo::Vector(i, j, k) + fluo::Vector(0.5)) / m_size_out);

		if (rot)
		{
			vec = fluo::Vector(xyz);
			vec -= ncenter;//center
			vec *= spcsize;//scale
			fluo::Quaternion qvec(vec);
			qvec = (-q_rot) * qvec * (q_rot);//rotate
			vec = qvec.GetVector();
			vec /= spcsize_in;//normalize
			vec += ncenter;//translate
			xyz = fluo::Point(vec);
		}
		if (trans)
		{
			xyz += ntrans;
		}

		if (m_bits == 32)
			((unsigned int*)raw_ptr)[index] = SampleInt(xyz);
		else
		{
			value = Sample(xyz);
			if (m_bits == 8)
				((unsigned char*)raw_ptr)[index] = (unsigned char)(value * 255);
			else if (m_bits == 16)
				((unsigned short*)raw_ptr)[index] = (unsigned short)(value * 65535);
		}
	}

	if (replace)
	{
		switch (type)
		{
		case SDT_Data:
			input->Replace(m_raw_result, true);
			break;
		case SDT_Mask:
			input->LoadMask(m_raw_result);
			break;
		case SDT_Label:
			input->LoadLabel(m_raw_result);
			break;
		}
	}
	else
	{
		//create m_result
		if (!m_result)
		{
			m_result = std::make_shared<VolumeData>();
			std::wstring name, path;
			if (type == SDT_Data)
				m_result->Load(m_raw_result, name, path);
		}
		else
		{
			if (type == SDT_Data)
				m_result->Replace(m_raw_result, false);
		}
		switch (type)
		{
		case SDT_Data:
			//m_result->Replace(m_raw_result, false);
			break;
		case SDT_Mask:
			m_result->LoadMask(m_raw_result);
			break;
		case SDT_Label:
			m_result->LoadLabel(m_raw_result);
			break;
		}
	}
}

double VolumeSampler::Sample(const fluo::Point& coord)
{
	switch (m_filter)
	{
	case 0:
		return SampleNearestNeighbor(coord);
	case 1:
		return SampleBiLinear(coord);
	case 2:
		return SampleTriLinear(coord);
	case 3:
		return SampleBox(coord);
	}
	return 0.0;
}

unsigned int VolumeSampler::SampleInt(const fluo::Point& coord)
{
	auto input = m_input.lock();
	if (!input)
		return 0;
	if (!m_raw_input)
		return 0;
	//pointer to raw data
	void* raw_ptr = m_raw_input->GetDataVoid();

	fluo::Point ijk = xyz2ijk(coord);
	if (!normalize_ijk(ijk))
		return 0;
	fluo::Vector size = input->GetResolution();
	unsigned long long index = (unsigned long long)size.intx()*(unsigned long long)size.inty()*
		(unsigned long long)ijk.intz() + (unsigned long long)size.intx() *
		(unsigned long long)ijk.inty() + (unsigned long long)ijk.intx();
	return ((unsigned int*)raw_ptr)[index];
}

bool VolumeSampler::normalize_ijk(fluo::Point& ijk)
{
	if (ijk.intx() < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			ijk.x(0);
			break;
		case 2:
			ijk.x(-1 - ijk.intx());
			break;
		}
	}
	if (ijk.intx() >= m_size_in.intx())
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			ijk.x(m_size_in.intx() - 1);
			break;
		case 2:
			ijk.x(m_size_in.intx() * 2 - ijk.intx() - 1);
		}
	}
	if (ijk.inty() < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			ijk.y(0);
			break;
		case 2:
			ijk.y(-1 - ijk.inty());
			break;
		}
	}
	if (ijk.inty() >= m_size_in.inty())
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			ijk.y(m_size_in.inty() - 1);
			break;
		case 2:
			ijk.y(m_size_in.inty() * 2 - ijk.inty() - 1);
		}
	}
	if (ijk.intz() < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			ijk.z(0);
			break;
		case 2:
			ijk.z(-1 - ijk.intz());
			break;
		}
	}
	if (ijk.intz() >= m_size_in.intz())
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			ijk.z(m_size_in.intz() - 1);
			break;
		case 2:
			ijk.z(m_size_in.intz() * 2 - ijk.intz() - 1);
		}
	}
	return true;
}

fluo::Point VolumeSampler::xyz2ijk(const fluo::Point& coord)
{
	return fluo::Point(fluo::Vector(coord) * m_size_in);
}

std::pair<fluo::Point, fluo::Vector> VolumeSampler::xyz2ijkt(const fluo::Point& coord)
{
	fluo::Vector d = fluo::Vector(coord) * m_size_in - fluo::Vector(0.5);
	fluo::Point ijk;
	ijk.x(d.x() >= 0.0 ? d.intx() : d.intx() - 1);
	ijk.y(d.y() >= 0.0 ? d.inty() : d.inty() - 1);
	ijk.z(d.z() >= 0.0 ? d.intz() : d.intz() - 1);
	fluo::Vector t = d - ijk;
	return { ijk, t };
}

double VolumeSampler::SampleNearestNeighbor(const fluo::Point& coord)
{
	auto ijk = xyz2ijk(coord);
	if (!normalize_ijk(ijk))
		return 0.0;
	//pointer to raw data
	void* raw_ptr = m_raw_input->GetDataVoid();
	unsigned long long index = (unsigned long long)m_size_in.intx()*(unsigned long long)m_size_in.inty()*
		(unsigned long long)ijk.intz() + (unsigned long long)m_size_in.intx() *
		(unsigned long long)ijk.inty() + (unsigned long long)ijk.intx();
	if (m_bits == 8)
		return double(((unsigned char*)raw_ptr)[index]) / 255.0;
	else if (m_bits == 16)
		return double(((unsigned short*)raw_ptr)[index]) / 65535.0;
	return 0.0;
}

double VolumeSampler::SampleBiLinear(const fluo::Point& coord)
{
	auto [ijk, t] = xyz2ijkt(coord);
	double q[4] = { 0 };
	int count = 0;
	unsigned long long index;
	//pointer to raw data
	void* raw_ptr = m_raw_input->GetDataVoid();
	for (int ii = 0; ii < 2; ++ii)
	for (int jj = 0; jj < 2; ++jj)
	{
		fluo::Point nijk = ijk + fluo::Vector(ii, jj, 0);

		if (normalize_ijk(nijk))
		{
			index = (unsigned long long)m_size_in.intx()*(unsigned long long)m_size_in.inty()*
				(unsigned long long)nijk.intz() + (unsigned long long)m_size_in.intx() *
				(unsigned long long)nijk.inty() + (unsigned long long)nijk.intx();
			if (m_bits == 8)
				q[count] = double(((unsigned char*)raw_ptr)[index]) / 255.0;
			else if (m_bits == 16)
				q[count] = double(((unsigned short*)raw_ptr)[index]) / 65535.0;
		}
		count++;
	}

	return bilerp(t.x(), t.y(),
		q[0], q[1], q[2], q[3]);
}

double VolumeSampler::SampleTriLinear(const fluo::Point& coord)
{
	auto [ijk, t] = xyz2ijkt(coord);
	double q[8] = { 0 };
	int count = 0;
	unsigned long long index;
	//pointer to raw data
	void* raw_ptr = m_raw_input->GetDataVoid();
	for (int ii = 0; ii < 2; ++ii)
	for (int jj = 0; jj < 2; ++jj)
	for (int kk = 0; kk < 2; ++kk)
	{
		fluo::Point nijk = ijk + fluo::Vector(ii, jj, kk);
		if (normalize_ijk(nijk))
		{
			index = (unsigned long long)m_size_in.intx()*(unsigned long long)m_size_in.inty()*
				(unsigned long long)nijk.intz() + (unsigned long long)m_size_in.intx() *
				(unsigned long long)nijk.inty() + (unsigned long long)nijk.intx();
			if (m_bits == 8)
				q[count] = double(((unsigned char*)raw_ptr)[index]) / 255.0;
			else if (m_bits == 16)
				q[count] = double(((unsigned short*)raw_ptr)[index]) / 65535.0;
		}
		count++;
	}

	return trilerp(t.x(), t.y(), t.z(),
		q[0], q[1], q[2], q[3],
		q[4], q[5], q[6], q[7]);
}

double VolumeSampler::SampleBox(const fluo::Point& coord)
{
	auto ijk = xyz2ijk(coord);
	double sum = 0.0;
	int count = 0;
	unsigned long long index;
	//pointer to raw data
	void* raw_ptr = m_raw_input->GetDataVoid();
	for (int kk = ijk.intz() - m_filter_size.intz(); kk <= ijk.intz() + m_filter_size.intz(); ++kk)
	for (int jj = ijk.inty() - m_filter_size.inty(); jj <= ijk.inty() + m_filter_size.inty(); ++jj)
	for (int ii = ijk.intx() - m_filter_size.intx(); ii <= ijk.intx() + m_filter_size.intx(); ++ii)
	{
		fluo::Point iijjkk(ii, jj, kk);
		if (normalize_ijk(iijjkk))
		{
			index = (unsigned long long)m_size_in.intx()*(unsigned long long)m_size_in.inty()*
				(unsigned long long)iijjkk.intz() + (unsigned long long)m_size_in.intx() *
				(unsigned long long)iijjkk.inty() + (unsigned long long)iijjkk.intx();
			if (m_bits == 8)
				sum += double(((unsigned char*)raw_ptr)[index]) / 255.0;
			else if (m_bits == 16)
				sum += double(((unsigned short*)raw_ptr)[index]) / 65535.0;
		}
		count++;
	}
	if (count)
		sum /= count;
	index = (unsigned long long)m_size_in.intx()*(unsigned long long)m_size_in.inty()*
		(unsigned long long)ijk.intz() + (unsigned long long)m_size_in.intx() *
		(unsigned long long)ijk.inty() + (unsigned long long)ijk.intx();
	//double test = double(((unsigned char*)(m_vd->data))[index]) / 255.0;
	return sum;
}
