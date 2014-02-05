#include "TraceDlg.h"
#include "VRenderFrame.h"
#include "VRenderView.h"

BEGIN_EVENT_TABLE(TraceListCtrl, wxListCtrl)
	EVT_KEY_DOWN(TraceListCtrl::OnKeyDown)
	EVT_CONTEXT_MENU(TraceListCtrl::OnContextMenu)
	EVT_MENU(Menu_ExportText, TraceListCtrl::OnExportSelection)
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
	this->InsertColumn(0, itemCol);
	itemCol.SetText("Size");
	this->InsertColumn(1, itemCol);
	itemCol.SetText("Span");
	this->InsertColumn(2, itemCol);
}

TraceListCtrl::~TraceListCtrl()
{
}

void TraceListCtrl::Append(unsigned int id, wxColor color, int size, int stime, int etime)
{
	wxString str;
	str = wxString::Format("%u", id);
	long tmp = InsertItem(GetItemCount(), str, 0);
	SetItemBackgroundColour(tmp, color);
	str = wxString::Format("%d", size);
	SetItem(tmp, 1, str);
	str = wxString::Format("%d -- %d", stime, etime);
	SetItem(tmp, 2, str);
}

void TraceListCtrl::Update(VRenderView* vrv)
{
	if (vrv)
		m_view = vrv;

	TraceGroup* traces = m_view->GetTraceGroup();
	if (!traces)
		return;

	DeleteAllItems();

	hash_map<unsigned int, unsigned int>* ids = traces->GetIDs();
	if (!ids)
		return;

	hash_map<unsigned int, unsigned int>::iterator id_iter;
	for (id_iter=ids->begin(); id_iter!=ids->end(); id_iter++)
	{
		Trace* trace = traces->GetTrace(id_iter->second);
		if (trace)
		{
			unsigned int id = trace->GetID();
			double hue = id % 360;
			Color c(HSVColor(hue, 1.0, 1.0));
			wxColor color(c.r()*255, c.g()*255, c.b()*255);
			int size = trace->GetNumCells();
			int stime = trace->GetCell(0).GetTime();
			int etime = trace->GetCell(size-1).GetTime();

			Append(id, color, size, stime, etime);
		}
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

		PopupMenu(&menu, point.x, point.y);
	}
}

void TraceListCtrl::OnExportSelection(wxCommandEvent& event)
{
	ExportSelection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(TraceDlg, wxPanel)
	EVT_BUTTON(ID_LoadTraceBtn, TraceDlg::OnLoadTrace)
END_EVENT_TABLE()

TraceDlg::TraceDlg(wxWindow* frame, wxWindow* parent)
: wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(450, 600),
0, "TraceDlg"),
m_frame(parent),
m_view(0)
{
	//load trace
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_load_trace_text = new wxTextCtrl(this, ID_LoadTraceText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_load_trace_btn = new wxButton(this, ID_LoadTraceBtn, "Load",
		wxDefaultPosition, wxSize(-1, 23));
	sizer_1->Add(m_load_trace_text, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_1->Add(m_load_trace_btn, 0, wxALIGN_CENTER);

	//list
	m_tracelist = new TraceListCtrl(frame, this, wxID_ANY);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_tracelist, 1, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

TraceDlg::~TraceDlg()
{
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
	m_tracelist->Update(m_view);
}

void TraceDlg::OnLoadTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the trace data file", 
		"", "", "*.trc", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

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