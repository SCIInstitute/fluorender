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
#include <Undoable.h>
#include <HistoryIndicator.h>
#include <cmath>
#include <Debug.h>

double Undoable::time_span_ = 1;

Undoable::Undoable() :
	stack_pointer_(-1)
{
}

void Undoable::Clear()
{
	if (stack_.size() > 1)
	{
		stack_pointer_ = 0;
		stack_.erase(stack_.begin() + 1, stack_.end());

		//history
		if (indicator_)
		{
			indicator_->SetLength(0);
			indicator_->SetPosition(0);
			indicator_->UpdateHistory();
		}
	}
}

double Undoable::GetTimeUndo()
{
	size_t size = stack_.size();
	if (!size || stack_pointer_ < 1 || stack_pointer_ >= size)
		return 0;
	return stack_[stack_pointer_].first;
}

double Undoable::GetTimeRedo()
{
	size_t size = stack_.size();
	if (!size || stack_pointer_ == -1 || stack_pointer_ + 1 >= size)
		return std::numeric_limits<double>::max();
	return stack_[stack_pointer_ + 1].first;
}

void Undoable::Undo()
{
	if (stack_.empty())
		return;
	size_t size = stack_.size();
	if (stack_pointer_ == 0 ||
		stack_pointer_ == -1 ||
		stack_pointer_ > size - 1)
		return;

	//move pointer
	stack_pointer_--;

	//update
	update();

	if (indicator_)
	{
		indicator_->SetPosition(stack_pointer_);
		indicator_->UpdateHistory();
	}
}

void Undoable::Redo()
{
	size_t size = stack_.size();
	if (!size ||
		size == 1)
		return;
	if (stack_pointer_ == -1 ||
		stack_pointer_ > size - 2)
		return;

	//move pointer
	stack_pointer_++;

	//update
	update();

	if (indicator_)
	{
		indicator_->SetPosition(stack_pointer_);
		indicator_->UpdateHistory();
	}
}

void Undoable::pop()
{
	if (!stack_.empty())
	{
		size_t size = stack_.size();
		if (stack_pointer_ == size - 1)
			stack_pointer_--;
		stack_.pop_back();

		if (indicator_)
		{
			indicator_->SetLength(size);
			indicator_->SetPosition(stack_pointer_);
			indicator_->UpdateHistory();
		}
	}
}

double Undoable::get_time_span()
{
	return time_span_;
}

bool Undoable::time_sample(double& t)
{
	std::chrono::duration<double> time_span =
		std::chrono::system_clock::now().time_since_epoch();
	t = time_span.count();
	if (stack_.empty())
		return true;
	double d = std::fabs(t - stack_[stack_pointer_].first);
	return d > get_time_span();
}