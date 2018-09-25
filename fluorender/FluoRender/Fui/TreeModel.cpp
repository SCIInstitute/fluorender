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
#include <Scenegraph/Group.h>

using namespace FUI;

TreeModel::TreeModel():
	m_root(0)
{

}

//observer functions
void TreeModel::objectDeleted(void* ptr)
{
	FL::Referenced* refd = static_cast<FL::Referenced*>(ptr);

	//remove observee
	removeObservee(refd);
}

void TreeModel::objectChanging(void* ptr, void* orig_node, const std::string &exp)
{
	//before change
}

void TreeModel::objectChanged(void* ptr, void* orig_node, const std::string &exp)
{
}

void TreeModel::nodeAdded(void* ptr, void* parent, void* child)
{
	if (parent && child)
	{
		wxDataViewItem parent_item = wxDataViewItem(parent);
		wxDataViewItem child_item = wxDataViewItem(child);
		ItemAdded(parent_item, child_item);
	}
}

void TreeModel::nodeRemoved(void* ptr, void* parent, void* child)
{
	if (parent && child)
	{
		wxDataViewItem parent_item = wxDataViewItem(parent);
		wxDataViewItem child_item = wxDataViewItem(child);
		ItemDeleted(parent_item, child_item);
	}
}

int TreeModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
	return wxDataViewModel::Compare(item1, item2, column, ascending);
}

unsigned int TreeModel::GetColumnCount() const
{
	return 2;
}

wxString TreeModel::GetColumnType(unsigned int col) const
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

void TreeModel::GetValue(wxVariant &variant,
	const wxDataViewItem &item, unsigned int col) const
{
	//if (!item.IsOk())
	//	return;
	FL::Node* node = (FL::Node*)item.GetID();
	switch (col)
	{
	case 0:
		variant = wxString(node->getName());
		break;
	case 1:
		variant = wxString(node->className());
		break;
	}
}

bool TreeModel::SetValue(const wxVariant &variant,
	const wxDataViewItem &item, unsigned int col)
{
	if (!item.IsOk())
		return false;
	FL::Node* node = (FL::Node*)item.GetID();
	switch (col)
	{
	case 0:
		node->setName(variant.GetString().ToStdString());
		return true;
	case 1:
		return false;
	}
	return false;
}

bool TreeModel::IsEnabled(const wxDataViewItem &item,
	unsigned int col) const
{
	return true;
}

wxDataViewItem TreeModel::GetParent(const wxDataViewItem &item) const
{
	if (!item.IsOk())
		return wxDataViewItem(0);
	FL::Node *node = (FL::Node*)item.GetID();
	if (node->getNumParents())
		return wxDataViewItem((void*)node->getParent(0));
	else
		return wxDataViewItem(0);
}

bool TreeModel::IsContainer(const wxDataViewItem &item) const
{
	return false;
	//if (!item.IsOk())
	//	return wxDataViewItem(0);
	//FL::Node *node = (FL::Node*)item.GetID();
	//return node->asGroup();
}

unsigned int TreeModel::GetChildren(const wxDataViewItem &parent,
	wxDataViewItemArray &array) const
{
	FL::Node *node = (FL::Node*)parent.GetID();
	if (!node)
	{
		array.Add(wxDataViewItem((void*)m_root));
		return 1;
	}

	FL::Group* group = node->asGroup();
	if (!group)
		return 0;

	unsigned int size = group->getNumChildren();
	for (size_t i = 0; i < group->getNumChildren(); ++i)
	{
		FL::Node* child = group->getChild(i);
		array.Add(wxDataViewItem((void*)child));
	}

	return size;
}

