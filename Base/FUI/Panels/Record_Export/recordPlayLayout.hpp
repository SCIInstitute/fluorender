#ifndef RECORD_PLAY_LAYOUT
#define RECORD_PLAY_LAYOUT

#include <QGridLayout>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoSlider.hpp>

class RecordPlayLayout : public QGridLayout
{
  Q_OBJECT

  public:
    RecordPlayLayout();

  private:

    void loadCombobox();
    void addRow0();
    void addRow1();

    QLabel *fpsLabel = new QLabel("FPS: ");
    QLabel *captureLabel = new QLabel("Capture: ");
    QLabel *secondsLabel = new QLabel("s");

    QComboBox *captureCombobox = new QComboBox();

    QToolButton *helpToolbutton = new QToolButton();
    QToolButton *playToolbutton = new QToolButton();
    QToolButton *backToolbutton = new QToolButton();

    FluoSlider *playBackSlider = new FluoSlider(Qt::Horizontal,0,500);
    FluoSpinbox *fpsSpinbox = new FluoSpinbox(30,144,false);
    FluoSpinboxDouble *secondsSpinbox = new FluoSpinboxDouble(0.0,5.0,false);

    QPushButton *savePushbutton = new QPushButton();

};

#endif
