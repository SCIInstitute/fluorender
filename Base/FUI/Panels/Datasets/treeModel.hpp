#ifndef TREE_MODEL_HPP
#define TREE_MODEL_HPP

//#include <wx/dataview.h>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <Panels/Base_Agent/InterfaceAgent.hpp>
#include <Node.hpp>
#include <NodeVisitor.hpp>

#include <vector>

class AgentFactory;
class DatasetsPanel;

class TreeModel : public QAbstractItemModel, public InterfaceAgent
{
  public:
    TreeModel(DatasetsPanel &panel);

		int Compare(const QModelIndex &item1, const QModelIndex &item2,
			unsigned int column, bool ascending) const; 

		unsigned int GetColumnCount() const;
      

		QString GetColumnType(unsigned int col) const;

		void GetValue(QVariant &variant,
			const QModelIndex &item, unsigned int col) const;

		bool SetValue(const QVariant &variant,
			const QModelIndex &item, unsigned int col) ;

		bool IsEnabled(const QModelIndex &item,
			unsigned int col) const;

		bool IsContainer(const QModelIndex &item) const;

		bool HasContainerColumns(const QModelIndex & item) const;

		QModelIndex GetParent(const QModelIndex &item) const;

		unsigned int GetChildren(const QModelIndex &parent,
			std::vector<QModelIndex> &array) const;
			//QModelIndexArray &array) const;

		//ref
		bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const TreeModel*>(obj) != NULL;
		}

		const char* className() const { return "TreeModel"; }

		//interface agent functions
		void setObject(fluo::Node* root)
		{ InterfaceAgent::setObject(root); }
		 fluo::Node* getObject()
		{ return dynamic_cast<fluo::Node*>(InterfaceAgent::getObject()); }

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
