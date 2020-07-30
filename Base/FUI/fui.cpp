#include "fui.hpp"
#include "ui_FUI.h"

#include "readers.hpp"
#include <Global/Global.hpp>

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QResource>
#include <iostream>

FUI::FUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FUI)
{
  fluo::Global::instance().getVolumeFactory().setValue(defaultFilename, location);
  ui->setupUi(this);

  // This sets the central widget needed for the QDockWidgets
  setCentralWidget(ui->centralWidget);

  // This will set the Dock Widget sizes to the screen of the user
  setDockWidgetSizes();

  // This creates our main Render Viewer and adds it to the splitter.
  auto baseRenderWindow = createNewRenderWindow(false);
  ui->splitter->addWidget(baseRenderWindow);


  // This opens the program in full screen and sets the allowed ares to dock.
  QMainWindow::showMaximized();
  QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // I cast the integer Half Renderview Size to a constant.
  const_cast<int&>(HALFRENDERVIEWSIZE) = ui->splitter->widget(MAINRENDERINDEX)->height()/2;

  //ui->dockWidgetContents = propertiesPanel;
}

FUI::~FUI()
{
  delete ui;
}

/*
 * This gets the size of the windows and sets the widget sizes on the
 * GUI. There may be a better way to do this.
 */
void FUI::setDockWidgetSizes()
{
  int windowSize = this->window()->height();
  int thirdHeight = (windowSize/3) - 100;
  int modifiedHeight = thirdHeight + thirdHeight;

  ui->DatasetsWidget->setMinimumHeight(thirdHeight);
  ui->WorkspaceWidget->setMinimumHeight(thirdHeight);
  ui->RecordWidget->setMinimumHeight(thirdHeight);
  ui->OutputAdjustmentsWidget->setMinimumHeight(thirdHeight);
  ui->ClippingPanesWidget->setMinimumHeight(thirdHeight);

  ui->DatasetsWidget->setMaximumHeight(modifiedHeight);
  ui->WorkspaceWidget->setMaximumHeight(modifiedHeight);
  ui->RecordWidget->setMaximumHeight(modifiedHeight);
  ui->OutputAdjustmentsWidget->setMaximumHeight(windowSize - 100);
  ui->ClippingPanesWidget->setMaximumHeight(windowSize - 100);

}

void FUI::updateRenderviewID(RenderView* view)
{
  for (int i = 0; i < renderviews.size(); ++i)
  {
    if(renderviews[i] == nullptr)
    {
      view->updateID(i);
      renderviews[i] = view;
      break;
    }
  }
}

void FUI::deleteRenderviewID(const RenderView* view)
{
  int id = view->getId();
  renderviews[id] = nullptr;
}
/*
 * This function creates a new Render Window inside a QFrame. The boolean
 * hasFeatures, differentiates whether or not it is the main window. The main
 * Window, at this time, is not removable from the dock.
 *
 * This function creates a new Grid Layout so the Widget will stretch across
 * the Frame that the Render View is assigned to. I then give it a style so
 * it is more noticeable within the splitter. I then add the Frame, which
 * has a Render View, to the layout. I increase the number of view windows
 * and then return the Frame.
 *
 * TODO: Find better way to number windows.
 */
QFrame* FUI::createNewRenderWindow(bool hasFeatures)
{
  QGridLayout *baseLayout = new QGridLayout();
  auto newBaseFrame = new QFrame();
  auto newRenderView = new RenderView(hasFeatures);

  newBaseFrame->setFrameStyle(QFrame::StyledPanel);
  newBaseFrame->setFrameShadow(QFrame::Sunken);

  updateRenderviewID(newRenderView);
  baseLayout->addWidget(newRenderView);
  newBaseFrame->setLayout(baseLayout);

  return newBaseFrame;

}


/*
 * This simply extracts the Main Render View. This is needed when the user
 * wants to change the Splitter back into one screen as it is not allowed
 * to copy widgets or reassign them.
 *
 * This function takes a QObject, extracts the target child and puts it
 * back into a new Frame.
 */
