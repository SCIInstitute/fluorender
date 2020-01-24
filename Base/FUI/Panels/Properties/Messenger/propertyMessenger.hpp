#ifndef PROPERTY_MESSENGER_HPP
#define PROPERTY_MESSENGER_HPP

#include <QObject>
#include <any>

class PropertyMessenger : public QObject
{
  Q_OBJECT

  signals:
    void sendGammaMessageToPanel(std::any value);
    void sendGammaMessageToAgent(std::any value);

  public:
    PropertyMessenger();

    void gammaMessageFromAgent(std::any value) { emit sendGammaMessageToPanel(value); }
    void gammaMessageFromPanel(std::any value) { emit sendGammaMessageToAgent(value); }

  private:

};

#endif
