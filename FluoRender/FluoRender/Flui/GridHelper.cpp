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

#include <GridHelper.h>

void GridHelper::Populate(
	wxGrid* grid,
	const GridData& data,
	const GridPopulateOptions& options)
{
	if (!grid)
		return;

	grid->BeginBatch();

	// columns
	size_t col_count = data.columns.size();

	if (grid->GetNumberCols() < static_cast<int>(col_count))
	{
		grid->AppendCols(
			static_cast<int>(col_count) -
			grid->GetNumberCols());
	}

	for (size_t col = 0; col < col_count; ++col)
	{
		grid->SetColLabelValue(
			static_cast<int>(col),
			wxString(data.columns[col]));
	}

	// rows
	int start_row = 0;

	if (options.append_rows)
		start_row = grid->GetNumberRows();

	int row_count = static_cast<int>(data.rows.size());

	if (options.max_rows > 0)
		row_count = std::min(row_count, options.max_rows);

	int needed_rows = start_row + row_count;

	if (grid->GetNumberRows() < needed_rows)
	{
		grid->AppendRows(
			needed_rows -
			grid->GetNumberRows());
	}

	// populate cells
	for (int row = 0; row < row_count; ++row)
	{
		const auto& src_row = data.rows[row];
		int dst_row = start_row + row;

		for (size_t col = 0; col < src_row.cells.size(); ++col)
		{
			const auto& cell = src_row.cells[col];

			grid->SetCellValue(
				dst_row,
				static_cast<int>(col),
				wxString(cell.text));

			if (cell.has_bg_color)
			{
				wxColor color(
					cell.bg_color.r() * 255,
					cell.bg_color.g() * 255,
					cell.bg_color.b() * 255);

				grid->SetCellBackgroundColour(
					dst_row,
					static_cast<int>(col),
					color);
			}
			else
			{
				grid->SetCellBackgroundColour(
					dst_row,
					static_cast<int>(col),
					*wxWHITE);
			}
		}
	}

	// cleanup
	if (!options.append_rows)
	{
		if (options.remove_extra_cols &&
			grid->GetNumberCols() > static_cast<int>(col_count))
		{
			grid->DeleteCols(
				static_cast<int>(col_count),
				grid->GetNumberCols() -
				static_cast<int>(col_count));
		}

		if (options.remove_extra_rows &&
			grid->GetNumberRows() > row_count)
		{
			grid->DeleteRows(
				row_count,
				grid->GetNumberRows() - row_count);
		}
	}

	grid->EndBatch();
}

void GridHelper::Clear(wxGrid* grid)
{
	if (!grid)
		return;

	int cols = grid->GetNumberCols();
	if (cols > 0)
		grid->DeleteCols(0, cols);

	int rows = grid->GetNumberRows();
	if (rows > 0)
		grid->DeleteRows(0, rows);
}

GridSelection GridHelper::GetSelection(wxGrid* grid)
{
	GridSelection result;

	if (!grid)
		return result;

	result.rows = GetSelectedRows(grid);

	const auto& cells = grid->GetSelectedCells();

	for (const auto& cell : cells)
	{
		result.cells.push_back(
			{
				cell.GetRow(),
				cell.GetCol()
			});
	}

	return result;
}

void GridHelper::SelectRows(
	wxGrid* grid,
	const std::set<int>& rows)
{
	if (!grid)
		return;

	grid->ClearSelection();

	int last_row = -1;

	for (int row : rows)
	{
		if (row >= 0 &&
			row < grid->GetNumberRows())
		{
			grid->SelectRow(row, true);
			last_row = row;
		}
	}

	if (last_row >= 0)
		grid->GoToCell(last_row, 0);
}

std::set<int> GridHelper::GetSelectedRows(wxGrid* grid)
{
	std::set<int> result;

	if (!grid)
		return result;

	// selected rows explicitly
	wxArrayInt sel_rows = grid->GetSelectedRows();

	for (size_t i = 0; i < sel_rows.Count(); ++i)
		result.insert(sel_rows[i]);

	// selected cells
	const auto& cells = grid->GetSelectedCells();

	for (const auto& cell : cells)
		result.insert(cell.GetRow());

	// selected blocks
	const auto& top_left =
		grid->GetSelectionBlockTopLeft();

	const auto& bottom_right =
		grid->GetSelectionBlockBottomRight();

	for (size_t i = 0; i < top_left.size(); ++i)
	{
		for (int row = top_left[i].GetRow();
			row <= bottom_right[i].GetRow();
			++row)
		{
			result.insert(row);
		}
	}

	// cursor fallback
	if (result.empty())
	{
		int row = grid->GetGridCursorRow();

		if (row >= 0)
			result.insert(row);
	}

	return result;
}

std::string GridHelper::CopySelection(wxGrid* grid)
{
	if (!grid)
		return "";

	wxString result;

	for (int row = 0; row < grid->GetNumberRows(); ++row)
	{
		bool row_has_data = false;

		for (int col = 0; col < grid->GetNumberCols(); ++col)
		{
			if (!grid->IsInSelection(row, col))
				continue;

			if (!row_has_data)
			{
				if (!result.IsEmpty())
					result += "\n";

				row_has_data = true;
			}
			else
				result += "\t";

			result += grid->GetCellValue(row, col);
		}
	}

	return result.ToStdString();
}

void GridHelper::PasteSelection(
	wxGrid* grid,
	const std::string& text)
{
	if (!grid)
		return;

	wxString copy_data(text);

	int start_row = grid->GetGridCursorRow();
	int start_col = grid->GetGridCursorCol();

	int row = start_row;

	while (!copy_data.IsEmpty())
	{
		wxString line = copy_data.BeforeFirst('\n');
		copy_data = copy_data.AfterFirst('\n');

		int col = start_col;

		while (!line.IsEmpty())
		{
			wxString field = line.BeforeFirst('\t');
			line = line.AfterFirst('\t');

			if (row >= grid->GetNumberRows())
				grid->AppendRows();

			if (col >= grid->GetNumberCols())
				grid->AppendCols();

			grid->SetCellValue(row, col, field);

			++col;
		}

		++row;
	}
}

std::string GridHelper::GetCellText(
	wxGrid* grid,
	int row,
	int col)
{
	if (!grid)
		return "";

	if (row < 0 || row >= grid->GetNumberRows())
		return "";

	if (col < 0 || col >= grid->GetNumberCols())
		return "";

	return grid->GetCellValue(row, col).ToStdString();
}

bool GridHelper::GetCellULong(
	wxGrid* grid,
	int row,
	int col,
	unsigned long& value)
{
	if (!grid)
		return false;

	return grid->GetCellValue(row, col).
		ToULong(&value);
}

bool GridHelper::GetCellULLong(
	wxGrid* grid,
	int row,
	int col,
	unsigned long long& value)
{
	if (!grid)
		return false;

	return grid->GetCellValue(row, col).
		ToULongLong(&value);
}
