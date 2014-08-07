#include "TraceDlg.h"
#include "VRenderFrame.h"
#include "VRenderView.h"
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include <set>

BEGIN_EVENT_TABLE(TraceListCtrl, wxListCtrl)
	EVT_KEY_DOWN(TraceListCtrl::OnKeyDown)
	EVT_CONTEXT_MENU(TraceListCtrl::OnContextMenu)
	EVT_MENU(Menu_ExportText, TraceListCtrl::OnExportSelection)
	EVT_MENU(Menu_CopyText, TraceListCtrl::OnCopySelection)
END_EVENT_TABLE()

TraceListCtrl::TraceListCtrl(
	wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
wxListCtrl(parent, id, pos, size, style),
	m_frame(frame)
{
	wxListItem itemCol;
	itemCol.SetText("ID");
	InsertColumn(0, itemCol);
	SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Size");
	InsertColumn(1, itemCol);
	SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("X");
	InsertColumn(2, itemCol);
	SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Y");
	InsertColumn(3, itemCol);
	SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Z");
	InsertColumn(4, itemCol);
	SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
}

TraceListCtrl::~TraceListCtrl()
{
}

void TraceListCtrl::Append(unsigned int id, wxColor color,
	int size, double cx, double cy, double cz)
{
	wxString str;
	str = wxString::Format("%u", id);
	long tmp = InsertItem(GetItemCount(), str, 0);
	SetColumnWidth(0, wxLIST_AUTOSIZE);
	str = wxString::Format("%d", size);
	SetItem(tmp, 1, str);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cx);
	SetItem(tmp, 2, str);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cy);
	SetItem(tmp, 3, str);
	SetColumnWidth(3, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cz);
	SetItem(tmp, 4, str);
	SetColumnWidth(4, wxLIST_AUTOSIZE);

	SetItemBackgroundColour(tmp, color);
}

void TraceListCtrl::Update(VRenderView* vrv)
{
	if (vrv)
		m_view = vrv;

	TraceGroup* traces = m_view->GetTraceGroup();
	if (!traces)
		return;

	DeleteAllItems();

	IDMap* ids = traces->GetIDMap();
	if (!ids)
		return;

	IDMapIter id_iter;
	for (id_iter=ids->begin(); id_iter!=ids->end(); ++id_iter)
	{
		unsigned int id = id_iter->second.id;
		double hue = id % 360;
		Color c(HSVColor(hue, 1.0, 1.0));
		wxColor color(c.r()*255, c.g()*255, c.b()*255);
		int size = 0;
		Point center;
		Vertex vertex;
		int time = m_view->m_glview->m_tseq_cur_num;
		if (traces->FindIDInFrame(id, time, vertex))
			size = vertex.vsize;
		else
			size = id_iter->second.size;
		center = id_iter->second.center;
		Append(id, color, size,
			center.x(), center.y(), center.z());
	}
}

void TraceListCtrl::DeleteSelection()
{
	if (!m_view) return;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetItemText(item);
	}
}

void TraceListCtrl::DeleteAll()
{
}

wxString TraceListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void TraceListCtrl::ExportSelection()
{
	if (!m_view) return;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetItemText(item);
		unsigned long id;
		name.ToULong(&id);

		wxFileDialog *fdlg = new wxFileDialog(
			this, "Save the selected trace",
			"", "", "*.txt", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		int rval = fdlg->ShowModal();
		if (rval == wxID_OK)
		{
			wxString filename = fdlg->GetPath();
			m_view->ExportTrace(filename, id);
		}

		if (fdlg)
			delete fdlg;
	}
}

void TraceListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
	{
		wxCommandEvent evt;
		OnCopySelection(evt);
	}
	event.Skip();
}

void TraceListCtrl::OnContextMenu(wxContextMenuEvent &event)
{
	if (GetSelectedItemCount()>0)
	{
		wxPoint point = event.GetPosition();
		//if from keyboard
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
		menu.Append(Menu_ExportText, "Export as text file");
		menu.Append(Menu_CopyText, "Copy");

		PopupMenu(&menu, point.x, point.y);
	}
}

void TraceListCtrl::OnExportSelection(wxCommandEvent& event)
{
	ExportSelection();
}

