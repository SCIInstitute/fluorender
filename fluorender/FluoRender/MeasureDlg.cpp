/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "MeasureDlg.h"
#include "VRenderFrame.h"
#include <Distance/RulerHandler.h>
#include <Distance/DistCalculator.h>
#include <Distance/RulerAlign.h>
#include <wx/artprov.h>
#include <wx/valnum.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/clipbrd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
//resources
#include "Formats/png_resource.h"
#include "ruler.xpm"
#include "img/icons.h"

BEGIN_EVENT_TABLE(RulerListCtrl, wxListCtrl)
	EVT_KEY_DOWN(RulerListCtrl::OnKeyDown)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, RulerListCtrl::OnSelection)
	EVT_LIST_ITEM_DESELECTED(wxID_ANY, RulerListCtrl::OnEndSelection)
	EVT_TEXT(ID_NameText, RulerListCtrl::OnNameText)
	EVT_TEXT(ID_CenterText, RulerListCtrl::OnCenterText)
	EVT_COLOURPICKER_CHANGED(ID_ColorPicker, RulerListCtrl::OnColorChange)
	EVT_SCROLLWIN(RulerListCtrl::OnScroll)
	EVT_MOUSEWHEEL(RulerListCtrl::OnScroll)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, RulerListCtrl::OnAct)
	EVT_CONTEXT_MENU(RulerListCtrl::OnContextMenu)
	EVT_MENU(ID_ToggleDisp, RulerListCtrl::OnToggleDisp)
END_EVENT_TABLE()

#define IntCol 3
#define ColorCol 4
#define CenterCol 8
#define PointCol 10

RulerListCtrl::RulerListCtrl(
	VRenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxListItem itemCol; int col = 0;
	itemCol.SetText("Name");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, 100); col++;
	itemCol.SetText("Group");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, 30); col++;
	itemCol.SetText("Count");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, 30); col++;
	itemCol.SetText("Intensity");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, 50); col++;
	itemCol.SetText("Color");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Branch");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Length");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Angle/Pitch");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Center");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Time");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Start/End Points (X, Y, Z)");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	itemCol.SetText("Voxels");
	this->InsertColumn(col, itemCol);
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);

	m_images = new wxImageList(16, 16, true);
	wxIcon icon = wxIcon(ruler_xpm);
	m_images->Add(icon);
	AssignImageList(m_images, wxIMAGE_LIST_SMALL);

	//frame edit
	m_name_text = new wxTextCtrl(this, ID_NameText, "",
		wxDefaultPosition, wxDefaultSize);
	m_name_text->Connect(ID_NameText, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(RulerListCtrl::OnTextFocus),
		NULL, this);
	m_name_text->Hide();
	m_center_text = new wxTextCtrl(this, ID_CenterText, "",
		wxDefaultPosition, wxDefaultSize);
	m_center_text->Connect(ID_CenterText, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(RulerListCtrl::OnTextFocus),
		NULL, this);
	m_center_text->Hide();
	m_color_picker = new wxColourPickerCtrl(this,
		ID_ColorPicker);
	m_color_picker->Hide();
}

RulerListCtrl::~RulerListCtrl()
{
}

void RulerListCtrl::Append(bool enable, unsigned int id,
	wxString name, unsigned int group, int count,
	wxString intensity, wxString &color,
	int branches, double length, wxString &unit,
	double angle, wxString &center, bool time_dep,
	int time, wxString &extra, wxString &points)
{
	int col = 0;
	wxString str;
	long tmp = InsertItem(GetItemCount(), name, 0);
	SetItemData(tmp, long(id)); col++;
	str = wxString::Format("%d", group);
	SetItem(tmp, col, str); col++;
	str = wxString::Format("%d", count);
	SetItem(tmp, col, str); col++;
	SetItem(tmp, col, intensity); col++;
	SetItem(tmp, col, color); col++;
	str = wxString::Format("%d", branches);
	SetItem(tmp, col, str); col++;
	str = wxString::Format("%.2f", length) + unit;
	SetItem(tmp, col, str); col++;
	str = wxString::Format("%.1f", angle) + "Deg";
	SetItem(tmp, col, str); col++;
	SetItem(tmp, col, center); col++;
	if (time_dep)
		str = wxString::Format("%d", time);
	else
		str = "N/A";
	SetItem(tmp, col, str); col++;
	SetItem(tmp, col, points); col++;
	SetItem(tmp, col, extra); col++;

	if (!enable)
		SetItemBackgroundColour(tmp, wxColour(200, 200, 200));
}

void RulerListCtrl::AdjustSize()
{
	int col = 0;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE); col++;
	SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER); col++;
}

void RulerListCtrl::UpdateRulers(VRenderGLView* vrv)
{
	m_name_text->Hide();
	m_center_text->Hide();
	m_color_picker->Hide();
	if (vrv)
		m_view = vrv;

	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;

	std::vector<int> sel;
	GetCurrSelection(sel);

	std::vector<unsigned int> groups;
	int group_num = ruler_list->GetGroupNum(groups);
	std::vector<int> group_count(group_num, 0);

	DeleteAllItems();

	wxString points;
	fluo::Point p;
	int num_points;
	for (int i=0; i<(int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		ruler->SetWorkTime(m_view->m_tseq_cur_num);
		if (ruler->GetTransient() &&
			ruler->GetTransTime() != m_view->m_tseq_cur_num)
			continue;

		wxString unit;
		switch (m_view->m_sb_unit)
		{
		case 0:
			unit = "nm";
			break;
		case 1:
		default:
			unit = L"\u03BCm";
			break;
		case 2:
			unit = "mm";
			break;
		}

		points = "";
		num_points = ruler->GetNumPoint();
		if (num_points > 0)
		{
			p = ruler->GetPoint(0);
			points += wxString::Format("(%.2f, %.2f, %.2f)", p.x(), p.y(), p.z());
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1);
			points += ", ";
			points += wxString::Format("(%.2f, %.2f, %.2f)", p.x(), p.y(), p.z());
		}
		unsigned int group = ruler->Group();
		int count = 0;
		auto iter = std::find(groups.begin(), groups.end(), group);
		if (iter != groups.end())
		{
			int index = std::distance(groups.begin(), iter);
			count = ++group_count[index];
		}
		double dval = ruler->GetProfileMaxValue();
		dval *= ruler->GetScalarScale();
		wxString intensity;
		if (m_rhdl->GetBackground())
		{
			double bg_int = m_rhdl->GetVolumeBgInt();
			dval = (dval - bg_int) / bg_int;
			intensity = wxString::Format("%.4f", dval);
		}
		else
			intensity = wxString::Format("%.0f", dval);
		wxString color;
		if (ruler->GetUseColor())
			color = wxString::Format("RGB(%d, %d, %d)",
			int(ruler->GetColor().r()*255),
			int(ruler->GetColor().g()*255),
			int(ruler->GetColor().b()*255));
		else
			color = "N/A";
		wxString center;
		fluo::Point cp = ruler->GetCenter();
		center = wxString::Format("(%.2f, %.2f, %.2f)",
			cp.x(), cp.y(), cp.z());
		wxString str = ruler->GetDelInfoValues(", ");
		Append(ruler->GetDisp(), ruler->Id(),
			ruler->GetName(), group, count,
			intensity, color,
			ruler->GetNumBranch(), ruler->GetLength(), unit,
			ruler->GetAngle(), center, ruler->GetTransient(),
			ruler->GetTransTime(), str, points);
	}

	AdjustSize();

	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		if (0 > index || ruler_list->size() <= index)
			continue;
		SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Rulers);
}

bool RulerListCtrl::GetCurrSelection(std::vector<int> &sel)
{
	long item = -1;
	for (;;)
	{
		item = GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		sel.push_back(int(item));
	}
	if (!sel.empty())
		return true;
	return false;
}

void RulerListCtrl::ClearSelection()
{
	long item = -1;
	for (;;)
	{
		item = GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		SetItemState(item, 0, wxLIST_STATE_SELECTED);
	}
}

