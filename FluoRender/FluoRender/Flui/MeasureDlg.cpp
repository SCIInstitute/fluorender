/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <GlobalStates.h>
#include <Names.h>
#include <MainSettings.h>
#include <Project.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <ModalDlg.h>
#include <VertexArray.h>
#include <VolumeRenderer.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <RulerAlign.h>
#include <RulerRenderer.h>
#include <RendererFactory.h>
#include <DistCalculator.h>
#include <VolumeSelector.h>
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
	wxWindow* parent,
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

void RulerListCtrl::Append(
	bool enable,
	unsigned int id,
	bool time_dep,
	const wxString& unit,
	const wxString& name,
	unsigned int group,
	int count,
	const wxString& intensity,
	const wxString& color,
	int branches,
	double length,
	double angle,
	const wxString& center,
	int time,
	const wxString& points,
	const wxString& voxels)
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
	SetItem(tmp, col, voxels); col++;

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

void RulerListCtrl::SetText(long item, int col, const wxString& str)
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
	m_silent_select = true;
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
	m_silent_select = false;
}

void RulerListCtrl::StartEdit(int type, bool use_color, const fluo::Color& color)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_editing_item = item;

	wxRect rect;
	//add frame text
	GetSubItemRect(item, 0, rect);
	m_name = GetText(item, 0);
	m_name_text->SetPosition(rect.GetTopLeft());
	m_name_text->SetSize(rect.GetSize());
	m_name_text->ChangeValue(m_name);
	m_name_text->Show();
	//add color picker
	GetSubItemRect(item, ColorCol, rect);
	m_color_picker->SetPosition(rect.GetTopLeft());
	m_color_picker->SetSize(rect.GetSize());
	if (type == 3)
	{
		//locator
		GetSubItemRect(item, CenterCol, rect);
		wxString str = GetText(item, CenterCol);
		m_center_text->SetPosition(rect.GetTopLeft());
		m_center_text->SetSize(rect.GetSize());
		m_center_text->ChangeValue(str);
		m_center_text->Show();
		m_center = GetPointFromString(str);
	}
	if (use_color)
	{
		wxColor c(int(std::round(color.r() * 255.0)),
			int(std::round(color.g() * 255.0)),
			int(std::round(color.b() * 255.0)));
		m_color_picker->SetColour(c);
		m_color = color;
	}
	m_color_set = false;
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

void RulerListCtrl::OnTextFocus(wxMouseEvent& event)
{
	wxTextCtrl* object = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (object)
		object->SetSelection(0, -1);
}

void RulerListCtrl::OnNameText(wxCommandEvent& event)
{
	m_name = m_name_text->GetValue();
	wxWindow* par = GetParent();
	MeasureDlg* md = dynamic_cast<MeasureDlg*>(par);
	if (md)
		md->SetCurrentRuler();
}

void RulerListCtrl::OnCenterText(wxCommandEvent& event)
{
	wxString str = m_center_text->GetValue();
	wxString old_str = GetText(m_editing_item, CenterCol);
	if (str == old_str)
		return;

	m_center = GetPointFromString(str);

	wxWindow* par = GetParent();
	MeasureDlg* md = dynamic_cast<MeasureDlg*>(par);
	if (md)
		md->SetCurrentRuler();
}

void RulerListCtrl::OnColorChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	m_color = GetColorFromWxColor(c);
	m_color_set = true;

	wxWindow* par = GetParent();
	MeasureDlg* md = dynamic_cast<MeasureDlg*>(par);
	if (md)
		md->SetCurrentRuler();
}

fluo::Point RulerListCtrl::GetPointFromString(const wxString& str)
{
	fluo::Point result;
	//get xyz
	double x = 0, y = 0, z = 0;
	int count = 0;
	std::string stemp = str.ToStdString();
	if (stemp.empty())
		return result;
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
				return result;
		}
		catch (std::invalid_argument)
		{
			stemp = stemp.substr(1, stemp.size() - 1);
			if (stemp.empty())
				return result;
		}
	}
	result = fluo::Point(x, y, z);
	return result;
}

fluo::Color RulerListCtrl::GetColorFromWxColor(const wxColor& c)
{
	return fluo::Color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MeasureDlg::MeasureDlg(MainFrame* frame)
	: TabbedPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 620)),
	0, "MeasureDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->AddPage(CreateToolPage(m_notebook), "Tools", true);
	m_notebook->AddPage(CreateListPage(m_notebook), "Rulers");
	m_notebook->AddPage(CreateAlignPage(m_notebook), "Align");

	Bind(wxEVT_MENU, &MeasureDlg::OnMenuItem, this);
	Bind(wxEVT_SCROLLWIN_TOP, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_BOTTOM, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_LINEUP, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_LINEDOWN, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_PAGEUP, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_PAGEDOWN, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, &MeasureDlg::OnScrollWin, this);
	Bind(wxEVT_MOUSEWHEEL, &MeasureDlg::OnScrollMouse, this);

	wxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);
	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

MeasureDlg::~MeasureDlg()
{
}

