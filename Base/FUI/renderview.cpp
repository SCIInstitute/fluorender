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
    //auto leftToolBar = genToolProp(Qt::Vertical);
    auto topToolBar = genToolProp(Qt::Horizontal);
    auto rightToolBar = genToolProp(Qt::Vertical);
    auto bottomToolBar = genToolProp(Qt::Horizontal);


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

    //populateLeftToolBar(leftToolBar);
    populateRightToolBar(rightToolBar);
    populateTopToolBar(topToolBar);
    populateBottomToolBar(bottomToolBar);

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

std::unique_ptr<QSlider> RenderView::genSlider(Qt::Orientation ori, int floor, int ceiling)
{
    auto newSlider = std::make_unique<QSlider>();
    newSlider->setOrientation(ori);
    newSlider->setRange(floor,ceiling);

    return newSlider;
}

std::unique_ptr<QLabel> RenderView::genLabel(const QString &text)
{
    auto newLabel = std::make_unique<QLabel>(text);
    return newLabel;
}


std::unique_ptr<QAction> RenderView::genActionButton(const QString &imgName)
{
    auto newActionButton = std::make_unique<QAction>();
    newActionButton->setIcon(QIcon(imgName));

    return newActionButton;
}

std::unique_ptr<QComboBox> RenderView::genComboBox(const QStringList &items)
{
    auto newComboBox = std::make_unique<QComboBox>();
    newComboBox->addItems(items);

    return newComboBox;
}

void RenderView::populateLeftToolBar(std::unique_ptr<QToolBar> &leftToolBar)
{

  auto widget = std::make_unique<QWidget>(new QWidget);
  QVBoxLayout *newVertLayout = new QVBoxLayout;

  std::unique_ptr<QSlider> newSlider = genSlider(Qt::Vertical,0,1); //TODO: Figure out some way to change to double
  auto newDoubleSpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,1.0);

  newVertLayout->setAlignment(Qt::AlignVCenter);
  newVertLayout->addWidget(newSlider.release());

  widget->setLayout(newVertLayout);

  auto resetButton = genActionButton(":/reset.svg");
  auto fullCircle = genActionButton(":/fullCircle.svg");

  leftToolBar->addAction(fullCircle.release());
  leftToolBar->addWidget(widget.release());
  leftToolBar->addWidget(newDoubleSpinBox.release());
  leftToolBar->insertSeparator(resetButton.get());
  leftToolBar->addAction(resetButton.release());

}

void RenderView::populateRightToolBar(std::unique_ptr<QToolBar> &rightToolBar)
{
  auto widget = std::make_unique<QWidget>();
  QVBoxLayout *newVertLayout = new QVBoxLayout;

  std::unique_ptr<QSlider> newSlider = genSlider(Qt::Vertical,35,999);

  auto newSpinBox = genSpinBox<QSpinBox,int>(35,999);
  auto resetButton = genActionButton(":/reset.svg");
  auto zoomTVButton = genActionButton(":/tv.svg");
  auto lightBulbOffButton = genActionButton(":/lightOff.svg");
  auto zoomLabel = genLabel("Zoom");
  zoomLabel->setAlignment(Qt::AlignVCenter);

  newVertLayout->setAlignment(Qt::AlignCenter);
  newVertLayout->addWidget(newSlider.release());

  widget->setLayout(newVertLayout);

  rightToolBar->addWidget(zoomLabel.release());
  rightToolBar->addAction(lightBulbOffButton.release());
  rightToolBar->addWidget(widget.release());
  rightToolBar->addWidget(newSpinBox.release());
  rightToolBar->insertSeparator(zoomTVButton.get());
  rightToolBar->addAction(zoomTVButton.release());
  rightToolBar->addAction(resetButton.release());
}

