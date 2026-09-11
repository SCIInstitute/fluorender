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
#include <GridBuilder.h>
#include <VolumeData.h>
#include <compatibility.h>
#include <sstream>

std::vector<std::string> GridBuilder::ParseLine(
	const std::string& line,
	char delimiter)
{
	std::vector<std::string> result;

	std::stringstream ss(line);
	std::string field;

	while (std::getline(ss, field, delimiter))
		result.push_back(field);

	// preserve trailing empty field
	if (!line.empty() && line.back() == delimiter)
		result.emplace_back();

	return result;
}

GridData GridBuilder::Build(
	const std::string& titles,
	const std::string& values)
{
	GridData result;

	//----------------------------------------
	// columns
	//----------------------------------------

	result.columns = ParseLine(titles, '\t');

	//----------------------------------------
	// rows
	//----------------------------------------

	std::stringstream ss(values);
	std::string line;

	while (std::getline(ss, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		GridRowData row;

		auto fields = ParseLine(line, '\t');

		row.cells.reserve(fields.size());

		for (const auto& field : fields)
		{
			GridCellData cell;
			cell.text = field;

			row.cells.push_back(std::move(cell));
		}

		result.rows.push_back(std::move(row));
	}

	return result;
}

void GridFormatter::ApplyComponentColors(
	GridData& data,
	int shuffle)
{
	// Find the ID column.
	auto it = std::find(
		data.columns.begin(),
		data.columns.end(),
		"ID");

	if (it == data.columns.end())
		return;

	size_t id_col =
		std::distance(
			data.columns.begin(),
			it);

	for (auto& row : data.rows)
	{
		if (id_col >= row.cells.size())
			continue;

		unsigned long id;
		if (!TryToULong(
			row.cells[id_col].text, id))
		{
			row.cells[id_col].has_bg_color = true;
			row.cells[id_col].bg_color =
				fluo::Color(1.0, 1.0, 1.0);
			continue;
		}

		row.cells[id_col].has_bg_color = true;
		row.cells[id_col].bg_color =
			fluo::Color(
				id,
				shuffle);
	}
}