wxWindow* MeasureDlg::CreateToolPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText* st;
	wxBitmapBundle bitmap;

	//toolbar
	m_toolbar1 = new wxToolBar(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER|wxTB_TEXT| wxTB_HORIZONTAL);
	bitmap = wxGetBitmap(locator);
	m_toolbar1->AddCheckTool(ID_RulerLocator, "Locator",
		bitmap, wxNullBitmap,
		"Add locators by clicking on data",
		"Add locators by clicking on data");
	bitmap = wxGetBitmap(drill);
	m_toolbar1->AddCheckTool(ID_RulerProbe, "Probe",
		bitmap, wxNullBitmap,
		"Add probes of depth by clicking on data",
		"Add probes of depth by clicking on data");
	bitmap = wxGetBitmap(two_point);
	m_toolbar1->AddCheckTool(ID_RulerLine, "Line",
		bitmap, wxNullBitmap,
		"Add rulers by clicking twice at each end point",
		"Add rulers by clicking twice at each end point");
	bitmap = wxGetBitmap(protractor);
	m_toolbar1->AddCheckTool(ID_RulerAngle, "Angle",
		bitmap, wxNullBitmap,
		"Add protractors for angles by clicking three times",
		"Add protractors for angles by clicking three times");
	bitmap = wxGetBitmap(ellipse);
	m_toolbar1->AddCheckTool(ID_RulerEllipse, "Ellipse",
		bitmap, wxNullBitmap,
		"Add an ellipse for a region by clicking on data",
		"Add an ellipse for a region by clicking on data");
	bitmap = wxGetBitmap(multi_point);
	m_toolbar1->AddCheckTool(ID_RulerPolyline, "Polyline",
		bitmap, wxNullBitmap,
		"Add a polyline ruler by clicking at each point",
		"Add a polyline ruler by clicking at each point");
	bitmap = wxGetBitmap(pencil);
	m_toolbar1->AddCheckTool(ID_RulerPencil, "Pencil",
		bitmap, wxNullBitmap,
		"Draw ruler with multiple points continuously",
		"Draw ruler with multiple points continuously");
	bitmap = wxGetBitmap(grow);
	m_toolbar1->AddCheckTool(ID_RulerGrow, "Grow",
		bitmap, wxNullBitmap,
		"Click and hold to create ruler automatically by growth",
		"Click and hold to create ruler automatically by growth");
	m_toolbar1->Bind(wxEVT_TOOL, &MeasureDlg::OnToolbar, this);
	m_toolbar1->Realize();
	//toolbar2
	m_toolbar2 = new wxToolBar(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT | wxTB_HORIZONTAL);
	bitmap = wxGetBitmap(move);
	m_toolbar2->AddCheckTool(ID_RulerMoveBtn, "Move",
		bitmap, wxNullBitmap,
		"Select and move an entire ruler",
		"Select and move an entire ruler");
	bitmap = wxGetBitmap(ruler_edit);
	m_toolbar2->AddCheckTool(ID_RulerMovePointBtn, "Edit",
		bitmap, wxNullBitmap,
		"Select and move a ruler point",
		"Select and move a ruler point");
	bitmap = wxGetBitmap(magnet);
	m_toolbar2->AddCheckTool(ID_MagnetBtn, "Magnet",
		bitmap, wxNullBitmap,
		"Move ruler points by attracting neighbor points",
		"Move ruler points by attracting neighbor points");
	bitmap = wxGetBitmap(pencil);
	m_toolbar2->AddCheckTool(ID_RulerMovePencilBtn, "Redraw",
		bitmap, wxNullBitmap,
		"Move ruler points by redrawing continuously",
		"Move ruler points by redrawing continuously");
	bitmap = wxGetBitmap(flip_ruler);
	m_toolbar2->AddTool(ID_RulerFlipBtn, "Flip", bitmap,
		"Reverse the order of ruler points");
	m_toolbar2->SetToolLongHelp(ID_RulerFlipBtn,
		"Reverse the order of ruler points");
	bitmap = wxGetBitmap(average);
	m_toolbar2->AddTool(ID_RulerAvgBtn, "Center", bitmap,
		"Compute a center for selected rulers");
	m_toolbar2->SetToolLongHelp(ID_RulerAvgBtn,
		"Compute a center for selected rulers");
	bitmap = wxGetBitmap(lock);
	m_toolbar2->AddCheckTool(ID_LockBtn, "Lock",
		bitmap, wxNullBitmap,
		"Click to lock/unlock a ruler point for relaxing",
		"Click to lock/unlock a ruler point for relaxing");
	bitmap = wxGetBitmap(relax);
	m_toolbar2->AddTool(ID_RelaxBtn, "Relax", bitmap,
		"Smooth the curve of a multipoint ruler by data");
	m_toolbar2->SetToolLongHelp(ID_RelaxBtn,
		"Smooth the curve of a multipoint ruler by data");
	m_toolbar2->Bind(wxEVT_TOOL, &MeasureDlg::OnToolbar, this);
	m_toolbar2->Realize();
	//toolbar3
	m_toolbar3 = new wxToolBar(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT | wxTB_HORIZONTAL);
	bitmap = wxGetBitmap(delet);
	m_toolbar3->AddTool(ID_DeleteBtn, "Delete", bitmap,
		"Delete a selected ruler");
	m_toolbar3->SetToolLongHelp(ID_DeleteBtn,
		"Delete a selected ruler");
	bitmap = wxGetBitmap(del_all);
	m_toolbar3->AddTool(ID_DeleteAllBtn,"Del. All", bitmap,
		"Delete all rulers");
	m_toolbar3->SetToolLongHelp(ID_DeleteAllBtn,
		"Delete all rulers");
	bitmap = wxGetBitmap(ruler_del);
	m_toolbar3->AddCheckTool(ID_RulerDelBtn, "Del. Pt.",
		bitmap, wxNullBitmap,
		"Select and delete a ruler point");
	m_toolbar3->SetToolLongHelp(ID_RulerDelBtn,
		"Select and delete a ruler point");
	bitmap = wxGetBitmap(prune);
	m_toolbar3->AddTool(ID_PruneBtn, "Prune", bitmap,
		"Remove very short branches from ruler");
	m_toolbar3->SetToolLongHelp(ID_PruneBtn,
		"Remove very short branches from ruler");
	bitmap = wxGetBitmap(eyedrop);
	m_toolbar3->AddTool(ID_ProfileBtn, "Sample", bitmap,
		"Sample intensity values along ruler");
	m_toolbar3->SetToolLongHelp(ID_ProfileBtn,
		"Sample intensity values along ruler");
	bitmap = wxGetBitmap(tape);
	m_toolbar3->AddTool(ID_DistanceBtn, "Length", bitmap,
		"Calculate distances");
	m_toolbar3->SetToolLongHelp(ID_DistanceBtn,
		"Calculate distances");
	bitmap = wxGetBitmap(profile);
	m_toolbar3->AddTool(ID_ProjectBtn, "Project", bitmap,
		"Project components onto ruler");
	m_toolbar3->SetToolLongHelp(ID_ProjectBtn,
		"Project components onto ruler");
	bitmap = wxGetBitmap(save);
	m_toolbar3->AddTool(ID_ExportBtn, "Export", bitmap,
		"Export rulers to a text file");
	m_toolbar3->SetToolLongHelp(ID_ExportBtn,
		"Export rulers to a text file");
	m_toolbar3->Bind(wxEVT_TOOL, &MeasureDlg::OnToolbar, this);
	m_toolbar3->Realize();

	//options
	wxStaticBoxSizer *sizer_1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Settings");
	wxBoxSizer* sizer_11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Z-Depth Comp.:",
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
	m_view_plane_rd = new wxRadioButton(page, ID_ViewPlaneRd, "View Plane",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_max_intensity_rd = new wxRadioButton(page, ID_MaxIntensityRd, "Maximum Intensity",
		wxDefaultPosition, wxDefaultSize);
	m_acc_intensity_rd = new wxRadioButton(page, ID_AccIntensityRd, "Accumulated Intensity",
		wxDefaultPosition, wxDefaultSize);
	m_view_plane_rd->Bind(wxEVT_RADIOBUTTON, &MeasureDlg::OnIntensityMethodCheck, this);
	m_max_intensity_rd->Bind(wxEVT_RADIOBUTTON, &MeasureDlg::OnIntensityMethodCheck, this);
	m_acc_intensity_rd->Bind(wxEVT_RADIOBUTTON, &MeasureDlg::OnIntensityMethodCheck, this);
	sizer_11->Add(10, 10);
	sizer_11->Add(st, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_acc_intensity_rd, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_max_intensity_rd, 0, wxALIGN_CENTER);
	sizer_11->Add(10, 10);
	sizer_11->Add(m_view_plane_rd, 0, wxALIGN_CENTER);
	//more options
	wxBoxSizer* sizer_12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Properties:",
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
	m_transient_chk = new wxCheckBox(page, wxID_ANY, "Transient",
		wxDefaultPosition, wxDefaultSize);
	m_use_transfer_chk = new wxCheckBox(page, wxID_ANY, "Use Volume Properties",
		wxDefaultPosition, wxDefaultSize);
	m_transient_chk->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnTransientCheck, this);
	m_use_transfer_chk->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnUseTransferCheck, this);
	sizer_12->Add(10, 10);
	sizer_12->Add(st, 0, wxALIGN_CENTER);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_transient_chk, 0, wxALIGN_CENTER);
	sizer_12->Add(10, 10);
	sizer_12->Add(m_use_transfer_chk, 0, wxALIGN_CENTER);
	//display settings
	wxBoxSizer* sizer_13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Display:",
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
	m_disp_point_chk = new wxCheckBox(page, wxID_ANY, "Point",
		wxDefaultPosition, wxDefaultSize);
	m_disp_line_chk = new wxCheckBox(page, wxID_ANY, "Line",
		wxDefaultPosition, wxDefaultSize);
	m_disp_name_chk = new wxCheckBox(page, wxID_ANY, "Name",
		wxDefaultPosition, wxDefaultSize);
	m_disp_all_chk = new wxCheckBox(page, wxID_ANY, "All",
		wxDefaultPosition, wxDefaultSize);
	m_disp_point_chk->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnDispPointCheck, this);
	m_disp_line_chk->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnDispLineCheck, this);
	m_disp_name_chk->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnDispNameCheck, this);
	m_disp_all_chk->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnDispAllCheck, this);
	sizer_13->Add(10, 10);
	sizer_13->Add(st, 0, wxALIGN_CENTER);
	sizer_13->Add(10, 10);
	sizer_13->Add(m_disp_point_chk, 0, wxALIGN_CENTER);
	sizer_13->Add(10, 10);
	sizer_13->Add(m_disp_line_chk, 0, wxALIGN_CENTER);
	sizer_13->Add(10, 10);
	sizer_13->Add(m_disp_name_chk, 0, wxALIGN_CENTER);
	sizer_13->Add(10, 10);
	sizer_13->Add(m_disp_all_chk, 0, wxALIGN_CENTER);
	//relax settings
	wxBoxSizer* sizer_14 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Relax:",
		wxDefaultPosition, FromDIP(wxSize(90, -1)));
	sizer_14->Add(10, 10);
	sizer_14->Add(st, 0, wxALIGN_CENTER);
	sizer_14->Add(10, 10);
	st = new wxStaticText(page, 0, "Constraint ");
	sizer_14->Add(st, 0, wxALIGN_CENTER);
	m_relax_data_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> items = { "Free", "Volume", "Selection", "Analyzed Comp." };
	m_relax_data_cmb->Append(items);
	m_relax_data_cmb->Bind(wxEVT_COMBOBOX, &MeasureDlg::OnRelaxData, this);
	sizer_14->Add(m_relax_data_cmb, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Ex/In Ratio ");
	sizer_14->Add(10, 10);
	sizer_14->Add(st, 0, wxALIGN_CENTER);
	m_relax_value_spin = new wxSpinCtrlDouble(
		page, wxID_ANY, "2",
		wxDefaultPosition, FromDIP(wxSize(50, 23)),
		wxSP_ARROW_KEYS | wxSP_WRAP,
		0, 100, 2, 0.1);
	m_relax_value_spin->Bind(wxEVT_SPINCTRLDOUBLE, &MeasureDlg::OnRelaxValueSpin, this);
	m_relax_value_spin->Bind(wxEVT_TEXT, &MeasureDlg::OnRelaxValueText, this);
	sizer_14->Add(m_relax_value_spin, 0, wxALIGN_CENTER);
	//
	sizer_1->Add(sizer_11, 0, wxEXPAND);
	sizer_1->Add(10, 10);
	sizer_1->Add(sizer_12, 0, wxEXPAND);
	sizer_1->Add(10, 10);
	sizer_1->Add(sizer_13, 0, wxEXPAND);
	sizer_1->Add(10, 10);
	sizer_1->Add(sizer_14, 0, wxEXPAND);

	//sizer
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar3, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* MeasureDlg::CreateListPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st;

	//list
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	//group
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_new_group = new wxButton(page, wxID_ANY, "New Group",
		wxDefaultPosition, wxDefaultSize);
	st = new wxStaticText(page, 0, "Group ID:",
		wxDefaultPosition, FromDIP(wxSize(65, -1)));
	m_group_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 22)), wxTE_RIGHT, vald_int);
	m_chg_group = new wxButton(page, wxID_ANY, "Change",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_sel_group = new wxButton(page, wxID_ANY, "Select",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_disptgl_group = new wxButton(page, wxID_ANY, "Display",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_group_text->Bind(wxEVT_TEXT, &MeasureDlg::OnGroupText, this);
	m_new_group->Bind(wxEVT_BUTTON, &MeasureDlg::OnNewGroup, this);
	m_chg_group->Bind(wxEVT_BUTTON, &MeasureDlg::OnChgGroup, this);
	m_sel_group->Bind(wxEVT_BUTTON, &MeasureDlg::OnSelGroup, this);
	m_disptgl_group->Bind(wxEVT_BUTTON, &MeasureDlg::OnDispTglGroup, this);
	sizer1->Add(m_new_group, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer();
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_group_text, 0, wxALIGN_CENTER);
	sizer1->Add(m_chg_group, 0, wxALIGN_CENTER);
	sizer1->Add(m_sel_group, 0, wxALIGN_CENTER);
	sizer1->Add(m_disptgl_group, 0, wxALIGN_CENTER);
	//interpolate/key
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Time Interpolation: ",
		wxDefaultPosition, wxDefaultSize);
	m_interp_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> items2 = { "Step", "Linear", "Smooth" };
	m_interp_cmb->Append(items2);
	m_interp_cmb->Bind(wxEVT_COMBOBOX, &MeasureDlg::OnInterpCmb, this);
	m_delete_key_btn = new wxButton(page, wxID_ANY, "Del. Key",
		wxDefaultPosition, wxDefaultSize);
	m_delete_all_key_btn = new wxButton(page, wxID_ANY, "Del. All Keys",
		wxDefaultPosition, wxDefaultSize);
	m_delete_key_btn->Bind(wxEVT_BUTTON, &MeasureDlg::OnDeleteKeyBtn, this);
	m_delete_all_key_btn->Bind(wxEVT_BUTTON, &MeasureDlg::OnDeleteAllKeyBtn, this);
	sizer2->AddStretchSpacer();
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_interp_cmb, 0, wxALIGN_CENTER);
	sizer2->Add(m_delete_key_btn, 0, wxALIGN_CENTER);
	sizer2->Add(m_delete_all_key_btn, 0, wxALIGN_CENTER);
	//list
	m_ruler_list = new RulerListCtrl(page,
		wxDefaultPosition, FromDIP(wxSize(200, 200)), wxLC_REPORT);
	m_ruler_list->Bind(wxEVT_KEY_DOWN, &MeasureDlg::OnKeyDown, this);
	m_ruler_list->Bind(wxEVT_CONTEXT_MENU, &MeasureDlg::OnContextMenu, this);
	m_ruler_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MeasureDlg::OnSelection, this);
	m_ruler_list->Bind(wxEVT_LIST_ITEM_DESELECTED, &MeasureDlg::OnEndSelection, this);
	m_ruler_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MeasureDlg::OnAct, this);

	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(m_ruler_list, 1, wxEXPAND);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* MeasureDlg::CreateAlignPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText* st;

	//alignment
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center = new wxCheckBox(page, wxID_ANY,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_align_center->Bind(wxEVT_CHECKBOX, &MeasureDlg::OnAlignCenterChk, this);
	sizer1->Add(5, 5);
	sizer1->Add(m_align_center, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Mono Axis:",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_x = new wxButton(page, ID_AlignX, "X",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_y = new wxButton(page, ID_AlignY, "Y",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_z = new wxButton(page, ID_AlignZ, "Z",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_nx = new wxButton(page, ID_AlignNX, "-X",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_ny = new wxButton(page, ID_AlignNY, "-Y",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_nz = new wxButton(page, ID_AlignNZ, "-Z",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_x->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignRuler, this);
	m_align_y->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignRuler, this);
	m_align_z->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignRuler, this);
	m_align_nx->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignRuler, this);
	m_align_ny->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignRuler, this);
	m_align_nz->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignRuler, this);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_align_x, 0, wxALIGN_CENTER);
	sizer2->Add(m_align_y, 0, wxALIGN_CENTER);
	sizer2->Add(m_align_z, 0, wxALIGN_CENTER);
	sizer2->Add(m_align_nx, 0, wxALIGN_CENTER);
	sizer2->Add(m_align_ny, 0, wxALIGN_CENTER);
	sizer2->Add(m_align_nz, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tri Axes:",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xyz = new wxButton(page, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yxz = new wxButton(page, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zxy = new wxButton(page, ID_AlignZXY, "ZXY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xzy = new wxButton(page, ID_AlignXZY, "XZY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yzx = new wxButton(page, ID_AlignYZX, "YZX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zyx = new wxButton(page, ID_AlignZYX, "ZYX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xyz->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignPca, this);
	m_align_yxz->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignPca, this);
	m_align_zxy->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignPca, this);
	m_align_xzy->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignPca, this);
	m_align_yzx->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignPca, this);
	m_align_zyx->Bind(wxEVT_BUTTON, &MeasureDlg::OnAlignPca, this);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_align_xyz, 0, wxALIGN_CENTER);
	sizer3->Add(m_align_yxz, 0, wxALIGN_CENTER);
	sizer3->Add(m_align_zxy, 0, wxALIGN_CENTER);
	sizer3->Add(m_align_xzy, 0, wxALIGN_CENTER);
	sizer3->Add(m_align_yzx, 0, wxALIGN_CENTER);
	sizer3->Add(m_align_zyx, 0, wxALIGN_CENTER);
	//
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(5, 5);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void MeasureDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();

	bool bval;
	int ival;

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::None;
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
		//toolbar1
		m_toolbar1->ToggleTool(ID_RulerLocator, rul_mode == flrd::RulerMode::Locator);
		m_toolbar1->ToggleTool(ID_RulerProbe, rul_mode == flrd::RulerMode::Probe);
		m_toolbar1->ToggleTool(ID_RulerLine, rul_mode == flrd::RulerMode::Line);
		m_toolbar1->ToggleTool(ID_RulerAngle, rul_mode == flrd::RulerMode::Protractor);
		m_toolbar1->ToggleTool(ID_RulerEllipse, rul_mode == flrd::RulerMode::Ellipse);
		bval = rul_mode == flrd::RulerMode::Polyline &&
			(int_mode == InteractiveMode::Ruler ||
				int_mode == InteractiveMode::BrushRuler);
		m_toolbar1->ToggleTool(ID_RulerPolyline, bval);
		m_toolbar1->ToggleTool(ID_RulerPencil, int_mode == InteractiveMode::Pencil);
		m_toolbar1->ToggleTool(ID_RulerGrow, int_mode == InteractiveMode::GrowRuler);
		//toolbar2
		m_toolbar2->ToggleTool(ID_RulerMoveBtn, int_mode == InteractiveMode::MoveRuler);
		m_toolbar2->ToggleTool(ID_RulerMovePointBtn, int_mode == InteractiveMode::EditRulerPoint);
		bool bval2 = glbin_ruler_handler.GetRedistLength();
		m_toolbar2->ToggleTool(ID_MagnetBtn, int_mode == InteractiveMode::Magnet && !bval2);
		m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, int_mode == InteractiveMode::Magnet && bval2);
		m_toolbar2->ToggleTool(ID_LockBtn, int_mode == InteractiveMode::RulerLockPoint);
		//toolbar3
		m_toolbar3->ToggleTool(ID_RulerDelBtn, int_mode == InteractiveMode::RulerDelPoint);
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
			if (!ruler)
				continue;
			bval = ruler->GetDisp();
			c = bval ? wxColour(255, 255, 255) : wxColour(200, 200, 200);
			m_ruler_list->SetItemBackgroundColour(i, c);
		}
	}

	if (update_all || FOUND_VALUE(gstRulerListSel))
	{
		ival = glbin_ruler_handler.GetRulerIndex();
		m_ruler_list->SelectItemSilently(ival);
	}

	if (FOUND_VALUE(gstRulerGroupSel))
	{
		UpdateGroupSel();
	}

	if (update_all || FOUND_VALUE(gstRulerProfile))
	{
		UpdateProfile();
	}

	if (update_all || FOUND_VALUE(gstRulerMethod))
	{
		ival = glbin_settings.m_point_volume_mode;
		m_view_plane_rd->SetValue(ival == 0);
		m_max_intensity_rd->SetValue(ival == 1);
		m_acc_intensity_rd->SetValue(ival ==2);
	}

	if (update_all || FOUND_VALUE(gstRulerTransient))
	{
		flrd::Ruler* ruler = glbin_current.GetRuler();
		if(ruler)
		{
			bval = ruler->GetTransient();
			m_transient_chk->SetValue(bval);
		}
	}

	if (update_all || FOUND_VALUE(gstRulerUseTransf))
	{
		m_use_transfer_chk->SetValue(glbin_settings.m_ruler_use_transf);
	}

	if (update_all || FOUND_VALUE(gstRulerDisp))
	{
		flrd::Ruler* ruler = glbin_current.GetRuler();
		if (ruler)
		{
			if (ruler->GetDisp())
			{
				bval = ruler->GetDisplay(0);
				m_disp_point_chk->SetValue(bval);
				bval = ruler->GetDisplay(1);
				m_disp_line_chk->SetValue(bval);
				bval = ruler->GetDisplay(2);
				m_disp_name_chk->SetValue(bval);
				m_disp_all_chk->SetValue(true);
			}
			else
			{
				m_disp_all_chk->SetValue(false);
				m_disp_point_chk->SetValue(false);
				m_disp_line_chk->SetValue(false);
				m_disp_name_chk->SetValue(false);
			}
		}
	}

	if (update_all || FOUND_VALUE(gstRulerRelaxType))
	{
		m_relax_data_cmb->Select(glbin_settings.m_ruler_relax_type);
	}

	if (update_all || FOUND_VALUE(gstRulerF1))
	{
		m_relax_value_spin->SetValue(glbin_settings.m_ruler_relax_f1);
	}

	if (update_all || FOUND_VALUE(gstRulerInterpolation))
	{
		flrd::Ruler* ruler = glbin_current.GetRuler();
		if (ruler)
		{
			ival = ruler->GetInterp();
			m_interp_cmb->Select(ival);
		}
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		m_align_center->SetValue(bval);
	}
}

