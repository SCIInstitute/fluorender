#ifndef RENDERVIEW_TOP_TOOLBAR_HPP
#define RENDERVIEW_TOP_TOOLBAR_HPP

#include "genToolbarObjects.hpp"
#include <QPushButton>
#include <QColorDialog>
#include <QPalette>
#include <unordered_map>
#include <vector>

class TopToolbar : public QToolBar
{
  Q_OBJECT

  signals:
    void sendColor(const QColor &toSend);

  public slots:
    void on_layers_clicked() { checkFlags(renderViewLayersAction); }
    void on_depth_clicked() { checkFlags(renderViewDepthAction); }
    void on_colors_clicked() { checkFlags(renderViewColorsAction); }
    void on_scale_clicked() { rotateImage(defaultScale); }
    void on_color_clicked();

  private slots:

  public:
    TopToolbar();
    void updateBackgroundColor(){ emit sendColor(Color); }

  private:
    void initializeActionsAndWidgets();
    void initializeLayouts();
    void initializeWidgets();
    void initializePushButtons();
    void initializeColorDiaSlider();
    void initializeLabels();
    void initializeActions();
    void setRenderActionGroupSettings();
    void setInformationGroupSettings();
    void setScaleGroupSettings();
    void initialzeSpinBoxes();
    void addWidgetsToLayout();
    void setLayouts();
    void addActionsToToolbar();
    void setToolbarProperties();

    void checkFlags(QAction* currentFlag);
    void rotateImage(QAction* currentAction);
    void enableScaleGroupActions();
    void setColorWidgetcolor(QColor color);


    QHBoxLayout* perspectiveLayout;
    QHBoxLayout* backgroundLayout;
    QHBoxLayout* scaleLayout;

    QWidget* perspectiveWidget;
    QWidget* backgroundWidget;
    QWidget* scaleWidget;

    QPushButton* colorDialogWidget;
    QPushButton* captureWidget;

    QColorDialog* colorDialog;

    QSlider* perspectiveSlider;

    QLabel* perspectiveLabel;
    QLabel* backgroundLabel;

    QAction* renderViewLayersAction;
    QAction* renderViewDepthAction;
    QAction* renderViewColorsAction;
    QAction* centerAxisAction;
    QAction* infoAction;
    QAction* labelAction;
    QAction* defaultScale;
    QAction* freeFlyAction;
    QAction* saveConfigsAction;

    QSpinBox* scaleSpinBox;
    QSpinBox* perspectiveSpinBox;

    QComboBox* scaleDropDown;

    int imageID = 0;
    QColor Color;
    const QStringList scales = {"nm","μm","mm"};
    const std::vector<QString> images = { ":/default.svg",":/ruler.svg",":/byScale.svg" };

    std::unordered_map<bool,QAction*> flagControl;

};

#endif
