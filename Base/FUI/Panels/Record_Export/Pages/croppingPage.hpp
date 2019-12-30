#ifndef CROPPING_PAGE_HPP
#define CROPPING_PAGE_HPP

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

#include <CustomWidgets/fluoSpinbox.hpp>

class CroppingPage : public QWidget
{
  Q_OBJECT

	public:

      CroppingPage();

    private:

      void addRow0();
      void addRow1();
      void addRow2();

      QGridLayout *gridLayout = new QGridLayout();

      QCheckBox *enableCroppingCheckbox = new QCheckBox("Enable Cropping: ");

      QPushButton *resetButton = new QPushButton();

      QLabel *centerLabel = new QLabel("Center");
      QLabel *xLabel      = new QLabel("X: ");
      QLabel *yLabel      = new QLabel("Y: ");
      QLabel *sizeLabel   = new QLabel("Size");
      QLabel *widthLabel  = new QLabel("Width: ");
      QLabel *heightLabel = new QLabel("Height: ");

      FluoSpinbox *xSpinbox      = new FluoSpinbox(0,100,true);
      FluoSpinbox *ySpinbox      = new FluoSpinbox(0,100,true);
      FluoSpinbox *widthSpinbox  = new FluoSpinbox(0,100,true);
      FluoSpinbox *heightSpinbox = new FluoSpinbox(0,100,true);

};

#endif
