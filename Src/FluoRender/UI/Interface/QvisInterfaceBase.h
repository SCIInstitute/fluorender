#ifndef QVISINTERFACEBASE_H
#define QVISINTERFACEBASE_H

#ifdef QT_CREATOR_ONLY
  class InterfaceAgent;
#else
  namespace fluo {
    class InterfaceAgent;
  }
  using namespace fluo;
#endif

class QvisInterfaceBase
{
public:
    QvisInterfaceBase();

    virtual void SetAgent(InterfaceAgent* agent) = 0;

    virtual void updateWindow(bool doAll = true) = 0;
};

#endif // QVISINTERFACEBASE_H
