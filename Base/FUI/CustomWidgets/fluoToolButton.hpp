#ifndef FLUO_TOOL_BUTTON
#define FLUO_TOOL_BUTTON

#include <QToolButton>

class FluoToolButton : public QToolButton
{
  Q_OBJECT
  public:
    FluoToolButton(const QString &text, bool isChecked)
    {
      this->setCheckable(true);
      this->setChecked(isChecked);
      this->setAutoRaise(true);
      this->setText(text);
    }
};

#endif