void RulerListCtrl::SelectGroup(unsigned int group)
{
	ClearSelection();
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == group)
			SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

void RulerListCtrl::DeleteSelection()
{
	if (!m_rhdl)
		return;
	std::vector<int> sel;
	GetCurrSelection(sel);
	m_rhdl->DeleteSelection(sel);
	UpdateRulers();
	m_view->RefreshGL(39);
}

void RulerListCtrl::DeleteAll(bool cur_time)
{
	if (!m_rhdl)
		return;
	m_rhdl->DeleteAll(cur_time);
	UpdateRulers();
	m_view->RefreshGL(39);
}

void RulerListCtrl::Export(wxString filename)
{
	if (!m_view) return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (ruler_list)
	{
		std::ofstream os;
		OutputStreamOpen(os, filename.ToStdString());

		wxString str;
		wxString unit;
		int num_points;
		fluo::Point p;
		flrd::Ruler* ruler;
		switch (m_view->m_sb_unit)
		{
		case 0:
			unit = "nm";
			break;
		case 1:
		default:
			unit = L"\u03BCm";
			break;
		case 2:
			unit = "mm";
			break;
		}

		int ruler_num = ruler_list->size();
		std::vector<unsigned int> groups;
		std::vector<int> counts;
		int group_num = ruler_list->GetGroupNumAndCount(groups, counts);
		std::vector<int> group_count(group_num, 0);

		if (ruler_num > 1)
			os << "Ruler Count:\t" << ruler_num << "\n";
		if (group_num > 1)
		{
			//group count
			os << "Group Count:\t" << group_num << "\n";
			for (int i = 0; i < group_num; ++i)
			{
				os << "Group " << groups[i];
				if (i < group_num - 1)
					os << "\t";
				else
					os << "\n";
			}
			for (int i = 0; i < group_num; ++i)
			{
				os << counts[i];
				if (i < group_num - 1)
					os << "\t";
				else
					os << "\n";
			}
		}

		os << "Name\tGroup\tCount\tColor\tBranch\tLength(" << unit << ")\tAngle/Pitch(Deg)\tx1\ty1\tz1\txn\tyn\tzn\tTime\tv1\tv2\n";

		double f = 0.0;
		fluo::Color color;
		for (size_t i=0; i<ruler_list->size(); i++)
		{
			//for each ruler
			ruler = (*ruler_list)[i];
			if (!ruler) continue;
			ruler->SetWorkTime(m_view->m_tseq_cur_num);

			os << ruler->GetName() << "\t";

			//group and count
			unsigned int group = ruler->Group();
			os << group << "\t";
			int count = 0;
			auto iter = std::find(groups.begin(), groups.end(), group);
			if (iter != groups.end())
			{
				int index = std::distance(groups.begin(), iter);
				count = ++group_count[index];
			}
			os << count << "\t";

			//color
			if (ruler->GetUseColor())
			{
				color = ruler->GetColor();
				str = wxString::Format("RGB(%d, %d, %d)",
					int(color.r()*255), int(color.g()*255), int(color.b()*255));
			}
			else
				str = "N/A";
			os << str << "\t";

			//branch count
			str = wxString::Format("%d", ruler->GetNumBranch());
			os << str << "\t";
			//length
			str = wxString::Format("%.2f", ruler->GetLength());
			os << str << "\t";
			//angle
			str = wxString::Format("%.1f", ruler->GetAngle());
			os << str << "\t";

			str = "";
			//start and end points
			num_points = ruler->GetNumPoint();
			if (num_points > 0)
			{
				p = ruler->GetPoint(0);
				str += wxString::Format("%.2f\t%.2f\t%.2f\t", p.x(), p.y(), p.z());
			}
			if (num_points > 1)
			{
				p = ruler->GetPoint(num_points - 1);
				str += wxString::Format("%.2f\t%.2f\t%.2f\t", p.x(), p.y(), p.z());
			}
			else
				str += "\t\t\t";
			os << str;
			
			//time
			if (ruler->GetTransient())
				str = wxString::Format("%d", ruler->GetTransTime());
			else
				str = "N/A";
			os << str << "\t";

			//info values v1 v2
			os << ruler->GetInfoValues() << "\n";

			//export points
			if (ruler->GetNumPoint() > 2)
			{
				os << ruler->GetPosNames();
				os << ruler->GetPosValues();
			}

			//export profile
			vector<flrd::ProfileBin>* profile = ruler->GetProfile();
			if (profile && profile->size())
			{
				double sumd = 0.0;
				unsigned long long sumull = 0;
				os << ruler->GetInfoProfile() << "\n";
				for (size_t j=0; j<profile->size(); ++j)
				{
					//for each profile
					int pixels = (*profile)[j].m_pixels;
					if (pixels <= 0)
						os << "0.0\t";
					else
					{
						os << (*profile)[j].m_accum / pixels << "\t";
						sumd += (*profile)[j].m_accum;
						sumull += pixels;
					}
				}
				//if (m_ruler_df_f)
				//{
				//	double avg = 0.0;
				//	if (sumull != 0)
				//		avg = sumd / double(sumull);
				//	if (i == 0)
				//	{
				//		f = avg;
				//		os << "\t" << f << "\t";
				//	}
				//	else
				//	{
				//		double df = avg - f;
				//		if (f == 0.0)
				//			os << "\t" << df << "\t";
				//		else
				//			os << "\t" << df / f << "\t";
				//	}
				//}
				os << "\n";
			}
		}

		os.close();
	}
}

void RulerListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if ( event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
	{
		long item = GetNextItem(-1,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item != -1)
		{
			flrd::Ruler* ruler = m_view->GetRuler(GetItemData(item));
			if (ruler)
			{
				fluo::Point cp = ruler->GetCenter();
				wxString center = wxString::Format("%.2f\t%.2f\t%.2f",
					cp.x(), cp.y(), cp.z());
				if (wxTheClipboard->Open())
				{
					wxTheClipboard->SetData(new wxTextDataObject(center));
					wxTheClipboard->Close();
				}
			}
		}
	}
	event.Skip();
}

wxString RulerListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void RulerListCtrl::SetText(long item, int col, wxString &str)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	info.SetText(str);
	SetItem(info);
}

void RulerListCtrl::OnSelection(wxListEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_editing_item = item;
	if (!m_view)
		return;
	flrd::Ruler* ruler = m_view->GetRuler(GetItemData(item));
	if (!ruler || !ruler->GetDisp())
		return;

	wxRect rect;
	wxString str;
	//add frame text
	GetSubItemRect(item, 0, rect);
	str = GetText(item, 0);
	m_name_text->SetPosition(rect.GetTopLeft());
	m_name_text->SetSize(rect.GetSize());
	m_name_text->SetValue(str);
	m_name_text->Show();
	//add color picker
	GetSubItemRect(item, ColorCol, rect);
	m_color_picker->SetPosition(rect.GetTopLeft());
	m_color_picker->SetSize(rect.GetSize());
	if (ruler->GetRulerType() == 2)
	{
		//locator
		GetSubItemRect(item, CenterCol, rect);
		str = GetText(item, CenterCol);
		m_center_text->SetPosition(rect.GetTopLeft());
		m_center_text->SetSize(rect.GetSize());
		m_center_text->SetValue(str);
		m_center_text->Show();
	}
	if (ruler->GetUseColor())
	{
		fluo::Color color = ruler->GetColor();
		wxColor c(int(color.r()*255.0), int(color.g()*255.0), int(color.b()*255.0));
		m_color_picker->SetColour(c);
	}
	m_color_picker->Show();
}

void RulerListCtrl::EndEdit(bool update)
{
	if (m_name_text->IsShown())
	{
		m_name_text->Hide();
		m_center_text->Hide();
		m_color_picker->Hide();
		m_editing_item = -1;
		if (update) UpdateRulers();
	}
}

void RulerListCtrl::OnEndSelection(wxListEvent &event)
{
	EndEdit(false);
}

