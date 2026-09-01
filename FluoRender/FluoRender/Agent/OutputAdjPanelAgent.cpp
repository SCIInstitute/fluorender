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

#include <OutputAdjPanelAgent.h>
#include <OutputAdjPanel.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <MainSettings.h>
#include <VolumeGroup.h>
#include <VolumeData.h>
#include <ShaderProgram.h>
#include <RenderView.h>

OutputAdjPanelAgent::OutputAdjPanelAgent(
	OutputAdjPanel* dlg) :
	Agent(dlg)
{

}

bool OutputAdjPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void OutputAdjPanelAgent::Update(
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

void OutputAdjPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	int type = glbin_current.GetType();
	if (type == 0 ||
		type == 4 ||
		type == 7 ||
		type == 8)
	{
		if (m_enable_all)
		{
			m_enable_all = false;
			panel->EnableAll(false);
		}
		return;
	}
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty() || FOUND_VALUE(gstCurrentSelect);

	int ival;
	//mf button tips
	if (update_all || FOUND_VALUE(gstMultiFuncTips))
	{
		ival = glbin_settings.m_mulfunc;
	}

	bool bSyncR = FOUND_VALUE(gstSyncR);
	bool bSyncG = FOUND_VALUE(gstSyncG);
	bool bSyncB = FOUND_VALUE(gstSyncB);
	bool bGammaR = FOUND_VALUE(gstGammaR);
	bool bGammaG = FOUND_VALUE(gstGammaG);
	bool bGammaB = FOUND_VALUE(gstGammaB);
	bool bBrightnessR = FOUND_VALUE(gstBrightnessR);
	bool bBrightnessG = FOUND_VALUE(gstBrightnessG);
	bool bBrightnessB = FOUND_VALUE(gstBrightnessB);
	bool bHdrR = FOUND_VALUE(gstEqualizeR);
	bool bHdrG = FOUND_VALUE(gstEqualizeG);
	bool bHdrB = FOUND_VALUE(gstEqualizeB);

	if (FOUND_VALUE(gstUpdateSync))
	{
		UpdateSync();
		bSyncR = bSyncG = bSyncB = true;
	}

	if (!(update_all ||
		bSyncR || bSyncG || bSyncB ||
		bGammaR || bGammaG || bGammaB ||
		bBrightnessR || bBrightnessG || bBrightnessB ||
		bHdrR || bHdrG || bHdrB))
		return;

	fluo::Color gamma;
	fluo::Color brightness;
	fluo::Color hdr;

	switch (type)
	{
	case 1://view
	case 3://mesh
	case 6://mesh group
	{
		auto view = glbin_current.render_view.lock();
		if (view)
		{
			for (int i : {0, 1, 2})
				m_sync[i] = view->GetSync(i);
			gamma = view->GetGammaColor();
			brightness = view->GetBrightness();
			hdr = view->GetHdr();
		}
	}
	break;
	case 2://volume data
	{
		auto vd = glbin_current.vol_data.lock();

		if (vd)
		{
			for (int i : {0, 1, 2})
				m_sync[i] = vd->GetSync(i);
			gamma = vd->GetGammaColor();
			brightness = vd->GetBrightness();
			hdr = vd->GetHdr();
		}
	}
	break;
	case 5://group
	{
		auto group = glbin_current.vol_group.lock();

		if (group)
		{
			for (int i : {0, 1, 2})
				m_sync[i] = group->GetSync(i);
			gamma = group->GetGammaColor();
			brightness = group->GetBrightness();
			hdr = group->GetHdr();
		}
	}
	break;
	}

	//red
	if (update_all || bSyncR)
	{
		panel->UpdateSyncR(m_sync[0]);
	}
	if (update_all || bGammaR || bSyncR)
	{
		panel->UpdateGammaR(1.0 / gamma.r());
	}
	if (update_all || bBrightnessR || bSyncR)
	{
		panel->UpdateBrightnessR((brightness.r() - 1.0) * 256.0);
	}
	if (update_all || bHdrR || bSyncR)
	{
		panel->UpdateHdrR(hdr.r());
	}
	//green
	if (update_all || bSyncG)
	{
		panel->UpdateSyncG(m_sync[1]);
	}
	if (update_all || bGammaG || bSyncG)
	{
		panel->UpdateGammaG(1.0 / gamma.g());
	}
	if (update_all || bBrightnessG || bSyncG)
	{
		panel->UpdateBrightnessG(brightness.g() - 1.0 * 256.0);
	}
	if (update_all || bHdrG || bSyncG)
	{
		panel->UpdateHdrG(hdr.g());
	}
	//blue
	if (update_all || bSyncB)
	{
		panel->UpdateSyncB(m_sync[2]);
	}
	if (update_all || bGammaB || bSyncB)
	{
		panel->UpdateGammaB(1.0 / gamma.b());
	}
	if (update_all || bBrightnessB || bSyncB)
	{
		panel->UpdateBrightnessB((brightness.b() - 1.0) * 256.0);
	}
	if (update_all || bHdrB || bSyncB)
	{
		panel->UpdateHdrB(hdr.b());	
	}

	if (!m_enable_all)
	{
		m_enable_all = true;
		panel->EnableAll(true);
	}
}

