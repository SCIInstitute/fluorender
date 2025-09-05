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
#include <AutomateDefault.h>
#include <Names.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

AutomateDefault::AutomateDefault()
{
	m_histogram = 2;
	m_paint_size = 2;
	m_comp_gen = 2;
	m_colocalize = 2;
	m_relax_ruler = 0;
	m_conv_vol_mesh = 2;
}

AutomateDefault::~AutomateDefault()
{

}

void AutomateDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/automate default"))
		f->SetPath("/automate default");

	//histogram
	f->Read("histogram", &m_histogram, 2);
	//paint size
	f->Read("paint size", &m_paint_size, 2);
	//component generation
	f->Read("comp gen", &m_comp_gen, 2);
	//colocalization
	f->Read("colocalize", &m_colocalize, 2);
	//relax ruler
	f->Read("relax ruler", &m_relax_ruler, 0);
	//convert volume to mesh
	f->Read("conv vol mesh", &m_conv_vol_mesh, 2);
}

void AutomateDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/automate default");

	//histogram
	f->Write("histogram", m_histogram);
	//paint size
	f->Write("paint size", m_paint_size);
	//component generation
	f->Write("comp gen", m_comp_gen);
	//colocalization
	f->Write("colocalize", m_colocalize);
	//relax ruler
	f->Write("relax ruler", m_relax_ruler);
	//convert volume to mesh
	f->Write("conv vol mesh", m_conv_vol_mesh);
}