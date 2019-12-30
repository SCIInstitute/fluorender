#ifndef RECORD_EXPORT_LAYOUT_HPP
#define RECORD_EXPORT_LAYOUT_HPP

#include <QGridLayout>
#include <QTabWidget>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QString>
#include <QLabel>

#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoSlider.hpp>

#include "Pages/basicPage.hpp"
#include "Pages/advancedPage.hpp"
#include "Pages/autoKeyPage.hpp"
#include "Pages/croppingPage.hpp"
#include "Pages/4DScriptPage.hpp"

#include <utility>

class RecordExportLayout : public QGridLayout
{
  Q_OBJECT

  public:
    RecordExportLayout();

  private:

    QTabWidget *tabWidget = new QTabWidget();

    void addRow0();
    void addRow1();
    void addRow2();

    void constructPages();


    QLabel *fpsLabel = new QLabel("FPS: ");
    QLabel *captureLabel = new QLabel("Capture: ");
    QLabel *secondsLabel = new QLabel("s");

    FluoSpinbox *fpsSpinbox = new FluoSpinbox(30,144,false);
    FluoSpinboxDouble *secondsSpinbox = new FluoSpinboxDouble(0.0,5.0,false);

    FluoSlider *playBackSlider = new FluoSlider(Qt::Horizontal,0,500);

    QComboBox *captureCombobox = new QComboBox();

    QToolButton *helpToolbutton = new QToolButton();
    QToolButton *playToolbutton = new QToolButton();
    QToolButton *backToolbutton = new QToolButton();

    QPushButton *savePushbutton = new QPushButton();

    std::vector<std::pair<QWidget*,const QString>> pages = {
      {new BasicPage(), "Basic"},
      {new AdvancedPage(), "Advanced"},
      {new AutoKeyPage(), "Auto Key"},
      {new CroppingPage(), "Cropping"},
      {new FourDScriptPage(), "4D Script"}
    };

};


#endif
