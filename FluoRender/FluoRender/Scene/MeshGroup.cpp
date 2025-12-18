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
#include <MeshGroup.h>
#include <MeshData.h>

int MeshGroup::m_num = 0;
MeshGroup::MeshGroup()
{
	type = 6;//mesh group
	m_num++;
	m_name = L"MGroup " + std::to_wstring(m_num);
	m_disp = true;
	m_sync_mesh_prop = false;
}

MeshGroup::~MeshGroup()
{
}

//randomize color
void MeshGroup::RandomizeColor()
{
	for (auto& it : m_md_list)
	{
		if (it)
		{
			double hue = (double)std::rand()/(RAND_MAX) * 360.0;
			fluo::Color color(fluo::HSVColor(hue, 1.0, 1.0));
			it->SetColor(color);
		}
	}
}

void MeshGroup::SetOutline(bool val)
{
	TreeLayer::SetOutline(val);
	for (auto& it : m_md_list)
	{
		if (it)
			it->SetOutline(val);
	}
}