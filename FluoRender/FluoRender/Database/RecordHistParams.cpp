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
#include <RecordHistParams.h>
#include <Global.h>
#include <Table.h>
#include <File.h>
#include <cstring>

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
		m_input = std::make_shared<EntryHist>(*input);
	EntryParams* output = rec.m_output->asEntryParams();
	if (output)
		m_output = std::make_shared<EntryParams>(*output);
}

RecordHistParams::~RecordHistParams()
{
}

void RecordHistParams::assign(const std::shared_ptr<RecordHistParams>& rec)
{
	Record::assign(rec);
	EntryHist* input = rec->m_input->asEntryHist();
	if (input)
		m_input = std::make_shared<EntryHist>(*input);
	EntryParams* output = rec->m_output->asEntryParams();
	if (output)
		m_output = std::make_shared<EntryParams>(*output);
}

void RecordHistParams::open(File& file)
{
	if (file.check(Table::TAG_TABLE_ENT_IN))
	{
		if (file.check(Table::TAG_TABLE_ENT_HIST))
		{
			auto ent = std::make_shared<EntryHist>();
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
			auto ent = std::make_shared<EntryParams>();// glbin.get_params("comp_gen"));
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
	return m_params.size();
}

std::vector<float> RecordHistParams::getInputData()
{
	return m_input->getStdData();
}

std::vector<float> RecordHistParams::getOutputData()
{
	return m_output->getStdData();
}

float RecordHistParams::getHistPopl()
{
	auto e = std::dynamic_pointer_cast<EntryHist>(m_input);
	if (!e)
		return 0;
	return static_cast<float>(e->getPopulation());
}

float RecordHistParams::getParamIter()
{
	auto e = std::dynamic_pointer_cast<EntryParams>(m_output);
	if (!e)
		return 0;
	return e->getParam("iter");
}

float RecordHistParams::getParamMxdist()
{
	auto e = std::dynamic_pointer_cast<EntryParams>(m_output);
	if (!e)
		return 0;
	return e->getParam("max_dist");
}

float RecordHistParams::getParamCleanb()
{
	auto e = std::dynamic_pointer_cast<EntryParams>(m_output);
	if (!e)
		return 0;
	return e->getParam("cleanb");
}

float RecordHistParams::getParamCleanIter()
{
	auto e = std::dynamic_pointer_cast<EntryParams>(m_output);
	if (!e)
		return 0;
	return e->getParam("clean_iter");
}