void RenderView::populateTopToolBar(std::unique_ptr<QToolBar> &topToolBar)
{

  QHBoxLayout *perspectiveLayout = new QHBoxLayout;
  QHBoxLayout *backgroundLayout = new QHBoxLayout;
  QHBoxLayout *scaleLayout = new QHBoxLayout;

  auto perspectiveWidget = std::make_unique<QWidget>();
  auto backgroundWidget = std::make_unique<QWidget>();
  auto scaleWidget = std::make_unique<QWidget>();
  auto colorDialogWidget = std::make_unique<QPushButton>();
  auto captureWidget = std::make_unique<QPushButton>();
  auto perspectiveAngleSlider = genSlider(Qt::Horizontal,0,100);
  auto colorDialog = std::make_unique<QColorDialog>(colorDialogWidget.get());

  auto perspectiveAngleLabel = genLabel("Perspective Angle:");
  auto backgroundLabel = genLabel("Background:");

  auto renderViewLayersAction = genActionButton(":/ungroupLayers.svg");
  auto renderViewDepthAction = genActionButton(":/depth.svg");
  auto renderViewColorsAction = genActionButton(":/byColor.svg");
  auto centerAxisAction = genActionButton(":/3dPlane.svg");
  auto infoAction = genActionButton(":/info.svg");
  auto labelAction = genActionButton(":/tag.svg");
  auto defaultScale = genActionButton(":/default.svg");
  auto scaleSpinBox = genSpinBox<QSpinBox,int>(0,999);
  auto scaleDropDown = genComboBox(scales);
  auto perspectiveSpinBox = genSpinBox<QSpinBox,int>(0,100);
  auto freeFlyAction = genActionButton(":/bird.svg");
  auto saveConfigsAction = genActionButton(":/saveConfigs.svg");

  perspectiveLayout->addWidget(perspectiveAngleLabel.release());
  perspectiveLayout->addWidget(perspectiveAngleSlider.release());
  perspectiveLayout->addWidget(perspectiveSpinBox.release());

  backgroundLayout->addWidget(backgroundLabel.release());
  backgroundLayout->addWidget(colorDialogWidget.release());

  scaleLayout->addWidget(scaleSpinBox.release());
  scaleLayout->addWidget(scaleDropDown.release());

  perspectiveWidget->setLayout(perspectiveLayout);
  backgroundWidget->setLayout(backgroundLayout);
  scaleWidget->setLayout(scaleLayout);

  captureWidget->setIcon(QIcon(":/camera.svg"));
  captureWidget->setText(" Capture");

  topToolBar->addAction(renderViewLayersAction.release());
  topToolBar->addAction(renderViewDepthAction.release());
  topToolBar->addAction(renderViewColorsAction.release());
  topToolBar->addSeparator();
  topToolBar->addWidget(captureWidget.release());
  topToolBar->addSeparator();
  topToolBar->addAction(centerAxisAction.release());
  topToolBar->addAction(infoAction.release());
  topToolBar->addAction(labelAction.release());
  topToolBar->addSeparator();
  topToolBar->addAction(defaultScale.release());
  topToolBar->addWidget(scaleWidget.release());
  topToolBar->addWidget(perspectiveWidget.release());
  topToolBar->addAction(freeFlyAction.release());
  topToolBar->addSeparator();
  topToolBar->addWidget(backgroundWidget.release());
  topToolBar->addAction(saveConfigsAction.release());
}

void RenderView::populateBottomToolBar(std::unique_ptr<QToolBar> &bottomToolBar)
{
  auto sliderWidgets = std::make_unique<QWidget>();
  QHBoxLayout *newHoriLayout = new QHBoxLayout;

  auto xLabel = genLabel("X:");
  auto yLabel = genLabel("Y:");
  auto zLabel = genLabel("Z:");

  auto xSlider = genSlider(Qt::Horizontal,0,360);
  auto ySlider = genSlider(Qt::Horizontal,0,360);
  auto zSlider = genSlider(Qt::Horizontal,0,360);

  auto xSpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,360);
  auto ySpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,360);
  auto zSpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,360);

  auto labelDropDown = genComboBox(labels);
  auto resetButton = genActionButton(":/reset.svg");
  auto fortyFiveButton = genActionButton(":/fortyFive.svg");
  auto globeButton = genActionButton(":/globe.svg");
  auto planeButton = genActionButton(":/plane.svg");

  newHoriLayout->setAlignment(Qt::AlignHCenter);
  newHoriLayout->addWidget(xLabel.release());
  newHoriLayout->addWidget(xSlider.release());
  newHoriLayout->addWidget(xSpinBox.release());
  newHoriLayout->addWidget(yLabel.release());
  newHoriLayout->addWidget(ySlider.release());
  newHoriLayout->addWidget(ySpinBox.release());
  newHoriLayout->addWidget(zLabel.release());
  newHoriLayout->addWidget(zSlider.release());
  newHoriLayout->addWidget(zSpinBox.release());
  newHoriLayout->addWidget(labelDropDown.release());

  sliderWidgets->setLayout(newHoriLayout);

  bottomToolBar->addAction(fortyFiveButton.release());
  bottomToolBar->addAction(globeButton.release());
  bottomToolBar->addWidget(sliderWidgets.release());
  bottomToolBar->addAction(resetButton.release());
}

/*
 * This creates my toolbars, the orientation is set, and is returned.
 */
std::unique_ptr<QToolBar> RenderView::genToolProp(Qt::Orientation ori)
{
    auto newToolBar = std::make_unique<QToolBar>(new QToolBar);

    newToolBar->setMovable(false);
    newToolBar->setOrientation(ori);
    newToolBar->setStyleSheet("QToolBar {background: rgb(222,225,232)}");

    if(ori == Qt::Horizontal)
        newToolBar->setFixedHeight(35);
    else
        newToolBar->setFixedWidth(35);

    return newToolBar;
}

// This is simply for debugging.
bool RenderView::getMainWindowStatus()
{
    return isMainWindow;
}

