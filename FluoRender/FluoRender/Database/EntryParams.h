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
#ifndef _ENTRYPARAMS_H_
#define _ENTRYPARAMS_H_

#include <Entry.h>
#include <Params.h>

namespace flrd
{
	class EntryParams : public Entry
	{
	public:
		EntryParams();
		EntryParams(const EntryParams& ent);
		~EntryParams();

		virtual EntryParams* asEntryParams() { return this; }
		virtual const EntryParams* asEntryParams() const { return this; }

		void setParams(Params* params);

		template <typename T>
		void setParam(const std::string& name, T value)
		{
			if (!m_params)
				return;
			size_t i;
			if (m_params->getNameIndex(name, i))
				m_data[i] = float(value);
			m_valid = true;
		}

		float getParam(const std::string& name)
		{
			if (!m_params)
				return 0;
			size_t i;
			if (m_params->getNameIndex(name, i))
				return m_data[i];
			return 0;
		}

		virtual void open(File& file);
		virtual void save(File& file);

		virtual bool getValid() { return m_valid; }

	private:
		bool m_valid;
		Params* m_params;
	};
}

#endif//_ENTRYPARAMS_H_