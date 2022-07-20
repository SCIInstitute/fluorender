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
#include <MeasureDlg.h>
#include <RenderFrame.h>
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
#include <img/icons.h>

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

RulerListCtrl::RulerListCtrl(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	SetDoubleBuffered(true);

	wxListItem itemCol;
	itemCol.SetText("Name");
	this->InsertColumn(0, itemCol);
	SetColumnWidth(0, 100);
	itemCol.SetText("Group");
	this->InsertColumn(1, itemCol);
	SetColumnWidth(1, 30);
	itemCol.SetText("Count");
	this->InsertColumn(2, itemCol);
	SetColumnWidth(2, 30);
	itemCol.SetText("Color");
	this->InsertColumn(3, itemCol);
	SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Branch");
	this->InsertColumn(4, itemCol);
	SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Length");
	this->InsertColumn(5, itemCol);
	SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Angle/Pitch");
	this->InsertColumn(6, itemCol);
	SetColumnWidth(6, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Center");
	this->InsertColumn(7, itemCol);
	SetColumnWidth(7, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Time");
	this->InsertColumn(8, itemCol);
	SetColumnWidth(8, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Start/End Points (X, Y, Z)");
	this->InsertColumn(9, itemCol);
	SetColumnWidth(9, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Voxels");
	this->InsertColumn(10, itemCol);
	SetColumnWidth(10, wxLIST_AUTOSIZE_USEHEADER);

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
	wxString &color, int branches, double length, wxString &unit,
	double angle, wxString &center, bool time_dep,
	int time, wxString &extra, wxString &points)
{
	long tmp = InsertItem(GetItemCount(), name, 0);
	SetItemData(tmp, long(id));
	wxString str = wxString::Format("%d", group);
	SetItem(tmp, 1, str);
	str = wxString::Format("%d", count);
	SetItem(tmp, 2, str);
	SetItem(tmp, 3, color);
	str = wxString::Format("%d", branches);
	SetItem(tmp, 4, str);
	str = wxString::Format("%.2f", length) + unit;
	SetItem(tmp, 5, str);
	str = wxString::Format("%.1f", angle) + "Deg";
	SetItem(tmp, 6, str);
	SetItem(tmp, 7, center);
	if (time_dep)
		str = wxString::Format("%d", time);
	else
		str = "N/A";
	SetItem(tmp, 8, str);
	SetItem(tmp, 9, points);
	SetItem(tmp, 10, extra);

	if (!enable)
		SetItemBackgroundColour(tmp, wxColour(200, 200, 200));
}

void RulerListCtrl::AdjustSize()
{
	//SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
	SetColumnWidth(3, wxLIST_AUTOSIZE);
	SetColumnWidth(4, wxLIST_AUTOSIZE);
	SetColumnWidth(5, wxLIST_AUTOSIZE);
	SetColumnWidth(6, wxLIST_AUTOSIZE);
	SetColumnWidth(7, wxLIST_AUTOSIZE);
	SetColumnWidth(8, wxLIST_AUTOSIZE_USEHEADER);
	SetColumnWidth(9, wxLIST_AUTOSIZE);
	SetColumnWidth(10, wxLIST_AUTOSIZE_USEHEADER);
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

void RulerListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if ( event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		m_agent->DeleteSelection();
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
	{
		long item = GetNextItem(-1,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item != -1)
		{
			fluo::Point cp = m_agent->GetRulerCenter(GetItemData(item));
			wxString center = wxString::Format("%.2f\t%.2f\t%.2f",
				cp.x(), cp.y(), cp.z());
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(center));
				wxTheClipboard->Close();
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
	m_agent->SelectRuler(item);
}

void RulerListCtrl::OnEndSelection(wxListEvent &event)
{
	m_agent->EndEdit(false);
}

void RulerListCtrl::OnNameText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	wxString str = m_name_text->GetValue();
	m_agent->SetRulerName(GetItemData(m_editing_item), str.ToStdString());
}

void RulerListCtrl::OnCenterText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	wxString str = m_center_text->GetValue();
	wxString old_str = GetText(m_editing_item, 4);
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
	m_agent->SetRulerCenter(GetItemData(m_editing_item), fluo::Point(x, y, z));
}

void RulerListCtrl::OnColorChange(wxColourPickerEvent& event)
{
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
	SetText(m_editing_item, 3, str_color);

	for (size_t i = 0; i < sel.size(); ++i)
		m_agent->SetRulerColor(GetItemData(sel[i]), color);

	//m_view->Update(39);
}

void RulerListCtrl::OnScroll(wxScrollWinEvent& event)
{
	m_agent->EndEdit(false);
	event.Skip(true);
}

void RulerListCtrl::OnScroll(wxMouseEvent& event)
{
	m_agent->EndEdit(false);
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
	bool disp = m_agent->ToggleRulerDisp(GetItemData(item));
	if (disp)
		SetItemBackgroundColour(item, wxColour(255, 255, 255));
	else
		SetItemBackgroundColour(item, wxColour(200, 200, 200));
	m_name_text->Hide();
	m_center_text->Hide();
	m_color_picker->Hide();
	SetItemState(item, 0, wxLIST_STATE_SELECTED);
	//m_view->Update(39);
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
	std::vector<int> sel;
	if (!GetCurrSelection(sel))
		return;

	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		bool disp = m_agent->ToggleRulerDisp(GetItemData(index));
		if (disp)
			SetItemBackgroundColour(index, wxColour(255, 255, 255));
		else
			SetItemBackgroundColour(index, wxColour(200, 200, 200));
	}
	//m_name_text->Hide();
	//m_center_text->Hide();
	//m_color_picker->Hide();
	//SetItemState(item, 0, wxLIST_STATE_SELECTED);
	//m_view->Update(39);
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
	EVT_MENU(ID_RulerEditBtn, MeasureDlg::OnRulerEdit)
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
	EVT_CHECKBOX(ID_AlignCenter, MeasureDlg::OnAlignCenter)
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

MeasureDlg::MeasureDlg(RenderFrame* frame)
	: wxPanel(frame, wxID_ANY,
	wxDefaultPosition, wxSize(500, 600),
	0, "MeasureDlg"),
	m_frame(frame)
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
	m_toolbar1->AddCheckTool(ID_LocatorBtn, "Loctr",
		bitmap, wxNullBitmap,
		"Add locators to the render view by clicking");
	bitmap = wxGetBitmapFromMemory(drill);
	m_toolbar1->AddCheckTool(ID_ProbeBtn, "Probe",
		bitmap, wxNullBitmap,
		"Add probes to the render view by clicking once");
	bitmap = wxGetBitmapFromMemory(two_point);
	m_toolbar1->AddCheckTool(ID_RulerBtn, "2pnt",
		bitmap, wxNullBitmap,
		"Add rulers to the render view by clicking at two end points");
	bitmap = wxGetBitmapFromMemory(protractor);
	m_toolbar1->AddCheckTool(ID_ProtractorBtn, "Protr",
		bitmap, wxNullBitmap,
		"Add protractors to measure angles by clicking at three points");
	bitmap = wxGetBitmapFromMemory(ellipse);
	m_toolbar1->AddCheckTool(ID_EllipseBtn, "Ellips",
		bitmap, wxNullBitmap,
		"Add an ellipse to the render view by clicking at its points");
	bitmap = wxGetBitmapFromMemory(multi_point);
	m_toolbar1->AddCheckTool(ID_RulerMPBtn, "Mpnt",
		bitmap, wxNullBitmap,
		"Add a polyline ruler to the render view by clicking at its points");
	bitmap = wxGetBitmapFromMemory(pencil);
	m_toolbar1->AddCheckTool(ID_PencilBtn, "Pencl",
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
	m_toolbar2->AddCheckTool(ID_RulerEditBtn, "Edit",
		bitmap, wxNullBitmap,
		"Select and move a ruler point");
	bitmap = wxGetBitmapFromMemory(ruler_del);
	m_toolbar2->AddCheckTool(ID_RulerDelBtn, "Delet",
		bitmap, wxNullBitmap,
		"Select and delete a ruler point");
	bitmap = wxGetBitmapFromMemory(flip_ruler);
	m_toolbar2->AddTool(ID_RulerFlipBtn, "Flip", bitmap,
		"Reverse the order of ruler points");
	bitmap = wxGetBitmapFromMemory(average);
	m_toolbar2->AddTool(ID_RulerAvgBtn, "Averg", bitmap,
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
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT | wxTB_HORIZONTAL | wxTB_HORZ_LAYOUT);
	bitmap = wxGetBitmapFromMemory(delet);
#ifdef _DARWIN
	m_toolbar3->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar3->AddTool(ID_DeleteBtn, "Delete", bitmap,
		"Delete a selected ruler");
	bitmap = wxGetBitmapFromMemory(del_all);
	m_toolbar3->AddTool(ID_DeleteAllBtn,"Delete All", bitmap,
		"Delete all rulers");
	bitmap = wxGetBitmapFromMemory(profile);
	m_toolbar3->AddTool(ID_ProfileBtn, "Profile", bitmap,
		"Add intensity profile along curve. Use \"Export\" to view results");
	bitmap = wxGetBitmapFromMemory(tape);
	m_toolbar3->AddTool(ID_DistanceBtn, "Dist", bitmap,
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
	m_rulerlist = new RulerListCtrl(this,
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
}

void MeasureDlg::OnNewLocator(wxCommandEvent& event)
{
	m_agent->Locator();
}

void MeasureDlg::OnNewProbe(wxCommandEvent& event)
{
	m_agent->Probe();
}

void MeasureDlg::OnNewProtractor(wxCommandEvent& event)
{
	m_agent->Protractor();
}

void MeasureDlg::OnNewRuler(wxCommandEvent& event)
{
	m_agent->Ruler();
}

void MeasureDlg::OnNewRulerMP(wxCommandEvent& event)
{
	m_agent->RulerMP();
}

void MeasureDlg::OnEllipse(wxCommandEvent& event)
{
	m_agent->Ellipse();
}

void MeasureDlg::OnGrow(wxCommandEvent& event)
{
	m_agent->Grow();
}

void MeasureDlg::OnPencil(wxCommandEvent& event)
{
	m_agent->Pencil();
}

void MeasureDlg::OnRulerFlip(wxCommandEvent& event)
{
	m_agent->RulerFlip();
}

void MeasureDlg::OnRulerEdit(wxCommandEvent& event)
{
	m_agent->RulerEdit();
}

void MeasureDlg::OnRulerDel(wxCommandEvent& event)
{
	m_agent->RulerDelete();
}

void MeasureDlg::OnRulerMove(wxCommandEvent& event)
{
	m_agent->RulerMove();
}

void MeasureDlg::OnRulerAvg(wxCommandEvent& event)
{
	m_agent->RulerAvg();
}

void MeasureDlg::OnProfile(wxCommandEvent& event)
{
	m_agent->Profile();
}

void MeasureDlg::OnDistance(wxCommandEvent& event)
{
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

	m_agent->Distance(filename);

	if (fopendlg)
		delete fopendlg;
}

void MeasureDlg::OnProject(wxCommandEvent& event)
{
	m_agent->Project();
}

void MeasureDlg::OnLock(wxCommandEvent& event)
{
	m_agent->Lock();
}

void MeasureDlg::OnPrune(wxCommandEvent& event)
{
	m_agent->Prune(1);
}

void MeasureDlg::OnRelax(wxCommandEvent& event)
{
	m_agent->Relax();
}

void MeasureDlg::OnDelete(wxCommandEvent& event)
{
	m_agent->DeleteSelection();
}

void MeasureDlg::OnDeleteAll(wxCommandEvent& event)
{
	m_agent->DeleteAll(0);
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
		m_agent->Export(filename.ToStdString());
	}

	if (fopendlg)
		delete fopendlg;
}

void MeasureDlg::OnIntensityMethodCheck(wxCommandEvent& event)
{
	long mode = 0;
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
	m_agent->setValue(gstPointVolumeMode, mode);
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetPointVolumeMode(mode);
}

void MeasureDlg::OnUseTransferCheck(wxCommandEvent& event)
{
	bool use_transfer = m_use_transfer_chk->GetValue();
	m_agent->setValue(gstRulerUseTransf, use_transfer);
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetRulerUseTransf(use_transfer);
}

void MeasureDlg::OnTransientCheck(wxCommandEvent& event)
{
	bool val = m_transient_chk->GetValue();
	m_agent->setValue(gstRulerTransient, val);
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetRulerTimeDep(val);
}

void MeasureDlg::OnDF_FCheck(wxCommandEvent& event)
{
	bool val = m_df_f_chk->GetValue();
	m_agent->setValue(gstRulerDfoverf, val);
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetRulerDF_F(val);
}

void MeasureDlg::OnAutoRelax(wxCommandEvent& event)
{
	bool bval = m_auto_relax_btn->GetValue();
	m_agent->setValue(gstRulerRelax, bval);
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetRulerAutoRelax(bval);
}

void MeasureDlg::OnRelaxValueSpin(wxSpinDoubleEvent& event)
{
	double dval = m_relax_value_spin->GetValue();
	m_agent->setValue(gstRulerF1, dval);
	//m_calculator->SetF1(dval);
	//relax
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetRulerRelaxF1(dval);
}

void MeasureDlg::OnRelaxValueText(wxCommandEvent& event)
{
	wxString str = m_relax_value_spin->GetText()->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_agent->setValue(gstRulerF1, dval);
		//relax
		//if (m_frame && m_frame->GetSettingDlg())
		//	m_frame->GetSettingDlg()->SetRulerRelaxF1(dval);
	}
}

void MeasureDlg::OnRelaxData(wxCommandEvent& event)
{
	int ival = m_relax_data_cmb->GetSelection();
	//if (m_frame && m_frame->GetSettingDlg())
	//	m_frame->GetSettingDlg()->SetRulerRelaxType(ival);
}

//ruler list
void MeasureDlg::OnNewGroup(wxCommandEvent& event)
{
	m_agent->NewGroup();
}

void MeasureDlg::OnChgGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (m_group_text->GetValue().ToULong(&ival))
		m_agent->ChangeGroup(ival);
}

void MeasureDlg::OnSelGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (!m_group_text->GetValue().ToULong(&ival))
		return;
	m_agent->SelectGroup(ival);
}

void MeasureDlg::OnDispTglGroup(wxCommandEvent& event)
{
	unsigned long ival;
	if (!m_group_text->GetValue().ToULong(&ival))
		return;
	m_agent->ToggleGroupDisp(ival);
}

void MeasureDlg::OnAlignCenter(wxCommandEvent& event)
{
	bool bval = m_align_center->GetValue();
	m_agent->setValue(gstAlignCenter, bval);
}

void MeasureDlg::OnAlignRuler(wxCommandEvent& event)
{
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
	m_agent->setValue(gstAlignAxisType, long(axis_type));
	m_agent->AlignRuler();
}

void MeasureDlg::OnAlignPca(wxCommandEvent& event)
{
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
	m_agent->setValue(gstAlignAxisType, long(axis_type));
	m_agent->AlignPca();
}

