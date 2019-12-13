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
      this->setStyleSheet(colors.at(color));
      this->setLineWidth(5);
    }

  private:

    const std::unordered_map<std::string_view,const QString> colors =
    {
      {"Red","background-color: rgb(255,0,0);"},
      {"Green","background-color: rgb(0,255,17);"},
      {"Blue","background-color: rgb(0,0,255);"},
      {"Salmon","background-color: rgb(255,145,164);"},
      {"Magenta","background-color: rgb(255,85,255);"},
      {"Yellow","background-color: rgb(255,255,0);"},
      {"Purple","background-color: rgb(170,170,255);"},
      {"Teal","background-color: rgb(0,255,255);"}
    };

    QFrame::Shape frameShape;
    QString backGroundColor;


};

#endif