void RulerListCtrl::OnNameText(wxCommandEvent& event)
{
	if (!m_view)
		return;
	if (m_editing_item == -1)
		return;

	wxString str = m_name_text->GetValue();

	flrd::Ruler* ruler = m_view->GetRuler(GetItemData(m_editing_item));
	if (!ruler) return;
	ruler->SetName(str);
	SetText(m_editing_item, 0, str);
	m_view->RefreshGL(39);
}

void RulerListCtrl::OnCenterText(wxCommandEvent& event)
{
	if (!m_view)
		return;
	if (m_editing_item == -1)
		return;
	flrd::Ruler* ruler = m_view->GetRuler(GetItemData(m_editing_item));
	if (!ruler || ruler->GetRulerType() != 2) return;
	ruler->SetWorkTime(m_view->m_tseq_cur_num);

	wxString str = m_center_text->GetValue();
	wxString old_str = GetText(m_editing_item, CenterCol);
	if (str == old_str)
		return;

	//get xyz
	double x = 0, y = 0, z = 0;
	int count = 0;
	std::string stemp = str.ToStdString();
	if (stemp.empty())
		return;
	while (count < 3)
	{
		size_t read = 0;
		try
		{
			if (count == 0)
				x = std::stod(stemp, &read);
			else if (count == 1)
				y = std::stod(stemp, &read);
			else if (count == 2)
				z = std::stod(stemp, &read);
			stemp = stemp.substr(read, stemp.size() - read);
			count++;
			if (count < 3 && stemp.empty())
				return;
		}
		catch (std::invalid_argument)
		{
			stemp = stemp.substr(1, stemp.size() - 1);
			if (stemp.empty())
				return;
		}
	}
	fluo::Point tmp(x, y, z);
	ruler->SetPoint(0, tmp);
	str = wxString::Format("(%.2f, %.2f, %.2f)",
		x, y, z);
	SetText(m_editing_item, CenterCol, str);
	SetText(m_editing_item, PointCol, str);
	m_view->RefreshGL(39);
}

void RulerListCtrl::OnColorChange(wxColourPickerEvent& event)
{
	if (!m_view)
		return;
	if (m_editing_item == -1)
		return;
	std::vector<int> sel;
	if (!GetCurrSelection(sel))
		return;

	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	wxString str_color;
	str_color = wxString::Format("RGB(%d, %d, %d)",
		int(color.r()*255),
		int(color.g()*255),
		int(color.b()*255));
	SetText(m_editing_item, ColorCol, str_color);

	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = m_view->GetRuler(GetItemData(index));
		if (!ruler)
			continue;
		ruler->SetColor(color);
	}

	m_view->RefreshGL(39);
}