QFrame* FUI::extractWindow(QObject *toExtract)
{
  QGridLayout *baseLayout = new QGridLayout();
  auto newBaseFrame = new QFrame();
  RenderView *extracted = qobject_cast<RenderView*>(toExtract->children().at(1));

  newBaseFrame->setFrameStyle(QFrame::StyledPanel);
  newBaseFrame->setFrameShadow(QFrame::Sunken);

  baseLayout->addWidget(extracted);
  newBaseFrame->setLayout(baseLayout);

  return newBaseFrame;

}


/*
 * This function creates a new QSplitter with two render viewers inside of that
 * QSplitter. The Variables newTopWindow and newBottomWindow should be renamed
 * to something more meaningful in the future.
 *
 * This function takes an Orientation. The new QSplitter then has the orientation
 * set inside. The widgets are added to the new QSplitter, the sizes are then set
 * appropriately, and it is then returned.
 */
QSplitter* FUI::createNewSplitter(Qt::Orientation orientation)
{
  auto newSplitter = new QSplitter();
  auto newTopWindow = createNewRenderWindow(true);
  auto newBottomWindow = createNewRenderWindow(true);

  newSplitter->setOrientation(orientation);
  newSplitter->addWidget(newTopWindow);
  newSplitter->addWidget(newBottomWindow);

  newSplitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

  return newSplitter;
}


/*
 * This function should be renamed to something more meaningful. This creates a
 * new QSplitter and a new render view. However one of the widgets is then moved
 * into the new QSplitter as to preserve the current widget. This is used when
 * the user needs to have two render viewers on the left hand side and one large
 * render viewer on the right hand side. The main render view is then moved into
 * the new Qsplitter so it is not overwritten.
 *
 * This function takes an orientation as to set the orientation of the new
 * QSplitter. It also takes the index of the render view window that needs to be
 * moved. The size of the new QSplitter is then set appropriately and is returned.
 */
QSplitter* FUI::moveSplitter(Qt::Orientation orientation, int renderIndex)
{
  auto newSplitter = new QSplitter();
  auto newWindow = createNewRenderWindow(true);
  newSplitter->setOrientation(orientation);
  newSplitter->addWidget(std::move(ui->splitter->widget(renderIndex)));
  newSplitter->addWidget(newWindow);

  newSplitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

  return newSplitter;
}

/*
 * This function creates a new QSplitter and takes two render views that are
 * already inside the main Splitter. This is used when the user wants to go
 * from two render views on the right hand side and one large one on the left.
 * To two render views on the left hand side and one large one on the right.
 *
 * This function takes an orientation as to set the orientation of the new
 * QSplitter. It also takes two indexes of the render views that need to be
 * flipped. The target widgets are then removed from the splitter and placed
 * inside the new QSplitter. The size of the new QSplitter is then set
 * appropriately and it is returned.
 */
QSplitter* FUI::flipSplitter(Qt::Orientation orientation, int view1, int view2)
{
  auto newSplitter = new QSplitter();

  newSplitter->setOrientation(orientation);

  auto toAdd = std::move(ui->splitter->widget(view1)->children().at(0));
  newSplitter->addWidget(std::move(ui->splitter->widget(view2)));
  newSplitter->addWidget(static_cast<QWidget*>(toAdd));

  newSplitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

  return newSplitter;
}


/*
 * This function should be named something more meaningful in the future.
 * Simply put, this enables and disables buttons in the Split Render View
 * menu drop down. When a menu item is selected, that menu item will
 * become disabled, and if applicable, will enable the previously disabled
 * menu item.
 *
 * This function takes a QAction (note, the reference_wrapper will be
 * removed in the future as the cost of passing a pointer is really small.)
 * and sets the selected menu item to be disabled while enabling the
 * previous item if applicable. If the hash table, flag Control, is not
 * empty, the previous menu item is set to be enabled. The current menu
 * item is then set to disabled and replaces the previous menu item in the
 * hash table. If the hash table, flag control, is empty then the menu
 * item is set to be disabled and is inserted into the hash table.
 */
