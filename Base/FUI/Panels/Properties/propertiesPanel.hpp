#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include <QDockWidget>
#include <QTabWidget>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoTabWidget.hpp>

/*
 *
 * TODO: Decide whether or not creating a custom QDockWidget is the correct choice.
 * The QDockWidget may not need to be customized. However, the widgets inside of
 * them are going to need to be customized. Including the sliders.
 *
 * I need to look at the differences between the volumes and determine how those are
 * loaded in. I believe that a tab will need to appear no matter what since there is
 * always one renderview. However once it is loaded in then the tabs will need to be
 * populated with either a volume or mesh properties.
 *
*/
class PropertiesPanel : public QWidget
{
  Q_OBJECT

  public:
    PropertiesPanel(QWidget *parent = nullptr) : QWidget(parent)
    {
      QGridLayout* newLayout = new QGridLayout();
      FluoTabWidget *tabWidget = new FluoTabWidget();
      newLayout->addWidget(tabWidget);
      this->setLayout(newLayout);
    }

    //FluoTabWidget *tabWidget = new FluoTabWidget();

  private:

};

#endif
