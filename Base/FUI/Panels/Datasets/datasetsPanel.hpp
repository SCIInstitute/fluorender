#ifndef DATASETS_PANEL_HPP
#define DATASETS_PANEL_HPP

#include <QWidget>
#include <QGroupBox>
#include <QFrame>

#include "datasetsToolbarLayout.hpp"
#include "datasetsTreeLayout.hpp"

#include <CustomWidgets/fluoLine.hpp>



#include "treeModel.hpp"

class DatasetsPanel : public QWidget
{
  Q_OBJECT

  public:
    DatasetsPanel()
    {
      treeModel = new TreeModel(*this);
      treeLayout = new DatasetsTreeLayout(treeModel);
      //treeLayout->setModel(treeModel);
      topFrame->setLayout(toolLayout);
      botFrame->setLayout(treeLayout);
      frameLayout->addWidget(topFrame,0,0);
      frameLayout->addWidget(sepLine,1,0);
      frameLayout->addWidget(botFrame,2,0);
      datasetsFrame->setLayout(frameLayout);
      widgetLayout->addWidget(datasetsFrame);
      this->setLayout(widgetLayout);
    }

  private:
    QGroupBox *datasetsFrame = new QGroupBox();

    QFrame *topFrame = new QFrame();
    QFrame *botFrame = new QFrame();

    DatasetsToolbarLayout *toolLayout = new DatasetsToolbarLayout();
    DatasetsTreeLayout *treeLayout;

    QGridLayout *widgetLayout = new QGridLayout();
    QGridLayout *frameLayout = new QGridLayout();

    FluoLine *sepLine = new FluoLine();

    TreeModel *treeModel; 
};

#endif
