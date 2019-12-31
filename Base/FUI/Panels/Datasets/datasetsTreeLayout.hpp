#ifndef DATASETS_TREE_LAYOUT_HPP
#define DATASETS_TREE_LAYOUT_HPP

#include <QGridLayout>
#include <QTreeView>

class DatasetsTreeLayout : public QGridLayout
{
  Q_OBJECT
  
	public:
      DatasetsTreeLayout()
      {
        this->addWidget(tempTreeview);
      }

	private:
      QTreeView *tempTreeview = new QTreeView();

};

#endif
