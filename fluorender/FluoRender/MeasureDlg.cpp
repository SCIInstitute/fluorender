/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include <sstream>
#include <fstream>
#include <wx/artprov.h>
#include <wx/valnum.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include "Formats/png_resource.h"
#include "ruler.xpm"

//resources
#include "img/icons.h"

BEGIN_EVENT_TABLE(RulerListCtrl, wxListCtrl)
	EVT_KEY_DOWN(RulerListCtrl::OnKeyDown)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, RulerListCtrl::OnSelection)
	EVT_LIST_ITEM_DESELECTED(wxID_ANY, RulerListCtrl::OnEndSelection)
	EVT_TEXT(ID_NameText, RulerListCtrl::OnNameText)
	EVT_COLOURPICKER_CHANGED(ID_ColorPicker, RulerListCtrl::OnColorChange)
	EVT_SCROLLWIN(RulerListCtrl::OnScroll)
	EVT_MOUSEWHEEL(RulerListCtrl::OnScroll)
	END_EVENT_TABLE()

	RulerListCtrl::RulerListCtrl(
	wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
wxListCtrl(parent, id, pos, size, style)//,
	//m_frame(frame)
{
	wxListItem itemCol;
	itemCol.SetText("Name");
	this->InsertColumn(0, itemCol);
	SetColumnWidth(0, 100);
	itemCol.SetText("Color");
	this->InsertColumn(1, itemCol);
	SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Length");
	this->InsertColumn(2, itemCol);
	SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Angle/Pitch");
	this->InsertColumn(3, itemCol);
	SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Start/End Points (X, Y, Z)");
	this->InsertColumn(4, itemCol);
	SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Time");
	this->InsertColumn(5, itemCol);
	SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Volumes");
	this->InsertColumn(6, itemCol);
	SetColumnWidth(6, wxLIST_AUTOSIZE_USEHEADER);

	m_images = new wxImageList(16, 16, true);
	wxIcon icon = wxIcon(ruler_xpm);
	m_images->Add(icon);
	AssignImageList(m_images, wxIMAGE_LIST_SMALL);

	//frame edit
	m_name_text = new wxTextCtrl(this, ID_NameText, "",
		wxDefaultPosition, wxDefaultSize);
	m_name_text->Hide();
	m_color_picker = new wxColourPickerCtrl(this, 
		ID_ColorPicker);
	m_color_picker->Hide();
}

RulerListCtrl::~RulerListCtrl()
{
}

void RulerListCtrl::Append(wxString name, wxString &color, double length, wxString &unit,
	double angle, wxString &points, bool time_dep, int time, wxString extra)
{
	long tmp = InsertItem(GetItemCount(), name, 0);
	//    SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	SetItem(tmp, 1, color);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	wxString str = wxString::Format("%.2f", length) + unit;
	SetItem(tmp, 2, str);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
	str = wxString::Format("%.1f", angle) + "Deg";
	SetItem(tmp, 3, str);
	SetColumnWidth(3, wxLIST_AUTOSIZE);
	SetItem(tmp, 4, points);
	if (time_dep)
		str = wxString::Format("%d", time);
	else
		str = "N/A";
	SetItem(tmp, 5, str);
	SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);
	SetItem(tmp, 6, extra);
	SetColumnWidth(6, wxLIST_AUTOSIZE_USEHEADER);
}

void RulerListCtrl::UpdateRulers(VRenderView* vrv)
{
	m_name_text->Hide();
	m_color_picker->Hide();
	if (vrv)
		m_view = vrv;

	vector<Ruler*>* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;

	DeleteAllItems();

	wxString points;
	Point *p;
	int num_points;
	for (int i=0; i<(int)ruler_list->size(); i++)
	{
		Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->GetTimeDep() &&
			ruler->GetTime() != m_view->m_glview->m_tseq_cur_num)
			continue;

		wxString unit;
		switch (m_view->m_glview->m_sb_unit)
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
			points += wxString::Format("(%.2f, %.2f, %.2f)", p->x(), p->y(), p->z());
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1);
			points += ", ";
			points += wxString::Format("(%.2f, %.2f, %.2f)", p->x(), p->y(), p->z());
		}
		wxString color;
		if (ruler->GetUseColor())
			color = wxString::Format("RGB(%d, %d, %d)",
			int(ruler->GetColor().r()*255),
			int(ruler->GetColor().g()*255),
			int(ruler->GetColor().b()*255));
		else
			color = "N/A";
		Append(ruler->GetName(), color, ruler->GetLength(), unit,
			ruler->GetAngle(), points, ruler->GetTimeDep(), ruler->GetTime(), ruler->GetDelInfoValues(", "));
	}
}