void FUI::checkFlags(QAction* currentFlag)
{
  if(!flagControl.empty())
  {
    flagControl[false]->setEnabled(true);
    currentFlag->setEnabled(false);
    flagControl[false] = currentFlag;
  }
  else
  {
    currentFlag->setEnabled(false);
    flagControl[false] = currentFlag;
  }
}

/*
 * This function handles the main Splitter being split directly down
 * the middle. When the menu item, actionSplit_HalfV, is triggered it
 * checks to make sure that there is not another button disabled in the
 * hash table. It then goes through each of the if statements below:
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 1: this
 * simply will set the orientation to be Horizontal. Which creates the
 * line down the middle. No panels need to be created here.
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 2: this
 * does the same exact thing as the if statement above it, however it
 * replaces the main splitter's second child's widget.
 *
 * if mainWindowPanelCounter is 2 and it's neighbor's counter is 1: this
 * takes the main render widget from the left hand side and replaces the
 * first child in the main splitter. This deletes the widget that the
 * main render view was attached to in the beginning.
 *
 * if mainWindowPanelCounter is 2 and it's neigbor's counter is 2: this
 * extracts two QWidgets and assigns them into new frames. The widgets
 * then replaces the first and second child of the main splitter's
 * widgets.
 *
 * else: there is only one widget in the splitter, thus it creates a
 * new render view and adds it to the main Splitter.
 */
void FUI::on_actionSplit_HalfV_triggered()
{

  checkFlags(ui->actionSplit_HalfV);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    ui->splitter->setOrientation(Qt::Horizontal);
  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {
    QFrame *temp = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(1));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    ui->splitter->replaceWidget(M_RENDERIND_NEI,temp);
    ui->splitter->setOrientation(Qt::Horizontal);
    neighborWindowPanelCounter-=1;

  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {
    QFrame *temp = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    ui->splitter->replaceWidget(MAINRENDERINDEX,temp);
    ui->splitter->setOrientation(Qt::Horizontal);
    mainWindowPanelCounter-=1;

  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 2)
  {
    QFrame *mainWindow = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));
    QFrame *extractedNeighbor = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));

    QFrame *toDelete1 = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(0));
    QFrame *toDelete2 = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(1));

    RenderView *view1 = qobject_cast<RenderView*>(toDelete1->children().at(1));
    RenderView *view2 = qobject_cast<RenderView*>(toDelete2->children().at(1));

    deleteRenderviewID(view1);
    deleteRenderviewID(view2);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->replaceWidget(MAINRENDERINDEX,extractedNeighbor);
    ui->splitter->replaceWidget(M_RENDERIND_NEI,mainWindow);

    mainWindowPanelCounter-=1;
    neighborWindowPanelCounter-=1;
  }
  else
  {
    auto newRenderWindow = createNewRenderWindow(true);
    ui->splitter->addWidget(newRenderWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=1;
  }
}


/*
 * These functions are the same as the functions on actionSplit_HalfV
 * however the orientation is different.
 *
 * TODO: Maybe look to see if there is a way to templatize this with
 *       above function? Probably not since it is on it's own slot.
 */
void FUI::on_actionSplit_HalfH_triggered()
{

  checkFlags(ui->actionSplit_HalfH);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    ui->splitter->setOrientation(Qt::Vertical);
  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {
    QFrame *temp = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(1));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    ui->splitter->replaceWidget(M_RENDERIND_NEI,temp);
    ui->splitter->setOrientation(Qt::Vertical);
    neighborWindowPanelCounter-=1;
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {
    QFrame *temp = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    ui->splitter->replaceWidget(MAINRENDERINDEX,temp);
    ui->splitter->setOrientation(Qt::Vertical);
    mainWindowPanelCounter-=1;

  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 2)
  {
    QFrame *mainWindow = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));
    QFrame *extractedNeighbor = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));

    QFrame *toDelete1 = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(0));
    QFrame *toDelete2 = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(1));

    RenderView *view1 = qobject_cast<RenderView*>(toDelete1->children().at(1));
    RenderView *view2 = qobject_cast<RenderView*>(toDelete2->children().at(1));

    deleteRenderviewID(view1);
    deleteRenderviewID(view2);

    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->replaceWidget(MAINRENDERINDEX,extractedNeighbor);
    ui->splitter->replaceWidget(M_RENDERIND_NEI,mainWindow);

    mainWindowPanelCounter-=1;
    neighborWindowPanelCounter-=1;
  }
  else
  {
    auto newRenderWindow = createNewRenderWindow(true);
    ui->splitter->addWidget(newRenderWindow);
    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=1;
  }
}


