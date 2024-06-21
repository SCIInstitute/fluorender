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
#include <MeasureDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
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
#include <png_resource.h>
#include <ruler.xpm>
#include <icons.h>

RulerListCtrl::RulerListCtrl(
	MeasureDlg* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style)
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
	m_name_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_name_text->Bind(wxEVT_LEFT_DCLICK, &RulerListCtrl::OnTextFocus, this);
	m_name_text->Bind(wxEVT_TEXT, &RulerListCtrl::OnNameText, this);
	m_name_text->Hide();
	m_center_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_center_text->Bind(wxEVT_LEFT_DCLICK, &RulerListCtrl::OnTextFocus, this);
	m_center_text->Bind(wxEVT_TEXT, &RulerListCtrl::OnCenterText, this);
	m_center_text->Hide();
	m_color_picker = new wxColourPickerCtrl(this, wxID_ANY);
	m_color_picker->Bind(wxEVT_COLOURPICKER_CHANGED, &RulerListCtrl::OnColorChange, this);
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

wxString RulerListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void RulerListCtrl::SetText(long item, int col, wxString& str)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	info.SetText(str);
	SetItem(info);
}

bool RulerListCtrl::GetCurrSelection(std::set<int> &sel)
{
	long item = -1;
	for (;;)
	{
		item = GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		sel.insert(int(item));
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

void RulerListCtrl::StartEdit(int type, bool use_color, const fluo::Color& color)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_editing_item = item;

	wxRect rect;
	wxString str;
	//add frame text
	GetSubItemRect(item, 0, rect);
	str = GetText(item, 0);
	m_name_text->SetPosition(rect.GetTopLeft());
	m_name_text->SetSize(rect.GetSize());
	m_name_text->ChangeValue(str);
	m_name_text->Show();
	//add color picker
	GetSubItemRect(item, ColorCol, rect);
	m_color_picker->SetPosition(rect.GetTopLeft());
	m_color_picker->SetSize(rect.GetSize());
	if (type == 2)
	{
		//locator
		GetSubItemRect(item, CenterCol, rect);
		str = GetText(item, CenterCol);
		m_center_text->SetPosition(rect.GetTopLeft());
		m_center_text->SetSize(rect.GetSize());
		m_center_text->ChangeValue(str);
		m_center_text->Show();
	}
	if (use_color)
	{
		wxColor c(int(std::round(color.r() * 255.0)),
			int(std::round(color.g() * 255.0)),
			int(std::round(color.b() * 255.0)));
		m_color_picker->SetColour(c);
	}
	m_color_picker->Show();
}

void RulerListCtrl::EndEdit()
{
	if (m_name_text->IsShown())
	{
		m_name_text->Hide();
		m_center_text->Hide();
		m_color_picker->Hide();
		m_editing_item = -1;
	}
}

void RulerListCtrl::OnTextFocus(wxCommandEvent& event)
{
	wxTextCtrl* object = (wxTextCtrl*)event.GetEventObject();
	object->SetSelection(0, -1);
}

void RulerListCtrl::OnNameText(wxCommandEvent& event)
{
	m_name = m_name_text->GetValue();
}

void RulerListCtrl::OnCenterText(wxCommandEvent& event)
{
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
	m_center = fluo::Point(x, y, z);
}

void RulerListCtrl::OnColorChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	m_color = fluo::Color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//BEGIN_EVENT_TABLE(MeasureDlg, wxPanel)
//	EVT_MENU(ID_LocatorBtn, MeasureDlg::OnNewLocator)
//	EVT_MENU(ID_ProbeBtn, MeasureDlg::OnNewProbe)
//	EVT_MENU(ID_ProtractorBtn, MeasureDlg::OnNewProtractor)
//	EVT_MENU(ID_RulerBtn, MeasureDlg::OnNewRuler)
//	EVT_MENU(ID_RulerMPBtn, MeasureDlg::OnNewRulerMP)
//	EVT_MENU(ID_EllipseBtn, MeasureDlg::OnEllipse)
//	EVT_MENU(ID_GrowBtn, MeasureDlg::OnGrow)
//	EVT_MENU(ID_PencilBtn, MeasureDlg::OnPencil)
//	EVT_MENU(ID_RulerMoveBtn, MeasureDlg::OnRulerMove)
//	EVT_MENU(ID_RulerMovePointBtn, MeasureDlg::OnRulerMovePoint)
//	EVT_MENU(ID_MagnetBtn, MeasureDlg::OnMagnet)
//	EVT_MENU(ID_RulerMovePencilBtn, MeasureDlg::OnRulerMovePencil)
//	EVT_MENU(ID_RulerDelBtn, MeasureDlg::OnRulerDel)
//	EVT_MENU(ID_RulerFlipBtn, MeasureDlg::OnRulerFlip)
//	EVT_MENU(ID_RulerAvgBtn, MeasureDlg::OnRulerAvg)
//	EVT_MENU(ID_LockBtn, MeasureDlg::OnLock)
//	EVT_MENU(ID_PruneBtn, MeasureDlg::OnPrune)
//	EVT_MENU(ID_RelaxBtn, MeasureDlg::OnRelax)
//	EVT_MENU(ID_DeleteBtn, MeasureDlg::OnDelete)
//	EVT_MENU(ID_DeleteAllBtn, MeasureDlg::OnDeleteAll)
//	EVT_MENU(ID_ProfileBtn, MeasureDlg::OnProfile)
//	EVT_MENU(ID_DistanceBtn, MeasureDlg::OnDistance)
//	EVT_MENU(ID_ProjectBtn, MeasureDlg::OnProject)
//	EVT_MENU(ID_ExportBtn, MeasureDlg::OnExport)
//	EVT_RADIOBUTTON(ID_ViewPlaneRd, MeasureDlg::OnIntensityMethodCheck)
//	EVT_RADIOBUTTON(ID_MaxIntensityRd, MeasureDlg::OnIntensityMethodCheck)
//	EVT_RADIOBUTTON(ID_AccIntensityRd, MeasureDlg::OnIntensityMethodCheck)
//	EVT_CHECKBOX(ID_UseTransferChk, MeasureDlg::OnUseTransferCheck)
//	EVT_CHECKBOX(ID_TransientChk, MeasureDlg::OnTransientCheck)
//	EVT_CHECKBOX(ID_DF_FChk, MeasureDlg::OnDF_FCheck)
//	EVT_TOGGLEBUTTON(ID_AutoRelaxBtn, MeasureDlg::OnAutoRelax)
//	EVT_SPINCTRLDOUBLE(ID_RelaxValueSpin, MeasureDlg::OnRelaxValueSpin)
//	EVT_TEXT(ID_RelaxValueSpin, MeasureDlg::OnRelaxValueText)
//	EVT_COMBOBOX(ID_RelaxDataCmb, MeasureDlg::OnRelaxData)
//	//ruler list
//	EVT_BUTTON(ID_NewGroup, MeasureDlg::OnNewGroup)
//	EVT_BUTTON(ID_ChgGroup, MeasureDlg::OnChgGroup)
//	EVT_BUTTON(ID_SelGroup, MeasureDlg::OnSelGroup)
//	EVT_BUTTON(ID_DispTglGroup, MeasureDlg::OnDispTglGroup)
//	//interpolate/key
//	EVT_COMBOBOX(ID_InterpCmb, MeasureDlg::OnInterpCmb)
//	EVT_BUTTON(ID_DeleteKeyBtn, MeasureDlg::OnDeleteKeyBtn)
//	EVT_BUTTON(ID_DeleteAllKeyBtn, MeasureDlg::OnDeleteAllKeyBtn)
//	//align
//	EVT_BUTTON(ID_AlignX, MeasureDlg::OnAlignRuler)
//	EVT_BUTTON(ID_AlignY, MeasureDlg::OnAlignRuler)
//	EVT_BUTTON(ID_AlignZ, MeasureDlg::OnAlignRuler)
//	EVT_BUTTON(ID_AlignNX, MeasureDlg::OnAlignRuler)
//	EVT_BUTTON(ID_AlignNY, MeasureDlg::OnAlignRuler)
//	EVT_BUTTON(ID_AlignNZ, MeasureDlg::OnAlignRuler)
//	EVT_BUTTON(ID_AlignXYZ, MeasureDlg::OnAlignPca)
//	EVT_BUTTON(ID_AlignYXZ, MeasureDlg::OnAlignPca)
//	EVT_BUTTON(ID_AlignZXY, MeasureDlg::OnAlignPca)
//	EVT_BUTTON(ID_AlignXZY, MeasureDlg::OnAlignPca)
//	EVT_BUTTON(ID_AlignYZX, MeasureDlg::OnAlignPca)
//	EVT_BUTTON(ID_AlignZYX, MeasureDlg::OnAlignPca)
//END_EVENT_TABLE()

MeasureDlg::MeasureDlg(MainFrame* frame)
	: PropPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 600)),
	0, "MeasureDlg"),
	m_edited(false)
{
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
	m_toolbar1->Bind(wxEVT_TOOL, &MeasureDlg::OnToolbar1, this);
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
	m_toolbar2->AddCheckTool(ID_RulerMovePointBtn, "Edit",
		bitmap, wxNullBitmap,
		"Select and move a ruler point");
	bitmap = wxGetBitmapFromMemory(magnet);
	m_toolbar2->AddCheckTool(ID_MagnetBtn, "Magnet",
		bitmap, wxNullBitmap,
		"Move ruler points by magnet");
	bitmap = wxGetBitmapFromMemory(pencil);
	m_toolbar2->AddCheckTool(ID_RulerMovePencilBtn, "Redraw",
		bitmap, wxNullBitmap,
		"Move ruler points by redraw");
	bitmap = wxGetBitmapFromMemory(flip_ruler);
	m_toolbar2->AddTool(ID_RulerFlipBtn, "Flip", bitmap,
		"Reverse the order of ruler points");
	bitmap = wxGetBitmapFromMemory(average);
	m_toolbar2->AddTool(ID_RulerAvgBtn, "Average", bitmap,
		"Compute a center for selected rulers");
	bitmap = wxGetBitmapFromMemory(lock);
	m_toolbar2->AddCheckTool(ID_LockBtn, "Lock",
		bitmap, wxNullBitmap,
		"Click to lock/unlock a ruler point for relaxing");
	bitmap = wxGetBitmapFromMemory(relax);
	m_toolbar2->AddTool(ID_RelaxBtn, "Relax", bitmap,
		"Relax ruler by components");
	m_toolbar2->Bind(wxEVT_TOOL, &MeasureDlg::OnToolbar2, this);
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
	bitmap = wxGetBitmapFromMemory(ruler_del);
	m_toolbar3->AddCheckTool(ID_RulerDelBtn, "Delete",
		bitmap, wxNullBitmap,
		"Select and delete a ruler point");
	bitmap = wxGetBitmapFromMemory(prune);
	m_toolbar3->AddTool(ID_PruneBtn, "Prune", bitmap,
		"Remove very short branches from ruler");
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
	m_toolbar3->Bind(wxEVT_TOOL, &MeasureDlg::OnToolbar3, this);
	m_toolbar3->Realize();

	//options
	wxBoxSizer *sizer_1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Settings"), wxVERTICAL);
	wxBoxSizer* sizer_11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Z-Depth Comp.:",
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
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
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
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
	m_df_f_chk->Hide();
	//relax settings
	wxBoxSizer* sizer_13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Relax:",
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
	sizer_13->Add(10, 10);
	sizer_13->Add(st, 0, wxALIGN_CENTER);
	m_auto_relax_btn = new wxToggleButton(this, ID_AutoRelaxBtn,
		"Auto Relax", wxDefaultPosition, FromDIP(wxSize(75, -1)));
	sizer_13->Add(10, 10);
	sizer_13->Add(m_auto_relax_btn, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Constraint ");
	sizer_13->Add(10, 10);
	sizer_13->Add(st, 0, wxALIGN_CENTER);
	m_relax_data_cmb = new wxComboBox(this, ID_RelaxDataCmb, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
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
		wxDefaultPosition, FromDIP(wxSize(50, 23)),
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
	//group
	wxBoxSizer* sizer21 = new wxBoxSizer(wxHORIZONTAL);
	m_new_group = new wxButton(this, ID_NewGroup, "New Group",
		wxDefaultPosition, wxDefaultSize);
	st = new wxStaticText(this, 0, "Group ID:",
		wxDefaultPosition, FromDIP(wxSize(65, -1)));
	m_group_text = new wxTextCtrl(this, ID_GroupText, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 22)), wxTE_RIGHT, vald_int);
	m_chg_group = new wxButton(this, ID_ChgGroup, "Change",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_sel_group = new wxButton(this, ID_SelGroup, "Select",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_disptgl_group = new wxButton(this, ID_DispTglGroup, "Display",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	sizer21->Add(m_new_group, 0, wxALIGN_CENTER);
	sizer21->AddStretchSpacer();
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(m_group_text, 0, wxALIGN_CENTER);
	sizer21->Add(m_chg_group, 0, wxALIGN_CENTER);
	sizer21->Add(m_sel_group, 0, wxALIGN_CENTER);
	sizer21->Add(m_disptgl_group, 0, wxALIGN_CENTER);
	//interpolate/key
	wxBoxSizer* sizer22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Time Interpolation: ",
		wxDefaultPosition, wxDefaultSize);
	m_interp_cmb = new wxComboBox(this, ID_InterpCmb, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_interp_cmb->Append("Step");
	m_interp_cmb->Append("Linear");
	m_interp_cmb->Append("Smooth");
	m_interp_cmb->Select(1);
	m_delete_key_btn = new wxButton(this, ID_DeleteKeyBtn, "Del. Key",
		wxDefaultPosition, wxDefaultSize);
	m_delete_all_key_btn = new wxButton(this, ID_DeleteAllKeyBtn, "Del. All Keys",
		wxDefaultPosition, wxDefaultSize);
	sizer22->AddStretchSpacer();
	sizer22->Add(st, 0, wxALIGN_CENTER);
	sizer22->Add(m_interp_cmb, 0, wxALIGN_CENTER);
	sizer22->Add(m_delete_key_btn, 0, wxALIGN_CENTER);
	sizer22->Add(m_delete_all_key_btn, 0, wxALIGN_CENTER);
	//list
	m_ruler_list = new RulerListCtrl(this,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	m_ruler_list->Bind(wxEVT_KEY_DOWN, &MeasureDlg::OnKeyDown, this);
	m_ruler_list->Bind(wxEVT_CONTEXT_MENU, &MeasureDlg::OnContextMenu, this);
	m_ruler_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MeasureDlg::OnSelection, this);
	m_ruler_list->Bind(wxEVT_LIST_ITEM_DESELECTED, &MeasureDlg::OnEndSelection, this);
	sizer_2->Add(sizer21, 0, wxEXPAND);
	sizer_2->Add(5, 5);
	sizer_2->Add(sizer22, 0, wxEXPAND);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_ruler_list, 1, wxEXPAND);

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
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_x = new wxButton(this, ID_AlignX, "X",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_y = new wxButton(this, ID_AlignY, "Y",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_z = new wxButton(this, ID_AlignZ, "Z",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_nx = new wxButton(this, ID_AlignNX, "-X",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_ny = new wxButton(this, ID_AlignNY, "-Y",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_nz = new wxButton(this, ID_AlignNZ, "-Z",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
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
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xyz = new wxButton(this, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yxz = new wxButton(this, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zxy = new wxButton(this, ID_AlignZXY, "ZXY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xzy = new wxButton(this, ID_AlignXZY, "XZY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yzx = new wxButton(this, ID_AlignYZX, "YZX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zyx = new wxButton(this, ID_AlignZYX, "ZYX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
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

	Bind(wxEVT_MENU, &MeasureDlg::OnMenuItem, this);
	Bind(wxEVT_SCROLLWIN_TOP, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_BOTTOM, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_LINEUP, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_LINEDOWN, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_PAGEUP, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_PAGEDOWN, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_MOUSEWHEEL, &MeasureDlg::OnScrollMouse, this);

	SetSizer(sizerV);
	Layout();
}

MeasureDlg::~MeasureDlg()
{
}

void MeasureDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();

	bool bval;
	int ival;

	if (update_all || FOUND_VALUE(gstRulerTools))
	{
		RenderCanvas* canvas = glbin_current.canvas;
		bval = canvas && canvas->GetIntMode() == 5;
		ival = glbin_ruler_handler.GetType();
		int mode = canvas ? canvas->GetIntMode() : 0;
		//toolbar1
		m_toolbar1->ToggleTool(ID_LocatorBtn, bval && ival == 2);
		m_toolbar1->ToggleTool(ID_ProbeBtn, bval && ival == 3);
		m_toolbar1->ToggleTool(ID_RulerBtn, bval && ival == 0);
		m_toolbar1->ToggleTool(ID_ProtractorBtn, bval && ival == 4);
		m_toolbar1->ToggleTool(ID_EllipseBtn, bval && ival == 5);
		m_toolbar1->ToggleTool(ID_RulerMPBtn, bval && ival == 1);
		m_toolbar1->ToggleTool(ID_PencilBtn,mode == 13);
		m_toolbar1->ToggleTool(ID_GrowBtn, mode == 12);
		//toolbar2
		m_toolbar2->ToggleTool(ID_RulerMoveBtn, mode == 9);
		m_toolbar2->ToggleTool(ID_RulerMovePointBtn, mode == 6);
		bool bval2 = glbin_ruler_handler.GetRedistLength();
		m_toolbar2->ToggleTool(ID_MagnetBtn, mode == 15 && !bval2);
		m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, mode == 15 && bval2);
		m_toolbar2->ToggleTool(ID_LockBtn, mode == 11);
		//toolbar3
		m_toolbar3->ToggleTool(ID_RulerDelBtn, mode == 14);
	}

	if (update_all || FOUND_VALUE(gstRulerList))
	{
		UpdateRulerList();
	}

	if (FOUND_VALUE(gstRulerListCur))
	{
		UpdateRulerListCur();
	}

	if (update_all || FOUND_VALUE(gstRulerListDisp))
	{
		wxColour c;
		for (int i = 0; i < m_ruler_list->GetItemCount(); ++i)
		{
			flrd::Ruler* ruler = glbin_ruler_handler.GetRuler(i);
			if (ruler)
				continue;
			bval = ruler->GetDisp();
			c = bval ? wxColour(255, 255, 255) : wxColour(200, 200, 200);
			m_ruler_list->SetItemBackgroundColour(i, c);
		}
	}

	if (update_all || FOUND_VALUE(gstRulerListSel))
	{
		ival = glbin_ruler_handler.GetRulerIndex();
		m_ruler_list->SetItemState(ival, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	if (update_all || FOUND_VALUE(gstRulerTransient))
	{
		flrd::Ruler* ruler = glbin_current.GetRuler();
		bval = ruler->GetTransient();
		m_transient_chk->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstRulerInterpolation))
	{
		flrd::Ruler* ruler = glbin_current.GetRuler();
		ival = ruler->GetInterp();
		m_interp_cmb->Select(ival);
	}
}

void MeasureDlg::UpdateRulerList()
{
	m_ruler_list->m_name_text->Hide();
	m_ruler_list->m_center_text->Hide();
	m_ruler_list->m_color_picker->Hide();

	RenderCanvas* canvas = glbin_current.canvas;
	flrd::RulerList* ruler_list = glbin_current.GetRulerList();
	if (!ruler_list)
		return;

	std::vector<unsigned int> groups;
	int group_num = ruler_list->GetGroupNum(groups);
	std::vector<int> group_count(group_num, 0);

	m_ruler_list->DeleteAllItems();

	wxString points;
	fluo::Point p;
	int num_points;
	size_t t;
	if (canvas->m_frame_num_type == 1)
		t = canvas->m_param_cur_num;
	else
		t = canvas->m_tseq_cur_num;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		ruler->SetWorkTime(t);
		if (ruler->GetTransient() &&
			ruler->GetTransTime() != t)
			continue;

		wxString unit;
		switch (canvas->m_sb_unit)
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
		if (glbin_ruler_handler.GetBackground())
		{
			double bg_int = glbin_ruler_handler.GetVolumeBgInt();
			dval = (dval - bg_int) / bg_int;
			intensity = wxString::Format("%.4f", dval);
		}
		else
			intensity = wxString::Format("%.0f", dval);
		wxString color;
		if (ruler->GetUseColor())
			color = wxString::Format("RGB(%d, %d, %d)",
				int(std::round(ruler->GetColor().r() * 255)),
				int(std::round(ruler->GetColor().g() * 255)),
				int(std::round(ruler->GetColor().b() * 255)));
		else
			color = "N/A";
		wxString center;
		fluo::Point cp = ruler->GetCenter();
		center = wxString::Format("(%.2f, %.2f, %.2f)",
			cp.x(), cp.y(), cp.z());
		wxString str = ruler->GetDelInfoValues(", ");
		m_ruler_list->Append(ruler->GetDisp(), ruler->Id(),
			ruler->GetName(), group, count,
			intensity, color,
			ruler->GetNumBranch(), ruler->GetLength(), unit,
			ruler->GetAngle(), center, ruler->GetTransient(),
			ruler->GetTransTime(), str, points);
	}

	m_ruler_list->AdjustSize();

	glbin_vertex_array_manager.set_dirty(flvr::VA_Rulers);
}

void MeasureDlg::UpdateRulerListCur()
{
	flrd::Ruler* ruler = glbin_ruler_handler.GetRuler();
	int item = glbin_ruler_handler.GetRulerIndex();
	if (!ruler)
		return;

	wxString str = ruler->GetName();
	fluo::Point p = ruler->GetCenter();
	fluo::Color c = ruler->GetColor();

	m_ruler_list->SetText(item, 0, str);
	str = wxString::Format("(%.2f, %.2f, %.2f)",
		p.x(), p.y(), p.z());
	m_ruler_list->SetText(item, CenterCol, str);
	m_ruler_list->SetText(item, PointCol, str);
	str = wxString::Format("RGB(%d, %d, %d)",
		int(std::round(c.r() * 255)),
		int(std::round(c.g() * 255)),
		int(std::round(c.b() * 255)));
	m_ruler_list->SetText(item, ColorCol, str);
}

void MeasureDlg::SelectGroup(unsigned int group)
{
	m_ruler_list->ClearSelection();
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;

	flrd::RulerList* ruler_list = canvas->GetRulerList();
	if (!ruler_list) return;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == group)
			m_ruler_list->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

void MeasureDlg::ToggleDisplay()
{
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	glbin_ruler_handler.ToggleDisplay(sel);
	FluoRefresh(2, { gstRulerListDisp }, { m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::SetCurrentRuler()
{
	flrd::Ruler* ruler = glbin_ruler_handler.GetRuler();
	RenderCanvas* canvas = glbin_current.canvas;
	if (!ruler || !canvas)
		return;
	ruler->SetName(m_ruler_list->m_name);
	if (ruler->GetRulerType() == 2)
	{
		ruler->SetWorkTime(canvas->m_tseq_cur_num);
		ruler->SetPoint(0, m_ruler_list->m_center);
	}
	ruler->SetColor(m_ruler_list->m_color);
	FluoRefresh(2, { gstRulerListCur },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::Locator()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	bool bval = canvas->GetIntMode() == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (bval && ival == 2)
	{
		canvas->SetIntMode(1);
	}
	else
	{
		canvas->SetIntMode(5);
		glbin_ruler_handler.SetType(2);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Probe()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	bool bval = canvas->GetIntMode() == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (bval && ival == 3)
	{
		canvas->SetIntMode(1);
	}
	else
	{
		canvas->SetIntMode(5);
		glbin_ruler_handler.SetType(3);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Ruler()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	bool bval = canvas->GetIntMode() == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (bval && ival == 0)
	{
		canvas->SetIntMode(1);
	}
	else
	{
		canvas->SetIntMode(5);
		glbin_ruler_handler.SetType(0);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Protractor()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	bool bval = canvas->GetIntMode() == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (bval && ival == 4)
	{
		canvas->SetIntMode(1);
	}
	else
	{
		canvas->SetIntMode(5);
		glbin_ruler_handler.SetType(4);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Ellipse()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	bool bval = canvas->GetIntMode() == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (bval && ival == 5)
	{
		canvas->SetIntMode(1);
	}
	else
	{
		canvas->SetIntMode(5);
		glbin_ruler_handler.SetType(5);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::RulerMP()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	bool bval = canvas->GetIntMode() == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
	{
		canvas->SetIntMode(1);
		glbin_ruler_handler.FinishRuler();
	}
	else
	{
		canvas->SetIntMode(5);
		glbin_ruler_handler.SetType(1);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Pencil()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int ival = canvas->GetIntMode();

	glbin_ruler_handler.FinishRuler();

	if (ival == 13)
	{
		canvas->SetIntMode(1);
		//if (m_view->m_canvas->GetRulerRenderer())
		//	m_view->m_canvas->GetRulerRenderer()->SetDrawText(true);
	}
	else
	{
		canvas->SetIntMode(13);
		glbin_ruler_handler.SetType(1);
		//if (m_view->m_canvas->GetRulerRenderer())
		//	m_view->m_canvas->GetRulerRenderer()->SetDrawText(false);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Grow()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int ival = canvas->GetIntMode();

	glbin_ruler_handler.FinishRuler();

	if (ival == 12)
	{
		canvas->SetIntMode(1);
		glbin_ruler_renderer.SetDrawText(true);
	}
	else
	{
		canvas->SetIntMode(12);
		glbin_ruler_handler.SetType(1);
		glbin_ruler_renderer.SetDrawText(false);
		//reset label volume
		if (canvas->m_cur_vol)
		{
			canvas->m_cur_vol->
				GetVR()->clear_tex_mask();
			canvas->m_cur_vol->
				GetVR()->clear_tex_label();
			canvas->m_cur_vol->
				AddEmptyMask(0, true);
			canvas->m_cur_vol->
				AddEmptyLabel(0, true);
		}
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::RulerMove()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int mode = canvas->GetIntMode();
	bool bval = mode == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (mode == 9)
		canvas->SetIntMode(1);
	else
		canvas->SetIntMode(9);

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::RulerMovePoint()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int mode = canvas->GetIntMode();
	bool bval = mode == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (mode == 6)
		canvas->SetIntMode(1);
	else
		canvas->SetIntMode(6);

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Magnet()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int mode = canvas->GetIntMode();
	bool bval = mode == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	bool bval2 = glbin_ruler_handler.GetRedistLength();
	if (mode == 15 && !bval2)
		canvas->SetIntMode(1);
	else
	{
		canvas->SetIntMode(15);
		glbin_ruler_handler.SetRedistLength(false);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::RulerMovePencil()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int mode = canvas->GetIntMode();
	bool bval = mode == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	bool bval2 = glbin_ruler_handler.GetRedistLength();
	if (mode == 15 && bval2)
		canvas->SetIntMode(1);
	else
	{
		canvas->SetIntMode(15);
		glbin_ruler_handler.SetRedistLength(false);
	}

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::RulerFlip()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Flip(sel);
	m_edited = true;

	FluoRefresh(2, { gstRulerList },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::RulerAvg()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.AddAverage(sel);

	FluoRefresh(2, { gstRulerList },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::Lock()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int mode = canvas->GetIntMode();
	bool bval = mode == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (mode == 11)
		canvas->SetIntMode(1);
	else
		canvas->SetIntMode(11);

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Relax()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Relax(sel);

	FluoRefresh(2, { gstRulerList },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::DeleteSelection()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.DeleteSelection(sel);
	FluoRefresh(2, { gstRulerList, gstRulerListSel },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::DeleteAll()
{
	glbin_ruler_handler.DeleteAll(false);
	FluoRefresh(2, { gstRulerList, gstRulerListSel },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::DeletePoint()
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;
	int mode = canvas->GetIntMode();
	bool bval = mode == 5;
	int ival = glbin_ruler_handler.GetType();

	if (bval && ival == 1)
		glbin_ruler_handler.FinishRuler();

	if (mode == 14)
		canvas->SetIntMode(1);
	else
		canvas->SetIntMode(14);

	FluoUpdate({ gstRulerTools });
}

void MeasureDlg::Prune()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Prune(sel);

	FluoRefresh(2, { gstRulerList },
		{ m_frame->GetRenderCanvas(glbin_current.canvas) });
}

void MeasureDlg::Profile()
{
	if (m_view)
	{
		glbin_ruler_handler.SetVolumeData(m_view->m_cur_vol);
		std::set<int> sel;
		if (m_ruler_list->GetCurrSelection(sel))
		{
			//export selected
			for (size_t i = 0; i < sel.size(); ++i)
			{
				glbin_ruler_handler.Profile(sel[i]);
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
					glbin_ruler_handler.Profile(i);
					SetProfile(i);
				}
			}
		}
	}
}

void MeasureDlg::Distance()
{
	if (!m_view || !m_frame)
		return;

	glbin_ruler_handler.SetCompAnalyzer(&glbin_comp_analyzer);

	std::string filename;
	wxFileDialog* fopendlg = new wxFileDialog(
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

	std::set<int> sel;
	std::string fi;
	if (m_ruler_list->GetCurrSelection(sel))
	{
		//export selected
		for (size_t i = 0; i < sel.size(); ++i)
		{
			fi = filename + std::to_string(i) + ".txt";
			glbin_ruler_handler.Distance(sel[i], fi);
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
				glbin_ruler_handler.Distance(i, fi);
			}
		}
	}

	if (fopendlg)
		delete fopendlg;
}

void MeasureDlg::Project()
{
	if (m_view)
	{
		std::set<int> sel;
		if (m_ruler_list->GetCurrSelection(sel))
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
	flrd::CelpList* list = 0;
	list = glbin_comp_analyzer.GetCelpList();
	if (list->empty())
		return;

	glbin_dist_calculator.SetCelpList(list);
	glbin_dist_calculator.SetRuler(ruler);
	glbin_dist_calculator.Project();

	std::vector<flrd::Celp> comps;
	for (auto it = list->begin();
		it != list->end(); ++it)
		comps.push_back(it->second);
	std::sort(comps.begin(), comps.end(),
		[](const flrd::Celp& a, const flrd::Celp& b) -> bool
		{
			fluo::Point pa = a->GetProjp();
			fluo::Point pb = b->GetProjp();
			if (pa.z() != pb.z()) return pa.z() < pb.z();
			else return pa.x() < pb.x();
		});

	//export
	wxFileDialog* fopendlg = new wxFileDialog(
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

void MeasureDlg::Export()
{
	wxFileDialog* fopendlg = new wxFileDialog(
		m_frame, "Export rulers", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();

	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		glbin_ruler_handler.Export(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

//void MeasureDlg::GetSettings(RenderCanvas* vrv)
//{
//	m_view = vrv;
//	if (!m_view)
//		return;
//
//	UpdateList();
//	m_toolbar1->ToggleTool(ID_LocatorBtn, false);
//	m_toolbar1->ToggleTool(ID_ProbeBtn, false);
//	m_toolbar1->ToggleTool(ID_ProtractorBtn, false);
//	m_toolbar1->ToggleTool(ID_RulerBtn, false);
//	m_toolbar1->ToggleTool(ID_RulerMPBtn, false);
//	m_toolbar1->ToggleTool(ID_EllipseBtn, false);
//	m_toolbar1->ToggleTool(ID_GrowBtn, false);
//	m_toolbar1->ToggleTool(ID_PencilBtn, false);
//	m_toolbar3->ToggleTool(ID_RulerDelBtn, false);
//	m_toolbar2->ToggleTool(ID_RulerMoveBtn, false);
//	m_toolbar2->ToggleTool(ID_RulerMovePointBtn, false);
//	m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, false);
//	m_toolbar2->ToggleTool(ID_MagnetBtn, false);
//	m_toolbar2->ToggleTool(ID_LockBtn, false);
//
//	int int_mode = m_view->GetIntMode();
//	if (int_mode == 5 || int_mode == 7)
//	{
//		int ruler_type = glbin_ruler_handler.GetType();
//		if (ruler_type == 0)
//			m_toolbar1->ToggleTool(ID_RulerBtn, true);
//		else if (ruler_type == 1)
//			m_toolbar1->ToggleTool(ID_RulerMPBtn, true);
//		else if (ruler_type == 2)
//			m_toolbar1->ToggleTool(ID_LocatorBtn, true);
//		else if (ruler_type == 3)
//			m_toolbar1->ToggleTool(ID_ProbeBtn, true);
//		else if (ruler_type == 4)
//			m_toolbar1->ToggleTool(ID_ProtractorBtn, true);
//		else if (ruler_type == 5)
//			m_toolbar1->ToggleTool(ID_EllipseBtn, true);
//	}
//	else if (int_mode == 6)
//		m_toolbar2->ToggleTool(ID_RulerMovePointBtn, true);
//	else if (int_mode == 9)
//		m_toolbar2->ToggleTool(ID_RulerMoveBtn, true);
//	else if (int_mode == 11)
//		m_toolbar2->ToggleTool(ID_LockBtn, true);
//	else if (int_mode == 12)
//		m_toolbar1->ToggleTool(ID_GrowBtn, true);
//	else if (int_mode == 13)
//		m_toolbar1->ToggleTool(ID_PencilBtn, true);
//	else if (int_mode == 14)
//		m_toolbar3->ToggleTool(ID_RulerDelBtn, true);
//	else if (int_mode == 15)
//	{
//		bool mag_len = glbin_ruler_handler.GetRedistLength();
//		if (mag_len)
//			m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, true);
//		else
//			m_toolbar2->ToggleTool(ID_MagnetBtn, true);
//	}
//
//	switch (glbin_settings.m_point_volume_mode)
//	{
//	case 0:
//		m_view_plane_rd->SetValue(true);
//		m_max_intensity_rd->SetValue(false);
//		m_acc_intensity_rd->SetValue(false);
//		break;
//	case 1:
//		m_view_plane_rd->SetValue(false);
//		m_max_intensity_rd->SetValue(true);
//		m_acc_intensity_rd->SetValue(false);
//		break;
//	case 2:
//		m_view_plane_rd->SetValue(false);
//		m_max_intensity_rd->SetValue(false);
//		m_acc_intensity_rd->SetValue(true);
//		break;
//	}
//
//	m_use_transfer_chk->SetValue(glbin_settings.m_ruler_use_transf);
//	//ruler exports df/f
//	m_df_f_chk->SetValue(glbin_settings.m_ruler_df_f);
//	glbin_ruler_handler.SetBackground(glbin_settings.m_ruler_df_f);
//	//relax
//	m_relax_value_spin->SetValue(glbin_settings.m_ruler_relax_f1);
//	m_auto_relax_btn->SetValue(glbin_settings.m_ruler_auto_relax);
//	m_view->m_ruler_autorelax = glbin_settings.m_ruler_auto_relax;
//	m_relax_data_cmb->Select(glbin_settings.m_ruler_relax_type);
//}

void MeasureDlg::OnToolbar1(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_LocatorBtn:
		Locator();
		break;
	case ID_ProbeBtn:
		Probe();
		break;
	case ID_RulerBtn:
		Ruler();
		break;
	case ID_ProtractorBtn:
		Protractor();
		break;
	case ID_EllipseBtn:
		Ellipse();
		break;
	case ID_RulerMPBtn:
		RulerMP();
		break;
	case ID_PencilBtn:
		Pencil();
		break;
	case ID_GrowBtn:
		Grow();
		break;
	}
}

void MeasureDlg::OnToolbar2(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_RulerMoveBtn:
		RulerMove();
		break;
	case ID_RulerMovePointBtn:
		RulerMovePoint();
		break;
	case ID_MagnetBtn:
		Magnet();
		break;
	case ID_RulerMovePencilBtn:
		RulerMovePencil();
		break;
	case ID_RulerFlipBtn:
		RulerFlip();
		break;
	case ID_RulerAvgBtn:
		RulerAvg();
		break;
	case ID_LockBtn:
		Lock();
		break;
	case ID_RelaxBtn:
		Relax();
		break;
	}
}

void MeasureDlg::OnToolbar3(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_DeleteBtn:
		DeleteSelection();
		break;
	case ID_DeleteAllBtn:
		DeleteAll();
		break;
	case ID_RulerDelBtn:
		DeletePoint();
		break;
	case ID_PruneBtn:
		Prune();
		break;
	case ID_ProfileBtn:
		Profile();
		break;
	case ID_DistanceBtn:
		Distance();
		break;
	case ID_ProjectBtn:
		Project();
		break;
	case ID_ExportBtn:
		Export();
		break;
	}
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
	glbin_settings.m_point_volume_mode = mode;
}

void MeasureDlg::OnUseTransferCheck(wxCommandEvent& event)
{
	if (!m_view)
		return;

	bool use_transfer = m_use_transfer_chk->GetValue();
	glbin_settings.m_ruler_use_transf = use_transfer;
}

void MeasureDlg::OnTransientCheck(wxCommandEvent& event)
{
	if (!m_view)
		return;

	bool val = m_transient_chk->GetValue();

	//change ruler setting
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = m_view->GetRuler(
			m_ruler_list->GetItemData(index));
		if (!ruler)
			continue;
		ruler->SetTransient(val);
		if (val)
			ruler->SetTransTime(m_view->m_tseq_cur_num);
	}
	m_ruler_list->UpdateRulers(m_view);
}

void MeasureDlg::OnDF_FCheck(wxCommandEvent& event)
{
	bool val = m_df_f_chk->GetValue();
	glbin_ruler_handler.SetBackground(val);
	if (val)
		OnProfile(event);
	m_ruler_list->UpdateRulers(m_view);
	glbin_settings.m_ruler_df_f = val;
}

void MeasureDlg::OnAutoRelax(wxCommandEvent& event)
{
	bool bval = m_auto_relax_btn->GetValue();
	glbin_settings.m_ruler_auto_relax = bval;
	if (m_view)
		m_view->m_ruler_autorelax = bval;
}

void MeasureDlg::OnRelaxValueSpin(wxSpinDoubleEvent& event)
{
	double dval = m_relax_value_spin->GetValue();
	glbin_dist_calculator.SetF1(dval);
	glbin_settings.m_ruler_relax_f1 = dval;
}

void MeasureDlg::OnRelaxValueText(wxCommandEvent& event)
{
	double dval = m_relax_value_spin->GetValue();
	glbin_dist_calculator.SetF1(dval);
	glbin_settings.m_ruler_relax_f1 = dval;
}

void MeasureDlg::OnRelaxData(wxCommandEvent& event)
{
	glbin_settings.m_ruler_relax_type = m_relax_data_cmb->GetSelection();
}

//ruler list
void MeasureDlg::OnNewGroup(wxCommandEvent& event)
{
	glbin_ruler_handler.NewGroup();
}

void MeasureDlg::OnChgGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (m_group_text->GetValue().ToULong(&ival))
		glbin_ruler_handler.SetGroup(ival);

	//update group
	if (!m_view)
		return;
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = m_view->GetRuler(
			m_ruler_list->GetItemData(index));
		if (!ruler)
			continue;
		ruler->Group(ival);
	}
	m_ruler_list->UpdateRulers(m_view);
}

void MeasureDlg::OnSelGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (!m_group_text->GetValue().ToULong(&ival))
		return;
	m_ruler_list->SelectGroup(ival);
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
				m_ruler_list->SetItemBackgroundColour(i, wxColour(255, 255, 255));
			else
				m_ruler_list->SetItemBackgroundColour(i, wxColour(200, 200, 200));
		}
	}
	m_view->RefreshGL(39);
}

//interpolation/key
void MeasureDlg::OnInterpCmb(wxCommandEvent& event)
{
	bool refresh = false;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	int interp = m_interp_cmb->GetSelection();
	std::set<int> sel;
	if (m_ruler_list->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			if (sel[i] < 0 || sel[i] >= ruler_list->size())
				continue;
			flrd::Ruler* ruler = ruler_list->at(sel[i]);
			if (!ruler)
				continue;
			ruler->SetInterp(interp);
			refresh = true;
		}
	}
	if (refresh)
	{
		m_ruler_list->UpdateRulers();
		m_view->RefreshGL(39);
	}
}

void MeasureDlg::OnDeleteKeyBtn(wxCommandEvent& event)
{
	bool refresh = false;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	int interp = m_interp_cmb->GetSelection();
	std::set<int> sel;
	if (m_ruler_list->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			if (sel[i] < 0 || sel[i] >= ruler_list->size())
				continue;
			flrd::Ruler* ruler = ruler_list->at(sel[i]);
			if (!ruler)
				continue;
			ruler->SetWorkTime(m_view->m_tseq_cur_num);
			ruler->DeleteKey();
			refresh = true;
		}
	}
	if (refresh)
	{
		m_ruler_list->UpdateRulers();
		m_view->RefreshGL(39);
	}
}

void MeasureDlg::OnDeleteAllKeyBtn(wxCommandEvent& event)
{
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	int interp = m_interp_cmb->GetSelection();
	std::set<int> sel;
	if (m_ruler_list->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			if (sel[i] < 0 || sel[i] >= ruler_list->size())
				continue;
			flrd::Ruler* ruler = ruler_list->at(sel[i]);
			if (!ruler)
				continue;
			ruler->SetWorkTime(m_view->m_tseq_cur_num);
			ruler->DeleteAllKey();
		}
	}
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
	if (glbin_ruler_handler.GetBackground())
	{
		double bg_int = glbin_ruler_handler.GetVolumeBgInt();
		dval = (dval - bg_int) / bg_int;
		str = wxString::Format("%.4f", dval);
	}
	else
		str = wxString::Format("%.0f", dval);
	m_ruler_list->SetText(i, IntCol, str);
}

void MeasureDlg::OnAlignRuler(wxCommandEvent& event)
{
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	flrd::Ruler* ruler = ruler_list->at(sel[0]);
	glbin_aligner.SetRuler(ruler);

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
	glbin_aligner.SetAxisType(axis_type);
	glbin_aligner.AlignRuler();
	if (m_align_center->GetValue())
		AlignCenter(ruler, 0);
}

void MeasureDlg::OnAlignPca(wxCommandEvent& event)
{
	flrd::RulerList list;
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	for (int i = 0; i < sel.size(); ++i)
		list.push_back((*ruler_list)[sel[i]]);
	glbin_aligner.SetRulerList(&list);

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
	glbin_aligner.SetAxisType(axis_type);
	glbin_aligner.SetAlignCenter(m_align_center->GetValue());
	glbin_aligner.AlignPca(true);
	//if (m_align_center->GetValue())
	//	AlignCenter(0, &list);
}

void MeasureDlg::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
	{
		long item = m_ruler_list->GetNextItem(-1,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item != -1)
		{
			flrd::Ruler* ruler = glbin_current.GetRuler();
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

void MeasureDlg::OnContextMenu(wxContextMenuEvent& event)
{
	if (!m_ruler_list->GetSelectedItemCount())
		return;

	wxPoint point = event.GetPosition();
	// If from keyboard
	if (point.x == -1 && point.y == -1)
	{
		wxSize size = m_ruler_list->GetSize();
		point.x = size.x / 2;
		point.y = size.y / 2;
	}
	else
	{
		point = m_ruler_list->ScreenToClient(point);
	}

	wxMenu menu;
	menu.Append(ID_ToggleDisp, "ToggleDisplay");
	PopupMenu(&menu, point.x, point.y);
}

void MeasureDlg::OnMenuItem(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_ToggleDisp:
		ToggleDisplay();
		break;
	}
}

void MeasureDlg::OnSelection(wxListEvent& event)
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;

	long ival = event.GetIndex();

	flrd::Ruler* ruler = canvas->GetRuler(
		m_ruler_list->GetItemData(ival));
	if (!ruler || !ruler->GetDisp())
		return;

	int type = ruler->GetRulerType();
	bool use_color = ruler->GetUseColor();
	fluo::Color color = ruler->GetColor();
	m_ruler_list->StartEdit(type, use_color, color);
	
	FluoUpdate({ gstRulerTransient, gstRulerInterpolation });
}

void MeasureDlg::OnEndSelection(wxListEvent& event)
{
	m_ruler_list->EndEdit();
	SetCurrentRuler();
}

void MeasureDlg::OnScrollWin(wxScrollWinEvent& event)
{
	m_ruler_list->EndEdit();
	SetCurrentRuler();
	event.Skip(true);
}

void MeasureDlg::OnScrollMouse(wxMouseEvent& event)
{
	m_ruler_list->EndEdit();
	SetCurrentRuler();
	event.Skip(true);
}

void MeasureDlg::OnAct(wxListEvent& event)
{
	ToggleDisplay();
}