void TraceListCtrl::OnCopySelection(wxCommandEvent& event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetItemText(item);
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(name));
			wxTheClipboard->Close();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(TraceDlg, wxPanel)
	//load/save trace
	EVT_BUTTON(ID_LoadTraceBtn, TraceDlg::OnLoadTrace)
	EVT_BUTTON(ID_SaveTraceBtn, TraceDlg::OnSaveTrace)
	EVT_BUTTON(ID_SaveasTraceBtn, TraceDlg::OnSaveasTrace)
	//ghost num
	EVT_COMMAND_SCROLL(ID_GhostNumSldr, TraceDlg::OnGhostNumChange)
	EVT_TEXT(ID_GhostNumText, TraceDlg::OnGhostNumText)
	EVT_COMMAND_SCROLL(ID_CellSizeSldr, TraceDlg::OnCellSizeChange)
	EVT_TEXT(ID_CellSizeText, TraceDlg::OnCellSizeText)
	//component tools
	EVT_BUTTON(ID_CompClearBtn, TraceDlg::OnCompClear)
	EVT_BUTTON(ID_CompFullBtn, TraceDlg::OnCompFull)
	EVT_BUTTON(ID_CompAppendBtn, TraceDlg::OnCompAppend)
	EVT_BUTTON(ID_CompExclusiveBtn, TraceDlg::OnCompExclusive)
	//ID link controls
	EVT_BUTTON(ID_CellUpdateBtn, TraceDlg::OnCellUpdate)
	EVT_BUTTON(ID_CellLinkBtn, TraceDlg::OnCellLink)
	EVT_BUTTON(ID_CellExclusiveLinkBtn, TraceDlg::OnCellExclusiveLink)
	EVT_BUTTON(ID_CellUnlinkBtn, TraceDlg::OnCellUnlink)
	//ID edit controls
	EVT_BUTTON(ID_CellModifyBtn, TraceDlg::OnCellModify)
	EVT_BUTTON(ID_CellNewIDBtn, TraceDlg::OnCellNewID)
	EVT_BUTTON(ID_CellCombineIDBtn, TraceDlg::OnCellCombineID)
	//magic tool
	EVT_BUTTON(ID_CellMagic1Btn, TraceDlg::OnCellMagic1Btn)
	EVT_BUTTON(ID_CellMagic2Btn, TraceDlg::OnCellMagic2Btn)
	EVT_BUTTON(ID_CellMagic3Btn, TraceDlg::OnCellMagic3Btn)
	//time controls
	EVT_BUTTON(ID_CellPrevBtn, TraceDlg::OnCellPrev)
	EVT_BUTTON(ID_CellNextBtn, TraceDlg::OnCellNext)
END_EVENT_TABLE()

