#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include <QDockWidget>
#include <QTabWidget>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>

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
class PropertiesPanel : public QDockWidget
{
  Q_OBJECT

  public:
    PropertiesPanel(QWidget *parent = nullptr) : QDockWidget(parent){}

  private:
    QTabWidget *tabWidget = new QTabWidget();
};

#endif
