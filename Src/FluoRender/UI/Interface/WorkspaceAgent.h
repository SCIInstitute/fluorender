#ifndef WORKSPACEAGENT_H
#define WORKSPACEAGENT_H

#include <string>
#include <list>

class DataTreeNode;

class WorkspaceAgent
{
public:
     WorkspaceAgent() {};
     ~WorkspaceAgent() {};

     void setDataTree(DataTreeNode * dataTreeNode) { mDatatreeRoot = dataTreeNode; };
     DataTreeNode * getDataTree() const { return mDatatreeRoot; };

private:
     DataTreeNode *mDatatreeRoot {nullptr};
};

#endif // WORKSPACEAGENT_H
