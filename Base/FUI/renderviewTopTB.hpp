#ifndef RENDERVIEW_TOP_TOOLBAR_HPP
#define RENDERVIEW_TOP_TOOLBAR_HPP

#include "genToolbarObjects.hpp"
#include <QPushButton>
#include <QColorDialog>

class TopToolbar : public QToolBar
{
  public:
    TopToolbar();
    void initializeActionsAndWidgets();
    void initializeLayouts();
    void initializeWidgets();
    void initializePushButtons();
    void initializeColorDiaSlider();
    void initializeLabels();
    void initializeActions();
    void initialzeSpinBoxes();
    void addWidgetsToLayout();
    void setLayouts();
    void addActionsToToolbar();
    void setToolbarProperties();

  private:
    std::unique_ptr<QHBoxLayout> perspectiveLayout;
    std::unique_ptr<QHBoxLayout> backgroundLayout;
    std::unique_ptr<QHBoxLayout> scaleLayout;

    std::unique_ptr<QWidget> perspectiveWidget;
    std::unique_ptr<QWidget> backgroundWidget;
    std::unique_ptr<QWidget> scaleWidget;

    std::unique_ptr<QPushButton> colorDialogWidget;
    std::unique_ptr<QPushButton> captureWidget;

    std::unique_ptr<QColorDialog> colorDialog;

    std::unique_ptr<QSlider> perspectiveSlider;

    std::unique_ptr<QLabel> perspectiveLabel;
    std::unique_ptr<QLabel> backgroundLabel;

    std::unique_ptr<QAction> renderViewLayersAction;
    std::unique_ptr<QAction> renderViewDepthAction;
    std::unique_ptr<QAction> renderViewColorsAction;
    std::unique_ptr<QAction> centerAxisAction;
    std::unique_ptr<QAction> infoAction;
    std::unique_ptr<QAction> labelAction;
    std::unique_ptr<QAction> defaultScale;
    std::unique_ptr<QAction> freeFlyAction;
    std::unique_ptr<QAction> saveConfigsAction;

    std::unique_ptr<QSpinBox> scaleSpinBox;
    std::unique_ptr<QSpinBox> perspectiveSpinBox;

    std::unique_ptr<QComboBox> scaleDropDown;

    const QStringList scales = {"nm","μm","mm"};

};

#endif
