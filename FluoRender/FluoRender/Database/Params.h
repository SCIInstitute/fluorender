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
#ifndef _PARAMS_H_
#define _PARAMS_H_

#include <Entry.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace flrd
{
	class Params
	{
	public:
		Params(
			std::vector<std::string>& names,
			std::unordered_map<std::string, size_t>& index,
			std::vector<Entry::ParamTypes>& types) :
			m_names(names),
			m_name_index(index),
			m_types(types)
		{};
		~Params() {};

		size_t size() { return m_names.size(); }

		bool getNameIndex(const std::string& name, size_t& i)
		{
			std::unordered_map<std::string, size_t>::const_iterator it =
				m_name_index.find(name);
			if (it != m_name_index.end())
			{
				i = it->second;
				return true;
			}
			return false;
		}

		std::string getName(size_t i)
		{
			if (i < m_names.size())
				return m_names[i];
			return "";
		}

		Entry::ParamTypes getType(size_t i)
		{
			if (i < m_names.size())
				return m_types[i];
			return Entry::IPT_VOID;
		}

	private:
		std::vector<std::string> m_names;//parameter names
		std::unordered_map<std::string, size_t> m_name_index;//index of names
		std::vector<Entry::ParamTypes> m_types;//parameter types for external program
	};
}

#endif//_PARAMS_H_