TraceDlg::TraceDlg(wxWindow* frame, wxWindow* parent)
: wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(500, 600),
0, "TraceDlg"),
m_frame(parent),
m_view(0),
m_mask(0),
m_cur_time(-1),
m_prv_time(-1)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//load trace
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Link map:",
		wxDefaultPosition, wxSize(70, 20));
	m_load_trace_text = new wxTextCtrl(this, ID_LoadTraceText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_load_trace_btn = new wxButton(this, ID_LoadTraceBtn, "Load",
		wxDefaultPosition, wxSize(60, 23));
	m_save_trace_btn = new wxButton(this, ID_SaveTraceBtn, "Save",
		wxDefaultPosition, wxSize(60, 23));
	m_saveas_trace_btn = new wxButton(this, ID_SaveasTraceBtn, "Save As...",
		wxDefaultPosition, wxSize(60, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_load_trace_text, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_1->Add(m_load_trace_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_trace_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_saveas_trace_btn, 0, wxALIGN_CENTER);

	//ghost num
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Ghost num:",
		wxDefaultPosition, wxSize(70, 20));
	m_ghost_num_sldr = new wxSlider(this, ID_GhostNumSldr, 10, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ghost_num_text = new wxTextCtrl(this, ID_GhostNumText, "10",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_ghost_num_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_2->Add(m_ghost_num_text, 0, wxALIGN_CENTER);

	//edit tools
	wxBoxSizer *sizer_3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Edit"),
		wxVERTICAL);
	//cell size filter
	wxBoxSizer* sizer_31 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Component size:",
		wxDefaultPosition, wxSize(130, 20));
	m_cell_size_sldr = new wxSlider(this, ID_CellSizeSldr, 20, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cell_size_text = new wxTextCtrl(this, ID_CellSizeText, "20",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	sizer_31->Add(5, 5);
	sizer_31->Add(st, 0, wxALIGN_CENTER);
	sizer_31->Add(m_cell_size_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_31->Add(m_cell_size_text, 0, wxALIGN_CENTER);
	//selection tools
	wxBoxSizer* sizer_32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Selection tools:",
		wxDefaultPosition, wxSize(130, 20));
	m_comp_id_text = new wxTextCtrl(this, ID_CompIDText, "",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_append_btn = new wxButton(this, ID_CompAppendBtn, "Append",
		wxDefaultPosition, wxSize(60, 23));
	m_comp_exclusive_btn = new wxButton(this, ID_CompExclusiveBtn, "Exclusive",
		wxDefaultPosition, wxSize(60, 23));
	m_comp_full_btn = new wxButton(this, ID_CompFullBtn, "Full",
		wxDefaultPosition, wxSize(60, 23));
	m_comp_clear_btn = new wxButton(this, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(60, 23));
	sizer_32->Add(5, 5);
	sizer_32->Add(st, 0, wxALIGN_CENTER);
	sizer_32->Add(m_comp_id_text, 0, wxALIGN_CENTER);
	sizer_32->Add(10, 23);
	sizer_32->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer_32->Add(m_comp_exclusive_btn, 0, wxALIGN_CENTER);
	sizer_32->Add(m_comp_full_btn, 0, wxALIGN_CENTER);
	sizer_32->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	//ID link controls
	wxBoxSizer* sizer_33 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "ID link tools:",
		wxDefaultPosition, wxSize(130, 20));
	m_cell_update_btn = new wxButton(this, ID_CellUpdateBtn, "Add Selected",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_exclusive_link_btn = new wxButton(this, ID_CellExclusiveLinkBtn, "Exclusive Link",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_link_btn = new wxButton(this, ID_CellLinkBtn, "Link IDs",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_unlink_btn = new wxButton(this, ID_CellUnlinkBtn, "Unlink IDs",
		wxDefaultPosition, wxSize(80, 23));
	sizer_33->Add(5, 5);
	sizer_33->Add(st, 0, wxALIGN_CENTER);
	sizer_33->Add(m_cell_update_btn, 0, wxALIGN_CENTER);
	sizer_33->Add(10, 23);
	sizer_33->Add(m_cell_exclusive_link_btn, 0, wxALIGN_CENTER);
	sizer_33->Add(m_cell_link_btn, 0, wxALIGN_CENTER);
	sizer_33->Add(m_cell_unlink_btn, 0, wxALIGN_CENTER);
	//ID edit controls
	wxBoxSizer* sizer_34 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "ID edit tools:",
		wxDefaultPosition, wxSize(130, 20));
	m_cell_modify_btn = new wxButton(this, ID_CellModifyBtn, "Start Editing",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_new_id_btn = new wxButton(this, ID_CellNewIDBtn, "New ID",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_combine_id_btn = new wxButton(this, ID_CellCombineIDBtn, "Combine IDs",
		wxDefaultPosition, wxSize(80, 23));
	sizer_34->Add(5, 5);
	sizer_34->Add(st, 0, wxALIGN_CENTER);
	sizer_34->Add(m_cell_modify_btn, 0, wxALIGN_CENTER);
	sizer_34->Add(10, 23);
	sizer_34->Add(m_cell_new_id_btn, 0, wxALIGN_CENTER);
	sizer_34->Add(m_cell_combine_id_btn, 0, wxALIGN_CENTER);
	//magic tool
	wxBoxSizer* sizer_35 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Magic happens:",
		wxDefaultPosition, wxSize(130, 20));
	m_cell_magic1_btn = new wxButton(this, ID_CellMagic1Btn, "Diamond",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_magic2_btn = new wxButton(this, ID_CellMagic2Btn, "Diverge",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_magic3_btn = new wxButton(this, ID_CellMagic3Btn, "Over-S",
		wxDefaultPosition, wxSize(80, 23));
	sizer_35->Add(5, 5);
	sizer_35->Add(st, 0, wxALIGN_CENTER);
	sizer_35->Add(90, 23);
	sizer_35->Add(m_cell_magic1_btn, 0, wxALIGN_CENTER);
	sizer_35->Add(m_cell_magic2_btn, 0, wxALIGN_CENTER);
	sizer_35->Add(m_cell_magic3_btn, 0, wxALIGN_CENTER);
	//
	sizer_3->Add(sizer_31, 0, wxEXPAND);
	sizer_3->Add(sizer_32, 0, wxEXPAND);
	sizer_3->Add(sizer_33, 0, wxEXPAND);
	sizer_3->Add(sizer_34, 0, wxEXPAND);
	sizer_3->Add(sizer_35, 0, wxEXPAND);

	//lists
	wxBoxSizer *sizer_4 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "ID Lists"),
		wxVERTICAL);
	//titles
	wxBoxSizer* sizer_41 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_time_curr_st = new wxStaticText(this, 0, "\tCurrent T",
		wxDefaultPosition, wxDefaultSize);
	m_cell_time_prev_st = new wxStaticText(this, 0, "\tPrevious T",
		wxDefaultPosition, wxDefaultSize);
	m_cell_prev_btn = new wxButton(this, ID_CellPrevBtn, "Backward <",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_next_btn = new wxButton(this, ID_CellNextBtn, "Forward >",
		wxDefaultPosition, wxSize(80, 23));
	sizer_41->Add(m_cell_time_curr_st, 1, wxEXPAND|wxCENTER);
	sizer_41->Add(m_cell_prev_btn, 0, wxALIGN_CENTER);
	sizer_41->Add(m_cell_next_btn, 0, wxALIGN_CENTER);
	sizer_41->Add(m_cell_time_prev_st, 1, wxEXPAND|wxCENTER);
	//controls
	wxBoxSizer* sizer_42 = new wxBoxSizer(wxHORIZONTAL);
	m_trace_list_curr = new TraceListCtrl(frame, this, wxID_ANY);
	m_trace_list_prev = new TraceListCtrl(frame, this, wxID_ANY);
	sizer_42->Add(m_trace_list_curr, 1, wxEXPAND);
	sizer_42->Add(m_trace_list_prev, 1, wxEXPAND);
	//
	sizer_4->Add(sizer_41, 0, wxEXPAND);
	sizer_4->Add(sizer_42, 1, wxEXPAND);

	//stats text
	wxBoxSizer *sizer_5 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	m_stat_text = new wxTextCtrl(this, ID_StatText, "",
		wxDefaultPosition, wxSize(-1, 100), wxTE_MULTILINE);
	m_stat_text->SetEditable(false);
	sizer_5->Add(m_stat_text, 1, wxEXPAND);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_4, 1, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_5, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

TraceDlg::~TraceDlg()
{
	if (m_mask)
	{
		delete [] m_mask->data;
		nrrdNix(m_mask);
	}
}

void TraceDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv) return;
	m_view = vrv;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		m_load_trace_text->SetValue(trace_group->GetPath());
		UpdateList();
	}
}

VRenderView* TraceDlg::GetView()
{
	return m_view;
}

void TraceDlg::UpdateList()
{
	if (!m_view) return;
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		int cur_time = trace_group->GetCurTime();
		int prv_time = trace_group->GetPrvTime();
		if (cur_time != m_cur_time)
		{
			wxString item_id;
			wxString item_size;
			wxString item_x;
			wxString item_y;
			wxString item_z;
			unsigned long id;
			double hue;
			long size;
			double x, y, z;
			//copy current to previous
			m_trace_list_prev->DeleteAllItems();
			long item = -1;
			for (;;)
			{
				item = m_trace_list_curr->GetNextItem(item,
					wxLIST_NEXT_ALL,
					wxLIST_STATE_DONTCARE);
				if (item != -1)
				{
					item_id = m_trace_list_curr->GetText(item, 0);
					item_size = m_trace_list_curr->GetText(item, 1);
					item_x = m_trace_list_curr->GetText(item, 2);
					item_y = m_trace_list_curr->GetText(item, 3);
					item_z = m_trace_list_curr->GetText(item, 4);
					item_id.ToULong(&id);
					hue = id % 360;
					Color c(HSVColor(hue, 1.0, 1.0));
					wxColor color(c.r()*255, c.g()*255, c.b()*255);
					item_size.ToLong(&size);
					item_x.ToDouble(&x);
					item_y.ToDouble(&y);
					item_z.ToDouble(&z);
					m_trace_list_prev->Append(id, color, size, x, y, z);
				}
				else break;
			}

			m_cur_time = cur_time;
			m_prv_time = prv_time;
		}

		//set tiem text
		wxString str;
		str = wxString::Format("\tCurrent T: %d", m_cur_time);
		m_cell_time_curr_st->SetLabel(str);
		if (m_prv_time != m_cur_time)
			m_cell_time_prev_st->SetLabel(
				wxString::Format("\tPrevious T: %d", m_prv_time));
		else
			m_cell_time_prev_st->SetLabel("\tPrevious T");
	}
	m_trace_list_curr->Update(m_view);
	Layout();
}

void TraceDlg::OnLoadTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the FluoRender link data file", 
		"", "", "*.fll", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		rval = m_view->LoadTraceGroup(filename);
		if (rval)
			m_load_trace_text->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void TraceDlg::OnSaveTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxString filename = m_load_trace_text->GetValue();
	m_view->SaveTraceGroup(filename);
}

void TraceDlg::OnSaveasTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the FluoRender link data file", 
		"", "", "*.fll", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		rval = m_view->SaveTraceGroup(filename);
		if (rval)
			m_load_trace_text->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void TraceDlg::OnGhostNumChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_ghost_num_text->SetValue(str);
}

void TraceDlg::OnGhostNumText(wxCommandEvent &event)
{
	wxString str = m_ghost_num_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_ghost_num_sldr->SetValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetGhostNum(ival);
			m_view->RefreshGL();
		}
	}
}

