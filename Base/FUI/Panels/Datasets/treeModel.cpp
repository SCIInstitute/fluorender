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

#include <Global.hpp>
#include <SearchVisitor.hpp>

TreeModel::TreeModel(DatasetsPanel &panel):
	InterfaceAgent(),
	panel_(panel)
{
  rootItem = new fluo::Node();
  rootItem->setName("Test Name");
  int row = rowCount(QModelIndex());
  insertRow(row);
  setData(createIndex(row,0), QVariant::fromValue<QString>("Ples"),Qt::EditRole);
  //createIndex(0,0,rootItem);a

  std::cout << rowCount() << std::endl;
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

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
  if(!index.isValid())
    return QModelIndex();

  fluo::Node *childItem = static_cast<fluo::Node*>(index.internalPointer());
  fluo::Node *parentItem = childItem->getParent(parentItem->getId()); //this will be a bug

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
 
  return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
  fluo::Node *parentItem = new fluo::Node();

  if(parent.column() > 0)
    return 0;

  if(!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<fluo::Node*>(parent.internalPointer());

  fluo::Group *group = parentItem->asGroup();

  if(group == nullptr)
  {
    std::cout << "I am null, this is bad." << std::endl;
	return 0;
  }
  else
    return group->getNumChildren(); //this creates a bug
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
  {
    fluo::Group *group = static_cast<fluo::Node*>(parent.internalPointer())->asGroup();
	return group->getNumParents();
  }
  return rootItem->getNumParents();
}
