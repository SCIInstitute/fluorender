#ifndef FLUO_TAB_WIDGET
#define FLUO_TAB_WIDGET

#include <vector>
#include <tuple>

#include <QTabWidget>
#include <QGridLayout>
#include <QLabel>

#include "fluoSlider.hpp"
#include "fluoSpinbox.hpp"
#include "fluoSpinboxDouble.hpp"
#include "fluoToolButton.hpp"

class FluoTabWidget : public QTabWidget
{
  Q_OBJECT
  
  public:
    FluoTabWidget(QWidget *parent = nullptr) : QTabWidget(parent)
    {
      QFrame *newFrame = new QFrame();
      QGridLayout *newPage = volumePage();

      newFrame->setLayout(newPage);
      this->addTab(newFrame,"Renderview: 0");
      //this->setLayout(newPage);
      //this->setTabText(0,"Renderview: 0");
    }

    QGridLayout* volumePage();
    QGridLayout* meshPage();

  private:

    std::vector<FluoSlider*> genMeshSliders();

    typedef std::tuple<FluoSlider*,FluoSlider*,
                       FluoSlider*,FluoSlider*,FluoSlider*,
                       FluoSlider*,FluoSlider*,
                       FluoSlider*,FluoSlider*,
                       FluoSlider*,FluoSlider*,FluoSlider*,FluoSlider*> VolumeSliders;

    typedef std::tuple<FluoSpinboxDouble*,FluoSpinboxDouble*,
                       FluoSpinbox*,FluoSpinbox*,FluoSpinbox*,
                       FluoSpinbox*,FluoSpinboxDouble*,
                       FluoSpinbox*,FluoSpinboxDouble*,
                       FluoSpinboxDouble*,FluoSpinboxDouble*,FluoSpinbox*,FluoSpinbox*> VolumeSpinboxes;

    typedef std::tuple<QLabel*, QLabel*,
                       QLabel*, QLabel*,
                       QLabel*, FluoToolButton*,
                       FluoToolButton*, QLabel*,
                       FluoToolButton*, FluoToolButton*> VolumeLabels;


    std::tuple<FluoSlider*,FluoSlider*,
               FluoSlider*,FluoSlider*,FluoSlider*,
               FluoSlider*,FluoSlider*,
               FluoSlider*,FluoSlider*,
               FluoSlider*,FluoSlider*,FluoSlider*,FluoSlider*> genVolumeSliders()
    {
      return std::make_tuple(new FluoSlider(Qt::Horizontal,0,100), new FluoSlider(Qt::Horizontal,0,100),
                             new FluoSlider(Qt::Horizontal,0,255), new FluoSlider(Qt::Horizontal,0,255), new FluoSlider(Qt::Horizontal,0,255),
                             new FluoSlider(Qt::Horizontal,0,255), new FluoSlider(Qt::Horizontal,0,100),
                             new FluoSlider(Qt::Horizontal,0,255), new FluoSlider(Qt::Horizontal,1,500),
                             new FluoSlider(Qt::Horizontal,0,1000), new FluoSlider(Qt::Horizontal,0,200), new FluoSlider(Qt::Horizontal,0,255), new FluoSlider(Qt::Horizontal,0,255)
                             );
    }

    std::tuple<FluoSpinboxDouble*,FluoSpinboxDouble*,
               FluoSpinbox*,FluoSpinbox*,FluoSpinbox*,
               FluoSpinbox*,FluoSpinboxDouble*,
               FluoSpinbox*,FluoSpinboxDouble*,
               FluoSpinboxDouble*,FluoSpinboxDouble*,FluoSpinbox*,FluoSpinbox*> genVolumeSpinboxes()
    {
        return std::make_tuple(new FluoSpinboxDouble(0.0,1.0,false),new FluoSpinboxDouble(0.0,0.5,false),
                               new FluoSpinbox(0,255,false), new FluoSpinbox(0,255,false), new FluoSpinbox(0,255,false),
                               new FluoSpinbox(0,255,false), new FluoSpinboxDouble(0.0,1.0,false),
                               new FluoSpinbox(0,255,false), new FluoSpinboxDouble(0.1,5.0,false),
                               new FluoSpinboxDouble(0.0,10.0,false), new FluoSpinboxDouble(0.0,2.0,false),new FluoSpinbox(0,255,false), new FluoSpinbox(0,255,false)
                               );

    }

    std::tuple<QLabel*, QLabel*,
               QLabel*, QLabel*,
               QLabel*, FluoToolButton*,
               FluoToolButton*, QLabel*,
               FluoToolButton*, FluoToolButton*> genVolumeLables()
    {
      return std::make_tuple(new QLabel(" :Gamma"), new QLabel("Extract Boundary: "),
                             new QLabel(" :Saturation"), new QLabel("Threshold: "),
                             new QLabel(" :Luminance"), new FluoToolButton("Shadow: ",false),
                             new FluoToolButton(" :Alpha",false), new QLabel("Sample Rate: "),
                             new FluoToolButton(" :Shading",false), new FluoToolButton("Color Map: ",false)
                             );
    }

    void addRow0(QGridLayout* layout, const VolumeSliders& sliders,
                 const VolumeSpinboxes& spinboxes, const VolumeLabels& labels);
    void addRow1(QGridLayout* layout, const VolumeSliders& sliders,
                 const VolumeSpinboxes& spinboxes, const VolumeLabels& labels);
    void addRow2(QGridLayout* layout, const VolumeSliders& sliders,
                 const VolumeSpinboxes& spinboxes, const VolumeLabels& labels);
    void addRow3(QGridLayout* layout, const VolumeSliders& sliders,
                 const VolumeSpinboxes& spinboxes, const VolumeLabels& labels);
    void addRow4(QGridLayout* layout, const VolumeSliders& sliders,
                 const VolumeSpinboxes& spinboxes, const VolumeLabels& labels);


};


#endif
