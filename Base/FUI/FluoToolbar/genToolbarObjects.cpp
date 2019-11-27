#include "genToolbarObjects.hpp"

QSlider* genSlider(Qt::Orientation ori, int floor, int ceiling)
{
  auto newSlider = new QSlider();
  newSlider->setOrientation(ori);
  newSlider->setRange(floor,ceiling);

  return newSlider;
}

QLabel* genLabel(const QString &text)
{
  auto newLabel = new QLabel(text);
  return newLabel;
}

QAction* genActionButton(const QString &imgName)
{
  auto newActionButton = new QAction();
  newActionButton->setIcon(QIcon(imgName));

  return newActionButton;
}

QComboBox* genComboBox(const QStringList &items)
{
  auto newComboBox = new QComboBox();
  newComboBox->addItems(items);

  return newComboBox;
}
