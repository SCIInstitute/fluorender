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
#include <Fui/TreePanel.h>
#include <Scenegraph/Group.h>
#include <Scenegraph/SearchVisitor.h>

using namespace FUI;

TreeModel::TreeModel(TreePanel &panel):
	InterfaceAgent(),
	panel_(panel)
{

}

void TreeModel::nodeAdded(FL::Event& event)
{
	if (event.parent && event.child)
	{
		wxDataViewItem parent_item = wxDataViewItem(event.parent);
		wxDataViewItem child_item = wxDataViewItem(event.child);
		ItemAdded(parent_item, child_item);
		panel_.m_tree_ctrl->Expand(parent_item);
	}
}

void TreeModel::nodeRemoved(FL::Event& event)
{
	if (event.parent && event.child)
	{
		wxDataViewItem parent_item = wxDataViewItem(event.parent);
		wxDataViewItem child_item = wxDataViewItem(event.child);
		ItemDeleted(parent_item, child_item);
	}
}

int TreeModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
	//return wxDataViewModel::Compare(item1, item2, column, ascending);
	wxVariant var1, var2;
	GetValue(var1, item1, column);
	GetValue(var2, item2, column);
	wxString str1 = var1.GetString();
	wxString str2 = var2.GetString();
	if (ascending)
		return str2.Cmp(str1);
	else
		return str1.Cmp(str2);
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
	//return false;
	if (!item.IsOk())
		return wxDataViewItem(0);
	FL::Node *node = (FL::Node*)item.GetID();
	return node->asGroup();
}

bool TreeModel::HasContainerColumns(const wxDataViewItem & item) const
{
	if (!item.IsOk())
		return wxDataViewItem(0);
	FL::Referenced *refd = (FL::Referenced*)item.GetID();
	if (refd->className() == std::string("Group"))
		return false;
	else
		return true;
}

unsigned int TreeModel::GetChildren(const wxDataViewItem &parent,
	wxDataViewItemArray &array) const
{
	FL::Node *node = (FL::Node*)parent.GetID();
	if (!node)
	{
		FL::Node* root = const_cast<TreeModel*>(this)->getObject();
		array.Add(wxDataViewItem((void*)root));
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

//operations
void TreeModel::MoveNode(const std::string &source, FL::Node* target)
{
	FL::Node* root = getObject();
	if (!root)
		return;
	FL::SearchVisitor visitor;
	visitor.matchName(source);
	root->accept(visitor);
	FL::ObjectList* list = visitor.getResult();
}