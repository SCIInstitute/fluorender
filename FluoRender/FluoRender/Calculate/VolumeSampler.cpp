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
	m_nx_in(0),
	m_ny_in(0),
	m_nz_in(0),
	m_fix_size(false),
	m_nx(0),
	m_ny(0),
	m_nz(0),
	m_bits(0),
	m_crop(false),
	m_neg_mask(false),
	m_ox(0),
	m_oy(0),
	m_oz(0),
	m_lx(0),
	m_ly(0),
	m_lz(0),
	m_filter(0),
	m_fx(0),
	m_fy(0),
	m_fz(0),
	m_border(0)
{
}

VolumeSampler::~VolumeSampler()
{
	//if (m_result)
	//	delete m_result;
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

void VolumeSampler::SetSize(int nx, int ny, int nz)
{
	m_nx = nx;
	m_ny = ny;
	m_nz = nz;
}

void VolumeSampler::SetFilter(int type)
{
	m_filter = type;
}

void VolumeSampler::SetFilterSize(int fx, int fy, int fz)
{
	m_fx = fx;
	m_fy = fy;
	m_fz = fz;
}

void VolumeSampler::SetCrop(bool crop)
{
	m_crop = crop;
}

void VolumeSampler::SetNegMask(bool bval)
{
	m_neg_mask = bval;
}

void VolumeSampler::SetClipRotation(const fluo::Quaternion &q)
{
	m_q_cl = q;
}

void VolumeSampler::SetCenter(const fluo::Point &p)
{
	m_center = p;
}

void VolumeSampler::SetTranslate(const fluo::Point &t)
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
	Nrrd* input_nrrd = GetNrrd(input.get(), type);
	if (!input_nrrd)
		return;
	m_raw_input = input_nrrd->data;
	if (!m_raw_input)
		return;

	//input size
	m_nx_in = static_cast<int>(input_nrrd->axis[0].size);
	m_ny_in = static_cast<int>(input_nrrd->axis[1].size);
	m_nz_in = static_cast<int>(input_nrrd->axis[2].size);
	//bits
	switch (input_nrrd->type)
	{
	case nrrdTypeChar:
	case nrrdTypeUChar:
		m_bits = 8;
		break;
	case nrrdTypeShort:
	case nrrdTypeUShort:
		m_bits = 16;
		break;
	case nrrdTypeInt:
	case nrrdTypeUInt:
		m_bits = 32;
		break;
	}

	//use input size if no resizing
	if (m_nx <= 0 || m_ny <= 0 || m_nz <= 0 || m_fix_size)
	{
		m_nx = m_nx_in;
		m_ny = m_ny_in;
		m_nz = m_nz_in;
	}
	//check rotation & translation
	bool rot = !m_q_cl.IsIdentity();
	bool trans = m_trans != fluo::Point();
	fluo::Vector size(m_nx - 0.5, m_ny - 0.5, m_nz - 0.5);
	fluo::Vector size_in(m_nx_in - 0.5, m_ny_in - 0.5, m_nz_in - 0.5);
	//spacing
	double spcx_in, spcy_in, spcz_in;
	input->GetSpacings(spcx_in, spcy_in, spcz_in);
	fluo::Vector spc_in(spcx_in, spcy_in, spcz_in);
	fluo::Vector spc;
	double x, y, z;

	if (m_crop || rot)
	{
		if (!m_fix_size && rot &&
			m_nx && m_ny && m_nz)
		{
			rotate_scale(size_in, spc_in, size, spc);
			m_nx = int(size.x() + 0.5);
			m_ny = int(size.y() + 0.5);
			m_nz = int(size.z() + 0.5);
			m_nx = m_nx < 1 ? 1 : m_nx;
			m_ny = m_ny < 1 ? 1 : m_ny;
			m_nz = m_nz < 1 ? 1 : m_nz;
		}

		//recalculate range
		std::vector<fluo::Plane*> *planes =
			input->GetVR()->get_planes();
		fluo::Plane p[6];
		int np = int(planes->size());

		//get six planes
		for (int i = 0; i < 6; ++i)
		{
			if (i < np)
			{
				p[i] = *((*planes)[i]);
				p[i].Restore();
			}
			else
			{
				switch (i)
				{
				case 0:
					p[i] = fluo::Plane(
						fluo::Point(0.0, 0.0, 0.0)
						, fluo::Vector(1.0, 0.0, 0.0));
					break;
				case 1:
					p[i] = fluo::Plane(
						fluo::Point(1.0, 0.0, 0.0),
						fluo::Vector(-1.0, 0.0, 0.0));
					break;
				case 2:
					p[i] = fluo::Plane(
						fluo::Point(0.0, 0.0, 0.0),
						fluo::Vector(0.0, 1.0, 0.0));
					break;
				case 3:
					p[i] = fluo::Plane(
						fluo::Point(0.0, 1.0, 0.0),
						fluo::Vector(0.0, -1.0, 0.0));
					break;
				case 4:
					p[i] = fluo::Plane(
						fluo::Point(0.0, 0.0, 0.0),
						fluo::Vector(0.0, 0.0, 1.0));
					break;
				case 5:
					p[i] = fluo::Plane(
						fluo::Point(0.0, 0.0, 1.0),
						fluo::Vector(0.0, 0.0, -1.0));
					break;
				}
			}
		}

		m_ox = int(-m_nx * p[0].d() + 0.499);
		m_oy = int(-m_ny * p[2].d() + 0.499);
		m_oz = int(-m_nz * p[4].d() + 0.499);
		m_lx = int(m_nx * p[1].d() + 0.499) - m_ox;
		m_ly = int(m_ny * p[3].d() + 0.499) - m_oy;
		m_lz = int(m_nz * p[5].d() + 0.499) - m_oz;
	}
	else
	{
		m_ox = m_oy = m_oz = 0;
		m_lx = m_nx;
		m_ly = m_ny;
		m_lz = m_nz;
	}
	//normalized translation
	fluo::Point ntrans(m_trans.x() / m_nx, m_trans.y() / m_ny, m_trans.z() / m_nz);
	fluo::Vector ncenter;
	if (m_center == fluo::Point())
		ncenter = fluo::Vector(0.5);
	else
		ncenter = fluo::Vector(m_center.x() / m_nx, m_center.y() / m_ny, m_center.z() / m_nz);
	fluo::Quaternion q_cl = m_q_cl;
	bool neg = m_neg_mask && (type == SDT_Mask || type == SDT_Label);
	if (neg)
	{
		//ntrans = -ntrans;
		//q_cl = -q_cl;
		trans = false;
		rot = false;
	}

	if (spc.x() == 0.0 || spc.y() == 0.0 || spc.z() == 0.0)
		spc = spc_in * fluo::Vector(double(m_nx_in) / double(m_nx),
			double(m_ny_in) / double(m_ny),
			double(m_nz_in) / double(m_nz));

	//output raw
	unsigned long long total_size = (unsigned long long)m_lx*
		(unsigned long long)m_ly*(unsigned long long)m_lz;
	m_raw_result = (void*)(new unsigned char[total_size * (m_bits /8)]);
	if (!m_raw_result)
		return;

	unsigned long long index;
	int i, j, k;
	double value;
	fluo::Vector vec;
	fluo::Vector spcsize, spcsize_in;
	if (rot)
	{
		spcsize_in = spc_in * size_in;
		spcsize = spc * size;
	}
	for (k = 0; k < m_lz; ++k)
	for (j = 0; j < m_ly; ++j)
	for (i = 0; i < m_lx; ++i)
	{
		index = (unsigned long long)m_lx*(unsigned long long)m_ly*
			(unsigned long long)k + (unsigned long long)m_lx*
			(unsigned long long)j + (unsigned long long)i;
		x = (double(m_ox+i) + 0.5) / double(m_nx);
		y = (double(m_oy+j) + 0.5) / double(m_ny);
		z = (double(m_oz+k) + 0.5) / double(m_nz);

		if (rot)
		{
			vec.Set(x, y, z);
			vec -= ncenter;//center
			vec *= spcsize;//scale
			fluo::Quaternion qvec(vec);
			qvec = (-q_cl) * qvec * (q_cl);//rotate
			vec = qvec.GetVector();
			vec /= spcsize_in;//normalize
			vec += ncenter;//translate
			x = vec.x();
			y = vec.y();
			z = vec.z();
		}
		if (trans)
		{
			x += ntrans.x();
			y += ntrans.y();
			z += ntrans.z();
		}

		if (m_bits == 32)
			((unsigned int*)m_raw_result)[index] = SampleInt(x, y, z);
		else
		{
			value = Sample(x, y, z);
			if (m_bits == 8)
				((unsigned char*)m_raw_result)[index] = (unsigned char)(value * 255);
			else if (m_bits == 16)
				((unsigned short*)m_raw_result)[index] = (unsigned short)(value * 65535);
		}
	}

	//write to nrrd
	Nrrd* nrrd_result = nrrdNew();
	if (m_bits == 8)
		nrrdWrap_va(nrrd_result, (uint8_t*)m_raw_result, nrrdTypeUChar,
			3, (size_t)m_lx, (size_t)m_ly, (size_t)m_lz);
	else if (m_bits == 16)
		nrrdWrap_va(nrrd_result, (uint16_t*)m_raw_result, nrrdTypeUShort,
			3, (size_t)m_lx, (size_t)m_ly, (size_t)m_lz);
	else if (m_bits == 32)
		nrrdWrap_va(nrrd_result, (uint32_t*)m_raw_result, nrrdTypeUInt,
			3, (size_t)m_lx, (size_t)m_ly, (size_t)m_lz);

	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoSpacing, spc.x(), spc.y(), spc.z());
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoMax, spc.x()*m_lx,
		spc.y()*m_ly, spc.z()*m_lz);
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoSize, (size_t)m_lx,
		(size_t)m_ly, (size_t)m_lz);

	if (replace)
	{
		switch (type)
		{
		case SDT_Data:
			input->Replace(nrrd_result, true);
			break;
		case SDT_Mask:
			input->LoadMask(nrrd_result);
			break;
		case SDT_Label:
			input->LoadLabel(nrrd_result);
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
				m_result->Load(nrrd_result, name, path);
		}
		else
		{
			if (type == SDT_Data)
				m_result->Replace(nrrd_result, false);
		}
		switch (type)
		{
		case SDT_Data:
			//m_result->Replace(nrrd_result, false);
			break;
		case SDT_Mask:
			m_result->LoadMask(nrrd_result);
			break;
		case SDT_Label:
			m_result->LoadLabel(nrrd_result);
			break;
		}
	}
}

