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
#include <Reshape.h>
#include <cmath>

using namespace flrd;

Params* Reshape::get_params(const std::string& name)
{
	auto it = params_list_.find(name);
	if (it == params_list_.end())
		return 0;
	return &(it->second);
}

EntryParams* Reshape::get_entry_params(const std::string& name, float* val)
{
	auto it = params_list_.find(name);
	if (it == params_list_.end())
		return 0;
	Params* p = &(it->second);
	result_ = new EntryParams();
	std::string str;

	result_->setParams(p);
	for (size_t i = 0; i < p->size(); ++i)
	{
		str = p->getName(i);
		switch (p->getType(i))
		{
		case Entry::IPT_BOOL:
			result_->setParam(str, bool(val[i] > 0.5));
			break;
		case Entry::IPT_CHAR:
			result_->setParam(str, char(std::round(val[i])));
			break;
		case Entry::IPT_UCHAR:
			result_->setParam(str, (unsigned char)(std::round(val[i])));
			break;
		case Entry::IPT_SHORT:
			result_->setParam(str, short(std::round(val[i])));
			break;
		case Entry::IPT_USHORT:
			result_->setParam(str, (unsigned short)(std::round(val[i])));
			break;
		case Entry::IPT_INT:
			result_->setParam(str, int(std::round(val[i])));
			break;
		case Entry::IPT_UINT:
			result_->setParam(str, (unsigned int)(std::round(val[i])));
			break;
		case Entry::IPT_FLOAT:
			result_->setParam(str, val[i]);
			break;
		case Entry::IPT_DOUBLE:
			result_->setParam(str, double(val[i]));
			break;
		}
	}

	return result_;
}

size_t Reshape::get_param_size(const std::string& name)
{
	auto it = params_list_.find(name);
	if (it == params_list_.end())
		return 0;
	Params* p = &(it->second);
	return p->size();
}

void Reshape::clear()
{
	if (result_)
	{
		delete result_;
		result_ = 0;
	}
}