/*
 * This function handles the main Splitter being split into two different
 * parts: The main render view as a large widget on the left hand side
 * and two render views on the right hand side. This also checks the hash
 * table to see if there is a button that needs to be enabled. It then
 * goes through each of the if statements below:
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 1: this
 * will move the widget from the right hand side and place it into a new
 * QSplitter. This new QSplitter will add another render view along side
 * the extracted widget. The new QSplitter is then added to the main
 * Splitter as extracting the target widget removes it from the main
 * QSplitter.
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 2: this
 * simply grabs the targeted render view index and switches the
 * orientation of the entire main Splitter and it's child.
 *
 * if mainWindowPanelCounter is 2 and it's neighbor's counter is 1: this
 * will flip the splitter. Essentially, the main render view is in
 * another QSplitter, the main render view is then extracted out of the
 * other QSplitter and replaces the first child on the main Splitter.
 * The accompanying widget that was with the other QSplitter is then
 * moved to another newly created QSplitter and placed along side the
 * main Splitter's seconds child splitter.
 *
 * if mainWindowPanelCounter is 2 and it's neigbor's counter is 2: this
 * will extract the main window from the original splitter and replace
 * the first child of the main Splitter's widgets. The orientations are
 * the set for both of them.
 *
 * else: there is only one render view. This then creates a new splitter
 * with two new render views added to it.
 */
