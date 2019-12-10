#ifndef MESH_PROPERTIES_OPTIONS
#define MESH_PROPERTIES_OPTIONS

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoToolButton.hpp>

class MeshPropertiesOptions : public QGridLayout
{
  Q_OBJECT

  public:
    MeshPropertiesOptions();

    FluoSlider* transparencySlider = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* shadowSlider       = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* lightingSlider     = new FluoSlider(Qt::Horizontal,0,200);
    FluoSlider* sizeLimitSlider    = new FluoSlider(Qt::Horizontal,0,250);

    FluoSpinboxDouble* transparencySpinbox = new FluoSpinboxDouble(0.0,1.0,false);
    FluoSpinboxDouble* shadowSpinbox       = new FluoSpinboxDouble(0.0,1.0,false);
    FluoSpinboxDouble* lightingSpinbox     = new FluoSpinboxDouble(0.0,2.0,false);
    FluoSpinbox* sizeLimitSpinbox          = new FluoSpinbox(0,250,false);

    QLabel* transparencyLabel = new QLabel("Transparency: ");
    QLabel* shadowLabel       = new QLabel("Shadow: ");
    QLabel* lightingLabel     = new QLabel("Lighting Scaling: ");
    QLabel* sizeLimitLabel    = new QLabel("Size Limit: ");

    QCheckBox* shadowCheckbox    = new QCheckBox();
    QCheckBox* lightingCheckbox  = new QCheckBox();
    QCheckBox* sizeLimitCheckbox = new QCheckBox();

  private:
    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
};

#endif
