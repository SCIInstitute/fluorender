/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <ListAgent.hpp>
#include <AgentFactory.hpp>
#include <Global.hpp>
//#include <ListPanel.h>

using namespace fluo;

#pragma message ("replace dummy dialog")
class ListPanel : public wxWindow
{
public:
	ListPanel() {}
	~ListPanel() {}
};

ListAgent::ListAgent(ListPanel &panel) :
	panel_(panel)
{

}

void ListAgent::setupInputs()
{

}

int ListAgent::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
/*	//return wxDataViewModel::Compare(item1, item2, column, ascending);
	wxVariant var1, var2;
	GetValue(var1, item1, column);
	GetValue(var2, item2, column);
	wxString str1, str2;
	switch (column)
	{
	case 0:
	{
		wxDataViewIconText it1, it2;
		it1 << var1;
		it2 << var2;
		str1 = it1.GetText();
		str2 = it2.GetText();
	}
		break;
	default:
		str1 = var1.GetString();
		str2 = var2.GetString();
	}
	return ascending ? str1.CmpNoCase(str2) : str2.CmpNoCase(str1);
*/
	return 0;
}

//model definition
unsigned int ListAgent::GetColumnCount() const
{
	return 3;
}

wxString ListAgent::GetColumnType(unsigned int col) const
{
/*	switch (col)
	{
	case 0:
		return wxDataViewIconTextRenderer::GetDefaultType();
	case 1:
		return "string";
	case 2:
		return "string";
	}
	return "string";
*/
	return wxString();
}

void ListAgent::GetValue(wxVariant &variant,
	const wxDataViewItem &item, unsigned int col) const
{
/*	//if (!item.IsOk())
	//	return;
	Node* node = (Node*)item.GetID();
	if (!node)
		return;
	switch (col)
	{
	case 0:
		variant << wxDataViewIconText(
			wxString(node->getName()),
			glbin.getIconList(true).
			get(node->className()));
		break;
	case 1:
		variant = wxString(node->className());
		break;
	case 2:
	{
		std::wstring path;
		node->getValue(gstDataPath, path);
		variant = wxString(path);
		break;

	}
	}
*/}

bool ListAgent::SetValue(const wxVariant &variant,
	const wxDataViewItem &item, unsigned int col)
{
/*	if (!item.IsOk())
		return false;
	Node* node = (Node*)item.GetID();
	if (!node)
		return false;
	switch (col)
	{
	case 0:
		{
			wxDataViewIconText it;
			it << variant;
			node->setName(it.GetText().ToStdString());
		}
		return true;
	case 1:
		return false;
	case 2:
		return false;
	}
	return false;
*/
	return false;
}


bool ListAgent::IsEnabled(const wxDataViewItem &item,
	unsigned int col) const
{
	return true;
}

bool ListAgent::IsContainer(const wxDataViewItem &item) const
{
	return false;
}

bool ListAgent::HasContainerColumns(const wxDataViewItem & item) const
{
	return true;
}

wxDataViewItem ListAgent::GetParent(const wxDataViewItem &item) const
{
	return wxDataViewItem(0);
}

unsigned int ListAgent::GetChildren(const wxDataViewItem &parent,
	wxDataViewItemArray &array) const
{
/*	Node *node = (Node*)parent.GetID();
	if (!node)
	{
		size_t size = glbin.getObjNumInList();
		for (size_t i = 0; i < size; ++i)
		{
			Object* obj = glbin.getObjInList(i);
			array.Add(wxDataViewItem((void*)obj));
		}
		return size;
	}
	else
		return 0;
*/
	return 0;
}

void ListAgent::setObject(Node* root)
{
	glbin.addListObserver(this);
}

void ListAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
}

void ListAgent::OnItemAdded(Event& event)
{
	if (event.child)
	{
		//wxDataViewItem parent_item = wxDataViewItem(0);
		//wxDataViewItem child_item = wxDataViewItem(event.child);
		//ItemAdded(parent_item, child_item);
	}
}

void ListAgent::OnItemRemoved(Event& event)
{
	if (event.child)
	{
		//wxDataViewItem parent_item = wxDataViewItem(0);
		//wxDataViewItem child_item = wxDataViewItem(event.child);
		//ItemDeleted(parent_item, child_item);
	}
}

