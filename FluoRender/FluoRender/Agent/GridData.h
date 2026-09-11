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
#ifndef GridData_h
#define GridData_h

#include <Color.h>
#include <string>
#include <vector>

struct GridCellData
{
	std::string text;

	bool has_bg_color = false;
	fluo::Color bg_color;
};

struct GridRowData
{
	std::vector<GridCellData> cells;
};

struct GridData
{
	std::vector<std::string> columns;
	std::vector<GridRowData> rows;
};

struct GridCellCoord
{
	int row = -1;
	int col = -1;
};

struct GridSelection
{
	std::set<int> rows;
	std::vector<GridCellCoord> cells;
};

struct GridPopulateOptions
{
	bool append_rows = false;
	bool remove_extra_rows = true;
	bool remove_extra_cols = true;
	int max_rows = -1;
};

#endif//GridData_h