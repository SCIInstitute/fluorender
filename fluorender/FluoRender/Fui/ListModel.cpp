/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include <Fui/ListModel.h>

using namespace FUI;

ListModel::ListModel()
{

}

//observer functions
void ListModel::objectDeleted(void* ptr)
{
	FL::Referenced* refd = static_cast<FL::Referenced*>(ptr);

	//remove observee
	removeObservee(refd);
}

void ListModel::objectChanging(void* ptr, void* orig_node, const std::string &exp)
{
	//before change
}

void ListModel::objectChanged(void* ptr, void* orig_node, const std::string &exp)
{
}

//model definition
unsigned int ListModel::GetColumnCount() const
{
	return 3;
}

wxString ListModel::GetColumnType(unsigned int col) const
{
	switch (col)
	{
	case 0:
		return "string";
	case 1:
		return "string";
	}
	return "string";
}

void ListModel::GetValueByRow(wxVariant &variant,
	unsigned int row, unsigned int col) const
{

}

bool ListModel::GetAttrByRow(unsigned int row, unsigned int col,
	wxDataViewItemAttr &attr) const
{
	return true;
}

bool ListModel::SetValueByRow(const wxVariant &variant,
	unsigned int row, unsigned int col)
{
	return true;
}