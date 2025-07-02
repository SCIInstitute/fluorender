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
#include <ProjectPanel.h>
#include <MainFrame.h>
//resources
#include <png_resource.h>
#include <icons.h>

ProjectPanel::ProjectPanel(MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	TabbedPanel(frame, frame, pos, size, style, name)
{
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
	Freeze();

	wxBitmapBundle bitmap;
	//toolbar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_LEFT|wxTB_NODIVIDER);
	bitmap = wxGetBitmap(filter_small);
	m_toolbar->AddTool(ID_Ocl, "Volume Filter", bitmap,
		"Show Volume Filter Dialog");
	m_toolbar->SetToolLongHelp(ID_Ocl, "Show Volume Filter Dialog");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(brush_small);
	m_toolbar->AddTool(ID_Brush, "Paint Brush", bitmap,
		"Show Paint Brush Dialog");
	m_toolbar->SetToolLongHelp(ID_Brush, "Show Paint Brush Dialog");
	bitmap = wxGetBitmap(measure_small);
	m_toolbar->AddTool(ID_Measurement, "Measurement", bitmap,
		"Show Measurement Dialog");
	m_toolbar->SetToolLongHelp(ID_Measurement, "Show Measurement Dialog");
	bitmap = wxGetBitmap(component_small);
	m_toolbar->AddTool(ID_Component, "Component Analyzer", bitmap,
		"Show Component Analyzer Dialog");
	m_toolbar->SetToolLongHelp(ID_Component, "Show Component Analyzer Dialog");
	bitmap = wxGetBitmap(track_small);
	m_toolbar->AddTool(ID_Track, "Tracking", bitmap,
		"Show Tracking Dialog");
	m_toolbar->SetToolLongHelp(ID_Track, "Show Tracking Dialog");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(calculate_small);
	m_toolbar->AddTool(ID_Calculation, "Calculation", bitmap,
		"Show Calculation Dialog");
	m_toolbar->SetToolLongHelp(ID_Calculation, "Show Calculation Dialog");
	bitmap = wxGetBitmap(noise_red_small);
	m_toolbar->AddTool(ID_NoiseReduct, "Noise Reduction", bitmap,
		"Show Noise Reduction Dialog");
	m_toolbar->SetToolLongHelp(ID_NoiseReduct, "Show Noise Reduction Dialog");
	bitmap = wxGetBitmap(size_small);
	m_toolbar->AddTool(ID_VolumeSize, "Volume Size", bitmap,
		"Show Volume Size Dialog");
	m_toolbar->SetToolLongHelp(ID_VolumeSize, "Show Volume Size Dialog");
	bitmap = wxGetBitmap(colocal_small);
	m_toolbar->AddTool(ID_Colocalization, "Colocalization", bitmap,
		"Show Colocalization Dialog");
	m_toolbar->SetToolLongHelp(ID_Colocalization, "Show Colocalization Dialog");
	bitmap = wxGetBitmap(convert_small);
	m_toolbar->AddTool(ID_Convert, "Convert", bitmap,
		"Show Convert Dialog");
	m_toolbar->SetToolLongHelp(ID_Convert, "Show Convert Dialog");

	m_toolbar->Bind(wxEVT_TOOL, &ProjectPanel::OnToolbar, this);
	m_toolbar->Realize();

	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, size,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);

	wxBoxSizer *sizerv = new wxBoxSizer(wxHORIZONTAL);
	sizerv->Add(m_toolbar, 0, wxEXPAND | wxTOP, 20);
	sizerv->Add(m_notebook, 1, wxEXPAND);

	SetSizer(sizerv);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

void ProjectPanel::OnToolbar(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_Brush:
		m_frame->ShowBrushDlg();
		break;
	case ID_Measurement:
		m_frame->ShowMeasureDlg();
		break;
	case ID_Component:
		m_frame->ShowComponentDlg();
		break;
	case ID_Track:
		m_frame->ShowTrackDlg();
		break;
	case ID_Calculation:
		m_frame->ShowCalculationDlg();
		break;
	case ID_NoiseReduct:
		m_frame->ShowNoiseCancellingDlg();
		break;
	case ID_VolumeSize:
		m_frame->ShowCountingDlg();
		break;
	case ID_Colocalization:
		m_frame->ShowColocalizationDlg();
		break;
	case ID_Convert:
		m_frame->ShowConvertDlg();
		break;
	case ID_Ocl:
		m_frame->ShowOclDlg();
		break;
	}
}
