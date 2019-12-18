#ifndef WORKSPACE_LAYOUT_HPP
#define WORKSPACE_LAYOUT_HPP

#include <QGridLayout>
#include <QToolButton>
#include <QTreeView>

#include <CustomWidgets/fluoLine.hpp>

class WorkspaceLayout : public QGridLayout
{
  Q_OBJECT

  public:
    WorkspaceLayout(){}

  private:

    QToolButton *toggleToolbutton            = new QToolButton();
    QToolButton *addVolumeToolbutton         = new QToolButton();
    QToolButton *addMeshToolbutton           = new QToolButton();
    QToolButton *deleteCurrentToolbutton     = new QToolButton();
    QToolButton *highlightToolbutton         = new QToolButton();
    QToolButton *diffuseHighlightToolbutton  = new QToolButton();
    QToolButton *resetHighlightToolbutton    = new QToolButton();
    QToolButton *eraseHighlightToolbutton    = new QToolButton();
    QToolButton *extractHighlightToolbutton  = new QToolButton();
    QToolButton *resetallHighlightToolbutton = new QToolButton();

    FluoLine *sepLine1 = new FluoLine(QFrame::VLine);
    FluoLine *sepLine2 = new FluoLine(QFrame::VLine);
    FluoLine *sepLine3 = new FluoLine(QFrame::VLine);
    FluoLine *sepLine4 = new FluoLine();

    QTreeView *tempTreeview = new QTreeView();
};


#endif
