#ifndef WORKSPACE_TREE_LAYOUT_HPP
#define WORKSPACE_TREE_LAYOUT_HPP

#include <QGridLayout>
#include <QTreeView>

class WorkspaceTreeLayout : public QGridLayout
{
  Q_OBJECT

  public:
    WorkspaceTreeLayout()
    {
      this->addWidget(tempTreeview);
    }
  private:

    QTreeView *tempTreeview = new QTreeView();
};

#endif
