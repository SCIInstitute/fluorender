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
#include <Global/Global.h>

using namespace FUI;

ListModel::ListModel()
{

}

void ListModel::nodeAdded(FL::Event& event)
{
	if (event.child)
	{
		RowAppended();
	}
}

void ListModel::nodeRemoved(FL::Event& event)
{
	if (event.child)
	{
		FL::Referenced* refd = static_cast<FL::Referenced*>(event.child);
		if (refd)
		{
			FL::Object* obj = dynamic_cast<FL::Object*>(refd);
			size_t index = FL::Global::instance().getIndex(obj);
			RowDeleted(index);
		}
	}
}

int ListModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
	return wxDataViewModel::Compare(item1, item2, column, ascending);
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
	case 2:
		return "string";
	}
	return "string";
}

void ListModel::GetValueByRow(wxVariant &variant,
	unsigned int row, unsigned int col) const
{
	FL::Object* obj = FL::Global::instance().get(row);
	if (!obj)
		return;
	switch (col)
	{
	case 0:
		variant = obj->getName();
		break;
	case 1:
		variant = obj->className();
		break;
	case 2:
		{
			std::wstring path;
			obj->getValue("data path", path);
			variant = path;
		}
		break;
	}
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

void ListModel::setObject(FL::Node* root)
{
	FL::Global::instance().addListObserver(this);
}