void MeasureDlg::UpdateRulerList()
{
	m_ruler_list->m_name_text->Hide();
	m_ruler_list->m_center_text->Hide();
	m_ruler_list->m_color_picker->Hide();

	auto view = glbin_current.render_view.lock();
	flrd::RulerList* ruler_list = glbin_current.GetRulerList();
	if (!ruler_list)
		return;

	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);

	std::vector<unsigned int> groups;
	int group_num = ruler_list->GetGroupNum(groups);
	std::vector<int> group_count(group_num, 0);

	m_ruler_list->DeleteAllItems();

	wxString points;
	fluo::Point p;
	int num_points;
	size_t t;
	if (view->m_frame_num_type == 1)
		t = view->m_param_cur_num;
	else
		t = view->m_tseq_cur_num;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		ruler->SetWorkTime(t);
		if (ruler->GetTransient() &&
			ruler->GetTransTime() != t)
			continue;

		wxString unit;
		switch (view->m_sb_unit)
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
		wxString intensity = wxString::Format("%.0f", dval);
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
		wxString voxels = ruler->GetDelInfoValues(", ");
		m_ruler_list->Append(
			ruler->GetDisp(),
			ruler->Id(),
			ruler->GetTransient(),
			unit,
			ruler->GetName(),
			group,
			count,
			intensity,
			color,
			ruler->GetNumBranch(),
			ruler->GetLength(),
			ruler->GetAngle(),
			center,
			ruler->GetTransTime(),
			points,
			voxels);
	}

	m_ruler_list->AdjustSize();

	glbin_vertex_array_manager.set_dirty(flvr::VAType::VA_Rulers);

	//select
	int size = m_ruler_list->GetItemCount();
	for (auto it : sel)
	{
		if (it < size)
			m_ruler_list->SelectItemSilently(it);
	}
}

