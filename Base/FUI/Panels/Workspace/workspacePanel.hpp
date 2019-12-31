#ifndef WORKSPACE_PANEL_HPP
#define WORKSPACE_PANEL_HPP

#include <QWidget>
#include <QGroupBox>
#include <QFrame>

#include "workspaceToolLayout.hpp"
#include "workspaceTreeLayout.hpp"

#include <CustomWidgets/fluoLine.hpp>

class WorkspacePanel : public QWidget
{
  Q_OBJECT

  public:
    WorkspacePanel()
    {
      topFrame->setLayout(toolLayout);
      botFrame->setLayout(treeLayout);
      frameLayout->addWidget(topFrame,0,0);
      frameLayout->addWidget(sepLine,1,0);
      frameLayout->addWidget(botFrame,2,0);
      workspaceFrame->setLayout(frameLayout);
      widgetLayout->addWidget(workspaceFrame);
      this->setLayout(widgetLayout);
    }

  private:
    QGroupBox *workspaceFrame = new QGroupBox();

    QFrame *topFrame = new QFrame();
    QFrame *botFrame = new QFrame();

    WorkspaceToolLayout *toolLayout = new WorkspaceToolLayout();
    WorkspaceTreeLayout *treeLayout = new WorkspaceTreeLayout();

    QGridLayout *widgetLayout = new QGridLayout();
    QGridLayout *frameLayout = new QGridLayout();

    FluoLine *sepLine = new FluoLine();

};

#endif