void OutputAdjPanelAgent::UpdateData(const UpdateRequest& request)
{

}

OutputAdjPanel* OutputAdjPanelAgent::GetPanel() const
{
	return static_cast<OutputAdjPanel*>(GetWindow());
}

void OutputAdjPanelAgent::UpdateSync()
{
	int i;
	int cnt;
	bool r_v = false;
	bool g_v = false;
	bool b_v = false;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto group = glbin_current.vol_group.lock();

	if ((type == 2 && group) ||
		(type == 5 && group))
	{
		//use group
		for (i = 0; i < group->GetVolumeNum(); i++)
		{
			auto vd = group->GetVolumeData(i);
			if (vd)
			{
				if (vd->GetMainColorMode() == flvr::ColorMode::Colormap)
				{
					r_v = g_v = b_v = true;
				}
				else
				{
					fluo::Color c = vd->GetColor();
					bool r, g, b;
					r = g = b = false;
					cnt = 0;
					if (c.r() > 0) { cnt++; r = true; }
					if (c.g() > 0) { cnt++; g = true; }
					if (c.b() > 0) { cnt++; b = true; }

					if (cnt > 1)
					{
						r_v = r_v || r;
						g_v = g_v || g;
						b_v = b_v || b;
					}
				}
			}
		}
		cnt = 0;
		if (r_v) cnt++;
		if (g_v) cnt++;
		if (b_v) cnt++;

		SetSync(0, r_v, true);
		SetSync(1, g_v, true);
		SetSync(2, b_v, true);

		if (cnt > 1)
		{
			double gamma = 1.0, brightness = 1.0, hdr = 0.0;
			if (r_v)
			{
				gamma = group->GetGammaColor().r();
				brightness = group->GetBrightness().r();
				hdr = group->GetHdr().r();
			}
			else if (g_v)
			{
				gamma = group->GetGammaColor().g();
				brightness = group->GetBrightness().g();
				hdr = group->GetHdr().g();
			}

			if (g_v)
			{
				SetGamma(1, gamma, b_v ? false : true);
				SetBrightness(1, brightness, b_v ? false : true);
				SetHdr(1, hdr, b_v ? false : true);
			}
			if (b_v)
			{
				SetGamma(2, gamma, true);
				SetBrightness(2, brightness, true);
				SetHdr(2, hdr, true);
			}
		}
	}
	else if (view)
	{
		SetSync(0, true, true);
		SetSync(1, true, true);
		SetSync(2, true, true);
	}
}

void OutputAdjPanelAgent::SetSync(int i, bool val, bool update)
{
	m_sync[i] = val;
	fluo::Color gamma, brightness, hdr;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();

	switch (type)
	{
	case 1://view
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			view->SetSync(i, val);

			if (val)
			{
				gamma = view->GetGammaColor();
				brightness = view->GetBrightness();
				hdr = view->GetHdr();
				SyncColor(gamma, gamma[i]);
				SyncColor(brightness, brightness[i]);
				SyncColor(hdr, hdr[i]);
				view->SetGammaColor(gamma);
				view->SetBrightness(brightness);
				view->SetHdr(hdr);
			}
		}
		break;
	case 2://volume data
		if (vd)
		{
			vd->SetSync(i, val);
			if (group)
				group->SetSyncAll(i, val);
			if (val)
			{
				gamma = vd->GetGammaColor();
				brightness = vd->GetBrightness();
				hdr = vd->GetHdr();
				SyncColor(gamma, gamma[i]);
				SyncColor(brightness, brightness[i]);
				SyncColor(hdr, hdr[i]);
				vd->SetGammaColor(gamma);
				vd->SetBrightness(brightness);
				vd->SetHdr(hdr);
				if (group)
				{
					group->SetGammaAll(gamma);
					group->SetBrightnessAll(brightness);
					group->SetHdrAll(hdr);
				}
			}
		}
		break;
	case 5://group
		if (group)
		{
			group->SetSync(i, val);
			group->SetSyncAll(i, val);
			if (val)
			{
				gamma = group->GetGammaColor();
				brightness = group->GetBrightness();
				hdr = group->GetHdr();
				SyncColor(gamma, gamma[i]);
				SyncColor(brightness, brightness[i]);
				SyncColor(hdr, hdr[i]);
				group->SetGammaAll(gamma);
				group->SetBrightnessAll(brightness);
				group->SetHdrAll(hdr);
			}
		}
		break;
	}

	if (update)
	{
		fluo::ValueCollection vc;
		if (i != 0 && m_sync[0])
			vc.insert(gstSyncR);
		if (i != 1 && m_sync[1])
			vc.insert(gstSyncG);
		if (i != 2 && m_sync[2])
			vc.insert(gstSyncB);

		FluoRefresh(2, vc, { glbin_current.GetViewId() });
	}
}

