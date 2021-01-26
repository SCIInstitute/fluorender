/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef _INFOVISITOR_H_
#define _INFOVISITOR_H_

#include <Scenegraph/NodeVisitor.h>
#include <Scenegraph/Group.h>
#include <iostream>
#include <string>

namespace fls
{
	class InfoVisitor : public fls::NodeVisitor
	{
	public:
		InfoVisitor() : level_(0)
		{
			setTraversalMode(fls::NodeVisitor::TRAVERSE_ALL_CHILDREN);
		}

		std::string spaces()
		{
			return std::string(level_ * 2, ' ');
		}

		virtual void apply(fls::Node& node)
		{
			std::cout << spaces() << node.className() <<
				"-" << node.getName() << "\t" << &node << std::endl;
			std::cout << spaces() << "  Values: ";
			printValues(&node);
			std::cout << std::endl;
			level_++;
			traverse(node);
			level_--;
		}

		virtual void apply(fls::Group& group)
		{
			std::cout << spaces() << group.className() <<
				"-" << group.getName() << "\t" << &group << std::endl;
			std::cout << spaces() << "  Values: ";
			printValues(&group);
			std::cout << std::endl;

			level_++;
			traverse(group);
			level_--;
		}

	protected:
		unsigned int level_;

		void printValues(fls::Object* object)
		{
			if (!object)
				return;
			//get all value names
			std::vector<std::string> names =
				object->getValueNames();
			for (auto it = names.begin();
				it != names.end(); ++it)
			{
				Value* value = object->getValue(*it);
				if (value)
					std::cout << *it << "(" <<
					value->getType() << ")[" <<
					*value << "] ";
			}
		}
	};
}
#endif//_INFOVISITOR_H_