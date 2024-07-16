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
#include <TableHistParams.h>
#include <RecordHistParams.h>
#include <algorithm>
#include <DnnTrainer.h>

using namespace flrd;

TableHistParams::TableHistParams() :
	Table(),
	m_hist_popl(0),
	m_param_iter(0),
	m_param_mxdist(0),
	m_param_cleanb(0),
	m_param_clean_iter(0),
	m_dnn(new DnnTrainer())
{
}

TableHistParams::TableHistParams(const TableHistParams& table) :
	Table(table),
	m_hist_popl(table.m_hist_popl),
	m_param_iter(table.m_param_iter),
	m_param_mxdist(table.m_param_mxdist),
	m_param_cleanb(table.m_param_cleanb),
	m_param_clean_iter(table.m_param_clean_iter),
	m_dnn(new DnnTrainer())
{
}

TableHistParams::~TableHistParams()
{
	if (m_dnn)
		delete m_dnn;
}

EntryParams TableHistParams::infer(EntryHist* input)
{
	if (m_dnn && m_dnn->is_valid())
		return dnn(input);
	return nearest_neighbor(input);
}

void TableHistParams::addRecord(Record* rec)
{
	Table::addRecord(rec);
	compute(rec);
	dnn_add(rec);
}

void TableHistParams::open(const std::string& filename)
{
	Table::open(filename);
	compute();
}

void TableHistParams::compute(Record* rec)
{
	computeHistSize(rec);
	computeParamIter(rec);
}

void TableHistParams::computeHistSize(Record* rec)
{
	double sum = 0;
	if (m_data.empty())
	{
		m_hist_popl = 0;
		return;
	}

	if (rec)
	{
		RecordHistParams* r = dynamic_cast<RecordHistParams*>(rec);
		if (r)
		{
			m_hist_popl *= m_data.size() - 1;
			m_hist_popl += r->getHistPopl();
			m_hist_popl /= m_data.size();
		}
		return;
	}

	for (auto i : m_data)
	{
		RecordHistParams* r = dynamic_cast<RecordHistParams*>(i);
		if (r)
			sum += r->getHistPopl();
	}

	m_hist_popl = (float)(sum / m_data.size());
}

void TableHistParams::getParams(Record* rec)
{
	RecordHistParams* r = dynamic_cast<RecordHistParams*>(rec);
	if (r)
	{
		m_param_iter = std::max(m_param_iter, r->getParamIter());
		m_param_mxdist = std::max(m_param_mxdist, r->getParamMxdist());
		m_param_cleanb = std::max(m_param_cleanb, r->getParamCleanb());
		m_param_clean_iter = std::max(m_param_clean_iter, r->getParamCleanIter());
	}
}

void TableHistParams::computeParamIter(Record* rec)
{
	if (m_data.empty())
	{
		m_param_iter = 0;
		return;
	}

	if (rec)
	{
		getParams(rec);
		return;
	}

	for (auto i : m_data)
	{
		getParams(i);
	}
}

void TableHistParams::dnn_add(Record* rec)
{
	if (!m_dnn)
		return;

	RecordHistParams* r = dynamic_cast<RecordHistParams*>(rec);
	if (!r)
		return;

	std::vector<float> in = r->getInput()->getStdData();
	std::vector<float> out = r->getOutput()->getStdData();

	m_dnn->add(&in[0], &out[0]);
}

//models for inference
EntryParams TableHistParams::nearest_neighbor(EntryHist* input)
{
	Record* result = 0;
	float vmin = std::numeric_limits<float>::max();
	for (auto i : m_data)
	{
		float v = i->compare(input);
		if (v <= vmin)
		{
			result = i;
			vmin = v;
		}
	}
	if (result)
		return *dynamic_cast<EntryParams*>(result->getOutput());
	return EntryParams();
}

EntryParams TableHistParams::dnn(EntryHist* input)
{
	EntryParams ep;
	if (!m_dnn)
		return ep;

	std::vector<float> in = input->getStdData();
	float* pout = m_dnn->infer(&in[0]);
	std::vector<float> out(pout, pout + gno_vp_output_size);
	//ep.setParams(glbin.get_params("vol_prop"));
	ep.setParam("gamma3d", out[0]);
	ep.setParam("extract_boundary", out[1]);
	ep.setParam("low_offset", out[2]);
	ep.setParam("low_threshold", out[3]);
	ep.setParam("high_threshold", out[4]);
	ep.setParam("luminance", out[5]);
	ep.setParam("alpha_enable", out[6] > 0.5);
	ep.setParam("alpha", out[7]);
	ep.setParam("shading_enable", out[8] > 0.5);
	ep.setParam("low_shading", out[9]);
	ep.setParam("high_shading", out[10]);
	ep.setParam("shadow_enable", out[11] > 0.5);
	ep.setParam("shadow_intensity", out[12]);
	ep.setParam("sample_rate", out[13]);
	ep.setParam("colormap_enable", out[14] > 0.5);
	ep.setParam("colormap_inv", out[15]);
	ep.setParam("colormap_type", int(std::round(out[16])));
	ep.setParam("colormap_proj", int(std::roundf(out[17])));
	ep.setParam("colormap_low", out[18]);
	ep.setParam("colormap_hi", out[19]);
	ep.setParam("interp_enable", out[0] > 0.5);
	ep.setParam("invert_enable", out[0] > 0.5);
	ep.setParam("mip_enable", out[0] > 0.5);
	ep.setParam("transparent_enable", out[0] > 0.5);
	ep.setParam("denoise_enable", out[0] > 0.5);

	return ep;
}