void OutputAdjPanelAgent::SetGamma(int i, double val, bool notify)
{
	fluo::Color gamma;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();

	switch (type)
	{
	case 1://view
	case 3://mesh
	case 6://mesh group
		if (view)
			gamma = view->GetGammaColor();
		break;
	case 2://volume
		if (vd)
			gamma = vd->GetGammaColor();
		break;
	case 5://group
		if (group)
			gamma = group->GetGammaColor();
		break;
	}
	fluo::ValueCollection vc;
	SyncGamma(gamma, i, val, vc, notify);
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			if (view->GetGammaColor() == gamma)
				return;
			view->SetGammaColor(gamma);
		}
		break;
	case 2:
		if (vd)
		{
			if (vd->GetGammaColor() == gamma)
				return;
			vd->SetGammaColor(gamma);
		}
		if (group)
			group->SetGammaAll(gamma);
		break;
	case 5:
		if (group)
		{
			if (group->GetGammaColor() == gamma)
				return;
			group->SetGammaAll(gamma);
		}
		break;
	}

	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void OutputAdjPanelAgent::SetBrightness(int i, double val, bool notify)
{
	fluo::Color brightness;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();

	switch (type)
	{
	case 1://view
	case 3://mesh
	case 6://mesh group
		if (view)
			brightness = view->GetBrightness();
		break;
	case 2://volume
		if (vd)
			brightness = vd->GetBrightness();
		break;
	case 5://group
		if (group)
			brightness = group->GetBrightness();
		break;
	}
	fluo::ValueCollection vc;
	SyncBrightness(brightness, i, val, vc, notify);
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			if (view->GetBrightness() == brightness)
				return;
			view->SetBrightness(brightness);
		}
		break;
	case 2:
		if (vd)
		{
			if (vd->GetBrightness() == brightness)
				return;
			vd->SetBrightness(brightness);
		}
		if (group)
			group->SetBrightnessAll(brightness);
		break;
	case 5:
		if (group)
		{
			if (group->GetBrightness() == brightness)
				return;
			group->SetBrightnessAll(brightness);
		}
		break;
	}

	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void OutputAdjPanelAgent::SetHdr(int i, double val, bool notify)
{
	fluo::Color hdr;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();

	switch (type)
	{
	case 1://view
	case 3://mesh
	case 6://mesh group
		if (view)
			hdr = view->GetHdr();
		break;
	case 2://volume
		if (vd)
			hdr = vd->GetHdr();
		break;
	case 5://group
		if (group)
			hdr = group->GetHdr();
		break;
	}
	fluo::ValueCollection vc;
	SyncHdr(hdr, i, val, vc, notify);
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			if (view->GetHdr() == hdr)
				return;
			view->SetHdr(hdr);
		}
		break;
	case 2:
		if (vd)
		{
			if (vd->GetHdr() == hdr)
				return;
			vd->SetHdr(hdr);
		}
		if (group)
			group->SetHdrAll(hdr);
		break;
	case 5:
		if (group)
		{
			if (group->GetHdr() == hdr)
				return;
			group->SetHdrAll(hdr);
		}
		break;
	}

	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void OutputAdjPanelAgent::SyncColor(fluo::Color& c, double val)
{
	for (int i : {0, 1, 2})
		if (m_sync[i])
			c[i] = val;
}

void OutputAdjPanelAgent::SyncGamma(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify)
{
	for (int j : {0, 1, 2})
	{
		bool changed = false;
		if (j == i)
		{
			c[j] = val;
			changed = true;
		}
		else if (m_sync[j] && m_sync[i])
		{
			c[j] = val;
			changed = true;
		}
		if (changed)
		{
			if (notify == (i == j))
			{
				switch (j)
				{
				case 0:
					vc.insert(gstGammaR);
					break;
				case 1:
					vc.insert(gstGammaG);
					break;
				case 2:
					vc.insert(gstGammaB);
					break;
				}
			}
		}
	}
	if (vc.empty())
		vc.insert(gstNull);
}

