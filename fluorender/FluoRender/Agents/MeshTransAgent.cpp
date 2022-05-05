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

#include <MeshTransAgent.hpp>
#include <MeshTransPanel.h>

using namespace fluo;

MeshTransAgent::MeshTransAgent(MeshTransPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
	setupInputs();
}

void MeshTransAgent::setupInputs()
{
	inputs_ = ValueCollection
	{
		TransX,
		TransY,
		TransZ,
		RotX,
		RotY,
		RotZ,
		ScaleX,
		ScaleY,
		ScaleZ
	};
}

void MeshTransAgent::setObject(MeshData* obj)
{
	InterfaceAgent::setObject(obj);
}

MeshData* MeshTransAgent::getObject()
{
	return dynamic_cast<MeshData*>(InterfaceAgent::getObject());
}

void MeshTransAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
	wxString str;

	if (update_all || FOUND_VALUE(TransX))
	{
		double x;
		getValue(TransX, x);
		str = wxString::Format("%.2f", x);
		panel_.m_x_trans_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(TransY))
	{
		double y;
		getValue(TransY, y);
		str = wxString::Format("%.2f", y);
		panel_.m_y_trans_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(TransZ))
	{
		double z;
		getValue(TransZ, z);
		str = wxString::Format("%.2f", z);
		panel_.m_z_trans_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(RotX))
	{
		double x;
		getValue(RotX, x);
		str = wxString::Format("%.2f", x);
		panel_.m_x_rot_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(RotY))
	{
		double y;
		getValue(RotY, y);
		str = wxString::Format("%.2f", y);
		panel_.m_y_rot_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(RotZ))
	{
		double z;
		getValue(RotZ, z);
		str = wxString::Format("%.2f", z);
		panel_.m_z_rot_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(ScaleX))
	{
		double x;
		getValue(ScaleX, x);
		str = wxString::Format("%.2f", x);
		panel_.m_x_scl_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(ScaleY))
	{
		double y;
		getValue(ScaleY, y);
		str = wxString::Format("%.2f", y);
		panel_.m_y_scl_text->SetValue(str);
	}
	if (update_all || FOUND_VALUE(ScaleZ))
	{
		double z;
		getValue(ScaleZ, z);
		str = wxString::Format("%.2f", z);
		panel_.m_z_scl_text->SetValue(str);
	}
}