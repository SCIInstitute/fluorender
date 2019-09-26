#ifndef GEN_TOOLBAR_OBJECTS_HPP
#define GEN_TOOLBAR_OBJECTS_HPP

#include <memory>

#include <QSlider>
#include <QLabel>
#include <QAction>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

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

template<class SpinBoxType, typename V>
std::unique_ptr<SpinBoxType> genSpinBox(V floor, V ceiling)
{
  auto newSpinBox = std::make_unique<SpinBoxType>();
  newSpinBox->setRange(floor,ceiling);
  newSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
  return newSpinBox;
}


#endif
