#ifndef FLUO_COLORED_LINE
#define FLUO_COLORED_LINE

#include <QFrame>
#include <QString>

#include <unordered_map>
#include <string_view>

class FluoColoredLine : public QFrame
{
  Q_OBJECT
  
  public:

    FluoColoredLine(const QFrame::Shape &shape, std::string_view color)
    {
      this->setFrameShape(shape);
      this->setObjectName("ColorFrame");
      this->setStyleSheet(colors.at(color));
      this->setLineWidth(5);
    }

  private:

    const std::unordered_map<std::string_view,const QString> colors =
    {
      {"Red","#ColorFrame { border: 5px solid rgb(255,0,0); }"},
      {"Green","#ColorFrame { border: 5px solid rgb(0,255,17); }"},
      {"Blue","#ColorFrame { border: 5px solid rgb(0,0,255); }"},
      {"Salmon","#ColorFrame { border: 5px solid rgb(255,145,164); }"},
      {"Magenta","#ColorFrame { border: 5px solid rgb(255,85,255); }"},
      {"Yellow","#ColorFrame { border: 5px solid rgb(255,255,0); }"},
      {"Purple","#ColorFrame { border: 5px solid rgb(170,170,255); }"},
      {"Teal","#ColorFrame { border: 5px solid rgb(0,255,255); }"}
    };

    QFrame::Shape frameShape;
    QString backGroundColor;


};

#endif
