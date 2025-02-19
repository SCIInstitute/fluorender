/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <Event.hpp>
#include <Value.hpp>
#include <CopyOp.hpp>

using namespace fluo;

//for delete, changing, changed (general)
void Event::init(EventType tp, Referenced* sndr, bool push_sender)
{
	m_cur_level = 0;
	m_sum_level = 0;
	sender_chain.clear();
	type = tp;
	if (push_sender)
		push(sndr);
	origin = sndr;
}

// value added, changing and changed
void Event::init(EventType tp, Referenced* sndr, Value* va, bool push_sender)
{
	m_cur_level = 0;
	m_sum_level = 0;
	sender_chain.clear();
	type = tp;
	if (push_sender)
		push(sndr);
	origin = sndr;
	value = va;
	if (va)
		value_name = va->getName();
}

// node added and removed
void Event::init(EventType tp, Object* prt, Object* chd, bool push_sender)
{
	m_cur_level = 0;
	m_sum_level = 0;
	sender_chain.clear();
	type = tp;
	if (push_sender)
    {
      //push(prt);
      push(reinterpret_cast<Referenced*>(prt));
    }
    //origin = prt;
    origin = reinterpret_cast<Referenced*>(prt);
	parent = prt;
	child = chd;
}