void RulerListCtrl::OnScroll(wxScrollWinEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

void RulerListCtrl::OnScroll(wxMouseEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

void RulerListCtrl::OnTextFocus(wxCommandEvent& event)
{
	wxTextCtrl *object = (wxTextCtrl*)event.GetEventObject();
	object->SetSelection(0, -1);
}

void RulerListCtrl::OnAct(wxListEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (!m_view)
		return;
	flrd::Ruler* ruler = m_view->GetRuler(GetItemData(item));
	if (!ruler) return;
	ruler->ToggleDisp();
	bool disp = ruler->GetDisp();
	if (disp)
		SetItemBackgroundColour(item, wxColour(255, 255, 255));
	else
		SetItemBackgroundColour(item, wxColour(200, 200, 200));
	m_name_text->Hide();
	m_center_text->Hide();
	m_color_picker->Hide();
	SetItemState(item, 0, wxLIST_STATE_SELECTED);
	m_view->RefreshGL(39);
}

void RulerListCtrl::OnContextMenu(wxContextMenuEvent &event)
{
	if (!GetSelectedItemCount())
		return;

	wxPoint point = event.GetPosition();
	// If from keyboard
	if (point.x == -1 && point.y == -1)
	{
		wxSize size = GetSize();
		point.x = size.x / 2;
		point.y = size.y / 2;
	}
	else
	{
		point = ScreenToClient(point);
	}

	wxMenu menu;
	menu.Append(ID_ToggleDisp, "ToggleDisplay");
	PopupMenu(&menu, point.x, point.y);
}

void RulerListCtrl::OnToggleDisp(wxCommandEvent& event)
{
	if (!m_view)
		return;
	std::vector<int> sel;
	if (!GetCurrSelection(sel))
		return;

	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = m_view->GetRuler(GetItemData(index));
		if (!ruler) continue;
		ruler->ToggleDisp();
		bool disp = ruler->GetDisp();
		if (disp)
			SetItemBackgroundColour(index, wxColour(255, 255, 255));
		else
			SetItemBackgroundColour(index, wxColour(200, 200, 200));
	}
	//m_name_text->Hide();
	//m_center_text->Hide();
	//m_color_picker->Hide();
	//SetItemState(item, 0, wxLIST_STATE_SELECTED);
	m_view->RefreshGL(39);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(MeasureDlg, wxPanel)
	EVT_MENU(ID_LocatorBtn, MeasureDlg::OnNewLocator)
	EVT_MENU(ID_ProbeBtn, MeasureDlg::OnNewProbe)
	EVT_MENU(ID_ProtractorBtn, MeasureDlg::OnNewProtractor)
	EVT_MENU(ID_RulerBtn, MeasureDlg::OnNewRuler)
	EVT_MENU(ID_RulerMPBtn, MeasureDlg::OnNewRulerMP)
	EVT_MENU(ID_EllipseBtn, MeasureDlg::OnEllipse)
	EVT_MENU(ID_GrowBtn, MeasureDlg::OnGrow)
	EVT_MENU(ID_PencilBtn, MeasureDlg::OnPencil)
	EVT_MENU(ID_RulerMoveBtn, MeasureDlg::OnRulerMove)
	EVT_MENU(ID_RulerMovePointBtn, MeasureDlg::OnRulerMovePoint)
	EVT_MENU(ID_RulerMovePencilBtn, MeasureDlg::OnRulerMovePencil)
	EVT_MENU(ID_RulerDelBtn, MeasureDlg::OnRulerDel)
	EVT_MENU(ID_RulerFlipBtn, MeasureDlg::OnRulerFlip)
	EVT_MENU(ID_RulerAvgBtn, MeasureDlg::OnRulerAvg)
	EVT_MENU(ID_LockBtn, MeasureDlg::OnLock)
	EVT_MENU(ID_PruneBtn, MeasureDlg::OnPrune)
	EVT_MENU(ID_RelaxBtn, MeasureDlg::OnRelax)
	EVT_MENU(ID_DeleteBtn, MeasureDlg::OnDelete)
	EVT_MENU(ID_DeleteAllBtn, MeasureDlg::OnDeleteAll)
	EVT_MENU(ID_ProfileBtn, MeasureDlg::OnProfile)
	EVT_MENU(ID_DistanceBtn, MeasureDlg::OnDistance)
	EVT_MENU(ID_ProjectBtn, MeasureDlg::OnProject)
	EVT_MENU(ID_ExportBtn, MeasureDlg::OnExport)
	EVT_RADIOBUTTON(ID_ViewPlaneRd, MeasureDlg::OnIntensityMethodCheck)
	EVT_RADIOBUTTON(ID_MaxIntensityRd, MeasureDlg::OnIntensityMethodCheck)
	EVT_RADIOBUTTON(ID_AccIntensityRd, MeasureDlg::OnIntensityMethodCheck)
	EVT_CHECKBOX(ID_UseTransferChk, MeasureDlg::OnUseTransferCheck)
	EVT_CHECKBOX(ID_TransientChk, MeasureDlg::OnTransientCheck)
	EVT_CHECKBOX(ID_DF_FChk, MeasureDlg::OnDF_FCheck)
	EVT_TOGGLEBUTTON(ID_AutoRelaxBtn, MeasureDlg::OnAutoRelax)
	EVT_SPINCTRLDOUBLE(ID_RelaxValueSpin, MeasureDlg::OnRelaxValueSpin)
	EVT_TEXT(ID_RelaxValueSpin, MeasureDlg::OnRelaxValueText)
	EVT_COMBOBOX(ID_RelaxDataCmb, MeasureDlg::OnRelaxData)
	//ruler list
	EVT_BUTTON(ID_NewGroup, MeasureDlg::OnNewGroup)
	EVT_BUTTON(ID_ChgGroup, MeasureDlg::OnChgGroup)
	EVT_BUTTON(ID_SelGroup, MeasureDlg::OnSelGroup)
	EVT_BUTTON(ID_DispTglGroup, MeasureDlg::OnDispTglGroup)
	//align
	EVT_BUTTON(ID_AlignX, MeasureDlg::OnAlignRuler)
	EVT_BUTTON(ID_AlignY, MeasureDlg::OnAlignRuler)
	EVT_BUTTON(ID_AlignZ, MeasureDlg::OnAlignRuler)
	EVT_BUTTON(ID_AlignNX, MeasureDlg::OnAlignRuler)
	EVT_BUTTON(ID_AlignNY, MeasureDlg::OnAlignRuler)
	EVT_BUTTON(ID_AlignNZ, MeasureDlg::OnAlignRuler)
	EVT_BUTTON(ID_AlignXYZ, MeasureDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignYXZ, MeasureDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignZXY, MeasureDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignXZY, MeasureDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignYZX, MeasureDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignZYX, MeasureDlg::OnAlignPca)
END_EVENT_TABLE()

MeasureDlg::MeasureDlg(VRenderFrame* frame)
	: wxPanel(frame, wxID_ANY,
	wxDefaultPosition, wxSize(500, 600),
	0, "MeasureDlg"),
	m_frame(frame),
	m_view(0),
	m_rhdl(0),
	m_edited(false)
{
	m_calculator = new flrd::DistCalculator();
	m_aligner = new flrd::RulerAlign();

	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText* st;
	//toolbar
	m_toolbar1 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER|wxTB_TEXT| wxTB_HORIZONTAL);
	wxBitmap bitmap = wxGetBitmapFromMemory(locator);
#ifdef _DARWIN
	m_toolbar1->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar1->AddCheckTool(ID_LocatorBtn, "Locator",
		bitmap, wxNullBitmap,
		"Add locators to the render view by clicking");
	bitmap = wxGetBitmapFromMemory(drill);
	m_toolbar1->AddCheckTool(ID_ProbeBtn, "Probe",
		bitmap, wxNullBitmap,
		"Add probes to the render view by clicking once");
	bitmap = wxGetBitmapFromMemory(two_point);
	m_toolbar1->AddCheckTool(ID_RulerBtn, "Line",
		bitmap, wxNullBitmap,
		"Add rulers to the render view by clicking at two end points");
	bitmap = wxGetBitmapFromMemory(protractor);
	m_toolbar1->AddCheckTool(ID_ProtractorBtn, "Angle",
		bitmap, wxNullBitmap,
		"Add protractors to measure angles by clicking at three points");
	bitmap = wxGetBitmapFromMemory(ellipse);
	m_toolbar1->AddCheckTool(ID_EllipseBtn, "Ellipse",
		bitmap, wxNullBitmap,
		"Add an ellipse to the render view by clicking at its points");
	bitmap = wxGetBitmapFromMemory(multi_point);
	m_toolbar1->AddCheckTool(ID_RulerMPBtn, "Polyline",
		bitmap, wxNullBitmap,
		"Add a polyline ruler to the render view by clicking at its points");
	bitmap = wxGetBitmapFromMemory(pencil);
	m_toolbar1->AddCheckTool(ID_PencilBtn, "Pencil",
		bitmap, wxNullBitmap,
		"Draw ruler without clicking each point");
	bitmap = wxGetBitmapFromMemory(grow);
	m_toolbar1->AddCheckTool(ID_GrowBtn, "Grow",
		bitmap, wxNullBitmap,
		"Click and hold to create ruler automatically based on data");
	m_toolbar1->Realize();
	//toolbar2
	m_toolbar2 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT | wxTB_HORIZONTAL);
	bitmap = wxGetBitmapFromMemory(move);
#ifdef _DARWIN
	m_toolbar2->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar2->AddCheckTool(ID_RulerMoveBtn, "Move",
		bitmap, wxNullBitmap,
		"Select and move ruler");
	bitmap = wxGetBitmapFromMemory(ruler_edit);
	m_toolbar2->AddCheckTool(ID_RulerMovePointBtn, "MovePoint",
		bitmap, wxNullBitmap,
		"Select and move a ruler point");
	bitmap = wxGetBitmapFromMemory(pencil);
	m_toolbar2->AddCheckTool(ID_RulerMovePencilBtn, "Edit",
		bitmap, wxNullBitmap,
		"Move ruler points by drawing");
	bitmap = wxGetBitmapFromMemory(ruler_del);
	m_toolbar2->AddCheckTool(ID_RulerDelBtn, "Delete",
		bitmap, wxNullBitmap,
		"Select and delete a ruler point");
	bitmap = wxGetBitmapFromMemory(flip_ruler);
	m_toolbar2->AddTool(ID_RulerFlipBtn, "Flip", bitmap,
		"Reverse the order of ruler points");
	bitmap = wxGetBitmapFromMemory(average);
	m_toolbar2->AddTool(ID_RulerAvgBtn, "Average", bitmap,
		"Compute a center for selected rulers");
	bitmap = wxGetBitmapFromMemory(prune);
	m_toolbar2->AddTool(ID_PruneBtn, "Prune", bitmap,
		"Remove very short branches from ruler");
	bitmap = wxGetBitmapFromMemory(lock);
	m_toolbar2->AddCheckTool(ID_LockBtn, "Lock",
		bitmap, wxNullBitmap,
		"Click to lock/unlock a ruler point for relaxing");
	bitmap = wxGetBitmapFromMemory(relax);
	m_toolbar2->AddTool(ID_RelaxBtn, "Relax", bitmap,
		"Relax ruler by components");
	m_toolbar2->Realize();
	//toolbar3
	m_toolbar3 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT | wxTB_HORIZONTAL);
	bitmap = wxGetBitmapFromMemory(delet);
