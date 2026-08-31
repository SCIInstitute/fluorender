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

#include <MeasureDlgAgent.h>
#include <MeasureDlg.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <RenderView.h>
#include <RulerHandler.h>
#include <Ruler.h>
#include <RulerList.h>
#include <MainSettings.h>
#include <RulerAlign.h>

MeasureDlgAgent::MeasureDlgAgent(
	MeasureDlg* dlg) :
	Agent(dlg)
{

}

bool MeasureDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MeasureDlgAgent::Update(
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

void MeasureDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty();

	int ival;

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
		dlg->UpdateFreehandToolState(int_mode, rul_mode);
	}

	if (update_all || FOUND_VALUE(gstRulerList))
	{
		dlg->UpdateRulerList();
	}

	if (FOUND_VALUE(gstRulerListCur))
	{
		dlg->UpdateRulerListCur();
	}

	if (update_all || FOUND_VALUE(gstRulerListDisp))
	{
		RulerListDisplayInfo info;
		auto list = glbin_current.GetRulerList();
		auto ruler_list = list->get();
		if (!ruler_list.IsEmpty())
		{
			for (size_t i = 0; i < ruler_list.size(); ++i)
			{
				auto ruler = ruler_list.GetRuler(i);
				info.visible.push_back(ruler && ruler->GetDisp());
			}
		}

		dlg->UpdateRulerListDisp(info);
	}

	if (update_all || FOUND_VALUE(gstRulerListSel))
	{
		ival = glbin_ruler_handler.GetRulerIndex();
		dlg->UpdateRulerListSel(ival);
	}

	if (FOUND_VALUE(gstRulerGroupSel))
	{
		dlg->UpdateGroupSel();
	}

	if (update_all || FOUND_VALUE(gstRulerProfile))
	{
		dlg->UpdateProfile();
	}

	if (update_all || FOUND_VALUE(gstRulerMethod))
	{
		ival = glbin_settings.m_point_volume_mode;
		dlg->UpdateRulerMethod(ival);
	}

	if (update_all || FOUND_VALUE(gstRulerTransient))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			dlg->UpdateRulerTransient(ruler->GetTransient());
		}
	}

	if (update_all || FOUND_VALUE(gstRulerUseTransf))
	{
		dlg->UpdateRulerUseTransf(glbin_settings.m_ruler_use_transf);
	}

	if (update_all || FOUND_VALUE(gstRulerDisp))
	{
		auto ruler = glbin_current.GetRuler();
		bool bval0 = false;
		bool bval1 = false;
		bool bval2 = false;
		if (ruler)
		{
			if (ruler->GetDisp())
			{
				bval0 = ruler->GetDisplay(0);
				bval1 = ruler->GetDisplay(1);
				bval2 = ruler->GetDisplay(2);
			}
		}
		dlg->UpdateRulerDisp(bval0, bval1, bval2);
	}

	if (update_all || FOUND_VALUE(gstRulerRelaxType))
	{
		dlg->UpdateRulerRelaxType(glbin_settings.m_ruler_relax_type);
	}

	if (update_all || FOUND_VALUE(gstRulerF1))
	{
		dlg->UpdateRulerF1(glbin_settings.m_ruler_relax_f1);
	}

	if (update_all || FOUND_VALUE(gstRulerInterpolation))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			ival = ruler->GetInterp();
			dlg->UpdateRulerInterpolation(ival);
		}
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bool bval = glbin_aligner.GetAlignCenter();
		dlg->UpdateAlignCenter(bval);
	}
}

void MeasureDlgAgent::UpdateData(const UpdateRequest& request)
{

}

MeasureDlg* MeasureDlgAgent::GetDialog() const
{
	return static_cast<MeasureDlg*>(GetWindow());
}