void MeasureDlg::UpdateRulerListCur()
{
	flrd::Ruler* ruler = glbin_current.GetRuler();
	int item = glbin_ruler_handler.GetRulerIndex();
	if (!ruler)
		return;

	wxString str = ruler->GetName();
	fluo::Point p = ruler->GetCenter();
	fluo::Color c = ruler->GetColor();
	bool use_color = ruler->GetUseColor();

	m_ruler_list->SetText(item, 0, str);
	str = wxString::Format("(%.2f, %.2f, %.2f)",
		p.x(), p.y(), p.z());
	m_ruler_list->SetText(item, CenterCol, str);
	m_ruler_list->SetText(item, PointCol, str);
	if (use_color)
		str = wxString::Format("RGB(%d, %d, %d)",
			int(std::round(c.r() * 255)),
			int(std::round(c.g() * 255)),
			int(std::round(c.b() * 255)));
	else
		str = "N/A";
	m_ruler_list->SetText(item, ColorCol, str);
}

void MeasureDlg::UpdateGroupSel()
{
	m_ruler_list->ClearSelection();
	flrd::RulerList* ruler_list = glbin_current.GetRulerList();
	if (!ruler_list)
		return;

	size_t gi = glbin_ruler_handler.GetGroup();
	for (size_t i = 0; i < ruler_list->size(); ++i)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == gi)
			m_ruler_list->SelectItemSilently(i);
	}
}

