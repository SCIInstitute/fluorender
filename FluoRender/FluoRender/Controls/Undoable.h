/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _UNDOABLE_H_
#define _UNDOABLE_H_

#include <chrono>
#include <vector>
#include <any>

class Undoable
{
public:
	Undoable();

	virtual void Clear();
	virtual void Undo();
	virtual void Redo();
	virtual double GetTimeUndo();
	virtual double GetTimeRedo();

protected:
	//timer
	static double time_span_;
	//value stack
	size_t stack_pointer_;
	std::vector < std::pair<double, std::any>> stack_;

protected:
	virtual void replace(double t) = 0;
	virtual void push(double t) = 0;
	virtual void pop() = 0;
	virtual void backward() = 0;
	virtual void forward() = 0;

	virtual bool time_sample(double& t);
};

#endif//_UNDOABLE_H_
