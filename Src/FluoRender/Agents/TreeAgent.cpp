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
#include <TreeAgent.hpp>
#include <AgentFactory.hpp>
//#include <TreePanel.h>
//#include <DataViewColorRenderer.h>
#include <Global.hpp>
#include <Root.hpp>
#include <Group.hpp>
#include <SearchVisitor.hpp>

using namespace fluo;

TreeAgent::TreeAgent(TreePanel &panel):
	InterfaceAgent(),
	panel_(panel)
{

}

void TreeAgent::setupInputs()
{

}

int TreeAgent::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
	unsigned int column, bool ascending) const
{
	//return wxDataViewModel::Compare(item1, item2, column, ascending);
/*	wxVariant var1, var2;
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

unsigned int TreeAgent::GetColumnCount() const
{
	return 2;
}

wxString TreeAgent::GetColumnType(unsigned int col) const
{
/*	switch (col)
	{
	case 0:
		return wxDataViewIconTextRenderer::GetDefaultType();
	case 1:
		return "string";// DataViewColorRenderer::GetDefaultType();
	}
	return "string";
*/
	return wxString();
}

void TreeAgent::GetValue(wxVariant &variant,
	const wxDataViewItem &item, unsigned int col) const
{
/*	if (!item.IsOk())
		return;
	Node* node = (Node*)item.GetID();
	if (!node)
		return;
	bool display = true;
	node->getValue(gstDisplay, display);
	switch (col)
	{
	case 0:
		variant << wxDataViewIconText(
			wxString(node->getName()),
			glbin.getIconList(display).
			get(node->className()));
		break;
	case 1:
	{
		Color color;
		if (node->getValue(gstColor, color))
		{
			//wxColor wxc((unsigned char)(color.r() * 255 + 0.5),
			//	(unsigned char)(color.g() * 255 + 0.5),
			//	(unsigned char)(color.b() * 255 + 0.5));
			std::ostringstream oss;
			oss << color;
			variant = oss.str();
		}
	}
		break;
	}
*/}

bool TreeAgent::SetValue(const wxVariant &variant,
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
	}
*/	return false;
}

bool TreeAgent::IsEnabled(const wxDataViewItem &item,
	unsigned int col) const
{
	return true;
}

bool TreeAgent::IsContainer(const wxDataViewItem &item) const
{
/*	if (!item.IsOk())
		return wxDataViewItem(0);
	Node *node = (Node*)item.GetID();
	return node->asGroup();
*/
	return false;
}

bool TreeAgent::HasContainerColumns(const wxDataViewItem & item) const
{
/*	if (!item.IsOk())
		return wxDataViewItem(0);
	Referenced *refd = (Referenced*)item.GetID();
	if (refd->className() == std::string("Root"))
		return false;
	else
		return true;
*/
	return false;
}

wxDataViewItem TreeAgent::GetParent(const wxDataViewItem &item) const
{
/*	if (!item.IsOk())
		return wxDataViewItem(0);
	Node *node = (Node*)item.GetID();
	if (node->asRoot())
		return wxDataViewItem(0);
	if (node->asRenderview())
		return wxDataViewItem((void*)(glbin_root));
	if (node->asVolumeGroup())
		return wxDataViewItem((void*)node->getParentRenderview());
	if (node->asMeshGroup())
		return wxDataViewItem((void*)node->getParentRenderview());
	if (node->asVolumeData())
	{
		VolumeGroup* group = node->getParentVolumeGroup();
		if (group)
			return wxDataViewItem((void*)group);
		else
			return wxDataViewItem((void*)node->getParentRenderview());
	}
	if (node->asMeshData())
	{
		MeshGroup* group = node->getParentMeshGroup();
		if (group)
			return wxDataViewItem((void*)group);
		else
			return wxDataViewItem((void*)node->getParentRenderview());
	}
	if (node->asAnnotations())
		return wxDataViewItem((void*)node->getParentRenderview());
	return wxDataViewItem(0);
*/
	return wxDataViewItem();
}

unsigned int TreeAgent::GetChildren(const wxDataViewItem &parent,
	wxDataViewItemArray &array) const
{
/*	Node *node = (Node*)parent.GetID();
	if (!node)
	{
		Node* root = const_cast<TreeAgent*>(this)->getObject();
		array.Add(wxDataViewItem((void*)root));
		return 1;
	}

	Group* group = node->asGroup();
	if (!group)
		return 0;

	unsigned int size = group->getNumChildren();
	for (size_t i = 0; i < group->getNumChildren(); ++i)
	{
		Node* child = group->getChild(i);
		array.Add(wxDataViewItem((void*)child));
	}

	return size;
*/
	return 0;
}

void TreeAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
}

//operations
void TreeAgent::MoveNode(const std::string &source, Node* target)
{
	Node* root = getObject();
	if (!root)
		return;
	SearchVisitor visitor;
	visitor.matchName(source);
	root->accept(visitor);
	ObjectList* list = visitor.getResult();
}

void TreeAgent::UpdateSelections(NodeSet &nodes)
{
	class SelUpdater : public NodeVisitor
	{
	public:
		SelUpdater(NodeSet &nodes) : NodeVisitor(), nodes_(nodes)
		{
			setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
		}

		virtual void reset() {}

		virtual void apply(Node& node)
		{
			bool selected = nodes_.find(&node) != nodes_.end();
			node.setValue("selected", selected);
			traverse(node);
		}

		virtual void apply(Group& group)
		{
			bool selected = nodes_.find(&group) != nodes_.end();
			group.setValue("selected", selected);
			traverse(group);
		}

	private:
		NodeSet nodes_;
	};

	SelUpdater updater(nodes);
	Node* root = getObject();
	if (!root)
		return;
	root->accept(updater);
}

void TreeAgent::OnSelectionChanged(Event& event)
{

}

void TreeAgent::OnItemAdded(Event& event)
{
	if (event.parent && event.child)
	{
		//wxDataViewItem parent_item = wxDataViewItem(event.parent);
		//wxDataViewItem child_item = wxDataViewItem(event.child);
		//ItemAdded(parent_item, child_item);
		//panel_.m_tree_ctrl->Expand(parent_item);
	}
}

void TreeAgent::OnItemRemoved(Event& event)
{
	if (event.parent && event.child)
	{
		//wxDataViewItem parent_item = wxDataViewItem(event.parent);
		//wxDataViewItem child_item = wxDataViewItem(event.child);
		//ItemDeleted(parent_item, child_item);
	}
}

void TreeAgent::OnDisplayChanged(Event& event)
{
	Node* node = dynamic_cast<Node*>(event.origin);
	if (!node)
		return;
	if (node->asRoot())
		return;
	//wxDataViewItem item = wxDataViewItem(event.origin);
	//ItemChanged(item);
	//panel_.m_tree_ctrl->Refresh();
}

