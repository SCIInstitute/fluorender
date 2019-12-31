#ifndef FLUO_TOOL_BUTTON
#define FLUO_TOOL_BUTTON

#include <QToolButton>
#include <QIcon>

class FluoToolButton : public QToolButton
{
  Q_OBJECT
  public:
    FluoToolButton(const QString &image) : FluoToolButton(image,"",false,false,false){}
    FluoToolButton(const QString &text, bool isRaised, bool isCheckable, bool isChecked) : FluoToolButton("",text,isRaised,isCheckable,isChecked){}
    FluoToolButton(const QString &image, const QString &text, bool isRaised, bool isCheckable, bool isChecked)
    {
      this->setCheckable(isCheckable);

      if(image.isEmpty())
      {
        this->setChecked(isChecked);
        this->setAutoRaise(isRaised);
        this->setText(text);
      }
      else
      {
        this->setIcon(QIcon(image));
        this->setAutoRaise(isRaised);
        this->setText(text);
      }
    }
};

#endif