void FUI::on_actionSplit_Right_Half_triggered()
{

  checkFlags(ui->actionSplit_Right_Half);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    auto moveWindow = moveSplitter(Qt::Vertical, M_RENDERIND_NEI);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(moveWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=1;
  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {
    ui->splitter->setOrientation(Qt::Horizontal);
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(M_RENDERIND_NEI));
    temp->setOrientation(Qt::Vertical);
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {

    auto flippedSplitter = flipSplitter(Qt::Vertical, MAINRENDERINDEX, M_RENDERIND_NEI);
    auto extractMainWindow = extractWindow(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->replaceWidget(MAINRENDERINDEX, extractMainWindow);
    ui->splitter->addWidget(flippedSplitter);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

    mainWindowPanelCounter-=1;
    neighborWindowPanelCounter+=1;
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 2)
  {
    QFrame *extractMain = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(M_RENDERIND_NEI));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    temp->setOrientation(Qt::Vertical);

    ui->splitter->replaceWidget(MAINRENDERINDEX,extractMain);
    ui->splitter->setOrientation(Qt::Horizontal);
    mainWindowPanelCounter-=1;
  }
  else
  {
    auto newWindows = createNewSplitter(Qt::Vertical);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(newWindows);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=2;
  }

}


/*
 * These functions are the same as the functions on actionSpliter_Right_Half
 * however the orientation is different.
 *
 * TODO: Maybe look to see if there is a way to templatize this with
 *       above function? Probably not since it is on it's own slot.
 */
void FUI::on_actionSplit_Bottom_Half_triggered()
{

  checkFlags(ui->actionSplit_Bottom_Half);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    auto moveWindow = moveSplitter(Qt::Horizontal,M_RENDERIND_NEI);

    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->addWidget(moveWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=1;

  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {
    ui->splitter->setOrientation(Qt::Vertical);
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(M_RENDERIND_NEI));
    temp->setOrientation(Qt::Horizontal);
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {

    auto flippedSplitter = flipSplitter(Qt::Horizontal, MAINRENDERINDEX, M_RENDERIND_NEI);
    auto extractMainWindow = extractWindow(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));

    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->addWidget(flippedSplitter);
    ui->splitter->replaceWidget(MAINRENDERINDEX, extractMainWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter-=1;
    neighborWindowPanelCounter+=1;
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 2)
  {
    QFrame *extractMain = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(M_RENDERIND_NEI));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(MAINRENDERINDEX)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    temp->setOrientation(Qt::Horizontal);

    ui->splitter->replaceWidget(MAINRENDERINDEX,extractMain);
    ui->splitter->setOrientation(Qt::Vertical);
    mainWindowPanelCounter-=1;
  }
  else
  {
    auto newWindows = createNewSplitter(Qt::Horizontal);

    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->addWidget(newWindows);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=2;
  }

}


/*
 * This function handles the main Splitter being split into two different
 * parts: The main render view is extracted and inserted into a newly
 * created QSplitter with a new render view to accompany it. And a large
 * render view on the right hand side. It checks the hash table to see
 * if a previous menu item needs to be enabled and the goes through each
 * of the if statements below:
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 1: this
 * This extracts the main Splitter's second child's widget and moves the
 * main render view's widget into a new Qsplitter which has a new render
 * viewer to accompany it. The new Qsplitter and extracted widget are
 * then added to the main Splitter.
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 2: this
 * flips the splitter. The main render view is extracted and placed into
 * a newly created QSplitter which has also extracted the main Splitter's
 * second child's widget to accompany it. They are then added to the main
 * Splitter's widgets.
 *
 * if mainWindowPanelCounter is 2 and it's neighbor's counter is 1: this
 * simply changes the orientation.
 *
 * if mainWindowPanelCounter is 2 and it's neigbor's counter is 2: this
 * This simply extracts the second child of the second child of the main
 * Splitter's widgets. It then places the first child of the second child
 * of the main splitter's widgets into a new QFrame.
 *
 * else: There is only one render view. Therefore the current render view
 * is extracted with a call to moveSplitter and has a new widget added to
 * that newly created QSplitter. A new render view is then created and
 * both are added to the main QSplitter.
 */
void FUI::on_actionSplit_Left_Half_triggered()
{

  checkFlags(ui->actionSplit_Left_Half);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    // TODO: Rename this to something that describes this more accurately.
    auto newRenderView = std::move(ui->splitter->widget(M_RENDERIND_NEI));
    auto moveWindow = moveSplitter(Qt::Vertical, MAINRENDERINDEX);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(moveWindow);
    ui->splitter->addWidget(newRenderView);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;

  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {
    auto flippedSplitter = flipSplitter(Qt::Vertical, M_RENDERIND_NEI, MAINRENDERINDEX);
    auto extractNeighWindow = extractWindow(ui->splitter->widget(M_RENDERIND_NEI-1)->children().at(0));

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->replaceWidget(MAINRENDERINDEX,flippedSplitter);
    ui->splitter->addWidget(extractNeighWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

    mainWindowPanelCounter+=1;
    neighborWindowPanelCounter-=1;
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {
    ui->splitter->setOrientation(Qt::Horizontal);
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(MAINRENDERINDEX));
    temp->setOrientation(Qt::Vertical);
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 2)
  {
    QFrame *extractNeigh = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(1));
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(MAINRENDERINDEX));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);
    temp->setOrientation(Qt::Vertical);

    ui->splitter->replaceWidget(M_RENDERIND_NEI,extractNeigh);
    ui->splitter->setOrientation(Qt::Horizontal);
    neighborWindowPanelCounter-=1;
  }
  else
  {
    auto moveRenderView = moveSplitter(Qt::Vertical, MAINRENDERINDEX);
    auto newRenderView = createNewRenderWindow(true);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(moveRenderView);
    ui->splitter->addWidget(newRenderView);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;
    neighborWindowPanelCounter+=1;
  }
}

/*
 * These functions are the same as the functions on actionSpliter_Right_Half
 * however the orientation is different.
 *
 * TODO: Maybe look to see if there is a way to templatize this with
 *       above function? Probably not since it is on it's own slot.
 */
