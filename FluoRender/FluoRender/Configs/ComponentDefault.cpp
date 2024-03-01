/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <ComponentDefault.h>
#include <Names.h>

ComponentDefault::ComponentDefault()
{
}

ComponentDefault::~ComponentDefault()
{

}

void ComponentDefault::Read(wxFileConfig& f)
{
	double dval;
	int ival;
	bool bval;

	if (f.Exists("/comp default"))
		f.SetPath("/comp default");

	//selected only
	//if (f.Read("ca_select_only", &bval))
	//	m_ca_select_only_chk->SetValue(bval);
	////min voxel
	//if (f.Read("ca_min", &ival))
	//{
	//	str = wxString::Format("%d", ival);
	//	m_ca_min_text->SetValue(str);
	//}
	////max voxel
	//if (f.Read("ca_max", &ival))
	//{
	//	str = wxString::Format("%d", ival);
	//	m_ca_max_text->SetValue(str);
	//}
	////ignore max
	//if (f.Read("ca_ignore_max", &bval))
	//{
	//	m_ca_ignore_max_chk->SetValue(bval);
	//	if (bval)
	//		m_ca_max_text->Disable();
	//	else
	//		m_ca_max_text->Enable();
	//}
}

void ComponentDefault::Save(wxFileConfig& f)
{

}

