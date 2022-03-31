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

#include <RecorderAgent.hpp>
#include <RecorderDlg.h>

using namespace fluo;

RecorderAgent::RecorderAgent(RecorderDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void RecorderAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* RecorderAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void RecorderAgent::UpdateAllSettings()
{
}

void RecorderAgent::AutoKeyChanComb(int comb)
{
	if (!m_frame)
		return;
	if (!m_view)
	{
		m_view = glbin_root->getChild(0)->asRenderview();
	}

	DataManager* mgr = m_frame->GetDataManager();
	if (!mgr)
		return;
	Interpolator *interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	wxString str = m_duration_text->GetValue();
	double duration;
	str.ToDouble(&duration);

	FlKeyCode keycode;
	FlKeyBoolean* flkeyB = 0;

	double t = interpolator->GetLastT();
	t = t < 0.0 ? 0.0 : t;
	if (t > 0.0) t += duration;

	int i;
	fluo::VolumeList list = m_view->GetFullVolList();
	int numChan = int(list.size());
	vector<bool> chan_mask;
	//initiate mask
	for (i = 0; i < numChan; i++)
	{
		if (i < comb)
			chan_mask.push_back(true);
		else
			chan_mask.push_back(false);
	}

	do
	{
		interpolator->Begin(t, duration);

		//for all volumes
		for (auto vd : list)
		{
			if (!vd) continue;
			keycode.l0 = 1;
			keycode.l0_name = m_view->getName();
			keycode.l1 = 2;
			keycode.l1_name = vd->getName();
			//display only
			keycode.l2 = 0;
			keycode.l2_name = "display";
			flkeyB = new FlKeyBoolean(keycode, chan_mask[i]);
			interpolator->AddKey(flkeyB);
		}

		interpolator->End();
		t += duration;
	} while (MoveOne(chan_mask));

	m_keylist->Update();
}

void RecorderAgent::AddKey()
{
	wxString str;
	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;
	long item = m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	int index = -1;
	if (item != -1)
	{
		str = m_keylist->GetItemText(item);
		long id;
		str.ToLong(&id);
		index = interpolator->GetKeyIndex(id);
	}
	//check if 4D
	bool is_4d = false;
	fluo::VolumeData* vd = 0;
	for (size_t i = 0; i < glbin_volf->getNum(); ++i)
	{
		vd = glbin_volf->get(i);
		if (vd && vd->GetReader() &&
			vd->GetReader()->GetTimeNum() > 1)
		{
			is_4d = true;
			break;
		}
	}
	double duration = 0.0;
	if (is_4d)
	{
		Interpolator *interpolator = m_frame->GetInterpolator();
		if (interpolator && m_view)
		{
			long ct;
			vd->getValue(gstTime, ct);
			FlKeyCode keycode;
			keycode.l0 = 1;
			keycode.l0_name = m_view->getName();
			keycode.l1 = 2;
			keycode.l1_name = vd->getName();
			keycode.l2 = 0;
			keycode.l2_name = "frame";
			double frame;
			if (interpolator->GetDouble(keycode,
				interpolator->GetLastIndex(), frame))
				duration = fabs(ct - frame);
		}
	}
	else
	{
		str = m_duration_text->GetValue();
		str.ToDouble(&duration);
	}
	int interpolation = m_interpolation_cmb->GetSelection();
	InsertKey(index, duration, interpolation);

	m_keylist->Update();
	m_keylist->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void RecorderAgent::InsertKey(int index, double duration, int interpolation)
{
	if (!m_frame)
		return;
	if (!m_view)
	{
		m_view = glbin_root->getChild(0)->asRenderview();
	}

	DataManager* mgr = m_frame->GetDataManager();
	if (!mgr)
		return;
	Interpolator *interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;
	FlKeyCode keycode;
	FlKeyDouble* flkey = 0;
	FlKeyQuaternion* flkeyQ = 0;
	FlKeyBoolean* flkeyB = 0;
	FlKeyInt* flkeyI = 0;

	double t = interpolator->GetLastT();
	t = t < 0.0 ? 0.0 : t + duration;

	interpolator->Begin(t, duration);

	//for all volumes
	for (size_t i = 0; i < glbin_volf->getNum(); ++i)
	{
		fluo::VolumeData* vd = glbin_volf->get(i);
		keycode.l0 = 1;
		keycode.l0_name = m_view->getName();
		keycode.l1 = 2;
		keycode.l1_name = vd->getName();
		//display
		keycode.l2 = 0;
		keycode.l2_name = "display";
		bool bval;
		vd->getValue(gstDisplay, bval);
		flkeyB = new FlKeyBoolean(keycode, bval);
		interpolator->AddKey(flkeyB);
		//clipping planes
		std::vector<fluo::Plane*> * planes = vd->GetRenderer()->get_planes();
		if (!planes)
			continue;
		if (planes->size() != 6)
			continue;
		fluo::Plane* plane = 0;
		double abcd[4];
		//x1
		plane = (*planes)[0];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "x1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		interpolator->AddKey(flkey);
		//x2
		plane = (*planes)[1];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "x2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		interpolator->AddKey(flkey);
		//y1
		plane = (*planes)[2];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "y1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		interpolator->AddKey(flkey);
		//y2
		plane = (*planes)[3];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "y2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		interpolator->AddKey(flkey);
		//z1
		plane = (*planes)[4];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "z1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		interpolator->AddKey(flkey);
		//z2
		plane = (*planes)[5];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "z2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		interpolator->AddKey(flkey);
		//t
		long frame;
		vd->getValue(gstTime, frame);
		keycode.l2 = 0;
		keycode.l2_name = "frame";
		flkey = new FlKeyDouble(keycode, frame);
		interpolator->AddKey(flkey);
	}
	//for the view
	keycode.l0 = 1;
	keycode.l0_name = m_view->getName();
	keycode.l1 = 1;
	keycode.l1_name = m_view->getName();
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";
	fluo::Quaternion q;
	m_view->getValue(gstCamRotQ, q);
	flkeyQ = new FlKeyQuaternion(keycode, q);
	interpolator->AddKey(flkeyQ);
	//translation
	double tx, ty, tz;
	m_view->getValue(gstCamTransX, tx);
	m_view->getValue(gstCamTransY, ty);
	m_view->getValue(gstCamTransZ, tz);
	//x
	keycode.l2_name = "translation_x";
	flkey = new FlKeyDouble(keycode, tx);
	interpolator->AddKey(flkey);
	//y
	keycode.l2_name = "translation_y";
	flkey = new FlKeyDouble(keycode, ty);
	interpolator->AddKey(flkey);
	//z
	keycode.l2_name = "translation_z";
	flkey = new FlKeyDouble(keycode, tz);
	interpolator->AddKey(flkey);
	//centers
	m_view->getValue(gstCamCtrX, tx);
	m_view->getValue(gstCamCtrY, ty);
	m_view->getValue(gstCamCtrZ, tz);
	//x
	keycode.l2_name = "center_x";
	flkey = new FlKeyDouble(keycode, tx);
	interpolator->AddKey(flkey);
	//y
	keycode.l2_name = "center_y";
	flkey = new FlKeyDouble(keycode, ty);
	interpolator->AddKey(flkey);
	//z
	keycode.l2_name = "center_z";
	flkey = new FlKeyDouble(keycode, tz);
	interpolator->AddKey(flkey);
	//obj traslation
	m_view->getValue(gstObjTransX, tx);
	m_view->getValue(gstObjTransY, ty);
	m_view->getValue(gstObjTransZ, tz);
	//x
	keycode.l2_name = "obj_trans_x";
	flkey = new FlKeyDouble(keycode, tx);
	interpolator->AddKey(flkey);
	//y
	keycode.l2_name = "obj_trans_y";
	flkey = new FlKeyDouble(keycode, ty);
	interpolator->AddKey(flkey);
	//z
	keycode.l2_name = "obj_trans_z";
	flkey = new FlKeyDouble(keycode, tz);
	interpolator->AddKey(flkey);
	//scale
	double scale;
	m_view->getValue(gstScaleFactor, scale);
	keycode.l2_name = "scale";
	flkey = new FlKeyDouble(keycode, scale);
	interpolator->AddKey(flkey);
	//intermixing mode
	long lval;
	m_view->getValue(gstMixMethod, lval);
	keycode.l2_name = "volmethod";
	flkeyI = new FlKeyInt(keycode, lval);
	interpolator->AddKey(flkeyI);
	//perspective angle
	bool persp;
	m_view->getValue(gstPerspective, persp);
	double aov;
	m_view->getValue(gstAov, aov);
	if (!persp)
		aov = 9.9;
	keycode.l2_name = "aov";
	flkey = new FlKeyDouble(keycode, aov);
	interpolator->AddKey(flkey);

	interpolator->End();

	FlKeyGroup* group = interpolator->GetKeyGroup(interpolator->GetLastIndex());
	if (group)
		group->type = interpolation;

	dlg_.m_keylist->Update();
}

bool RecorderAgent::MoveOne(vector<bool>& chan_mask, int lv)
{
	int i;
	int cur_lv = 0;
	int lv_pos = -1;
	for (i = (int)chan_mask.size() - 1; i >= 0; i--)
	{
		if (chan_mask[i])
		{
			cur_lv++;
			if (cur_lv == lv)
			{
				lv_pos = i;
				break;
			}
		}
	}
	if (lv_pos >= 0)
	{
		if (lv_pos == (int)chan_mask.size() - lv)
			return MoveOne(chan_mask, ++lv);
		else
		{
			if (!chan_mask[lv_pos + 1])
			{
				for (i = lv_pos; i < (int)chan_mask.size(); i++)
				{
					if (i == lv_pos)
						chan_mask[i] = false;
					else if (i <= lv_pos + lv)
						chan_mask[i] = true;
					else
						chan_mask[i] = false;
				}
				return true;
			}
			else return false;//no space anymore
		}
	}
	else return false;
}

void RecorderAgent::DeleteSel()
{

}

void RecorderAgent::DeleteAll()
{

}

void RecorderAgent::OnSelectedKey(Event& event)
{
	long lval;
	getValue(gstSelectedKey, lval);
	long item = dlg_.m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (lval != item && item != -1)
		dlg_.m_keylist->SetItemState(lval,
			wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}