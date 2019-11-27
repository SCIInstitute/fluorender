#ifndef GEN_TOOLBAR_OBJECTS_HPP
#define GEN_TOOLBAR_OBJECTS_HPP

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

QSlider* genSlider(Qt::Orientation ori, int floor, int ceiling);

QLabel* genLabel(const QString &text);

QAction* genActionButton(const QString &imgName);

QComboBox* genComboBox(const QStringList &items);

template<class SpinBoxType, typename V>
inline SpinBoxType* genSpinBox(V floor, V ceiling)
{
  SpinBoxType* newSpinBox = new SpinBoxType();
  newSpinBox->setRange(floor,ceiling);
  newSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
  return newSpinBox;
}


#endif
