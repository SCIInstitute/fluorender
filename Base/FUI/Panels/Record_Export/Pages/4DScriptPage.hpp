#ifndef FOUR_D_SCRIPT_PAGE_HPP
#define FOUR_D_SCRIPT_PAGE_HPP

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QStringList>

#include <CustomWidgets/fluoToolButton.hpp>

#include <vector>
#include <functional>

class FourDScriptPage : public QWidget
{
  Q_OBJECT

  public:

    FourDScriptPage();

  private:

    void setTableContents();

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();

    void constructPage();

    QGridLayout *gridLayout = new QGridLayout();

    QCheckBox *enableCheckbox = new QCheckBox(
      "Enable execution of a script on 4D data during play"
    );

    QLabel *enableDescLabel = new QLabel(
      "Also enable this option to show component colors."
    );

    QLabel *scriptLabel = new QLabel("Script File: ");

    QPushButton *browseButton = new QPushButton("Browse...");

    QLineEdit *fileLocTextbar = new QLineEdit();

    FluoToolButton *deleteToolbutton = new FluoToolButton("X",false,false,false);

    QTableWidget *table = new QTableWidget(14,1);

    QStringList header = {"Built-in Script Files"};

    const std::vector<QTableWidgetItem*> rowItems = {
      new QTableWidgetItem("script_4D_opencl"),
      new QTableWidgetItem("script_4D_ruler_profile"),
      new QTableWidgetItem("script_4D_selection_tracking"),
      new QTableWidgetItem("script_4D_calculate"),
      new QTableWidgetItem("script_4D_ca_fm"),
      new QTableWidgetItem("script_4D_comp_analysis"),
      new QTableWidgetItem("script_4D_fetch_mask"),
      new QTableWidgetItem("script_4D_random_colors"),
      new QTableWidgetItem("script_4D_sparse_tracking"),
      new QTableWidgetItem("script_4D_save_fetch_mask"),
      new QTableWidgetItem("script_4D_generate_comp"),
      new QTableWidgetItem("script_4D_external_axe"),
      new QTableWidgetItem("script_4D_reduce_noise"),
      new QTableWidgetItem("script_4D_separate_channels")
    };

    const std::vector<std::function<void()>> rows = {
      std::bind(&FourDScriptPage::addRow0,this),
      std::bind(&FourDScriptPage::addRow1,this),
      std::bind(&FourDScriptPage::addRow2,this),
      std::bind(&FourDScriptPage::addRow3,this),
      std::bind(&FourDScriptPage::addRow4,this)
    };
};

#endif
