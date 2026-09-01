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

#include <NoiseCancellingDlgAgent.h>
#include <NoiseCancellingDlg.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <ComponentDefault.h>
#include <CompGenerator.h>

NoiseCancellingDlgAgent::NoiseCancellingDlgAgent(
	NoiseCancellingDlg* dlg) :
	Agent(dlg)
{

}

bool NoiseCancellingDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void NoiseCancellingDlgAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void NoiseCancellingDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool update_all = request.values.empty();
	m_max_value = vd->GetMaxValue();

	double dval;
	int ival;
	bool bval;

	if (update_all || FOUND_VALUE(gstNrThresh))
	{
		//threshold
		dval = glbin_comp_def.m_nr_thresh;
		dlg->UpdateNrThresh(dval, m_max_value);
	}

	if (update_all || FOUND_VALUE(gstNrSize))
	{
		//voxel
		ival = glbin_comp_def.m_nr_size;
		auto res = vd->GetResolution();
		dlg->UpdateNrSize(ival, res.intx());
	}

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		bval = glbin_comp_generator.GetUseSel();
		dlg->UpdateUseSelection(bval);
	}

	if (update_all || FOUND_VALUE(gstNrPreview))
	{
		bval = glbin_comp_def.m_nr_preview;
		dlg->UpdateNrPreview(bval);
	}
}

void NoiseCancellingDlgAgent::UpdateData(const UpdateRequest& request)
{

}

NoiseCancellingDlg* NoiseCancellingDlgAgent::GetDialog() const
{
	return static_cast<NoiseCancellingDlg*>(GetWindow());
}