void OutputAdjPanelAgent::SyncBrightness(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify)
{
	for (int j : {0, 1, 2})
	{
		bool changed = false;
		if (j == i)
		{
			c[j] = val;
			changed = true;
		}
		else if (m_sync[j] && m_sync[i])
		{
			c[j] = val;
			changed = true;
		}
		if (changed)
		{
			if (notify == (i == j))
			{
				switch (j)
				{
				case 0:
					vc.insert(gstBrightnessR);
					break;
				case 1:
					vc.insert(gstBrightnessG);
					break;
				case 2:
					vc.insert(gstBrightnessB);
					break;
				}
			}
		}
	}
	if (vc.empty())
		vc.insert(gstNull);
}

void OutputAdjPanelAgent::SyncHdr(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify)
{
	for (int j : {0, 1, 2})
	{
		bool changed = false;
		if (j == i)
		{
			c[j] = val;
			changed = true;
		}
		else if (m_sync[j] && m_sync[i])
		{
			c[j] = val;
			changed = true;
		}
		if (changed)
		{
			if (notify == (i == j))
			{
				switch (j)
				{
				case 0:
					vc.insert(gstEqualizeR);
					break;
				case 1:
					vc.insert(gstEqualizeG);
					break;
				case 2:
					vc.insert(gstEqualizeB);
					break;
				}
			}
		}
	}
	if (vc.empty())
		vc.insert(gstNull);
}

void OutputAdjPanelAgent::SyncGamma(int i)
{
	fluo::Color gamma;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
	{
		if (view)
			gamma = view->GetGammaColor();
	}
	break;
	case 2:
	{
		if (vd)
			gamma = vd->GetGammaColor();
	}
	break;
	case 5:
	{
		if (group)
			gamma = group->GetGammaColor();
	}
	break;
	}
	for (int j : {0, 1, 2})
		if (j != i) gamma[j] = gamma[i];
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			if (view->GetGammaColor() == gamma)
				return;
			view->SetGammaColor(gamma);
		}
		break;
	case 2:
		if (vd)
		{
			if (vd->GetGammaColor() == gamma)
				return;
			vd->SetGammaColor(gamma);
		}
		if (group)
			group->SetGammaAll(gamma);
		break;
	case 5:
		if (group)
		{
			if (group->GetGammaColor() == gamma)
				return;
			group->SetGammaAll(gamma);
		}
		break;
	}

	fluo::ValueCollection vc;
	if (i != 0)
		vc.insert(gstGammaR);
	if (i != 1)
		vc.insert(gstGammaG);
	if (i != 2)
		vc.insert(gstGammaB);
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void OutputAdjPanelAgent::SyncBrightness(int i)
{
	fluo::Color brightness;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
	{
		if (view)
			brightness = view->GetBrightness();
	}
	break;
	case 2:
	{
		if (vd)
			brightness = vd->GetBrightness();
		break;
	}
	case 5:
	{
		if (group)
			brightness = group->GetBrightness();
	}
	break;
	}
	for (int j : {0, 1, 2})
		if (j != i) brightness[j] = brightness[i];
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			if (view->GetBrightness() == brightness)
				return;
			view->SetBrightness(brightness);
		}
		break;
	case 2:
		if (vd)
		{
			if (vd->GetBrightness() == brightness)
				return;
			vd->SetBrightness(brightness);
		}
		if (group)
			group->SetBrightnessAll(brightness);
		break;
	case 5:
		if (group)
		{
			if (group->GetBrightness() == brightness)
				return;
			group->SetBrightnessAll(brightness);
		}
		break;
	}

	fluo::ValueCollection vc;
	if (i != 0)
		vc.insert(gstBrightnessR);
	if (i != 1)
		vc.insert(gstBrightnessG);
	if (i != 2)
		vc.insert(gstBrightnessB);
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void OutputAdjPanelAgent::SyncHdr(int i)
{
	fluo::Color hdr;
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
	{
		if (view)
			hdr = view->GetHdr();
	}
	break;
	case 2:
	{
		if (vd)
			hdr = vd->GetHdr();
	}
	break;
	case 5:
	{
		if (group)
			hdr = group->GetHdr();
	}
	break;
	}
	for (int j : {0, 1, 2})
		if (j != i) hdr[j] = hdr[i];
	switch (type)
	{
	case 1:
	case 3://mesh
	case 6://mesh group
		if (view)
		{
			if (view->GetHdr() == hdr)
				return;
			view->SetHdr(hdr);
		}
		break;
	case 2:
		if (vd)
		{
			if (vd->GetHdr() == hdr)
				return;
			vd->SetHdr(hdr);
		}
		if (group)
			group->SetHdrAll(hdr);
		break;
	case 5:
		if (group)
		{
			if (group->GetHdr() == hdr)
				return;
			group->SetHdrAll(hdr);
		}
		break;
	}

	fluo::ValueCollection vc;
	if (i != 0)
		vc.insert(gstEqualizeR);
	if (i != 1)
		vc.insert(gstEqualizeG);
	if (i != 2)
		vc.insert(gstEqualizeB);
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

