#ifndef AUTO_KEY_PAGE_HPP
#define AUTO_KEY_PAGE_HPP

#include <QWidget>
#include <QTableWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>

#include <vector>

class AutoKeyPage : public QWidget
{
  Q_OBJECT

  public:
    AutoKeyPage();

  private:

    void setTableContents();

    void addRow0();
    void addRow1();
    void addRow2();

    QGridLayout *gridLayout =  new QGridLayout();

    QLabel *autoKeyLabel = new QLabel("Choose an auto key type: ");

    QTableWidget *table = new QTableWidget(3,2);

    QPushButton *generatePushButton = new QPushButton();

    const QStringList headers = {"Auto Key Type", " "};

    const std::vector<QTableWidgetItem*> tableContents = {
      new QTableWidgetItem("Channel combination nC1"),
      new QTableWidgetItem("Channel combination nC2"),
      new QTableWidgetItem("Channel combination nC3"),
    };
};

#endif
