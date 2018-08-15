#pragma once
#include <Scenegraph/NodeVisitor.h>
#include <Scenegraph/Group.h>
#include <iostream>
#include <string>

class InfoVisitor : public FL::NodeVisitor
{
public:
	InfoVisitor() : level_(0)
	{
		setTraversalMode(FL::NodeVisitor::TRAVERSE_ALL_CHILDREN);
	}

	std::string spaces()
	{
		return std::string(level_ * 2, ' ');
	}

	virtual void apply(FL::Node& node)
	{
		std::cout << spaces() << node.className() << "\t" << &node << std::endl;
		level_++;
		traverse(node);
		level_--;
	}

	virtual void apply(FL::Group& group)
	{
		std::cout << spaces() << group.className() << std::endl;

		level_++;
		traverse(group);
		level_--;
	}

protected:
	unsigned int level_;
};