#ifndef VOLUME_PROPERTIES_MISC
#define VOLUME_PROPERTIES_MISC

#include <QGridLayout>
#include <QStringList>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QToolButton>
#include <QFrame>

#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>


class VolumePropertiesMisc : public QGridLayout
{
  Q_OBJECT

  public:
    VolumePropertiesMisc();

  private:

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();
    void addRow5();

    QWidget* genToolWidget();
    QComboBox* genComboBox(const QStringList& list);
    QFrame* genLine();
    QGridLayout *populateToolLayout();


    QLabel *voxelSizeLabel   = new QLabel("Voxel Size: ");
    QLabel *xLabel           = new QLabel("X: ");
    QLabel *yLabel           = new QLabel("Y: ");
    QLabel *zLabel           = new QLabel("Z: ");
    QLabel *primeColorLabel  = new QLabel("Prime Color: ");
    QLabel *secondColorLabel = new QLabel("Second Color: ");
    QLabel *primeRLabel           = new QLabel("R: ");
    QLabel *primeGLabel           = new QLabel("G: ");
    QLabel *primeBLabel           = new QLabel("B: ");
    QLabel *secondRLabel           = new QLabel("R: ");
    QLabel *secondGLabel           = new QLabel("G: ");
    QLabel *secondBLabel           = new QLabel("B: ");
    QLabel *effectsLabel     = new QLabel("Effects: ");

    FluoSpinboxDouble *xSpinbox = new FluoSpinboxDouble(0.0,180.0,false);
    FluoSpinboxDouble *ySpinbox = new FluoSpinboxDouble(0.0,180.0,false);
    FluoSpinboxDouble *zSpinbox = new FluoSpinboxDouble(0.0,180.0,false);

    FluoSpinbox *primeRSpinbox  = new FluoSpinbox(0,255,false);
    FluoSpinbox *primeGSpinbox  = new FluoSpinbox(0,255,false);
    FluoSpinbox *primeBSpinbox  = new FluoSpinbox(0,255,false);
    FluoSpinbox *secondRSpinbox = new FluoSpinbox(0,255,false);
    FluoSpinbox *secondGSpinbox = new FluoSpinbox(0,255,false);
    FluoSpinbox *secondBSpinbox = new FluoSpinbox(0,255,false);

    QPushButton *primeColorButton  = new QPushButton();
    QPushButton *secondColorButton = new QPushButton();

    QToolButton *effectsToolButton = new QToolButton();

    QToolButton *highTransparancyButton     = new QToolButton();
    QToolButton *mIPButton                  = new QToolButton();
    QToolButton *invertDataButton           = new QToolButton();
    QToolButton *spatialInterpolationButton = new QToolButton();
    QToolButton *renderingResultButton      = new QToolButton();
    QToolButton *syncChannelButton          = new QToolButton();
    QToolButton *depthModeButton            = new QToolButton();
    QToolButton *nameLegendButton           = new QToolButton();
    QToolButton *resetAllButton             = new QToolButton();
    QToolButton *saveDefaultsButton         = new QToolButton();

    QFrame *line = genLine();

    QWidget *toolButtonWidget = genToolWidget();

    const QStringList EFFECTS1_LIST = {
      "Rainbow", "Hot","Cool",
      "Diverging", "Monochrome",
      "High-key","Low-Key",
      "Hi Transparency"
    };

    const QStringList EFFECTS2_LIST = {
      "Intensity", "Z Value",
      "Y Value", "X Value",
      "Gradient", "Differential"
    };

    QComboBox *effectsList1 = genComboBox(EFFECTS1_LIST);
    QComboBox *effectsList2 = genComboBox(EFFECTS2_LIST);


};


#endif