//cell size filter
void TraceDlg::OnCellSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_cell_size_text->SetValue(str);
}

void TraceDlg::OnCellSizeText(wxCommandEvent &event)
{
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_cell_size_sldr->SetValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetCellSize(ival);
		}
	}
}

//Component tools
void TraceDlg::OnCompClear(wxCommandEvent &event)
{
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushClear();
	CellUpdate();
}

void TraceDlg::OnCompFull(wxCommandEvent &event)
{
	CellFull();
}

void TraceDlg::OnCompAppend(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get id
	wxString str = m_comp_id_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	unsigned int id = ival;
	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if(!data_mask)
		return;
	//get current label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	vd->GetResolution(nx, ny, nz);
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_label[index] == id)
		{
			data_mask[index] = 255;
		}
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//update view
	CellUpdate();
}

void TraceDlg::OnCompExclusive(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get id
	wxString str = m_comp_id_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	unsigned int id = ival;
	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if(!data_mask)
		return;
	//get current label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	vd->GetResolution(nx, ny, nz);
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_label[index] == id)
		{
			data_mask[index] = 255;
		}
		else
			data_mask[index] = 0;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//update view
	CellUpdate();
}

//ID link controls
void TraceDlg::OnCellUpdate(wxCommandEvent &event)
{
	CellUpdate();
}