int RulerListCtrl::GetCurrSelection()
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	return int(item);
}

void RulerListCtrl::DeleteSelection()
{
	if (!m_view) return;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetItemText(item);
		vector<Ruler*>* ruler_list = m_view->GetRulerList();
		if (ruler_list)
		{
			for (int i=0; i<(int)ruler_list->size(); i++)
			{
				Ruler* ruler = (*ruler_list)[i];
				if (ruler && ruler->GetName()==name)
				{
					ruler_list->erase(ruler_list->begin()+i);
					delete ruler;
				}
			}
			UpdateRulers();
			m_view->RefreshGL();
		}
	}
}

void RulerListCtrl::DeleteAll(bool cur_time)
{
	if (!m_view) return;

	vector<Ruler*>* ruler_list = m_view->GetRulerList();
	if (ruler_list)
	{
		if (cur_time)
		{
			int tseq = m_view->m_glview->m_tseq_cur_num;
			for (int i=ruler_list->size()-1; i>=0; i--)
			{
				Ruler* ruler = (*ruler_list)[i];
				if (ruler &&
					((ruler->GetTimeDep() &&
					ruler->GetTime() == tseq) ||
					!ruler->GetTimeDep()))
				{
					ruler_list->erase(ruler_list->begin()+i);
					delete ruler;
				}
			}
		}
		else
		{
			for (int i=ruler_list->size()-1; i>=0; i--)
			{
				Ruler* ruler = (*ruler_list)[i];
				if (ruler)
					delete ruler;
			}
			ruler_list->clear();
		}

		UpdateRulers();
		m_view->RefreshGL();
	}
}

void RulerListCtrl::Export(wxString filename)
{
	if (!m_view) return;
	vector<Ruler*>* ruler_list = m_view->GetRulerList();
	if (ruler_list)
	{
		wxFileOutputStream fos(filename);
		if (!fos.Ok())
			return;
		wxTextOutputStream tos(fos);

		wxString str;
		wxString unit;
		int num_points;
		Point *p;
		Ruler* ruler;
		switch (m_view->m_glview->m_sb_unit)
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

		tos << "Name\tColor\tLength(" << unit << ")\tAngle/Pitch(Deg)\tx1\ty1\tz1\txn\tyn\tzn\tTime\tv1\tv2\n";

		Color color;
		for (size_t i=0; i<ruler_list->size(); i++)
		{
			ruler = (*ruler_list)[i];
			if (!ruler) continue;

			tos << ruler->GetName() << "\t";
			if (ruler->GetUseColor())
			{
				color = ruler->GetColor();
				str = wxString::Format("RGB(%d, %d, %d)",
					int(color.r()*255), int(color.g()*255), int(color.b()*255));
			}
			else
				str = "N/A";
			tos << str << "\t";
			str = wxString::Format("%.2f", ruler->GetLength());
			tos << str << "\t";
			str = wxString::Format("%.1f", ruler->GetAngle());
			tos << str << "\t";
			str = "";
			num_points = ruler->GetNumPoint();
			if (num_points > 0)
			{
				p = ruler->GetPoint(0);
				str += wxString::Format("%.2f\t%.2f\t%.2f", p->x(), p->y(), p->z());
			}
			if (num_points > 1)
			{
				p = ruler->GetPoint(num_points - 1);
				str += "\t";
				str += wxString::Format("%.2f\t%.2f\t%.2f", p->x(), p->y(), p->z());
			}
			tos << str << "\t";
			if (ruler->GetTimeDep())
				str = wxString::Format("%d", ruler->GetTime());
			else
				str = "N/A";
			tos << str << "\t";
			tos << ruler->GetInfoValues() << "\n";

			vector<ProfileBin>* profile = ruler->GetProfile();
			if (profile && profile->size())
			{
				tos << ruler->GetInfoProfile() << "\n";
				for (size_t j=0; j<profile->size(); ++j)
				{
					int pixels = (*profile)[j].m_pixels;
					if (pixels <= 0)
						tos << "0.0\t";
					else
						tos << (*profile)[j].m_accum / pixels << "\t";
				}
				tos << "\n";
			}
		}
	}
}

void RulerListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if ( event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
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
	if (item != -1)
	{
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
		GetSubItemRect(item, 1, rect);
		m_color_picker->SetPosition(rect.GetTopLeft());
		m_color_picker->SetSize(rect.GetSize());
		if (m_view)
		{
			vector<Ruler*>* ruler_list = m_view->GetRulerList();
			Ruler* ruler = (*ruler_list)[m_editing_item];
			if (ruler)
			{
				Color color;
				if (ruler->GetUseColor())
				{
					color = ruler->GetColor();
					wxColor c(int(color.r()*255.0), int(color.g()*255.0), int(color.b()*255.0));
					m_color_picker->SetColour(c);
				}
			}
		}
		m_color_picker->Show();
	}
}

void RulerListCtrl::EndEdit(bool update)
{
	if (m_name_text->IsShown())
	{
		m_name_text->Hide();
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

	vector<Ruler*>* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;
	Ruler* ruler = (*ruler_list)[m_editing_item];
	if (!ruler) return;
	ruler->SetName(str);
	SetText(m_editing_item, 0, str);
	m_view->RefreshGL();
}

void RulerListCtrl::OnColorChange(wxColourPickerEvent& event)
{
	if (!m_view)
		return;
	if (m_editing_item == -1)
		return;

	wxColor c = event.GetColour();
	Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	vector<Ruler*>* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;
	Ruler* ruler = (*ruler_list)[m_editing_item];
	if (!ruler) return;
	ruler->SetColor(color);
	wxString str_color;
	str_color = wxString::Format("RGB(%d, %d, %d)",
		int(color.r()*255),
		int(color.g()*255),
		int(color.b()*255));
	SetText(m_editing_item, 1, str_color);
	m_view->RefreshGL();
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(MeasureDlg, wxPanel)
	EVT_MENU(ID_LocatorBtn, MeasureDlg::OnNewLocator)
	EVT_MENU(ID_ProbeBtn, MeasureDlg::OnNewProbe)
	EVT_MENU(ID_ProtractorBtn, MeasureDlg::OnNewProtractor)
	EVT_MENU(ID_RulerBtn, MeasureDlg::OnNewRuler)
	EVT_MENU(ID_RulerMPBtn, MeasureDlg::OnNewRulerMP)
	EVT_MENU(ID_RulerEditBtn, MeasureDlg::OnRulerEdit)
	EVT_MENU(ID_ProfileBtn, MeasureDlg::OnProfile)
	EVT_MENU(ID_DeleteBtn, MeasureDlg::OnDelete)
	EVT_MENU(ID_DeleteAllBtn, MeasureDlg::OnDeleteAll)
	EVT_MENU(ID_ExportBtn, MeasureDlg::OnExport)
	EVT_RADIOBUTTON(ID_ViewPlaneRd, MeasureDlg::OnIntensityMethodCheck)
	EVT_RADIOBUTTON(ID_MaxIntensityRd, MeasureDlg::OnIntensityMethodCheck)
	EVT_RADIOBUTTON(ID_AccIntensityRd, MeasureDlg::OnIntensityMethodCheck)
	EVT_CHECKBOX(ID_UseTransferChk, MeasureDlg::OnUseTransferCheck)
	EVT_CHECKBOX(ID_TransientChk, MeasureDlg::OnTransientCheck)
	END_EVENT_TABLE()

	MeasureDlg::MeasureDlg(wxWindow* frame, wxWindow* parent)
	: wxPanel(parent,wxID_ANY,
	wxPoint(500, 150), wxSize(520, 600),
	0, "MeasureDlg"),
	m_frame(parent),
	m_view(0)
{
	//toolbar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER|wxTB_TEXT);
	m_toolbar->AddCheckTool(ID_LocatorBtn, "Locator",
		wxGetBitmapFromMemory(listicon_locator),
		wxNullBitmap,
		"Add locators to the render view by clicking");
	m_toolbar->AddCheckTool(ID_ProbeBtn, "Probe",
		wxGetBitmapFromMemory(drill),
		wxNullBitmap,
		"Add probes to the render view by clicking once");
	m_toolbar->AddCheckTool(ID_ProtractorBtn, "Protractor",
		wxGetBitmapFromMemory(protractor),
		wxNullBitmap,
		"Add protractors to measure angles by clicking at three points");
	m_toolbar->AddCheckTool(ID_RulerBtn, "2pt Ruler",
		wxGetBitmapFromMemory(listicon_addruler),
		wxNullBitmap,
		"Add rulers to the render view by clicking at two end points");
	m_toolbar->AddCheckTool(ID_RulerMPBtn, "2+pt Ruler",
		wxGetBitmapFromMemory(listicon_addruler),
		wxNullBitmap,
		"Add a polyline ruler to the render view by clicking at its points");
	m_toolbar->AddCheckTool(ID_RulerEditBtn, "Edit",
		wxGetBitmapFromMemory(listicon_ruleredit),
		wxNullBitmap,
		"Select and move ruler points");
	m_toolbar->AddTool(ID_ProfileBtn, "Profile",
		wxGetBitmapFromMemory(prof_curve),
		"Generate intensity profile along curve");
	m_toolbar->AddTool(ID_DeleteBtn, "Delete",
		wxGetBitmapFromMemory(listicon_delete),
		"Delete a selected ruler");
	m_toolbar->AddTool(ID_DeleteAllBtn,"Delete All",
		wxGetBitmapFromMemory(listicon_delall),
		"Delete all rulers");
	m_toolbar->AddTool(ID_ExportBtn, "Export",
		wxGetBitmapFromMemory(listicon_save),
		"Export rulers to a text file");
	m_toolbar->Realize();

	//options
	wxBoxSizer *sizer_1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Settings"), wxVERTICAL);
	wxBoxSizer* sizer_11 = new wxBoxSizer(wxHORIZONTAL);
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
	sizer_11->Add(m_view_plane_rd, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_max_intensity_rd, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_acc_intensity_rd, 0, wxALIGN_CENTER);
	//more options
	wxBoxSizer* sizer_12 = new wxBoxSizer(wxHORIZONTAL);
	m_transient_chk = new wxCheckBox(this, ID_TransientChk, "Transient",
		wxDefaultPosition, wxDefaultSize);
	m_use_transfer_chk = new wxCheckBox(this, ID_UseTransferChk, "Use Volume Properties",
		wxDefaultPosition, wxDefaultSize);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_transient_chk, 0, wxALIGN_CENTER);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_use_transfer_chk, 0, wxALIGN_CENTER);
	//
	sizer_1->Add(sizer_11, 0, wxEXPAND);
	sizer_1->Add(10, 10);
	sizer_1->Add(sizer_12, 0, wxEXPAND);

	//list
	m_rulerlist = new RulerListCtrl(frame, this, wxID_ANY);

	//notes
	wxBoxSizer *sizer_2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "N.B."), wxHORIZONTAL);
	wxStaticText* st = new wxStaticText(this, wxID_ANY,
		"You can press and hold the Shift key to use the brush tool with\n" \
		"any of the measurement tools. The measurement point is placed\n" \
		"at the center of the selected structures automatically.");
	sizer_2->Add(10, 10);
	sizer_2->Add(st, 0, wxEXPAND);

	//sizer
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(m_toolbar, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_rulerlist, 1, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

MeasureDlg::~MeasureDlg()
{
}

void MeasureDlg::GetSettings(VRenderView* vrv)
{
	m_view = vrv;
	UpdateList();
	if (m_view && m_view->m_glview)
	{
		m_toolbar->ToggleTool(ID_LocatorBtn, false);
		m_toolbar->ToggleTool(ID_ProbeBtn, false);
		m_toolbar->ToggleTool(ID_ProtractorBtn, false);
		m_toolbar->ToggleTool(ID_RulerBtn, false);
		m_toolbar->ToggleTool(ID_RulerMPBtn, false);
		m_toolbar->ToggleTool(ID_RulerEditBtn, false);

		int int_mode = m_view->m_glview->GetIntMode();
		if (int_mode == 5 || int_mode == 7)
		{
			int ruler_type = m_view->GetRulerType();
			if (ruler_type == 0)
				m_toolbar->ToggleTool(ID_RulerBtn, true);
			else if (ruler_type == 1)
				m_toolbar->ToggleTool(ID_RulerMPBtn, true);
			else if (ruler_type == 2)
				m_toolbar->ToggleTool(ID_LocatorBtn, true);
			else if (ruler_type == 3)
				m_toolbar->ToggleTool(ID_ProbeBtn, true);
			else if (ruler_type == 4)
				m_toolbar->ToggleTool(ID_ProtractorBtn, true);
		}
		else if (int_mode == 6)
			m_toolbar->ToggleTool(ID_RulerEditBtn, true);

		switch (m_view->m_glview->m_point_volume_mode)
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

		m_use_transfer_chk->SetValue(m_view->m_glview->m_ruler_use_transf);
		m_transient_chk->SetValue(m_view->m_glview->m_ruler_time_dep);
	}
}

VRenderView* MeasureDlg::GetView()
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

	if (m_toolbar->GetToolState(ID_RulerMPBtn))
		m_view->FinishRuler();

	m_toolbar->ToggleTool(ID_ProbeBtn, false);
	m_toolbar->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar->ToggleTool(ID_RulerBtn, false);
	m_toolbar->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar->ToggleTool(ID_RulerEditBtn, false);

	if (m_toolbar->GetToolState(ID_LocatorBtn))
	{
		m_view->SetIntMode(5);
		m_view->SetRulerType(2);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewProbe(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar->GetToolState(ID_RulerMPBtn))
		m_view->FinishRuler();

	m_toolbar->ToggleTool(ID_LocatorBtn, false);
	m_toolbar->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar->ToggleTool(ID_RulerBtn, false);
	m_toolbar->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar->ToggleTool(ID_RulerEditBtn, false);

	if (m_toolbar->GetToolState(ID_ProbeBtn))
	{
		m_view->SetIntMode(5);
		m_view->SetRulerType(3);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewProtractor(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar->GetToolState(ID_RulerMPBtn))
		m_view->FinishRuler();

	m_toolbar->ToggleTool(ID_LocatorBtn, false);
	m_toolbar->ToggleTool(ID_ProbeBtn, false);
	m_toolbar->ToggleTool(ID_RulerBtn, false);
	m_toolbar->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar->ToggleTool(ID_RulerEditBtn, false);

	if (m_toolbar->GetToolState(ID_ProtractorBtn))
	{
		m_view->SetIntMode(5);
		m_view->SetRulerType(4);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewRuler(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar->GetToolState(ID_RulerMPBtn))
		m_view->FinishRuler();

	m_toolbar->ToggleTool(ID_LocatorBtn, false);
	m_toolbar->ToggleTool(ID_ProbeBtn, false);
	m_toolbar->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar->ToggleTool(ID_RulerMPBtn, false);
	m_toolbar->ToggleTool(ID_RulerEditBtn, false);

	if (m_toolbar->GetToolState(ID_RulerBtn))
	{
		m_view->SetIntMode(5);
		m_view->SetRulerType(0);
	}
	else
	{
		m_view->SetIntMode(1);
	}
}

void MeasureDlg::OnNewRulerMP(wxCommandEvent& event)
{
	if (!m_view) return;

	m_toolbar->ToggleTool(ID_LocatorBtn, false);
	m_toolbar->ToggleTool(ID_ProbeBtn, false);
	m_toolbar->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar->ToggleTool(ID_RulerBtn, false);
	m_toolbar->ToggleTool(ID_RulerEditBtn, false);

	if (m_toolbar->GetToolState(ID_RulerMPBtn))
	{
		m_view->SetIntMode(5);
		m_view->SetRulerType(1);
	}
	else
	{
		m_view->SetIntMode(1);
		m_view->FinishRuler();
	}
}

void MeasureDlg::OnRulerEdit(wxCommandEvent& event)
{
	if (!m_view) return;

	if (m_toolbar->GetToolState(ID_RulerMPBtn))
		m_view->FinishRuler();

	m_toolbar->ToggleTool(ID_LocatorBtn, false);
	m_toolbar->ToggleTool(ID_ProbeBtn, false);
	m_toolbar->ToggleTool(ID_ProtractorBtn, false);
	m_toolbar->ToggleTool(ID_RulerBtn, false);
	m_toolbar->ToggleTool(ID_RulerMPBtn, false);

	if (m_toolbar->GetToolState(ID_RulerEditBtn))
		m_view->SetIntMode(6);
	else
		m_view->SetIntMode(1);
}

void MeasureDlg::OnProfile(wxCommandEvent& event)
{
	if (m_view)
	{
		m_view->RulerProfile(m_rulerlist->GetCurrSelection());
	}
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
	if (!m_view || !m_view->m_glview)
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
	m_view->SetPointVolumeMode(mode);
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetSettingDlg())
		frame->GetSettingDlg()->SetPointVolumeMode(mode);
}

void MeasureDlg::OnUseTransferCheck(wxCommandEvent& event)
{
	if (!m_view || !m_view->m_glview)
		return;

	bool use_transfer = m_use_transfer_chk->GetValue();
	m_view->SetRulerUseTransf(use_transfer);
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetSettingDlg())
		frame->GetSettingDlg()->SetRulerUseTransf(use_transfer);
}

void MeasureDlg::OnTransientCheck(wxCommandEvent& event)
{
	if (!m_view || !m_view->m_glview)
		return;

	bool val = m_transient_chk->GetValue();
	m_view->SetRulerTimeDep(val);
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetSettingDlg())
		frame->GetSettingDlg()->SetRulerTimeDep(val);
}
