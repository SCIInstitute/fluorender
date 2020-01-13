#ifndef RECORD_TAB_LAYOUT_HPP
#define RECORD_TAB_LAYOUT_HPP

#include <QGridLayout>
#include <QTabWidget>
#include <QString>

#include "Pages/basicPage.hpp"
#include "Pages/advancedPage.hpp"
#include "Pages/autoKeyPage.hpp"
#include "Pages/croppingPage.hpp"
#include "Pages/4DScriptPage.hpp"

#include <utility>

class RecordTabLayout : public QGridLayout
{
  Q_OBJECT

  public:
    RecordTabLayout();

  private:

    QTabWidget *tabWidget = new QTabWidget();

    void constructPages();

    std::vector<std::pair<QWidget*,const QString>> pages = {
      {new BasicPage(), "Basic"},
      {new AdvancedPage(), "Advanced"},
      {new AutoKeyPage(), "Auto Key"},
      {new CroppingPage(), "Cropping"},
      {new FourDScriptPage(), "4D Script"}
    };

};


#endif
