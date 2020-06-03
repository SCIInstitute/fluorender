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

//#include <Fui/TreePanel.h>
//#include <Fui/DataViewColorRenderer.h>

#include <Panels/Datasets/datasetsPanel.hpp>

#include "selUpdater.hpp"
#include "treeModel.hpp"

#include <Global/Global.hpp>
#include <SearchVisitor.hpp>

TreeModel::TreeModel(DatasetsPanel &panel):
	InterfaceAgent(),
	panel_(panel)
{

}

int TreeModel::Compare(const QModelIndex &item1, const QModelIndex &item2,
	unsigned int column, bool ascending) const
{
	//return wxDataViewModel::Compare(item1, item2, column, ascending);
	QVariant var1, var2;
	GetValue(var1, item1, column);
	GetValue(var2, item2, column);
	QString str1;
    QString str2;

	switch (column)
	{
	case 0:
	{
		str1 = var1.toString();
		str2 = var2.toString();
	}
	break;
	default:
		str1 = var1.toString();
		str2 = var2.toString();
	}
	return ascending ? str1.compare(str2,Qt::CaseInsensitive) : str2.compare(str1,Qt::CaseInsensitive);
}

unsigned int TreeModel::GetColumnCount() const
{
	return 2;
}

QString TreeModel::GetColumnType(unsigned int col) const
{
	switch (col)
	{
	case 0:
        return "something"; //not sure what this is supposed to return to be honest.
		//return Qt::DisplayRole;
	case 1:
		return "string";// DataViewColorRenderer::GetDefaultType();
	}
	return "string";
}

void TreeModel::GetValue(QVariant &variant,
	const QModelIndex &item, unsigned int col) const
{
	if (!item.isValid())
		return;
	fluo::Node* node = (fluo::Node*)item.internalId(); //this or internalPointer?
	if (!node)
		return;
	bool display = true;
	node->getValue("display", display);
	switch (col)
	{
	case 0:
        variant = QString(node->getName());
        /*
		variant << wxDataViewIconText(
			QString(node->getName()),
			fluo::Global::instance().getIconList(display).
			get(node->className()));
         
         the original constructor took a string and an icon. However we will need 
         to figure out a better way to call an icon since Qt handles them better.
        */
		break;
	case 1:
	{
		FLTYPE::Color color;
		if (node->getValue("color", color))
		{
			//wxColor wxc((unsigned char)(color.r() * 255 + 0.5),
			//	(unsigned char)(color.g() * 255 + 0.5),
			//	(unsigned char)(color.b() * 255 + 0.5));
			std::ostringstream oss;
			oss << color;
			variant = QString::fromStdString(oss.str());
		}
	}
		break;
	}
}

bool TreeModel::SetValue(const QVariant &variant,
	const QModelIndex &item, unsigned int col)
{
	if (!item.isValid())
		return false;
	fluo::Node* node = (fluo::Node*)item.internalId();
	if (!node)
		return false;
	switch (col)
	{
	case 0:
		{
		  node->setName(variant.toString().toStdString()); // might need to specify std::string
		}
		return true;
	case 1:
		return false;
	}
	return false;
}

bool TreeModel::IsEnabled(const QModelIndex &item,
	unsigned int col) const
{
	return true;
}

bool TreeModel::IsContainer(const QModelIndex &item) const
{
	//return false;
	if (!item.isValid())
	  return false; //previously was QModelIndex();
	fluo::Node *node = (fluo::Node*)item.internalId();
	return node->asGroup();
}

bool TreeModel::HasContainerColumns(const QModelIndex & item) const
{
	if (!item.isValid())
	  return false; //previously was QModelIndex();
	fluo::Referenced *refd = (fluo::Referenced*)item.internalId();
	if (refd->className() == std::string("Root"))
		return false;
	else
		return true;
}

QModelIndex TreeModel::GetParent(const QModelIndex &item) const
{
	if (!item.isValid())
		return QModelIndex();
	fluo::Node *node = (fluo::Node*)item.internalId();
	if (node->getNumParents())
		return createIndex(item.row(),item.column(),node->getParent(0)); //return QModelIndex((void*)node->getParent(0));
	else
		return QModelIndex();
}

unsigned int TreeModel::GetChildren(const QModelIndex &parent,
	std::vector<QModelIndex> &array) const
{
	fluo::Node *node = (fluo::Node*)parent.internalId();
	if (!node)
	{
		fluo::Node* root = const_cast<TreeModel*>(this)->getObject();
		array.push_back(createIndex(0,0,root));
		return 1;
	}

	fluo::Group* group = node->asGroup();
	if (!group)
		return 0;

	unsigned int size = group->getNumChildren();
	for (size_t i = 0; i < group->getNumChildren(); ++i)
	{
		fluo::Node* child = group->getChild(i);
		array.push_back(createIndex(0,i,child)); //this is most definitely going to create a bug. Need to figure out
                                                 //how to safely do this. I'm assuming a vector is a bad choice. 
	}

	return size;
}

//operations
void TreeModel::MoveNode(const std::string &source, fluo::Node* target)
{
	fluo::Node* root = getObject();
	if (!root)
		return;
	fluo::SearchVisitor visitor;
	visitor.matchName(source);
	root->accept(visitor);
	fluo::ObjectList* list = visitor.getResult();
}

void TreeModel::UpdateSelections(fluo::NodeSet &nodes)
{
	SelUpdater updater(nodes);
	fluo::Node* root = getObject();
	if (!root)
		return;
	root->accept(updater);
}

void TreeModel::OnSelectionChanged(fluo::Event& event)
{

}

void TreeModel::OnItemAdded(fluo::Event& event)
{
	if (event.parent && event.child)
	{
		QModelIndex parent_item = createIndex(0,0,event.parent);
		QModelIndex child_item = createIndex(0,1,event.child);
		//ItemAdded(parent_item, child_item);
		//panel_.m_tree_ctrl->Expand(parent_item);
	}
}

void TreeModel::OnItemRemoved(fluo::Event& event)
{
	if (event.parent && event.child)
	{
		QModelIndex parent_item = createIndex(0,0,event.parent);
		QModelIndex child_item = createIndex(0,1,event.child);
		//ItemDeleted(parent_item, child_item);
	}
}

void TreeModel::OnDisplayChanged(fluo::Event& event)
{
	fluo::Node* node = dynamic_cast<fluo::Node*>(event.origin);
	if (node)
	{
		QModelIndex item = createIndex(0,0,event.origin); //this is going to create a bug
		//ItemChanged(item);
	}
	//panel_.m_tree_ctrl->Refresh();
} 

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
  if(!index.isValid())
    return QVariant();

  if(role != Qt::DisplayRole)
    return QVariant();

  fluo::Node *node = static_cast<fluo::Node*>(index.internalPointer());
}

QModelIndex TreeMode::parent(const QModelIndex &index) const
{
  if(!index.isValid())
    return QModelIndex();

  fluo::Node *childItem = static_cast<fluo::Node*>(index.internalPointer());
  fluo::Node *parentItem = childItem->getParent();

  if(parentItem == rootItem)
    return QModelIndex();

  return createIndex(0,0,parentItem);
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if(!hasIndex(row,column,parent))
    return QModelIndex();

  fluo::Node *parentItem;

  if(!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<fluo::Node*>(parent.internalPointer());

  fluo::Group *group = parentItem->asGroup();
  fluo::Node *child = group->getChild(row);

  if(child)
    return createIndex(row,column,child);
 
  return QModelInde();
}