Nrrd* VolumeSampler::GetNrrd(VolumeData* vd, SampDataType type)
{
	if (!vd || !vd->GetTexture())
		return 0;
	flvr::Texture* tex = vd->GetTexture();
	int index = 0;
	switch (type)
	{
	case SDT_Data:
		index = 0;
		break;
	case SDT_Mask:
		index = tex->nmask();
		break;
	case SDT_Label:
		index = tex->nlabel();
		break;
	}
	return tex->get_nrrd(index);
}

void* VolumeSampler::GetRaw(VolumeData* vd, SampDataType type)
{
	Nrrd* nrrd = GetNrrd(vd, type);
	if (nrrd)
		return nrrd->data;
	return 0;
}

double VolumeSampler::Sample(double x, double y, double z)
{
	switch (m_filter)
	{
	case 0:
		return SampleNearestNeighbor(x, y, z);
	case 1:
		return SampleBiLinear(x, y, z);
	case 2:
		return SampleTriLinear(x, y, z);
	case 3:
		return SampleBox(x, y, z);
	}
	return 0.0;
}

unsigned int VolumeSampler::SampleInt(double x, double y, double z)
{
	auto input = m_input.lock();
	if (!input)
		return 0;
	if (!m_raw_input)
		return 0;
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	if (!ijk(i, j, k))
		return 0;
	int nx, ny, nz;
	input->GetResolution(nx, ny, nz);
	unsigned long long index = (unsigned long long)nx*(unsigned long long)ny*
		(unsigned long long)k + (unsigned long long)nx*
		(unsigned long long)j + (unsigned long long)i;
	return ((unsigned int*)m_raw_input)[index];
}

