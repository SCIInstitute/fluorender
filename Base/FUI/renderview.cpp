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
RenderView::RenderView(QWidget *parent, bool hasFeatures, int renderNumber) : QMainWindow (parent)
{
    Q_INIT_RESOURCE(resources);

    auto newBaseWindow = std::make_unique<QMainWindow>();
    auto baseDockWidget = std::make_unique<QDockWidget>();


    // TODO: Instead of creating a new OpenGLWidget, I need to create a class
    //       that inherits an OpenGLWidget/OpenGLWindow.
    //auto newRenderView = std::make_unique<QOpenGLWidget>(new QOpenGLWidget);

    FluoGLWidget* newRenderView;
    //auto newRenderView = std::make_unique<FluoGLWidget>(new FluoGLWidget);

    if(!hasFeatures)
    {
        baseDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        isMainWindow = true;
        newRenderView = new FluoGLWidget(true);
    }
    else
    {
        newRenderView = new FluoGLWidget();
    }

    newBaseWindow->setCentralWidget(QWidget::createWindowContainer(newRenderView));
    newBaseWindow->addToolBar(Qt::LeftToolBarArea,leftToolBar);
    newBaseWindow->addToolBar(Qt::TopToolBarArea,topToolBar);
    newBaseWindow->addToolBar(Qt::RightToolBarArea, rightToolBar);
    newBaseWindow->addToolBar(Qt::BottomToolBarArea,bottomToolBar);

    baseDockWidget->setWidget(newBaseWindow.release());



    this->setWindowFlags(Qt::Widget);
    baseDockWidget->setWindowTitle("Renderview " + QString::number(renderNumber));
    this->addDockWidget(Qt::RightDockWidgetArea, baseDockWidget.release());

    connect(topToolBar, &TopToolbar::sendColor,newRenderView, &FluoGLWidget::receiveColor);

}

// This is simply for debugging.
bool RenderView::getMainWindowStatus()
{
    return isMainWindow;
}

