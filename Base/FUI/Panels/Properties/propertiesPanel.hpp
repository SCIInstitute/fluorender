#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include <QTabWidget>

#include "volumePropertiesOptions.hpp"
#include "volumePropertiesMisc.hpp"
#include "meshPropertiesOptions.hpp"
#include "meshPropertiesMaterials.hpp"

/*
 *
 * TODO: See if there is a way to inherit from a QMainWindow to use QDockWidgets
 *       instead. These tab widgets are not undockable so there is no way to
 *       take them off the dock.
 *
*/
class PropertiesPanel : public QWidget
{
  Q_OBJECT

  public slots:
    void onVolumeLoaded(int renderviewID);
    void onMeshLoaded(int renderviewID);

  public:
    PropertiesPanel();

  private:

    template<typename Property>
    QFrame *genLeftFrame(Property* left)
    {
      QFrame *leftFrame = new QFrame();
      leftFrame->setLayout(left);

      return leftFrame;
    }

    template<typename Property>
    QFrame *genRightFrame(Property* right)
    {
      QFrame *rightFrame = new QFrame();
      rightFrame->setLayout(right);

      return rightFrame;
    }

    QWidget *genMainWidget(QFrame *left, QFrame *right);

    QTabWidget *tabWidget = new QTabWidget();
    QGridLayout *myLayout = new QGridLayout();

};

#endif
