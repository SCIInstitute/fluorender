/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include "RecordHistParams.h"
#include <Table.h>
#include <FileIO/File.h>
#include <cstring>
#include <Global.h>

using namespace flrd;

RecordHistParams::RecordHistParams() :
	Record()
{
}

RecordHistParams::RecordHistParams(const RecordHistParams& rec) :
	Record(rec)
{
	EntryHist* input = rec.m_input->asEntryHist();
	if (input)
		m_input = new EntryHist(*input);
	EntryParams* output = rec.m_output->asEntryParams();
	if (output)
		m_output = new EntryParams(*output);
}

RecordHistParams::~RecordHistParams()
{
}

void RecordHistParams::assign(RecordHistParams* rec)
{
	Record::assign(rec);
	EntryHist* input = rec->m_input->asEntryHist();
	if (input)
		m_input = new EntryHist(*input);
	EntryParams* output = rec->m_output->asEntryParams();
	if (output)
		m_output = new EntryParams(*output);
}

void RecordHistParams::open(File& file)
{
	if (file.check(Table::TAG_TABLE_ENT_IN))
	{
		if (file.check(Table::TAG_TABLE_ENT_HIST))
		{
			EntryHist* ent = new EntryHist();
			if (ent)
			{
				ent->open(file);
				setInput(ent);
			}
		}
	}

	if (file.check(Table::TAG_TABLE_ENT_OUT))
	{
		if (file.check(Table::TAG_TABLE_ENT_PARAMS))
		{
			EntryParams* ent = new EntryParams();// glbin.get_params("comp_gen"));
			if (ent)
			{
				ent->setParams(m_params);
				ent->open(file);
				setOutput(ent);
			}
		}
	}
}

void RecordHistParams::save(File& file)
{
	//type
	file.writeValue(Table::TAG_TABLE_REC_HISTPARAM);
	Record::save(file);
}

size_t RecordHistParams::getInputSize()
{
	return EntryHist::m_bins;
}

size_t RecordHistParams::getOutputSize()
{
	if (m_params)
		return m_params->size();
	return 0;
}

void RecordHistParams::getInputData(float* data)
{
	if (m_input)
		std::memcpy(data, &(m_input->m_data[0]), sizeof(float)*getInputSize());
}

void RecordHistParams::getOutputData(float* data)
{
	if (m_output)
		std::memcpy(data, &(m_output->m_data[0]), sizeof(float)*getOutputSize());
}

void RecordHistParams::getInputData(std::vector<float>& data)
{
	if (m_input)
		data = m_input->getStdData();
}

void RecordHistParams::getOutputData(std::vector<float>& data)
{
	if (m_output)
		data = m_output->getStdData();
}

float RecordHistParams::getHistPopl()
{
	EntryHist* e = dynamic_cast<EntryHist*>(m_input);
	if (!e)
		return 0;
	return e->getPopulation();
}

float RecordHistParams::getParamIter()
{
	EntryParams* e = dynamic_cast<EntryParams*>(m_output);
	if (!e)
		return 0;
	return e->getParam("iter");
}

float RecordHistParams::getParamMxdist()
{
	EntryParams* e = dynamic_cast<EntryParams*>(m_output);
	if (!e)
		return 0;
	return e->getParam("max_dist");
}

float RecordHistParams::getParamCleanb()
{
	EntryParams* e = dynamic_cast<EntryParams*>(m_output);
	if (!e)
		return 0;
	return e->getParam("cleanb");
}

float RecordHistParams::getParamCleanIter()
{
	EntryParams* e = dynamic_cast<EntryParams*>(m_output);
	if (!e)
		return 0;
	return e->getParam("clean_iter");
}
