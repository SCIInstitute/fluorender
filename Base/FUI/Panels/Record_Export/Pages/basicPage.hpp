#ifndef BASIC_PAGE_HPP
#define BASIC_PAGE_HPP

#include <QWidget>
#include <QCheckBox>
#include <QButtonGroup>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QStringList>

#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoToolButton.hpp>

#include <vector>
#include <functional>

class BasicPage : public QWidget
{
  Q_OBJECT

  public:

    BasicPage();

  private:

    void setButtonGroup();
    void setComboBox();
    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();
    void addRow5();
    void addRow6();
    void constructLayout();

    QGridLayout *gridLayout = new QGridLayout();

    QCheckBox *rotationCheckBox  = new QCheckBox("Rotation");
    QCheckBox *timeBatchCheckBox = new QCheckBox("Time Sequence/Batch");

    QButtonGroup *buttonGroup = new QButtonGroup();

    QRadioButton *xButton = new QRadioButton();
    QRadioButton *yButton = new QRadioButton();
    QRadioButton *zButton = new QRadioButton();

    FluoSpinbox *degreesSpinbox     = new FluoSpinbox(0,360,true);
    FluoSpinbox *startTimeSpinbox   = new FluoSpinbox(0,100,true);
    FluoSpinbox *endTimeSpinbox     = new FluoSpinbox(0,100,true);
    FluoSpinbox *currentTimeSpinbox = new FluoSpinbox(0,100,true);
    FluoSpinbox *secondsSpinbox     = new FluoSpinbox(0,999,true);

    QLabel *degreesLabel     = new QLabel("Degrees");
    QLabel *styleLabel       = new QLabel("Style: ");
    QLabel *currentTimeLabel = new QLabel("Current Time: ");
    QLabel *startTimeLabel   = new QLabel("Start Time: ");
    QLabel *endTimeLabel     = new QLabel("End Time: ");
    QLabel *movieLenLabel    = new QLabel("Moview Length: ");
    QLabel *secondsLabel     = new QLabel("seconds");

    const QStringList styleList = {
      "Linear",
      "Smooth"
    };

    QComboBox *styleComboBox = new QComboBox();

    FluoToolButton *minusPushbutton = new FluoToolButton(":/record-minus.svg");
    FluoToolButton *plusPushbutton  = new FluoToolButton(":/record-plus.svg");

    const std::vector<std::function<void()>> rowFuncs = {
      std::bind(&BasicPage::addRow0,this),
      std::bind(&BasicPage::addRow1,this),
      std::bind(&BasicPage::addRow2,this),
      std::bind(&BasicPage::addRow3,this),
      std::bind(&BasicPage::addRow4,this),
      std::bind(&BasicPage::addRow5,this),
      std::bind(&BasicPage::addRow6,this)
    };

    virtual QSize sizeHint() const;
};

#endif
