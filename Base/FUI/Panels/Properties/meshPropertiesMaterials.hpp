#ifndef MESH_PROPERTIES_MATERIAL
#define MESH_PROPERTIES_MATERIAL

#include <QGridLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>

class MeshPropertiesMaterials : public QGridLayout
{
  Q_OBJECT
  public:
    MeshPropertiesMaterials();

  private:

    void addRow0();
    void addRow1();
    void addRow2();

    QPushButton* diffuseColorButton  = new QPushButton();
    QPushButton* specularColorButton = new QPushButton();

    FluoSlider *shininessSlider   = new FluoSlider(Qt::Horizontal,0,128);
    FluoSpinbox *shininessSpinBox = new FluoSpinbox(0,128,false);

    QLabel *diffuseColorLabel  = new QLabel("Diffuse Color: ");
    QLabel *specularColorLabel = new QLabel("Specular Color: ");
    QLabel *shininessLabel     = new QLabel("Shininess: ");
    //TODO: create diffuse color: button with color in it.
    //      create specular color: button with black color in it
    //      create shininess slider with range 0 - 128
    //      create shininess spinbox with range 0 - 128
    //      create all the labels for them.


};

#endif