bool VolumeSampler::ijk(int &i, int &j, int &k)
{
	if (i < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			i = 0;
			break;
		case 2:
			i = -1 - i;
			break;
		}
	}
	if (i >= m_nx_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			i = m_nx_in - 1;
			break;
		case 2:
			i = m_nx_in * 2 - i - 1;
		}
	}
	if (j < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			j = 0;
			break;
		case 2:
			j = -1 - j;
			break;
		}
	}
	if (j >= m_ny_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			j = m_ny_in - 1;
			break;
		case 2:
			j = m_ny_in * 2 - j - 1;
		}
	}
	if (k < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			k = 0;
			break;
		case 2:
			k = -1 - k;
			break;
		}
	}
	if (k >= m_nz_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			k = m_nz_in - 1;
			break;
		case 2:
			k = m_nz_in * 2 - k - 1;
		}
	}
	return true;
}

void VolumeSampler::xyz2ijk(double x, double y, double z,
	int &i, int &j, int &k)
{
	i = int(x*m_nx_in);
	j = int(y*m_ny_in);
	k = int(z*m_nz_in);
}

void VolumeSampler::xyz2ijkt(
	double x, double y, double z,
	int &i, int &j, int &k,
	double &tx, double &ty, double &tz)
{
	double id = x * m_nx_in - 0.5;
	double jd = y * m_ny_in - 0.5;
	double kd = z * m_nz_in - 0.5;
	i = id >= 0.0 ? int(id) : int(id) - 1;
	j = jd >= 0.0 ? int(jd) : int(jd) - 1;
	k = kd >= 0.0 ? int(kd) : int(kd) - 1;
	tx = id - i;
	ty = jd - j;
	tz = kd - k;
}