void TraceDlg::AddLabel(long item, TraceListCtrl* trace_list_ctrl, vector<Lbl> *list)
{
	wxString str;
	unsigned long id;
	unsigned long size;
	double x, y, z;
	Lbl label;

	str = trace_list_ctrl->GetText(item, 0);
	str.ToULong(&id);
	str = trace_list_ctrl->GetText(item, 1);
	str.ToULong(&size);
	str = trace_list_ctrl->GetText(item, 2);
	str.ToDouble(&x);
	str = trace_list_ctrl->GetText(item, 3);
	str.ToDouble(&y);
	str = trace_list_ctrl->GetText(item, 4);
	str.ToDouble(&z);
	label.id = id;
	label.size = size;
	label.center = Point(x, y, z);
	list->push_back(label);
}

void TraceDlg::CellUpdate()
{
	if (m_view)
	{
		wxString str = m_ghost_num_text->GetValue();
		long ival;
		str.ToLong(&ival);
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
			trace_group->SetGhostNum(ival);
		else
			m_view->CreateTraceGroup();

		m_view->m_glview->GetTraces();
		m_view->RefreshGL();
	}
}

void TraceDlg::CellFull()
{
	if (!m_view)
		return;

	//cell size filter
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	unsigned int slimit = (unsigned int)ival;
	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if(!data_mask)
		return;
	//get current label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//get selected IDs
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	unsigned int max_size = slimit;
	vd->GetResolution(nx, ny, nz);
	unsigned int id;
	boost::unordered_map<unsigned int, Lbl> id_list;
	boost::unordered_map<unsigned int, Lbl>::iterator id_iter;
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
		{
			id = data_label[index];
			id_iter = id_list.find(id);
			if (id_iter == id_list.end())
			{
				Lbl lbl;
				lbl.id = id;
				lbl.size = 1;
				lbl.center = Point(i, j, k);
				id_list.insert(pair<unsigned int, Lbl>(id, lbl));
			}
			else
			{
				id_iter->second.size++;
				id_iter->second.center += Point(i, j, k);
			}
		}
	}
	//calculate center
	for (id_iter=id_list.begin(); id_iter!=id_list.end(); ++id_iter)
	{
		if (id_iter->second.size > 0)
			id_iter->second.center /= id_iter->second.size;
	}
	//reselect
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_label[index])
		{
			id = data_label[index];
			id_iter = id_list.find(id);
			if (id_iter != id_list.end() &&
				id_iter->second.size > slimit)
				data_mask[index] = 255;
			else
				data_mask[index] = 0;
		}
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//update view
	CellUpdate();
}

