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
#include <Global.h>
#include <ColocalDefault.h>
#include <Names.h>

ColocalDefault::ColocalDefault()
{
	m_use_mask = false;
	m_auto_update = false;
	m_method = 2;
	m_int_weighted = false;
	m_get_ratio = false;
	m_physical_size = false;
	m_colormap = false;
	m_cm_min = 0;
	m_cm_max = 1;
}

ColocalDefault::~ColocalDefault()
{

}

void ColocalDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/colocal default"))
		f->SetPath("/colocal default");

	f->Read("use mask", &m_use_mask, false);
	f->Read("auto update", &m_auto_update, false);
	f->Read("method", &m_method, 2);
	f->Read("int weighted", &m_int_weighted, false);
	f->Read("get ratio", &m_get_ratio, false);
	f->Read("physical size", &m_physical_size, false);
	f->Read("colormap", &m_colormap, false);
	f->Read("colormap min", &m_cm_min, 0.0);
	f->Read("colormap max", &m_cm_max, 1.0);
}

void ColocalDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/colocal default");

	f->Write("use mask", m_use_mask);
	f->Write("auto update", m_auto_update);
	f->Write("method", m_method);
	f->Write("int weighted", m_int_weighted);
	f->Write("get ratio", m_get_ratio);
	f->Write("physical size", m_physical_size);
	f->Write("colormap", m_colormap);
	f->Write("colormap min", m_cm_min);
	f->Write("colormap max", m_cm_max);
}
