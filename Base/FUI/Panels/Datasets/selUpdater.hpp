#ifndef SEL_UPDATER_HPP
#define SEL_UPDATER_HPP

#include <Node.hpp>
#include <NodeVisitor.hpp>
#include <Group.hpp>

class SelUpdater : public fluo::NodeVisitor
{
  public:
    SelUpdater(fluo::NodeSet &nodes) : NodeVisitor(), nodes_(nodes)
    {
      setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
    }


    void apply(fluo::Node& node)
    {
      bool selected = nodes_.find(&node) != nodes_.end();
      node.setValue("selected",selected);
      traverse(node);
    }

    void apply(fluo::Group& group)
    {
      bool selected = nodes_.find(&group) != nodes_.end();
      group.setValue("selected",selected);
      traverse(group);
    }

  private:
    fluo::NodeSet nodes_;
};

#endif