void TraceDlg::CellLink(bool exclusive)
{
	if (!m_view)
		return;
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		m_view->CreateTraceGroup();

	//get selections
	long item;
	//current T
	vector<Lbl> list_cur;
	//previous T
	vector<Lbl> list_prv;
	//current list
	item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, &list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, &list_cur);
		}
	}
	//previous list
	item = -1;
	while (true)
	{
		item = m_trace_list_prev->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_prev, &list_prv);
	}
	if (list_prv.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_prev->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_prev, &list_prv);
		}
	}
	if (list_cur.size()==0 ||
		list_prv.size()==0)
		return;

	//find the two vertices
	Vertex vert;
	unsigned int i, j;
	for (i=0; i<list_cur.size(); ++i)
	{
		Lbl label = list_cur[i];
		if (!trace_group->FindIDInFrame(label.id, m_cur_time, vert))
			//create vertex
			trace_group->AddVertex(m_cur_time,
				label.id, label.size, label.center);
	}
	for (i=0; i<list_prv.size(); ++i)
	{
		Lbl label = list_prv[i];
		if (!trace_group->FindIDInFrame(label.id, m_prv_time, vert))
			//create vertex
			trace_group->AddVertex(m_prv_time,
				label.id, label.size, label.center);
	}
	//link them
	for (i=0; i<list_cur.size(); ++i)
	{
		unsigned int id1 = list_cur[i].id;
		for (j=0; j<list_prv.size(); ++j)
		{
			unsigned int id2 = list_prv[j].id;
			trace_group->LinkVertices(id1, m_cur_time, id2, m_prv_time, exclusive);
		}
	}
}

void TraceDlg::OnCellLink(wxCommandEvent &event)
{
	CellLink(false);
}

void TraceDlg::OnCellExclusiveLink(wxCommandEvent &event)
{
	CellLink(true);
}

void TraceDlg::OnCellUnlink(wxCommandEvent &event)
{
	if (!m_view)
		return;
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		m_view->CreateTraceGroup();

	//get selections
	long item;
	unsigned long id1 = 0;//current T
	unsigned long id2 = 0;//previous T
	wxString str;
	//current list
	item = m_trace_list_curr->GetNextItem(
		-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		str = m_trace_list_curr->GetText(item, 0);
		str.ToULong(&id1);
	}
	//previous list
	item = m_trace_list_prev->GetNextItem(
		-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		str = m_trace_list_prev->GetText(item, 0);
		str.ToULong(&id2);
	}
	if (id1==0 || id2==0)
		return;

	//find the two vertices
	Vertex vert1, vert2;
	if (!trace_group->FindIDInFrame(id1, m_cur_time, vert1))
	{
		//create vertex
	}
	if (!trace_group->FindIDInFrame(id2, m_prv_time, vert2))
	{
		//create vertex
	}
	trace_group->UnlinkVertices(id1, m_cur_time, id2, m_prv_time);
}