#ifdef _DARWIN
	m_toolbar3->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar3->AddTool(ID_DeleteBtn, "Delete", bitmap,
		"Delete a selected ruler");
	bitmap = wxGetBitmapFromMemory(del_all);
	m_toolbar3->AddTool(ID_DeleteAllBtn,"Delete All", bitmap,
		"Delete all rulers");
	bitmap = wxGetBitmapFromMemory(eyedrop);
	m_toolbar3->AddTool(ID_ProfileBtn, "Sample", bitmap,
		"Sample intensity values along ruler");
	bitmap = wxGetBitmapFromMemory(tape);
	m_toolbar3->AddTool(ID_DistanceBtn, "Length", bitmap,
		"Calculate distances");
	bitmap = wxGetBitmapFromMemory(profile);
	m_toolbar3->AddTool(ID_ProjectBtn, "Project", bitmap,
		"Project components onto ruler");
	bitmap = wxGetBitmapFromMemory(save);
	m_toolbar3->AddTool(ID_ExportBtn, "Export", bitmap,
		"Export rulers to a text file");
	m_toolbar3->Realize();

	//options
	wxBoxSizer *sizer_1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Settings"), wxVERTICAL);
	wxBoxSizer* sizer_11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Z-Depth Comp.:",
		wxDefaultPosition, wxSize(90, -1));
	m_view_plane_rd = new wxRadioButton(this, ID_ViewPlaneRd, "View Plane",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_max_intensity_rd = new wxRadioButton(this, ID_MaxIntensityRd, "Maximum Intensity",
		wxDefaultPosition, wxDefaultSize);
	m_acc_intensity_rd = new wxRadioButton(this, ID_AccIntensityRd, "Accumulated Intensity",
		wxDefaultPosition, wxDefaultSize);
	m_view_plane_rd->SetValue(false);
	m_max_intensity_rd->SetValue(true);
	m_acc_intensity_rd->SetValue(false);
	sizer_11->Add(10, 10);
	sizer_11->Add(st, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_view_plane_rd, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_max_intensity_rd, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_acc_intensity_rd, 0, wxALIGN_CENTER);
	//more options
	wxBoxSizer* sizer_12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Properties:",
		wxDefaultPosition, wxSize(90, -1));
	m_transient_chk = new wxCheckBox(this, ID_TransientChk, "Transient",
		wxDefaultPosition, wxDefaultSize);
	m_use_transfer_chk = new wxCheckBox(this, ID_UseTransferChk, "Use Volume Properties",
		wxDefaultPosition, wxDefaultSize);
	m_df_f_chk = new wxCheckBox(this, ID_DF_FChk, L"Compute \u2206F/F",
		wxDefaultPosition, wxDefaultSize);
	sizer_12->Add(10, 10);
	sizer_12->Add(st, 0, wxALIGN_CENTER);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_transient_chk, 0, wxALIGN_CENTER);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_use_transfer_chk, 0, wxALIGN_CENTER);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_df_f_chk, 0, wxALIGN_CENTER);
	//relax settings
	wxBoxSizer* sizer_13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Relax:",
		wxDefaultPosition, wxSize(90, -1));
	sizer_13->Add(10, 10);
	sizer_13->Add(st, 0, wxALIGN_CENTER);
	m_auto_relax_btn = new wxToggleButton(this, ID_AutoRelaxBtn,
		"Auto Relax", wxDefaultPosition, wxSize(75, -1));
	sizer_13->Add(10, 10);
	sizer_13->Add(m_auto_relax_btn, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Constraint ");
	sizer_13->Add(10, 10);
	sizer_13->Add(st, 0, wxALIGN_CENTER);
	m_relax_data_cmb = new wxComboBox(this, ID_RelaxDataCmb, "",
		wxDefaultPosition, wxSize(100, -1), 0, NULL, wxCB_READONLY);
	m_relax_data_cmb->Append("Free");
	m_relax_data_cmb->Append("Volume");
	m_relax_data_cmb->Append("Selection");
	m_relax_data_cmb->Append("Analyzed Comp.");
	m_relax_data_cmb->Select(3);
	sizer_13->Add(m_relax_data_cmb, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Ex/In Ratio ");
	sizer_13->Add(10, 10);
	sizer_13->Add(st, 0, wxALIGN_CENTER);
	m_relax_value_spin = new wxSpinCtrlDouble(
		this, ID_RelaxValueSpin, "2",
		wxDefaultPosition, wxSize(50, 23),
		wxSP_ARROW_KEYS | wxSP_WRAP,
		0, 100, 2, 0.1);
	sizer_13->Add(m_relax_value_spin, 0, wxALIGN_CENTER);
	//
	sizer_1->Add(sizer_11, 0, wxEXPAND);
	sizer_1->Add(10, 10);
	sizer_1->Add(sizer_12, 0, wxEXPAND);
	sizer_1->Add(10, 10);
	sizer_1->Add(sizer_13, 0, wxEXPAND);

	//list
	wxBoxSizer *sizer_2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Ruler List"), wxVERTICAL);
	wxBoxSizer* sizer21 = new wxBoxSizer(wxHORIZONTAL);
	m_new_group = new wxButton(this, ID_NewGroup, "New Group",
		wxDefaultPosition, wxSize(65, 22));
	st = new wxStaticText(this, 0, "Group ID:",
		wxDefaultPosition, wxSize(65, -1));
	m_group_text = new wxTextCtrl(this, ID_GroupText, "0",
		wxDefaultPosition, wxSize(40, 22), 0, vald_int);
	m_chg_group = new wxButton(this, ID_ChgGroup, "Change",
		wxDefaultPosition, wxSize(65, 22));
	m_sel_group = new wxButton(this, ID_SelGroup, "Select",
		wxDefaultPosition, wxSize(65, 22));
	m_disptgl_group = new wxButton(this, ID_DispTglGroup, "Display",
		wxDefaultPosition, wxSize(65, 22));
	sizer21->Add(m_new_group, 0, wxALIGN_CENTER);
	sizer21->AddStretchSpacer();
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(m_group_text, 0, wxALIGN_CENTER);
	sizer21->Add(m_chg_group, 0, wxALIGN_CENTER);
	sizer21->Add(m_sel_group, 0, wxALIGN_CENTER);
	sizer21->Add(m_disptgl_group, 0, wxALIGN_CENTER);
	m_rulerlist = new RulerListCtrl(frame, this,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	sizer_2->Add(sizer21, 0, wxEXPAND);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_rulerlist, 1, wxEXPAND);

	//alignment
	wxBoxSizer *sizer_3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Align Render View to Ruler(s)"), wxVERTICAL);
	wxBoxSizer* sizer31 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center = new wxCheckBox(this, ID_AlignCenter,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer31->Add(5, 5);
	sizer31->Add(m_align_center, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Mono Axis:",
		wxDefaultPosition, wxSize(65, 22));
	m_align_x = new wxButton(this, ID_AlignX, "X",
		wxDefaultPosition, wxSize(65, 22));
	m_align_y = new wxButton(this, ID_AlignY, "Y",
		wxDefaultPosition, wxSize(65, 22));
	m_align_z = new wxButton(this, ID_AlignZ, "Z",
		wxDefaultPosition, wxSize(65, 22));
	m_align_nx = new wxButton(this, ID_AlignNX, "-X",
		wxDefaultPosition, wxSize(65, 22));
	m_align_ny = new wxButton(this, ID_AlignNY, "-Y",
		wxDefaultPosition, wxSize(65, 22));
	m_align_nz = new wxButton(this, ID_AlignNZ, "-Z",
		wxDefaultPosition, wxSize(65, 22));
	sizer32->Add(5, 5);
	sizer32->Add(st, 0, wxALIGN_CENTER);
	sizer32->Add(m_align_x, 0, wxALIGN_CENTER);
	sizer32->Add(m_align_y, 0, wxALIGN_CENTER);
	sizer32->Add(m_align_z, 0, wxALIGN_CENTER);
	sizer32->Add(m_align_nx, 0, wxALIGN_CENTER);
	sizer32->Add(m_align_ny, 0, wxALIGN_CENTER);
	sizer32->Add(m_align_nz, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer33 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Tri Axes:",
		wxDefaultPosition, wxSize(65, 22));
	m_align_xyz = new wxButton(this, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, wxSize(65, 22));
	m_align_yxz = new wxButton(this, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, wxSize(65, 22));
	m_align_zxy = new wxButton(this, ID_AlignZXY, "ZXY",
		wxDefaultPosition, wxSize(65, 22));
	m_align_xzy = new wxButton(this, ID_AlignXZY, "XZY",
		wxDefaultPosition, wxSize(65, 22));
	m_align_yzx = new wxButton(this, ID_AlignYZX, "YZX",
		wxDefaultPosition, wxSize(65, 22));
	m_align_zyx = new wxButton(this, ID_AlignZYX, "ZYX",
		wxDefaultPosition, wxSize(65, 22));
	sizer33->Add(5, 5);
	sizer33->Add(st, 0, wxALIGN_CENTER);
	sizer33->Add(m_align_xyz, 0, wxALIGN_CENTER);
	sizer33->Add(m_align_yxz, 0, wxALIGN_CENTER);
	sizer33->Add(m_align_zxy, 0, wxALIGN_CENTER);
	sizer33->Add(m_align_xzy, 0, wxALIGN_CENTER);
	sizer33->Add(m_align_yzx, 0, wxALIGN_CENTER);
	sizer33->Add(m_align_zyx, 0, wxALIGN_CENTER);
	//
	sizer_3->Add(5, 5);
	sizer_3->Add(sizer31, 0, wxEXPAND);
	sizer_3->Add(5, 5);
	sizer_3->Add(sizer32, 0, wxEXPAND);
	sizer_3->Add(5, 5);
	sizer_3->Add(sizer33, 0, wxEXPAND);
	sizer_3->Add(5, 5);

	//sizer
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(m_toolbar1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_toolbar2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_toolbar3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 1, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

MeasureDlg::~MeasureDlg()
{
	if (m_calculator)
		delete m_calculator;
	if (m_aligner)
		delete m_aligner;
}

void MeasureDlg::GetSettings(VRenderGLView* vrv)
{
	m_view = vrv;
	if (!m_view)
		return;
	m_rhdl = m_view->GetRulerHandler();
	if (!m_rhdl)
		return;
	m_rulerlist->m_rhdl = m_rhdl;
	m_aligner->SetView(m_view);

	UpdateList();
	if (m_view)
	{
		m_toolbar1->ToggleTool(ID_LocatorBtn, false);
		m_toolbar1->ToggleTool(ID_ProbeBtn, false);
		m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
		m_toolbar1->ToggleTool(ID_RulerBtn, false);
		m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
		m_toolbar1->ToggleTool(ID_EllipseBtn, false);
		m_toolbar1->ToggleTool(ID_GrowBtn, false);
		m_toolbar1->ToggleTool(ID_PencilBtn, false);
		m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
		m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
		m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
		m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
		m_toolbar2->ToggleTool(ID_LockBtn, false);

		int int_mode = m_view->GetIntMode();
		if (int_mode == 5 || int_mode == 7)
		{
			int ruler_type = m_rhdl->GetType();
			if (ruler_type == 0)
				m_toolbar1->ToggleTool(ID_RulerBtn, true);
			else if (ruler_type == 1)
				m_toolbar1->ToggleTool(ID_RulerMPBtn, true);
			else if (ruler_type == 2)
				m_toolbar1->ToggleTool(ID_LocatorBtn, true);
			else if (ruler_type == 3)
				m_toolbar1->ToggleTool(ID_ProbeBtn, true);
			else if (ruler_type == 4)
				m_toolbar1->ToggleTool(ID_ProtractorBtn, true);
			else if (ruler_type == 5)
				m_toolbar1->ToggleTool(ID_EllipseBtn, true);
		}
		else if (int_mode == 6)
			m_toolbar2->ToggleTool(ID_RulerMovePointBtn, true);
		else if (int_mode == 9)
			m_toolbar2->ToggleTool(ID_RulerMoveBtn, true);
		else if (int_mode == 11)
			m_toolbar2->ToggleTool(ID_LockBtn, true);
		else if (int_mode == 12)
			m_toolbar1->ToggleTool(ID_GrowBtn, true);
		else if (int_mode == 13)
			m_toolbar1->ToggleTool(ID_PencilBtn, true);
		else if (int_mode == 14)
			m_toolbar2->ToggleTool(ID_RulerDelBtn, true);
		else if (int_mode == 15)
			m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, true);

		switch (m_view->m_point_volume_mode)
		{
		case 0:
			m_view_plane_rd->SetValue(true);
			m_max_intensity_rd->SetValue(false);
			m_acc_intensity_rd->SetValue(false);
			break;
		case 1:
			m_view_plane_rd->SetValue(false);
			m_max_intensity_rd->SetValue(true);
			m_acc_intensity_rd->SetValue(false);
			break;
		case 2:
			m_view_plane_rd->SetValue(false);
			m_max_intensity_rd->SetValue(false);
			m_acc_intensity_rd->SetValue(true);
			break;
		}

		m_use_transfer_chk->SetValue(m_view->m_ruler_use_transf);
		m_transient_chk->SetValue(m_view->m_ruler_time_dep);
		if (m_frame && m_frame->GetSettingDlg())
		{
			//ruler exports df/f
			bool bval = m_frame->GetSettingDlg()->GetRulerDF_F();
			m_df_f_chk->SetValue(bval);
			m_rhdl->SetBackground(bval);
			//relax
			m_relax_value_spin->SetValue(
				m_frame->GetSettingDlg()->GetRulerRelaxF1());
			m_auto_relax_btn->SetValue(
				m_frame->GetSettingDlg()->GetRulerAutoRelax());
			m_view->m_ruler_autorelax =
				m_frame->GetSettingDlg()->GetRulerAutoRelax();
			m_relax_data_cmb->Select(
				m_frame->GetSettingDlg()->GetRulerRelaxType());
		}
	}
}

VRenderGLView* MeasureDlg::GetView()
{
	return m_view;
}

void MeasureDlg::UpdateList()
{
	if (!m_view) return;
	m_rulerlist->UpdateRulers(m_view);
}

void MeasureDlg::OnNewLocator(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_LocatorBtn))
	{
		m_view->SetIntMode(5);
		m_rhdl->SetType(2);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewProbe(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_ProbeBtn))
	{
		m_view->SetIntMode(5);
		m_rhdl->SetType(3);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewProtractor(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_ProtractorBtn))
	{
		m_view->SetIntMode(5);
		m_rhdl->SetType(4);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewRuler(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_RulerBtn))
	{
		m_view->SetIntMode(5);
		m_rhdl->SetType(0);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewRulerMP(wxCommandEvent& event)
{
	if (!m_view) return;

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
	{
		m_view->SetIntMode(5);
		m_rhdl->SetType(1);
	}
	else
	{
		m_view->SetIntMode(1);
		m_rhdl->FinishRuler();
	}
}

void MeasureDlg::OnEllipse(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_EllipseBtn))
	{
		m_view->SetIntMode(5);
		m_rhdl->SetType(5);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnGrow(wxCommandEvent& event)
{
	if (!m_view) return;

	m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_GrowBtn))
	{
		m_view->SetIntMode(12);
		m_rhdl->SetType(1);
		if (m_view->GetRulerRenderer())
			m_view->GetRulerRenderer()->SetDrawText(false);
		//reset label volume
		if (m_view->m_cur_vol)
		{
			m_view->m_cur_vol->
				GetVR()->clear_tex_mask();
			m_view->m_cur_vol->
				GetVR()->clear_tex_label();
			m_view->m_cur_vol->
				AddEmptyMask(0, true);
			m_view->m_cur_vol->
				AddEmptyLabel(0, true);
		}
	}
	else
	{
		m_view->SetIntMode(1);
		if (m_view->GetRulerRenderer())
			m_view->GetRulerRenderer()->SetDrawText(true);
	}
}

void MeasureDlg::OnPencil(wxCommandEvent& event)
{
	if (!m_view) return;

	m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar1->GetToolState(ID_PencilBtn))
	{
		m_view->SetIntMode(13);
		m_rhdl->SetType(1);
		//if (m_view->m_glview->GetRulerRenderer())
		//	m_view->m_glview->GetRulerRenderer()->SetDrawText(false);
	}
	else
	{
		m_view->SetIntMode(1);
		//if (m_view->m_glview->GetRulerRenderer())
		//	m_view->m_glview->GetRulerRenderer()->SetDrawText(true);
	}
}

