#include "renderview.hpp"


/*
 * This constructor creates a new QMainWindow for a dock widget to
 * sit on top of. Toolbars are then created with specified orientations.
 * A new OpenGLWidget is then create and made as the central widget
 * of my QMainWindow. The toolbars are then added in their appropriate
 * locations. If there are no features, the dock widget is set to
 * no dock widget features, and the boolean isMainWindow is set to true.
 *
 * The newly created QMainWindow then adds the dock widget and all of
 * the features that comes with it.
 */
RenderView::RenderView(QWidget *parent, bool hasFeatures) : QMainWindow (parent)
{
    Q_INIT_RESOURCE(resources);

    auto newBaseWindow = new QMainWindow();



    // TODO: Instead of creating a new OpenGLWidget, I need to create a class
    //       that inherits an OpenGLWidget/OpenGLWindow.
    //auto newRenderView = std::make_unique<QOpenGLWidget>(new QOpenGLWidget);

    QSurfaceFormat format;
    format.setSamples(16);
    TriangleWindow* newTriangleWindow = new TriangleWindow();
    newTriangleWindow->setFormat(format);
    newTriangleWindow->setAnimating(true);

    if(!hasFeatures)
    {
        baseDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        isMainWindow = true;
    }


    newBaseWindow->setCentralWidget(QWidget::createWindowContainer(newTriangleWindow));
    newBaseWindow->addToolBar(Qt::LeftToolBarArea,leftToolBar);
    newBaseWindow->addToolBar(Qt::TopToolBarArea,topToolBar);
    newBaseWindow->addToolBar(Qt::RightToolBarArea, rightToolBar);
    newBaseWindow->addToolBar(Qt::BottomToolBarArea,bottomToolBar);

    baseDockWidget->setWidget(newBaseWindow);


    this->setWindowFlags(Qt::Widget);
    this->addDockWidget(Qt::RightDockWidgetArea, baseDockWidget);

    //connect(topToolBar, &TopToolbar::sendColor,newRenderView, &FluoGLWidget::receiveColor);

}

void RenderView::updateID(int i)
{
  id = i;
  baseDockWidget->setWindowTitle("Renderview " + QString::number(id));
}

// This is simply for debugging.
bool RenderView::getMainWindowStatus()
{
    return isMainWindow;
}

