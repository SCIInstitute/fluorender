#ifndef _WORKSPACEAGENT_H_
#define _WORKSPACEAGENT_H_

#include "InterfaceAgent.hpp"

class DataTreeNode;

namespace fluo
{
  class WorkspaceAgent : public InterfaceAgent
  {
  public:
    WorkspaceAgent() {};
    ~WorkspaceAgent() {};
    
    virtual void UpdateFui(const ValueCollection &names = {}) {};

    void setDataTree(DataTreeNode * dataTreeNode) { mDatatreeRoot = dataTreeNode; };
    DataTreeNode * getDataTree() const { return mDatatreeRoot; };

  protected:
    virtual void setupInputs() {};
    
  private:
    DataTreeNode *mDatatreeRoot {nullptr};
  };

}

#endif // _WORKSPACEAGENT_H_
