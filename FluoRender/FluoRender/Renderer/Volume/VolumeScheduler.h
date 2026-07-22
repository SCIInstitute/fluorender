/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#ifndef VolumeScheduler_h
#define VolumeScheduler_h

#include "VolumeTypes.h"

namespace flvr
{
	class VolumeScheduler
	{
	public:
		void reset();

		void push_finished_bricks(
			size_t count);

		void set_corrected_time(
			int mouse_speed);

		unsigned long time_budget() const;

		RenderBudget build_budget(
			bool interactive,
			double sample_rate);

	protected:
		int estimate_bricks(
			int mode,
			int init) const;

	private:
		BrickQueue history_{ 5 };

		unsigned long corrected_time_ = 100;

		size_t finished_bricks_ = 0;
	};
}

#endif