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
#include <Fui/TreeModel.h>

using namespace FUI;

TreeModel::TreeModel()
{

}

TreeModel::~TreeModel()
{

}

int TreeModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
	return 0;
}

unsigned int TreeModel::GetColumnCount() const
{
	return 0;
}

wxString TreeModel::GetColumnType(unsigned int col) const
{
	return "";
}

void TreeModel::GetValue(wxVariant &variant,
	const wxDataViewItem &item, unsigned int col) const
{

}

bool TreeModel::SetValue(const wxVariant &variant,
	const wxDataViewItem &item, unsigned int col)
{
	return true;
}

bool TreeModel::IsEnabled(const wxDataViewItem &item,
	unsigned int col) const
{
	return true;
}

wxDataViewItem TreeModel::GetParent(const wxDataViewItem &item) const
{
	return wxDataViewItem(0);
}

bool TreeModel::IsContainer(const wxDataViewItem &item) const
{
	return true;
}

unsigned int TreeModel::GetChildren(const wxDataViewItem &parent,
	wxDataViewItemArray &array) const
{
	return 0;
}