void MeasureDlg::ToggleDisplay()
{
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	glbin_ruler_handler.ToggleDisplay(sel);
	FluoRefresh(2, { gstRulerListDisp, gstRulerDisp }, { glbin_current.GetViewId() });
}

void MeasureDlg::SetCurrentRuler()
{
	flrd::Ruler* ruler = glbin_current.GetRuler();
	auto view = glbin_current.render_view.lock();
	if (!ruler || !view)
		return;
	ruler->SetName(m_ruler_list->m_name.ToStdWstring());
	if (ruler->GetRulerMode() == flrd::RulerMode::Locator)
	{
		ruler->SetWorkTime(view->m_tseq_cur_num);
		ruler->SetPoint(0, m_ruler_list->m_center);
	}
	if (m_ruler_list->m_color_set)
		ruler->SetColor(m_ruler_list->m_color);
	FluoRefresh(2, { gstRulerListCur },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::UpdateProfile()
{
	wxString str;
	for (int i = 0; i < m_ruler_list->GetItemCount(); ++i)
	{
		flrd::Ruler* ruler = glbin_ruler_handler.GetRuler(i);
		if (!ruler)
			continue;

		double dval = ruler->GetProfileMaxValue();
		dval *= ruler->GetScalarScale();
		str = wxString::Format("%.0f", dval);
		m_ruler_list->SetText(i, IntCol, str);
	}
}

void MeasureDlg::Locator()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Probe()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Probe);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::RulerLine()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Line);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Protractor()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Protractor);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Ellipse()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Ellipse);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::RulerPolyline()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Polyline);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Pencil()
{
	glbin_states.ToggleIntMode(InteractiveMode::Pencil);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Grow()
{
	bool bval = glbin_states.ToggleIntMode(InteractiveMode::GrowRuler);
	if (!bval)
	{
		//reset label volume
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			vd->GetVR()->clear_tex_mask();
			vd->GetVR()->clear_tex_label();
			vd->AddEmptyMask(0, true);
			vd->AddEmptyLabel(0, true);
		}
	}

	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void MeasureDlg::RulerMove()
{
	glbin_states.ToggleIntMode(InteractiveMode::MoveRuler);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::RulerMovePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::EditRulerPoint);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Magnet()
{
	glbin_states.ToggleMagnet(false);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::RulerMovePencil()
{
	glbin_states.ToggleMagnet(true);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::RulerFlip()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Flip(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::RulerAvg()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.AddAverage(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::Lock()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerLockPoint);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Relax()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Relax(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::DeleteSelection()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.DeleteSelection(sel);
	FluoRefresh(2, { gstRulerList, gstRulerListSel },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::DeleteAll()
{
	glbin_ruler_handler.DeleteAll(false);
	FluoRefresh(2, { gstRulerList, gstRulerListSel },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::DeletePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerDelPoint);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void MeasureDlg::Prune()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Prune(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::Profile()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Profile(sel);

	FluoUpdate({ gstRulerProfile });
}

void MeasureDlg::Distance()
{
	ModalDlg fopendlg(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxstr = fopendlg.GetPath();
		std::set<int> sel;
		m_ruler_list->GetCurrSelection(sel);
		glbin_ruler_handler.Distance(sel, wxstr.ToStdWstring());
	}
}

void MeasureDlg::Project()
{
	ModalDlg fopendlg(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxstr = fopendlg.GetPath();
		std::set<int> sel;
		m_ruler_list->GetCurrSelection(sel);
		glbin_ruler_handler.Project(sel, wxstr.ToStdWstring());
	}
}

void MeasureDlg::Export()
{
	ModalDlg fopendlg(
		m_frame, "Export rulers", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg.ShowModal();

	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		glbin_project.ExportRulerList(filename.ToStdWstring());
	}
}

void MeasureDlg::OnToolbar(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_RulerLocator:
		Locator();
		break;
	case ID_RulerProbe:
		Probe();
		break;
	case ID_RulerLine:
		RulerLine();
		break;
	case ID_RulerAngle:
		Protractor();
		break;
	case ID_RulerEllipse:
		Ellipse();
		break;
	case ID_RulerPolyline:
		RulerPolyline();
		break;
	case ID_RulerPencil:
		Pencil();
		break;
	case ID_RulerGrow:
		Grow();
		break;
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
	glbin_settings.m_point_volume_mode = event.GetId();
}

void MeasureDlg::OnTransientCheck(wxCommandEvent& event)
{
	bool bval = m_transient_chk->GetValue();

	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetTransient(bval, sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnUseTransferCheck(wxCommandEvent& event)
{
	glbin_settings.m_ruler_use_transf = m_use_transfer_chk->GetValue();
}

void MeasureDlg::OnDispPointCheck(wxCommandEvent& event)
{
	bool bval = m_disp_point_chk->GetValue();

	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetDisplay(bval, sel, 0);

	FluoRefresh(2, { gstRulerList, gstRulerDisp },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnDispLineCheck(wxCommandEvent& event)
{
	bool bval = m_disp_line_chk->GetValue();

	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetDisplay(bval, sel, 1);

	FluoRefresh(2, { gstRulerList, gstRulerDisp },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnDispNameCheck(wxCommandEvent& event)
{
	bool bval = m_disp_name_chk->GetValue();

	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetDisplay(bval, sel, 2);

	FluoRefresh(2, { gstRulerList, gstRulerDisp },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnDispAllCheck(wxCommandEvent& event)
{
	bool bval = m_disp_all_chk->GetValue();

	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetDisplay(bval, sel, 0);
	glbin_ruler_handler.SetDisplay(bval, sel, 1);
	glbin_ruler_handler.SetDisplay(bval, sel, 2);
	glbin_ruler_handler.SetDisplay(bval, sel);

	FluoRefresh(2, { gstRulerList, gstRulerDisp },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnRelaxData(wxCommandEvent& event)
{
	glbin_settings.m_ruler_relax_type = m_relax_data_cmb->GetSelection();
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

//ruler list
void MeasureDlg::OnGroupText(wxCommandEvent& event)
{
	unsigned long ival;
	if (!m_group_text->GetValue().ToULong(&ival))
		return;

	glbin_ruler_handler.SetGroup(ival);
}

void MeasureDlg::OnNewGroup(wxCommandEvent& event)
{
	glbin_ruler_handler.NewGroup();
}

void MeasureDlg::OnChgGroup(wxCommandEvent& event)
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.GroupRulers(sel);
	FluoUpdate({ gstRulerList });
}

void MeasureDlg::OnSelGroup(wxCommandEvent& event)
{
	FluoUpdate({ gstRulerGroupSel });
}

void MeasureDlg::OnDispTglGroup(wxCommandEvent& event)
{
	glbin_ruler_handler.ToggleGroupDisp();
	FluoRefresh(2, { gstRulerListDisp },
		{ glbin_current.GetViewId() });
}

//interpolation/key
void MeasureDlg::OnInterpCmb(wxCommandEvent& event)
{
	int ival = m_interp_cmb->GetSelection();
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetInterp(ival, sel);
	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnDeleteKeyBtn(wxCommandEvent& event)
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.DeleteKey(sel);
	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnDeleteAllKeyBtn(wxCommandEvent& event)
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.DeleteAllKeys(sel);
	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnAlignCenterChk(wxCommandEvent& event)
{
	bool bval = m_align_center->GetValue();
	glbin_aligner.SetAlignCenter(bval);
	FluoRefresh(1, { gstAlignCenter }, { -1 });
}

void MeasureDlg::OnAlignRuler(wxCommandEvent& event)
{
	flrd::Ruler* ruler = glbin_current.GetRuler();
	if (!ruler)
		return;

	glbin_aligner.SetRuler(ruler);
	glbin_aligner.SetAxisType(event.GetId());
	glbin_aligner.AlignRuler();
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnAlignPca(wxCommandEvent& event)
{
	flrd::RulerList list;
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.GetRulerList(sel, list);
	glbin_aligner.SetRulerList(&list);
	glbin_aligner.SetAxisType(event.GetId());
	glbin_aligner.AlignPca(true);
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
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
		point = ScreenToClient(point);
	}

	wxMenu menu;
	menu.Append(ID_ToggleDisp, "ToggleDisplay");
	PopupMenu(&menu, point.x, point.y);
}

void MeasureDlg::OnMenuItem(wxCommandEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxMenu* menu = dynamic_cast<wxMenu*>(obj);
	if (!menu)
	{
		event.Skip();
		return;
	}

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
	if (m_ruler_list->m_silent_select)
		return;

	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	long ival = event.GetIndex();

	flrd::Ruler* ruler = view->GetRuler(
		m_ruler_list->GetItemData(ival));
	if (!ruler || !ruler->GetDisp())
		return;

	flrd::RulerMode rul_mode = ruler->GetRulerMode();
	bool use_color = ruler->GetUseColor();
	fluo::Color color = ruler->GetColor();
	m_ruler_list->StartEdit(static_cast<int>(rul_mode), use_color, color);
	
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetSelRulers(sel);

	FluoRefresh(2, { gstRulerTransient, gstRulerInterpolation, gstRulerDisp },
		{ glbin_current.GetViewId() });
}

void MeasureDlg::OnEndSelection(wxListEvent& event)
{
	m_ruler_list->EndEdit();
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.SetSelRulers(sel);
	SetCurrentRuler();
}

void MeasureDlg::OnScrollWin(wxScrollWinEvent& event)
{
	m_ruler_list->EndEdit();
	SetCurrentRuler();
	event.Skip();
}

void MeasureDlg::OnScrollMouse(wxMouseEvent& event)
{
	m_ruler_list->EndEdit();
	SetCurrentRuler();
	event.Skip();
}

void MeasureDlg::OnAct(wxListEvent& event)
{
	ToggleDisplay();
}

//edit
//void MeasureDlg::OnNameChange(wxCommandEvent& event)
//{
//	SetCurrentRuler();
//}
//
//void MeasureDlg::OnCenterChange(wxCommandEvent& event)
//{
//	SetCurrentRuler();
//}
//
//void MeasureDlg::OnColorChange(wxColourPickerEvent& event)
//{
//	SetCurrentRuler();
//}

