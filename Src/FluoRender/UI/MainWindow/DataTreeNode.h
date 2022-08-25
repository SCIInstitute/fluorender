#ifndef DATATREENODE_H
#define DATATREENODE_H

#include "DatasetAttributes.h"

#include <string>
#include <list>

class DataTreeNode
{
public:
    DataTreeNode() {};
    ~DataTreeNode()
    {
        for(auto child : children)
            delete child;
    };

    std::string name{""};
    std::string fullname{""};
    DatasetAttributes::DatasetType type{DatasetAttributes::Unknown};
    int color[3] = {255, 255, 255};
    bool selected {false};
    bool active {false};
    bool visible{true};
    bool mask{false};
    std::list<DataTreeNode *> children;
};

#endif // DATATREENODE_H
