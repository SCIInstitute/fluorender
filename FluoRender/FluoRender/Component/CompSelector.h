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
#ifndef FL_CompSelector_h
#define FL_CompSelector_h

#include <Cell.h>
#include <vector>

namespace flvr
{
	class Texture;
}
namespace flrd
{
	class ComponentSelector
	{
	public:
		ComponentSelector();
		~ComponentSelector();

		void SetSelAll(bool value)
		{ m_sel_all = value; }
		bool GetSelAll()
		{ return m_sel_all; }
		void SetId(unsigned long long id, bool empty)
		{
			m_id = id; m_id_empty = empty;
		}
		void SetId(const std::string& str);
		unsigned long long GetId() { return m_id; }
		void SetList(const CelpList& list) { m_list = list; }
		CelpList& GetList() { return m_list; }
		bool GetIdEmpty() { return m_id_empty; }
		void SetUseMin(bool val) { m_use_min = val; }
		void SetUseMax(bool val) { m_use_max = val; }
		bool GetUseMin() { return m_use_min; }
		bool GetUseMax() { return m_use_max; }
		void SetMinNum(unsigned int num) { m_min_num = num; }
		void SetMaxNum(unsigned int num) { m_max_num = num; }
		int GetMinNum() { return m_min_num; }
		int GetMaxNum() { return m_max_num; }

		//high-level functions
		void SelectFullComp();
		void SelectCompsCanvas(const std::vector<unsigned long long>& ids, bool sel_all);
		void SetSelectedCompIds(const std::set<unsigned long long>& ids, int mode);
		void GetSelectedCompIds(std::set<unsigned long long>& ids) { ids = m_sel_ids; }
		int GetSelCompIdsMode() { return m_sel_mode; }

		//select functions
		void CompFull();
		void All();
		void Select(bool all, bool rmask = true);
		void Exclusive();
		void Clear(bool invalidate=true);
		void Delete();
		void DeleteList();
		void SelectList();
		void EraseList();

	private:
		bool m_sel_all;
		unsigned long long m_id;
		bool m_id_empty;
		bool m_use_min;
		bool m_use_max;
		unsigned int m_min_num;
		unsigned int m_max_num;
		std::set<unsigned long long> m_sel_ids;
		int m_sel_mode;//0:single; 1:multiple
		CelpList m_list;

	private:
		bool CompareSize(unsigned int size);
		CelpList* GetListFromAnalyzer(CelpList &list_in, CelpList &list_out);
		void SelectMask(unsigned char* mask,
			unsigned long long idx, unsigned char v, flvr::Texture* tex);
	};

	inline bool ComponentSelector::CompareSize(
		unsigned int size)
	{
		if (m_use_min && m_use_max)
		{
			if (size > m_min_num && size < m_max_num)
				return true;
			else
				return false;
		}
		else if (m_use_min)
		{
			if (size > m_min_num)
				return true;
			else
				return false;
		}
		else if (m_use_max)
		{
			if (size < m_max_num)
				return true;
			else
				return false;
		}
		else
			return false;
	}
}
#endif//FL_CompSelector_h