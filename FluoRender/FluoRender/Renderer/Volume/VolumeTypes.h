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

#ifndef VolumeTypes_h
#define VolumeTypes_h

#include <memory>
#include <vector>

namespace flvr
{
	class Brick;

	struct BrickPriority
	{
		std::shared_ptr<Brick> brick;

		double priority = 0.0;
		double distance = 0.0;

		bool visible = true;
	};

	struct RenderBudget
	{
		size_t max_bricks = 0;

		double sample_rate = 1.0;

		bool interactive = false;
	};

	struct ViewInfo
	{
		fluo::Ray view_ray;

		fluo::Vector view_dir;

		fluo::Point eye;

		double rate_scale = 1.0;
	};
}

#endif