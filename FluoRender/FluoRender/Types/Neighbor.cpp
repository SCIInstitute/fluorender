/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <Neighbor.h>

using namespace fluo;

const Point Neighbor::begin()
{
	x_ = -int(n_.x());
	y_ = -int(n_.y());
	z_ = -int(n_.z());
	return c_ - n_ * h_;
}

const Point Neighbor::end()
{
	if (n_.length2() == 0.0)
		return Point(0.1);
	return c_ + n_ * (h_ + h_.unit_sign() * Vector(0.1));
}

const Point Neighbor::operator++()
{
	int nx = int(n_.x());
	int ny = int(n_.y());
	int nz = int(n_.z());
	x_++;
	if (x_ > nx)
	{
		x_ = -nx;
		y_++;
	}
	if (y_ > ny)
	{
		y_ = -ny;
		z_++;
	}
	if (z_ > nz)
	{
		return end();
	}
	return c_ + Vector(x_, y_, z_) * h_;
}

const Point Neighbor::operator++(int i)
{
	int nx = int(n_.x());
	int ny = int(n_.y());
	int nz = int(n_.z());
	x_++;
	if (x_ > nx)
	{
		x_ = -nx;
		y_++;
	}
	if (y_ > ny)
	{
		y_ = -ny;
		z_++;
	}
	if (z_ > nz)
	{
		return end();
	}
	return c_ + Vector(x_, y_, z_) * h_;
}

const Point Range::begin()
{
	x_ = 0;
	y_ = 0;
	z_ = 0;
	return c_;
}

const Point Range::end()
{
	return c_ + n_ * (h_ + h_.unit_sign() * Vector(0.1));
}

const Point Range::operator++()
{
	int nx = int(n_.x());
	int ny = int(n_.y());
	int nz = int(n_.z());
	x_++;
	if (x_ > nx)
	{
		x_ = 0;
		y_++;
	}
	if (y_ > ny)
	{
		y_ = 0;
		z_++;
	}
	if (z_ > nz)
	{
		return end();
	}
	return c_ + Vector(x_, y_, z_) * h_;
}

const Point Range::operator++(int i)
{
	int nx = int(n_.x());
	int ny = int(n_.y());
	int nz = int(n_.z());
	x_++;
	if (x_ > nx)
	{
		x_ = 0;
		y_++;
	}
	if (y_ > ny)
	{
		y_ = 0;
		z_++;
	}
	if (z_ > nz)
	{
		return end();
	}
	return c_ + Vector(x_, y_, z_) * h_;
}