void FUI::on_actionSplit_Top_Half_triggered()
{

  checkFlags(ui->actionSplit_Top_Half);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    auto newRenderView = std::move(ui->splitter->widget(M_RENDERIND_NEI));
    auto moveWindow = moveSplitter(Qt::Horizontal,MAINRENDERINDEX);


    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->addWidget(moveWindow);
    ui->splitter->addWidget(newRenderView);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;

  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {
    auto flippedSplitter = flipSplitter(Qt::Horizontal, M_RENDERIND_NEI, MAINRENDERINDEX);
    auto extractNeighWindow = extractWindow(ui->splitter->widget(M_RENDERIND_NEI-1)->children().at(0));

    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->replaceWidget(MAINRENDERINDEX,flippedSplitter);
    ui->splitter->addWidget(extractNeighWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

    mainWindowPanelCounter+=1;
    neighborWindowPanelCounter-=1;
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {
    ui->splitter->setOrientation(Qt::Vertical);
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(MAINRENDERINDEX));
    temp->setOrientation(Qt::Horizontal);
  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 2)
  {
    QFrame *extractNeigh = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(1));
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(MAINRENDERINDEX));
    QFrame *toDelete = static_cast<QFrame*>(ui->splitter->widget(M_RENDERIND_NEI)->children().at(0));

    RenderView *view = qobject_cast<RenderView*>(toDelete->children().at(1));
    deleteRenderviewID(view);

    temp->setOrientation(Qt::Horizontal);

    ui->splitter->replaceWidget(M_RENDERIND_NEI,extractNeigh);
    ui->splitter->setOrientation(Qt::Vertical);
    neighborWindowPanelCounter-=1;
  }
  else
  {
    auto moveRenderView = moveSplitter(Qt::Horizontal, MAINRENDERINDEX);
    auto newRenderView = createNewRenderWindow(true);

    ui->splitter->setOrientation(Qt::Vertical);
    ui->splitter->addWidget(moveRenderView);
    ui->splitter->addWidget(newRenderView);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;
    neighborWindowPanelCounter+=1;
  }

}


/*
 * This function handles the main Splitter being split into two different
 * parts: the main render view is removed and inserted into a newly
 * created QSplitter and is accompanied by another render view. The right
 * hand side is also created into a new QSplitter and has two new render
 * views within it. The flags are then checked to see if a menu item
 * needs to be enabled. It then goes through the following if statements:
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 1: this
 * calls move splitter on both the main render view and the main render
 * view's neighbor. They are both then added to the main QSplitter.
 *
 * if mainWindowPanelCounter is 1 and it's neighbor's counter is 2: this
 * This extracts the QSplitter from the second child of the main Splitter.
 * A new render view is then created and is inserted into that extracted
 * QSplitter. The main render view then has an extracted neighbor added
 * to it's newly created splitter and both are inserted into the main
 * Splitter's widgets.
 *
 * if mainWindowPanelCounter is 2 and it's neighbor's counter is 1: this
 * calls move splitter on the neighbor as we want a newly created
 * QSplitter to take the second child from the main Splitter and inset it
 * with a new render view inside of it.
 *
 * else: There is only one render view. We then extract the main render
 * view and place it in a newly created QSplitter which has a new render
 * view added to it. Then createNewSplitter is called to create two new
 * render views and they are then added to the main Splitter.
 */
void FUI::on_actionSplit_All_triggered()
{
  checkFlags(ui->actionSplit_All);

  if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 1)
  {
    auto moveMainRenderView = moveSplitter(Qt::Vertical, MAINRENDERINDEX);
    auto moveNeighRenderView = moveSplitter(Qt::Vertical, M_RENDERIND_NEI - 1); //

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(moveMainRenderView);
    ui->splitter->addWidget(moveNeighRenderView);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;
    neighborWindowPanelCounter+=1;
  }
  else if(mainWindowPanelCounter == 1 && neighborWindowPanelCounter == 2)
  {

    QSplitter *neighborSplitter = static_cast<QSplitter*>(ui->splitter->widget(M_RENDERIND_NEI));
    auto flippedSplitter = flipSplitter(Qt::Vertical, M_RENDERIND_NEI, MAINRENDERINDEX);
    auto newWindow = createNewRenderWindow(true);

    neighborSplitter->addWidget(newWindow);
    neighborSplitter->setOrientation(Qt::Vertical);
    neighborSplitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));

    ui->splitter->addWidget(flippedSplitter);
    ui->splitter->addWidget(std::move(ui->splitter->widget(MAINRENDERINDEX)));
    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;

  }
  else if(mainWindowPanelCounter == 2 && neighborWindowPanelCounter == 1)
  {

    auto moveNeighborWindow = moveSplitter(Qt::Vertical, M_RENDERIND_NEI);
    QSplitter *temp = static_cast<QSplitter*>(ui->splitter->widget(MAINRENDERINDEX));
    temp->setOrientation(Qt::Vertical);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(moveNeighborWindow);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    neighborWindowPanelCounter+=1;

  }
  else
  {
    auto moveRenderView = moveSplitter(Qt::Vertical, MAINRENDERINDEX);
    auto newWindows = createNewSplitter(Qt::Vertical);

    ui->splitter->setOrientation(Qt::Horizontal);
    ui->splitter->addWidget(moveRenderView);
    ui->splitter->addWidget(newWindows);
    ui->splitter->setSizes(QList<int>({HALFRENDERVIEWSIZE, HALFRENDERVIEWSIZE}));
    mainWindowPanelCounter+=1;
    neighborWindowPanelCounter+=2;
  }

}

