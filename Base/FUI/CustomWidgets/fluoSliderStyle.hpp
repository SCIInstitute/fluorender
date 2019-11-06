#ifndef FLUO_SLIDER_STYLE_HPP
#define FLUO_SLIDER_STYLE_HPP

#include <QProxyStyle>

// This class enables us to have a freely moving slider. Implementation found
// and adopted from: https://stackoverflow.com/a/26281608/5824979

class MyStyle : public QProxyStyle
{
  public:
    using QProxyStyle::QProxyStyle;

    int styleHInt(Qstyle::SyleHint hint, const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const
    {
      if(hint == QStyle::SH_Slider_AbsoluteSetButtons)
        return(Qt::LeftButton | Qt::MidButton | Qt::RightButton);
      return QProxyStyle::styleHint(hint,option,widget,returnData);
    }
};

#endif
