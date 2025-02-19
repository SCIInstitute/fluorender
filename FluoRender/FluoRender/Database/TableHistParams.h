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
#ifndef _TABLEHISTPARAMS_H_
#define _TABLEHISTPARAMS_H_

#include <Table.h>

namespace flrd
{
	class Trainer;
	class TableHistParams : public Table
	{
	public:
		TableHistParams();
		TableHistParams(const TableHistParams& table);
		virtual ~TableHistParams();

		//query funcs
		EntryParams* infer(EntryHist* input);

		virtual void addRecord(Record* rec);

		virtual void open(const std::wstring& filename);

		void addUntrainedRecord();

		float getHistPopl()
		{
			return m_hist_popl;
		}

		float getParamIter()
		{
			return m_param_iter;
		}

		float getParamMxdist()
		{
			return m_param_mxdist;
		}

		float getParamCleanb()
		{
			return m_param_cleanb;
		}

		float getParamCleanIter()
		{
			return m_param_clean_iter;
		}

	private:
		float m_hist_popl;
		float m_param_iter;
		float m_param_mxdist;
		float m_param_cleanb;
		float m_param_clean_iter;

		//trainer
		Trainer* m_dnn;

	private:
		void compute(Record* rec = 0);
		void computeHistSize(Record* rec = 0);
		void getParams(Record* rec);
		void computeParamIter(Record* rec = 0);
		void create_dnn();
		void dnn_add(Record*);

		//models for inference
		EntryParams* nearest_neighbor(EntryHist* input);
		EntryParams* dnn(EntryHist* input);
	};
}

#endif//_TABLEHISTPARAMS_H_