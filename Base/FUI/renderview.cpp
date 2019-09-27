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

    auto newBaseWindow = std::make_unique<QMainWindow>(new QMainWindow);
    auto baseDockWidget = std::make_unique<QDockWidget>(new QDockWidget);

    auto leftToolBar = std::make_unique<LeftToolbar>();
    auto topToolBar = std::make_unique<TopToolbar>();
    auto rightToolBar = std::make_unique<RightToolbar>();
    auto bottomToolBar = std::make_unique<BottomToolbar>();


    // TODO: Instead of creating a new OpenGLWidget, I need to create a class
    //       that inherits an OpenGLWidget/OpenGLWindow.
    //auto newRenderView = std::make_unique<QOpenGLWidget>(new QOpenGLWidget);

    std::unique_ptr<FluoGLWidget> newRenderView;
    //auto newRenderView = std::make_unique<FluoGLWidget>(new FluoGLWidget);

    if(!hasFeatures)
    {
        baseDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        isMainWindow = true;
        newRenderView = std::make_unique<FluoGLWidget>(true);
    }
    else
    {
        newRenderView = std::make_unique<FluoGLWidget>();
    }

    newBaseWindow->setCentralWidget(QWidget::createWindowContainer(newRenderView.release()));
    newBaseWindow->addToolBar(Qt::LeftToolBarArea,leftToolBar.release());
    newBaseWindow->addToolBar(Qt::TopToolBarArea,topToolBar.release());
    newBaseWindow->addToolBar(Qt::RightToolBarArea, rightToolBar.release());
    newBaseWindow->addToolBar(Qt::BottomToolBarArea,bottomToolBar.release());

    baseDockWidget->setWidget(newBaseWindow.release());



    this->setWindowFlags(Qt::Widget);
    baseDockWidget->setWindowTitle("Renderview " + QString::number(renderNumber));
    this->addDockWidget(Qt::RightDockWidgetArea, baseDockWidget.release());
}

// This is simply for debugging.
bool RenderView::getMainWindowStatus()
{
    return isMainWindow;
}