void MeasureDlg::OnRulerFlip(wxCommandEvent& event)
{
	if (!m_view)
		return;

	int count = 0;
	std::vector<int> sel;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			int index = sel[i];
			if (0 > index || ruler_list->size() <= index)
				continue;
			flrd::Ruler* r = (*ruler_list)[index];
			if (r)
				r->Reverse();
			count++;
		}
	}
	else
	{
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			flrd::Ruler* r = (*ruler_list)[i];
			if (r)
				r->Reverse();
			count++;
		}
	}

	if (count)
	{
		m_view->RefreshGL(39);
		GetSettings(m_view);
	}
	m_edited = true;
}

void MeasureDlg::OnRulerMovePoint(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar2->GetToolState(ID_RulerMovePointBtn))
		m_view->SetIntMode(6);
	else
		m_view->SetIntMode(1);

	//m_edited = true;
}

void MeasureDlg::OnRulerMovePencil(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar2->GetToolState(ID_RulerMovePencilBtn))
		m_view->SetIntMode(15);
	else
		m_view->SetIntMode(1);

	//m_edited = true;
}

void MeasureDlg::OnRulerDel(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar2->GetToolState(ID_RulerDelBtn))
		m_view->SetIntMode(14);
	else
		m_view->SetIntMode(1);

	//m_edited = true;
}

