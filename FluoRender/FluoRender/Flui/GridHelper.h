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
#ifndef GridHelper_h
#define GridHelper_h

#include <GridData.h>
#include <wx/grid.h>
#include <set>

class GridHelper
{
public:
	static void Populate(
		wxGrid* grid,
		const GridData& data,
		const GridPopulateOptions& options = {});

	static void Clear(
		wxGrid* grid);

	static GridSelection GetSelection(
		wxGrid* grid);

	static void SelectRows(
		wxGrid* grid,
		const std::set<int>& rows);

	static std::string CopySelection(
		wxGrid* grid);

	static void PasteSelection(
		wxGrid* grid,
		const std::string& text);

	static std::set<int> GetSelectedRows(
		wxGrid* grid);

	static std::string GetCellText(
		wxGrid* grid,
		int row,
		int col);

	static bool GetCellULong(
		wxGrid* grid,
		int row,
		int col,
		unsigned long& value);

	static bool GetCellULLong(
		wxGrid* grid,
		int row,
		int col,
		unsigned long long& value);
};

#endif//GridHelper_h