void TraceDlg::OnCellModify(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//save current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd = vd->GetMask();
	if (!nrrd)
		return;
	if (!m_mask)
		m_mask = nrrdNew();
	nrrdCopy(m_mask, nrrd);
	
	//clear current mask
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushClear();
}

void TraceDlg::OnCellNewID(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//get ID of current masked region
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	unsigned int id_init = 0;
	vd->GetResolution(nx, ny, nz);
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
		{
			id_init = data_label[index];
			i = nx;
			j = ny;
			k = nz;
		}
	}
	//generate a unique ID
	int time = m_view->m_glview->m_tseq_cur_num;
	Vertex vertex;
	while (trace_group->FindIDInFrame(id_init, time, vertex))
		id_init -= 180;
	if (id_init == 0)
		return;
	//update label volume, set mask region to the new ID
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
			data_label[index] = id_init;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//save label mask to disk
	BaseReader* reader = vd->GetReader();
	if (reader)
	{
		wxString data_name = reader->GetCurName(time, vd->GetCurChannel());
		wxString label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
		MSKWriter msk_writer;
		msk_writer.SetData(nrrd_label);
		msk_writer.Save(label_name.ToStdWstring(), 1);
	}

	//update view
	m_view->RefreshGL();
}

//magic
void TraceDlg::OnCellMagic1Btn(wxCommandEvent &event)
{
	Test2(1);
}

void TraceDlg::OnCellMagic2Btn(wxCommandEvent &event)
{
	Test2(2);
}

void TraceDlg::OnCellMagic3Btn(wxCommandEvent &event)
{
	Test1();
}

void TraceDlg::Test1()
{
	//wxMessageBox("It happens.");
	if (!m_view)
		return;

	//get data
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//get statistics on selection
	//get these values for each selected ID:
	//total voxels/surface voxels/contact voxels
	vector<comp_info> info_list;
	int ilist;
	int found;
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	unsigned long long indexn;
	unsigned int id;
	bool surface_vox, contact_vox;
	vd->GetResolution(nx, ny, nz);
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
		{
			id = data_label[index];
			//determine the numbers
			if (i==0 || i==nx-1 ||
				j==0 || j==ny-1 ||
				k==0 || k==nz-1)
			{
				//border voxel
				surface_vox = true;
				//determine contact
				contact_vox = false;
				if (i>0)
				{
					indexn = index-1;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						contact_vox = true;
				}
				if (!contact_vox && i<nx-1)
				{
					indexn = index+1;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						contact_vox = true;
				}
				if (!contact_vox && j>0)
				{
					indexn = index-nx;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						contact_vox = true;
				}
				if (!contact_vox && j<ny-1)
				{
					indexn = index+nx;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						contact_vox = true;
				}
				if (!contact_vox && k>0)
				{
					indexn = index-nx*ny;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						contact_vox = true;
				}
				if (!contact_vox && k<nz-1)
				{
					indexn = index+nx*ny;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						contact_vox = true;
				}
			}
			else
			{
				surface_vox = false;
				contact_vox = false;
				//i-1
				indexn = index-1;
				if (data_label[indexn]==0)
					surface_vox = true;
				if (data_label[indexn] &&
					data_label[indexn]!=id)
					surface_vox = contact_vox = true;
				//i+1
				if (!surface_vox || !contact_vox)
				{
					indexn = index+1;
					if (data_label[indexn]==0)
						surface_vox = true;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						surface_vox = contact_vox = true;
				}
				//j-1
				if (!surface_vox || !contact_vox)
				{
					indexn = index-nx;
					if (data_label[indexn]==0)
						surface_vox = true;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						surface_vox = contact_vox = true;
				}
				//j+1
				if (!surface_vox || !contact_vox)
				{
					indexn = index+nx;
					if (data_label[indexn]==0)
						surface_vox = true;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						surface_vox = contact_vox = true;
				}
				//k-1
				if (!surface_vox || !contact_vox)
				{
					indexn = index-nx*ny;
					if (data_label[indexn]==0)
						surface_vox = true;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						surface_vox = contact_vox = true;
				}
				//k+1
				if (!surface_vox || !contact_vox)
				{
					indexn = index+nx*ny;
					if (data_label[indexn]==0)
						surface_vox = true;
					if (data_label[indexn] &&
						data_label[indexn]!=id)
						surface_vox = contact_vox = true;
				}
			}

			//update list
			//find in info list
			found = -1;
			for (ilist=0; ilist<(int)info_list.size(); ++ilist)
			{
				if (info_list[ilist].id == id)
				{
					found = ilist;
					break;
				}
			}
			if (found == -1)
			{
				//not found
				comp_info info;
				info.id = id;
				info.total_num = 1;
				info.surface_num = surface_vox?1:0;
				info.contact_num = contact_vox?1:0;
				info_list.push_back(info);
			}
			else
			{
				//found
				info_list[found].total_num++;
				info_list[found].surface_num += surface_vox?1:0;
				info_list[found].contact_num += contact_vox?1:0;
			}
		}
	}
	wxString str = "Statistics on the selection:\n";
	for (i=0; i<(int)info_list.size(); ++i)
	{
		str += wxString::Format("ID: %u, ", info_list[i].id);
		str += wxString::Format("TotalN: %d, ", info_list[i].total_num);
		str += wxString::Format("SurfaceN: %d, ", info_list[i].surface_num);
		str += wxString::Format("ContactN: %d, ", info_list[i].contact_num);
		str += wxString::Format("Ratio: %.2f\n", (double)info_list[i].contact_num/(double)info_list[i].surface_num);
	}
	m_stat_text->SetValue(str);
}