void MeasureDlg::OnRulerMove(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_LockBtn, false);

	if (m_toolbar2->GetToolState(ID_RulerMoveBtn))
		m_view->SetIntMode(9);
	else
		m_view->SetIntMode(1);

	//m_edited = true;
}

void MeasureDlg::OnRulerAvg(wxCommandEvent& event)
{
	if (!m_view)
		return;

	fluo::Point avg;
	int count = 0;
	std::vector<int> sel;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			int index = sel[i];
			if (0 > index || ruler_list->size() <= index)
				continue;
			flrd::Ruler* r = (*ruler_list)[index];
			avg += r->GetCenter();
			count++;
		}
	}
	else
	{
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			flrd::Ruler* r = (*ruler_list)[i];
			if (r->GetDisp())
			{
				avg += r->GetCenter();
				count++;
			}
		}
	}

	if (count)
	{
		avg /= double(count);
		flrd::Ruler* ruler = new flrd::Ruler();
		ruler->SetRulerType(2);
		ruler->SetName("Average");
		ruler->AddPoint(avg);
		ruler->SetTransient(m_view->m_ruler_time_dep);
		ruler->SetTransTime(m_view->m_tseq_cur_num);
		ruler_list->push_back(ruler);
		m_view->RefreshGL(39);
		GetSettings(m_view);
	}
}

void MeasureDlg::OnProfile(wxCommandEvent& event)
{
	if (m_view)
	{
		m_rhdl->SetVolumeData(m_view->m_cur_vol);
		std::vector<int> sel;
		if (m_rulerlist->GetCurrSelection(sel))
		{
			//export selected
			for (size_t i = 0; i < sel.size(); ++i)
			{
				m_rhdl->Profile(sel[i]);
				SetProfile(sel[i]);
			}
		}
		else
		{
			//export all
			flrd::RulerList* ruler_list = m_view->GetRulerList();
			for (size_t i = 0; i < ruler_list->size(); ++i)
			{
				if ((*ruler_list)[i]->GetDisp())
				{
					m_rhdl->Profile(i);
					SetProfile(i);
				}
			}
		}
	}
}

void MeasureDlg::OnDistance(wxCommandEvent& event)
{
	if (!m_view || !m_frame)
		return;

	flrd::ComponentAnalyzer* analyzer =
		m_frame->GetComponentDlg()->GetAnalyzer();
	if (!analyzer)
		return;
	m_rhdl->SetCompAnalyzer(analyzer);

	std::string filename;
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxtr = fopendlg->GetPath();
		filename = wxtr.ToStdString();
		//remove suffix
		filename = filename.substr(0, filename.find_last_of('.'));
	}

	std::vector<int> sel;
	std::string fi;
	if (m_rulerlist->GetCurrSelection(sel))
	{
		//export selected
		for (size_t i = 0; i < sel.size(); ++i)
		{
			fi = filename + std::to_string(i) + ".txt";
			m_rhdl->Distance(sel[i], fi);
		}
	}
	else
	{
		flrd::RulerList* ruler_list = m_view->GetRulerList();
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			if ((*ruler_list)[i]->GetDisp())
			{
				fi = filename + std::to_string(i) + ".txt";
				m_rhdl->Distance(i, fi);
			}
		}
	}

	if (fopendlg)
		delete fopendlg;
}

void MeasureDlg::OnProject(wxCommandEvent& event)
{
	if (m_view)
	{
		std::vector<int> sel;
		if (m_rulerlist->GetCurrSelection(sel))
		{
			//export selected
			for (size_t i = 0; i < sel.size(); ++i)
				Project(sel[i]);
		}
	}
}

void MeasureDlg::Project(int idx)
{
	if (!m_view || !m_frame)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	if (idx < 0 || idx >= ruler_list->size())
		return;
	flrd::Ruler* ruler = ruler_list->at(idx);
	flrd::ComponentAnalyzer* analyzer =
		m_frame->GetComponentDlg()->GetAnalyzer();
	flrd::CelpList* list = 0;
	if (!analyzer)
		return;
	list = analyzer->GetCelpList();
	if (list->empty())
		return;

	m_calculator->SetCelpList(list);
	m_calculator->SetRuler(ruler);
	m_calculator->Project();

	std::vector<flrd::Celp> comps;
	for (auto it = list->begin();
		it != list->end(); ++it)
		comps.push_back(it->second);
	std::sort(comps.begin(), comps.end(),
		[](const flrd::Celp &a, const flrd::Celp &b) -> bool
		{
		fluo::Point pa = a->GetProjp();
		fluo::Point pb = b->GetProjp();
		if (pa.z() != pb.z()) return pa.z() < pb.z();
		else return pa.x() < pb.x();
		});

	//export
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		string str = filename.ToStdString();
		
		std::ofstream ofs;
		ofs.open(str, std::ofstream::out);
		
		for (auto it = comps.begin();
			it != comps.end(); ++it)
		{
			ofs << (*it)->Id() << "\t";
			fluo::Point p = (*it)->GetProjp();
			ofs << p.x() << "\t";
			ofs << p.y() << "\t";
			ofs << p.z() << "\n";
		}
		ofs.close();
	}
	if (fopendlg)
		delete fopendlg;
}

void MeasureDlg::Relax()
{
	std::vector<int> sel;
	if (m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
			Relax(sel[i]);
	}
}

void MeasureDlg::Relax(int idx)
{
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	if (idx < 0 || idx >= ruler_list->size())
		return;
	flrd::Ruler* ruler = ruler_list->at(idx);
	flrd::ComponentAnalyzer* analyzer = 0;
	if (m_frame && m_frame->GetComponentDlg())
		analyzer = m_frame->GetComponentDlg()->GetAnalyzer();
	if (!analyzer)
		return;
	flrd::CelpList* list = 0;
	list = analyzer->GetCelpList();
	if (list && list->empty())
		list = 0;
	double infr = 2.0;
	int type = 1;
	int iter = 10;
	if (m_frame && m_frame->GetSettingDlg())
	{
		iter = m_frame->GetSettingDlg()->GetRulerRelaxIter();
		infr = m_frame->GetSettingDlg()->GetRulerInfr();
		type = m_frame->GetSettingDlg()->GetRulerRelaxType();
	}

	m_calculator->SetF1(m_relax_value_spin->GetValue());
	m_calculator->SetInfr(infr);
	m_calculator->SetCelpList(list);
	m_calculator->SetRuler(ruler);
	m_calculator->SetVolume(m_view->m_cur_vol);
	m_calculator->CenterRuler(type, m_edited, iter);
	m_edited = false;
	m_view->RefreshGL(39);
	GetSettings(m_view);
}

void MeasureDlg::Prune(int len)
{
	std::vector<int> sel;
	if (m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
			Prune(sel[i], len);
	}
	if (m_view)
		m_view->RefreshGL(39);
	GetSettings(m_view);
}

void MeasureDlg::Prune(int idx, int len)
{
	m_rhdl->Prune(idx, len);
}

void MeasureDlg::OnLock(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar1->GetToolState(ID_RulerMPBtn))
		m_rhdl->FinishRuler();

	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar1->ToggleTool(ID_RulerBtn, false);
	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
	m_toolbar1->ToggleTool(ID_GrowBtn, false);
	m_toolbar1->ToggleTool(ID_PencilBtn, false);
	m_toolbar2->ToggleTool(ID_RulerDelBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);

	if (m_toolbar2->GetToolState(ID_LockBtn))
		m_view->SetIntMode(11);
	else
		m_view->SetIntMode(1);
}

