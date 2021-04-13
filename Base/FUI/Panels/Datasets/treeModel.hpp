#ifndef TREE_MODEL_HPP
#define TREE_MODEL_HPP

//#include <wx/dataview.h>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <InterfaceAgent.hpp>
#include <Node.hpp>
#include <NodeVisitor.hpp>

#include <vector>

class AgentFactory;
class DatasetsPanel;

class TreeModel : public QAbstractItemModel, public InterfaceAgent
{
  public:
    TreeModel(DatasetsPanel &panel);

    bool isSameKindAs(const Object* obj) const
    {
      return dynamic_cast<const TreeModel*>(obj) != NULL;
    }
    
    const char* className() const { return "TreeModel"; }
    
    //interface agent functions
    void setObject(fluo::Node* root)
    { 
      InterfaceAgent::setObject(root); 
    }
    
    fluo::Node* getObject()
    { 
      return dynamic_cast<fluo::Node*>(InterfaceAgent::getObject()); 
    }
    
    //operations
    void MoveNode(const std::string &source, fluo::Node* target);
    void UpdateSelections(fluo::NodeSet &nodes);
    
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index,int role) const override;
    QModelIndex index(int row, int column, 
    			const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  private:
    friend class AgentFactory;
    DatasetsPanel &panel_;
    
    void OnSelectionChanged(fluo::Event& event);
    void OnItemAdded(fluo::Event& event);
    void OnItemRemoved(fluo::Event& event);
    void OnDisplayChanged(fluo::Event& event);
    
    fluo::Node *rootItem;
};


#endif