/*
 * This function checks to see if any menu items need to be enabled. It then
 * checks to see if there are two windows on the left hand side. If there
 * are, then mainRenderView is extracted from the QSplitter's first second
 * child. If it does not, then it is extracted from the main Splitter's
 * first child.
 *
 * extractWindow is then called since we need to create a new QFrame with
 * the all the properties of the main render view. Since we cannot copy
 * QWidgets or reassign them. I have to extract the class from the widget
 * and place it in a new frame. I then delete all of the existing splitters,
 * and then I delete the existing QFrames. The counters are all reset, and
 * finally the extracted window is then added back into the main Splitter.
 */
void FUI::on_actionOne_View_triggered()
{
  checkFlags(ui->actionOne_View);

  for(int i = 1; i < renderviews.size();++i)
    renderviews[i] = nullptr;

  QObject *mainRenderView;

  if(mainWindowPanelCounter == 2)
    mainRenderView = std::move(ui->splitter->widget(MAINRENDERINDEX)->children().at(1));
  else
    mainRenderView = std::move(ui->splitter->widget(MAINRENDERINDEX));

  auto extractedWindow = extractWindow(mainRenderView);

  qDeleteAll(ui->splitter->findChildren<RenderView*>());
  qDeleteAll(ui->splitter->findChildren<QSplitter*>());
  qDeleteAll(ui->splitter->findChildren<QFrame*>());

  mainWindowPanelCounter = 1;
  neighborWindowPanelCounter = 0;
  renderViewcounter = 1;

  ui->splitter->addWidget(extractedWindow);

}

QString FUI::getFilename()
{
  return QFileDialog::getOpenFileName(this,
    tr("Open File"), "",
    tr("Open Tiff (*.tiff *.tif);; Open NRRD(*.nrrd);; Open Other(*.oib *.oif *.lsm)")
  );
}

QString FUI::getSuffix(QString& filename)
{
  QFileInfo info(filename);
  return info.suffix();
}

auto FUI::getReader(const QString& suffix)
{
  Readers tempReader(suffix);
  return tempReader.returnReader();
}

// TODO: Restructure this function so it is more readable.
void FUI::on_actionLoad_Volume_0_triggered()
{
  QString filename = getFilename();
  QString suffix = getSuffix(filename);

  if(!suffix.isEmpty())
  { 
    auto reader = getReader(suffix);
 
    std::wstring fileSetter = filename.toStdWString();
    fluo::VolumeData* vd = fluo::Global::instance().getVolumeFactory().build();

    reader->SetFile(fileSetter);
    reader->Preprocess();

    Nrrd* nrrdStructure = reader->Convert(true);

    QString name = QString::fromStdWString(reader->GetDataName());
    if(vd->LoadData(nrrdStructure, name.toStdString(), filename.toStdWString()))
      vd->SetReader(reader.get());
    ui->propertiesPanel->onVolumeLoaded(0);
  }
}

void FUI::on_actionLoad_Mesh_0_triggered()
{
  ui->propertiesPanel->onMeshLoaded(0);
}