void MeasureDlg::OnPrune(wxCommandEvent& event)
{
	Prune(1);
}

void MeasureDlg::OnRelax(wxCommandEvent& event)
{
	Relax();
}

void MeasureDlg::OnDelete(wxCommandEvent& event)
{
	m_rulerlist->DeleteSelection();
}

void MeasureDlg::OnDeleteAll(wxCommandEvent& event)
{
	m_rulerlist->DeleteAll();
}

void MeasureDlg::OnExport(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Export rulers", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();

	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_rulerlist->Export(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void MeasureDlg::OnIntensityMethodCheck(wxCommandEvent& event)
{
	if (!m_view)
		return;

	int mode = 0;
	int sender_id = event.GetId();
	switch (sender_id)
	{
	case ID_ViewPlaneRd:
		mode = 0;
		break;
	case ID_MaxIntensityRd:
		mode = 1;
		break;
	case ID_AccIntensityRd:
		mode = 2;
		break;
	}
	m_view->m_point_volume_mode = mode;
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetPointVolumeMode(mode);
}

void MeasureDlg::OnUseTransferCheck(wxCommandEvent& event)
{
	if (!m_view)
		return;

	bool use_transfer = m_use_transfer_chk->GetValue();
	m_view->m_ruler_use_transf = use_transfer;
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRulerUseTransf(use_transfer);
}

void MeasureDlg::OnTransientCheck(wxCommandEvent& event)
{
	if (!m_view)
		return;

	bool val = m_transient_chk->GetValue();
	m_view->m_ruler_time_dep = val;
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRulerTimeDep(val);

	//change ruler setting
	std::vector<int> sel;
	if (!m_rulerlist->GetCurrSelection(sel))
		return;
	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = m_view->GetRuler(
			m_rulerlist->GetItemData(index));
		if (!ruler)
			continue;
		ruler->SetTransient(val);
	}
	m_rulerlist->UpdateRulers(m_view);
}

void MeasureDlg::OnDF_FCheck(wxCommandEvent& event)
{
	if (!m_rhdl)
		return;

	bool val = m_df_f_chk->GetValue();
	m_rhdl->SetBackground(val);
	if (val)
		OnProfile(event);
	m_rulerlist->UpdateRulers(m_view);

	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRulerDF_F(val);
}

void MeasureDlg::OnAutoRelax(wxCommandEvent& event)
{
	bool bval = m_auto_relax_btn->GetValue();
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRulerAutoRelax(bval);
	if (m_view)
		m_view->m_ruler_autorelax = bval;
}

void MeasureDlg::OnRelaxValueSpin(wxSpinDoubleEvent& event)
{
	double dval = m_relax_value_spin->GetValue();
	m_calculator->SetF1(dval);
	//relax
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRulerRelaxF1(dval);
}

void MeasureDlg::OnRelaxValueText(wxCommandEvent& event)
{
	wxString str = m_relax_value_spin->GetText()->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_calculator->SetF1(dval);
		//relax
		if (m_frame && m_frame->GetSettingDlg())
			m_frame->GetSettingDlg()->SetRulerRelaxF1(dval);
	}
}

void MeasureDlg::OnRelaxData(wxCommandEvent& event)
{
	int ival = m_relax_data_cmb->GetSelection();
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRulerRelaxType(ival);
}

//ruler list
void MeasureDlg::OnNewGroup(wxCommandEvent& event)
{
	if (m_rhdl)
		m_rhdl->NewGroup();
}

void MeasureDlg::OnChgGroup(wxCommandEvent& event)
{
	if (!m_rhdl)
		return;
	unsigned long ival;
	if (m_group_text->GetValue().ToULong(&ival))
		m_rhdl->SetGroup(ival);

	//update group
	if (!m_view)
		return;
	std::vector<int> sel;
	if (!m_rulerlist->GetCurrSelection(sel))
		return;
	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = m_view->GetRuler(
			m_rulerlist->GetItemData(index));
		if (!ruler)
			continue;
		ruler->Group(ival);
	}
	m_rulerlist->UpdateRulers(m_view);
}

void MeasureDlg::OnSelGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (!m_group_text->GetValue().ToULong(&ival))
		return;
	m_rulerlist->SelectGroup(ival);
}

void MeasureDlg::OnDispTglGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (!m_group_text->GetValue().ToULong(&ival))
		return;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;
	bool disp;
	bool first = true;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == ival)
		{
			if (first)
			{
				first = false;
				disp = !ruler->GetDisp();
			}
			ruler->SetDisp(disp);
			if (disp)
				m_rulerlist->SetItemBackgroundColour(i, wxColour(255, 255, 255));
			else
				m_rulerlist->SetItemBackgroundColour(i, wxColour(200, 200, 200));
		}
	}
	m_view->RefreshGL(39);
}

void MeasureDlg::AlignCenter(flrd::Ruler* ruler, flrd::RulerList* ruler_list)
{
	fluo::Point center;
	bool valid_center = false;
	if (ruler)
	{
		center = ruler->GetCenter();
		valid_center = true;
	}
	else if (ruler_list && !ruler_list->empty())
	{
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			flrd::Ruler* r = (*ruler_list)[i];
			center += r->GetCenter();
		}
		center /= double(ruler_list->size());
		valid_center = true;
	}
	if (valid_center)
	{
		double tx, ty, tz;
		m_view->GetObjCenters(tx, ty, tz);
		m_view->SetObjTrans(
			tx - center.x(),
			center.y() - ty,
			center.z() - tz);
	}
}

void MeasureDlg::SetProfile(int i)
{
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;
	flrd::Ruler* ruler = (*ruler_list)[i];
	double dval = ruler->GetProfileMaxValue();
	dval *= ruler->GetScalarScale();
	wxString str;
	if (m_rhdl->GetBackground())
	{
		double bg_int = m_rhdl->GetVolumeBgInt();
		dval = (dval - bg_int) / bg_int;
		str = wxString::Format("%.4f", dval);
	}
	else
		str = wxString::Format("%.0f", dval);
	m_rulerlist->SetText(i, IntCol, str);
}

void MeasureDlg::OnAlignRuler(wxCommandEvent& event)
{
	std::vector<int> sel;
	if (!m_rulerlist->GetCurrSelection(sel))
		return;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	flrd::Ruler* ruler = ruler_list->at(sel[0]);
	m_aligner->SetRuler(ruler);

	int axis_type = 0;
	switch (event.GetId())
	{
	case ID_AlignX:
		axis_type = 0;
		break;
	case ID_AlignY:
		axis_type = 1;
		break;
	case ID_AlignZ:
		axis_type = 2;
		break;
	case ID_AlignNX:
		axis_type = 3;
		break;
	case ID_AlignNY:
		axis_type = 4;
		break;
	case ID_AlignNZ:
		axis_type = 5;
		break;
	}
	m_aligner->AlignRuler(axis_type);
	if (m_align_center->GetValue())
		AlignCenter(ruler, 0);
}

void MeasureDlg::OnAlignPca(wxCommandEvent& event)
{
	flrd::RulerList list;
	std::vector<int> sel;
	if (!m_rulerlist->GetCurrSelection(sel))
		return;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	for (int i = 0; i < sel.size(); ++i)
		list.push_back((*ruler_list)[sel[i]]);
	m_aligner->SetRulerList(&list);

	int axis_type = 0;
	switch (event.GetId())
	{
	case ID_AlignXYZ:
		axis_type = 0;
		break;
	case ID_AlignYXZ:
		axis_type = 1;
		break;
	case ID_AlignZXY:
		axis_type = 2;
		break;
	case ID_AlignXZY:
		axis_type = 3;
		break;
	case ID_AlignYZX:
		axis_type = 4;
		break;
	case ID_AlignZYX:
		axis_type = 5;
		break;
	}
	m_aligner->AlignPca(axis_type);
	if (m_align_center->GetValue())
		AlignCenter(0, &list);
}