int VolumeSampler::rotate_scale(fluo::Vector &vsize_in, fluo::Vector &vspc_in,
	fluo::Vector &vsize, fluo::Vector &vspc)
{
	fluo::Vector rsf = vsize / vsize_in;//rescale factor
	std::vector<fluo::Quaternion> qs;
	std::vector<fluo::Vector> vs;
	std::vector<fluo::Vector> vs2;
	qs.push_back(fluo::Quaternion(1, 0, 0, 0));
	qs.push_back(fluo::Quaternion(0, 1, 0, 0));
	qs.push_back(fluo::Quaternion(0, 0, 1, 0));
	fluo::Vector vec_in = vsize_in * vspc_in;
	for (auto &q : qs)
	{
		q = (-m_q_cl) * q * (m_q_cl);
		vs.push_back(q.GetVector() * vec_in);
		vs2.push_back(q.GetVector() * vspc_in);
	}
	fluo::Vector rv;
	int i = 0;
	for (auto &v : vs)
		if (i < 3) rv[i++] = v.length();
	i = 0;
	for (auto &v : vs2)
		if (i < 3) vspc[i++] = v.length();
	if (vspc.x() == 0.0 ||
		vspc.y() == 0.0 ||
		vspc.z() == 0.0)
		return 0;//invalid
	//rescale spcs to maintain sample size
	consv_volume(vspc, vspc_in);
	if (m_crop)
	{
		vspc /= rsf;
		vsize = rv / vspc;
	}
	else
	{
		vsize = rv * rsf / vspc;
	}
	return 1;
}

int VolumeSampler::consv_volume(fluo::Vector &vec, fluo::Vector &vec_in)
{
	double vol = vec.volume();
	double vol_in = vec_in.volume();
	if (vol == 0.0 || vol_in == 0.0)
		return 0;
	//conserve volume
	if (std::abs(vol - vol_in) >
		fluo::Epsilon(3) * vec_in[vec_in.max()])
	{
		//diff array
		double dif[9] = {
			vec[0] - vec_in[0], vec[0] - vec_in[1], vec[0] - vec_in[2],
			vec[1] - vec_in[0], vec[1] - vec_in[1], vec[1] - vec_in[2],
			vec[2] - vec_in[0], vec[2] - vec_in[1], vec[2] - vec_in[2] };
		int ind_min;
		double val_min, val;
		for (int i = 0; i < 9; ++i)
		{
			val = std::abs(dif[i]);
			if (i == 0)
			{
				val_min = val;
				ind_min = i;
			}
			else
			{
				if (val < val_min)
				{
					val_min = val;
					ind_min = i;
				}
			}
		}
		int ind = ind_min / 3;
		int ind_in = ind_min % 3;
		//align min dif
		double f = std::sqrt(vol_in*vec[ind]/vol/vec_in[ind_in]);
		vec[ind] = vec_in[ind_in];
		//the remaining 2
		for (int i = 0; i < 3; ++i)
		{
			if (i == ind)
				continue;
			vec[i] *= f;
		}

		return 1;
	}
	return 0;
}

