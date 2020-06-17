#ifndef DATASETS_TREE_LAYOUT_HPP
#define DATASETS_TREE_LAYOUT_HPP

#include <QGridLayout>
#include <QTreeView>
#include <QAbstractItemModel>

class DatasetsTreeLayout : public QGridLayout
{
  Q_OBJECT
  
  public:
    DatasetsTreeLayout(QAbstractItemModel* model)
    {
      setModel(model);
      this->addWidget(tempTreeview);
    }

    void setModel(QAbstractItemModel *model)
    {
      tempTreeview->setModel(model);
    }


  private:
    QTreeView *tempTreeview = new QTreeView();
    

};

#endif
