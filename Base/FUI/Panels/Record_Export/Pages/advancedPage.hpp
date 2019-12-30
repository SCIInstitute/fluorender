#ifndef ADVANCED_PAGE_HPP
#define ADVANCED_PAGE_HPP

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>

#include <CustomWidgets/fluoSpinbox.hpp>

class AdvancedPage : public QWidget
{
  Q_OBJECT

  public:

    AdvancedPage();

  private:

    void setDropDown();
    void setTableWidgetContents();

    void addRow0();
    void addRow1();
    void addRow2();

    QGridLayout *gridLayout = new QGridLayout();

    QLabel *keyFramesLabel = new QLabel("Key Frames: ");
    QLabel *defaultLabel   = new QLabel("Default: ");

    QTableWidget *table = new QTableWidget(1,5);
    QComboBox *dropDown = new QComboBox();

    QPushButton *addButton       = new QPushButton();
    QPushButton *deleteButton    = new QPushButton();
    QPushButton *deleteAllButton = new QPushButton();

    FluoSpinbox *fpsSpinbox = new FluoSpinbox(30,144,false);

    const QStringList dropText    = {"Linear", "Smooth"};
    const QStringList tableLabels = {
      "ID",
      "Smooth",
      "In-betweens",
      "Interpolation",
      "Description"
    };

};

#endif