double VolumeSampler::SampleNearestNeighbor(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	if (!ijk(i, j, k))
		return 0.0;
	unsigned long long index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	if (m_bits == 8)
		return double(((unsigned char*)m_raw_input)[index]) / 255.0;
	else if (m_bits == 16)
		return double(((unsigned short*)m_raw_input)[index]) / 65535.0;
	return 0.0;
}

double VolumeSampler::SampleBiLinear(double x, double y, double z)
{
	int i, j, k;
	double tx, ty, tz;
	xyz2ijkt(x, y, z, i, j, k, tx, ty, tz);
	double q[4] = { 0 };
	int count = 0;
	int in, jn;
	unsigned long long index;
	for (int ii = 0; ii < 2; ++ii)
	for (int jj = 0; jj < 2; ++jj)
	{
		in = i + ii;
		jn = j + jj;
		if (ijk(in, jn, k))
		{
			index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
				(unsigned long long)k + (unsigned long long)m_nx_in*
				(unsigned long long)jn + (unsigned long long)in;
			if (m_bits == 8)
				q[count] = double(((unsigned char*)m_raw_input)[index]) / 255.0;
			else if (m_bits == 16)
				q[count] = double(((unsigned short*)m_raw_input)[index]) / 65535.0;
		}
		count++;
	}

	return bilerp(tx, ty,
		q[0], q[1], q[2], q[3]);
}

double VolumeSampler::SampleTriLinear(double x, double y, double z)
{
	int i, j, k;
	double tx, ty, tz;
	xyz2ijkt(x, y, z, i, j, k, tx, ty, tz);
	double q[8] = { 0 };
	int count = 0;
	int in, jn, kn;
	unsigned long long index;
	for (int ii = 0; ii < 2; ++ii)
	for (int jj = 0; jj < 2; ++jj)
	for (int kk = 0; kk < 2; ++kk)
	{
		in = i + ii;
		jn = j + jj;
		kn = k + kk;
		if (ijk(in, jn, kn))
		{
			index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
				(unsigned long long)kn + (unsigned long long)m_nx_in*
				(unsigned long long)jn + (unsigned long long)in;
			if (m_bits == 8)
				q[count] = double(((unsigned char*)m_raw_input)[index]) / 255.0;
			else if (m_bits == 16)
				q[count] = double(((unsigned short*)m_raw_input)[index]) / 65535.0;
		}
		count++;
	}

	return trilerp(tx, ty, tz,
		q[0], q[1], q[2], q[3],
		q[4], q[5], q[6], q[7]);
}

double VolumeSampler::SampleBox(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	double sum = 0.0;
	int count = 0;
	unsigned long long index;
	for (int kk=k-m_fz; kk<=k+m_fz; ++kk)
	for (int jj=j-m_fy; jj<=j+m_fy; ++jj)
	for (int ii=i-m_fx; ii<=i+m_fx; ++ii)
	{
		if (ijk(ii, jj, kk))
		{
			index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
				(unsigned long long)kk + (unsigned long long)m_nx_in*
				(unsigned long long)jj + (unsigned long long)ii;
			if (m_bits == 8)
				sum += double(((unsigned char*)m_raw_input)[index]) / 255.0;
			else if (m_bits == 16)
				sum += double(((unsigned short*)m_raw_input)[index]) / 65535.0;
		}
		count++;
	}
	if (count)
		sum /= count;
	index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	//double test = double(((unsigned char*)(m_vd->data))[index]) / 255.0;
	return sum;
}