void TraceDlg::Test2(int type)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	//get data
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//get current selection
	unsigned long id;
	wxString str;
	set<unsigned int> id_list;
	set<unsigned int>::iterator id_iter;
	long item = -1;
	for (;;)
	{
		item = m_trace_list_curr->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_DONTCARE);
		if (item != -1)
		{
			str = m_trace_list_curr->GetText(item ,0);
			str.ToULong(&id);
			id_list.insert(id);
		}
		else break;
	}
	if (id_list.empty())
		return;

	//search from current frame onward
	str = "";
	int time = m_view->m_glview->m_tseq_cur_num;
	for (id_iter=id_list.begin();
		id_iter!=id_list.end(); ++id_iter)
	{
		id = *id_iter;
		if (trace_group->FindPattern(type, id, time))
		{
			str += wxString::Format("ID: %u, ", id);
			str += wxString::Format("time: %d\n", time);
		}
	}
	m_stat_text->SetValue(str);
}

void TraceDlg::OnCellCombineID(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//cell size filter
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	unsigned int slimit = (unsigned int)ival;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask();
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//get an valid ID from selection
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	unsigned int id_init = 0;
	unsigned int max_size = slimit;
	vd->GetResolution(nx, ny, nz);
	Vertex vertex;
	int time = m_view->m_glview->m_tseq_cur_num;
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
		{
			if (trace_group->FindIDInFrame(
				data_label[index], time, vertex))
				if (vertex.vsize > max_size)
				{
					id_init = vertex.id;
					max_size = vertex.vsize;
				}
		}
	}
	if (id_init == 0)
		return;
	//combine IDs within the masked region
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
			data_label[index] = id_init;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//save label mask to disk
	BaseReader* reader = vd->GetReader();
	if (reader)
	{
		wxString data_name = reader->GetCurName(time, vd->GetCurChannel());
		wxString label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
		MSKWriter msk_writer;
		msk_writer.SetData(nrrd_label);
		msk_writer.Save(label_name.ToStdWstring(), 1);
	}

	//update view
	m_view->RefreshGL();
}

void TraceDlg::OnCellPrev(wxCommandEvent &event)
{
	if (m_view)
		m_view->Set4DSeqFrame(m_view->m_glview->m_tseq_cur_num-1, true);
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetMovieView())
			vr_frame->GetMovieView()->SetTimeFrame(m_view->m_glview->m_tseq_cur_num);
}

void TraceDlg::OnCellNext(wxCommandEvent &event)
{
	if (m_view)
	{
		m_view->Set4DSeqFrame(m_view->m_glview->m_tseq_cur_num+1, true);
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetMovieView())
			vr_frame->GetMovieView()->SetTimeFrame(m_view->m_glview->m_tseq_cur_num);
	}
}

