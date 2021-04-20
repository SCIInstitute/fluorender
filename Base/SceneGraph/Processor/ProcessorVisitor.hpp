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

//the draw volume visitor simply copies code from the old renderglview
//a new render pipeline should replace this
#ifndef PROCESSORVISITOR_HPP
#define PROCESSORVISITOR_HPP

#include <NodeVisitor.hpp>

namespace fluo
{
	enum ProcessorBranchType
	{
		PBT_STOP = 0,
		PBT_01 = (1u << 0),
		PBT_02 = (1u << 1),
		PBT_03 = (1u << 2),
		PBT_04 = (1u << 3),
		PBT_05 = (1u << 4),
		PBT_06 = (1u << 5),
		PBT_07 = (1u << 6),
		PBT_08 = (1u << 7),
		PBT_09 = (1u << 8),
		PBT_10 = (1u << 9),
		PBT_11 = (1u << 10),
		PBT_12 = (1u << 11),
		PBT_13 = (1u << 12),
		PBT_14 = (1u << 13),
		PBT_15 = (1u << 14),
		PBT_16 = (1u << 15),
		PBT_17 = (1u << 16),
		PBT_18 = (1u << 17),
		PBT_19 = (1u << 18),
		PBT_20 = (1u << 19),
		PBT_21 = (1u << 20),
		PBT_22 = (1u << 21),
		PBT_23 = (1u << 22),
		PBT_24 = (1u << 23),
		PBT_25 = (1u << 24),
		PBT_26 = (1u << 25),
		PBT_27 = (1u << 26),
		PBT_28 = (1u << 27),
		PBT_29 = (1u << 28),
		PBT_30 = (1u << 29),
		PBT_31 = (1u << 30),
		PBT_32 = (1u << 31),
		PBT_DONTCARE = 0xffffffff
	};
	class ProcessorVisitor : public NodeVisitor
	{
	public:
		ProcessorVisitor(Event& event) :
			NodeVisitor(),
			event_(event)
		{
			setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
		}

		virtual void apply(Node& node);

		virtual void apply(Group& group);

	private:
		Event event_;
	};
}
#endif//PROCESSORVISITOR_HPP
