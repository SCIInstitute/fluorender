#include "genToolbarObjects.hpp"

std::unique_ptr<QSlider> genSlider(Qt::Orientation ori, int floor, int ceiling)
{
  auto newSlider = std::make_unique<QSlider>();
  newSlider->setOrientation(ori);
  newSlider->setRange(floor,ceiling);

  return newSlider;
}

std::unique_ptr<QLabel> genLabel(const QString &text)
{
  auto newLabel = std::make_unique<QLabel>(text);
  return newLabel;
}

std::unique_ptr<QAction> genActionButton(const QString &imgName)
{
  auto newActionButton = std::make_unique<QAction>();
  newActionButton->setIcon(QIcon(imgName));

  return newActionButton;
}

std::unique_ptr<QComboBox> genComboBox(const QStringList &items)
{
  auto newComboBox = std::make_unique<QComboBox>();
  newComboBox->addItems(items);

  return newComboBox;
}
