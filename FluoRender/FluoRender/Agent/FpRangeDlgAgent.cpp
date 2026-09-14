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

#include <FpRangeDlgAgent.h>
#include <FpRangeDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>

FpRangeDlgAgent::FpRangeDlgAgent(
	FpRangeDlg* dlg) :
	Agent(dlg)
{

}

void FpRangeDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool update_all = request.values.empty();

	double dval;

	if (update_all || request.HasValue(gstFpRangeMin))
	{
		dval = glbin_settings.m_fp_min;
		dlg->UpdateFpRangeMin(dval);
	}
	if (update_all || request.HasValue(gstFpRangeMax))
	{
		dval = glbin_settings.m_fp_max;
		dlg->UpdateFpRangeMax(dval);
	}
}

void FpRangeDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstFpRangeUpdate))
	{
		glbin_settings.m_fp_min = m_fp_min;
		glbin_settings.m_fp_max = m_fp_max;
	}
}

FpRangeDlg* FpRangeDlgAgent::GetDialog() const
{
	return static_cast<FpRangeDlg*>(GetWindow());
}

void FpRangeDlgAgent::SetFpMin(double dval)
{
	m_fp_min = dval;
}

void FpRangeDlgAgent::SetFpMax(double dval)
{
	m_fp_max = dval;
}
