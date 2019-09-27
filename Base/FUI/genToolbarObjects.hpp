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
#include <QStringList>

std::unique_ptr<QSlider> genSlider(Qt::Orientation ori, int floor, int ceiling);

std::unique_ptr<QLabel> genLabel(const QString &text);


std::unique_ptr<QAction> genActionButton(const QString &imgName);

std::unique_ptr<QComboBox> genComboBox(const QStringList &items);

template<class SpinBoxType, typename V>
inline std::unique_ptr<SpinBoxType> genSpinBox(V floor, V ceiling)
{
  auto newSpinBox = std::make_unique<SpinBoxType>();
  newSpinBox->setRange(floor,ceiling);
  newSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
  return newSpinBox;
}


#endif
