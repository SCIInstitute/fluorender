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
#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <vector>
#include <algorithm>
#include <limits>

namespace flrd
{
	class File;
	class RecordHistParams;
	class EntryHist;
	class EntryParams;
	class Entry
	{
		public:
			enum EntryTags
			{
				TAG_ENT_SIZE = 1,
				TAG_ENT_MIN,
				TAG_ENT_MAX,
				TAG_ENT_POPL,
				TAG_ENT_DATA
			};

			enum ParamTypes
			{
				IPT_VOID = 0,
				IPT_BOOL,
				IPT_CHAR,
				IPT_UCHAR,
				IPT_SHORT,
				IPT_USHORT,
				IPT_INT,
				IPT_UINT,
				IPT_FLOAT,
				IPT_DOUBLE,
			};

			Entry();
			Entry(const Entry& ent);
			virtual ~Entry();

			virtual EntryHist* asEntryHist() { return 0; }
			virtual const EntryHist* asEntryHist() const { return 0; }
			virtual EntryParams* asEntryParams() { return 0; }
			virtual const EntryParams* asEntryParams() const { return 0; }

			virtual void open(File& file);
			virtual void save(File& file);

			virtual bool getValid() { return !m_data.empty(); }
			virtual std::vector<float>& getStdData()
			{
				return m_data;
			}

			virtual bool compare(Entry* ent)
			{
				if (!ent) return false;
				return std::equal(m_data.begin(), m_data.end(), ent->m_data.begin());
			}

			virtual float distance(Entry* ent)
			{
				float d = std::numeric_limits<float>::max();
				if (!ent)
					return d;
				if (m_data.empty() || ent->m_data.size() != m_data.size())
					return d;
				d = 0;
				for (size_t i = 0; i < m_data.size(); ++i)
				{
					float temp = m_data[i] - ent->m_data[i];
					d += temp * temp;
				}
				return d;
			}

			friend class RecordHistParams;

		protected:
			std::vector<float> m_data;

		private:
	};
}

#endif//_ENTRY_H_