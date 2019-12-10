#include "fluoTabWidget.hpp"

QGridLayout* FluoTabWidget::volumePage()
{
  QGridLayout* volumeLayout = new QGridLayout();

  VolumeSliders sliders = genVolumeSliders();
  VolumeSpinboxes spinboxes = genVolumeSpinboxes();
  VolumeLabels labels = genVolumeLables();

  addRow0(volumeLayout,sliders,spinboxes,labels);
  addRow1(volumeLayout,sliders,spinboxes,labels);
  addRow2(volumeLayout,sliders,spinboxes,labels);
  addRow3(volumeLayout,sliders,spinboxes,labels);
  addRow4(volumeLayout,sliders,spinboxes,labels);

  return volumeLayout;
}

QGridLayout* FluoTabWidget::meshPage()
{
  QGridLayout* meshLayout = new QGridLayout();

  return meshLayout;
}

void FluoTabWidget::addRow0(QGridLayout *layout, const VolumeSliders &sliders,
                            const VolumeSpinboxes &spinboxes, const VolumeLabels &labels)
{
  layout->addWidget(std::get<0>(sliders),0,0,1,3);
  layout->addWidget(std::get<0>(spinboxes),0,3);
  layout->addWidget(std::get<0>(labels),0,4);
  layout->addWidget(std::get<1>(labels),0,5);
  layout->addWidget(std::get<1>(spinboxes),0,6);
  layout->addWidget(std::get<1>(sliders),0,7,1,3);
}

void FluoTabWidget::addRow1(QGridLayout *layout, const VolumeSliders &sliders,
                            const VolumeSpinboxes &spinboxes, const VolumeLabels &labels)
{
  layout->addWidget(std::get<2>(sliders),1,0,1,3);
  layout->addWidget(std::get<2>(spinboxes),1,3);
  layout->addWidget(std::get<2>(labels),1,4);
  layout->addWidget(std::get<3>(labels),1,5);
  layout->addWidget(std::get<3>(spinboxes),1,6);
  layout->addWidget(std::get<3>(sliders),1,7);
  layout->addWidget(std::get<4>(spinboxes),1,8);
  layout->addWidget(std::get<4>(sliders),1,9);
}

void FluoTabWidget::addRow2(QGridLayout *layout, const VolumeSliders &sliders,
                            const VolumeSpinboxes &spinboxes, const VolumeLabels &labels)
{
  layout->addWidget(std::get<5>(sliders),2,0,1,3);
  layout->addWidget(std::get<5>(spinboxes),2,3);
  layout->addWidget(std::get<4>(labels),2,4);
  layout->addWidget(std::get<5>(labels),2,5);
  layout->addWidget(std::get<6>(spinboxes),2,6);
  layout->addWidget(std::get<6>(sliders),2,7,1,3);
}

void FluoTabWidget::addRow3(QGridLayout *layout, const VolumeSliders &sliders,
                            const VolumeSpinboxes &spinboxes, const VolumeLabels &labels)
{
  layout->addWidget(std::get<7>(sliders),3,0,1,3);
  layout->addWidget(std::get<7>(spinboxes),3,3);
  layout->addWidget(std::get<6>(labels),3,4);
  layout->addWidget(std::get<7>(labels),3,5);
  layout->addWidget(std::get<8>(spinboxes),3,6);
  layout->addWidget(std::get<8>(sliders),3,7,1,3);
}

void FluoTabWidget::addRow4(QGridLayout *layout, const VolumeSliders &sliders,
                            const VolumeSpinboxes &spinboxes, const VolumeLabels &labels)
{
  layout->addWidget(std::get<9>(sliders),4,0);
  layout->addWidget(std::get<9>(spinboxes),4,1);
  layout->addWidget(std::get<10>(sliders),4,2);
  layout->addWidget(std::get<10>(spinboxes),4,3);
  layout->addWidget(std::get<8>(labels),4,4);
  layout->addWidget(std::get<9>(labels),4,5);
  layout->addWidget(std::get<11>(spinboxes),4,6);
  layout->addWidget(std::get<11>(sliders),4,7);
  layout->addWidget(std::get<12>(spinboxes),4,8);
  layout->addWidget(std::get<12>(sliders),4,9);
}


