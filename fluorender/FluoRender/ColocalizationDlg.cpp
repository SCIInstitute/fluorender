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
#include "ColocalizationDlg.h"
#include "VRenderFrame.h"
#include "DataManager.h"
#include <Calculate/Compare.h>
#include <fstream>

BEGIN_EVENT_TABLE(ColocalizationDlg, wxPanel)
EVT_TEXT(ID_OutputText, ColocalizationDlg::OnOutputText)
EVT_BUTTON(ID_OutputBtn, ColocalizationDlg::OnOutputBtn)
EVT_BUTTON(ID_CalcColocalizationBtn, ColocalizationDlg::OnColocalizationBtn)
END_EVENT_TABLE()

ColocalizationDlg::ColocalizationDlg(wxWindow* frame,
	wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxDefaultPosition, wxSize(400, 165),
0, "ColocalizationDlg"),
m_frame(parent),
m_group(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	wxStaticText *st = new wxStaticText(this, 0,
		"Select a group of volume channels and compute pair-wise collocalization");
	sizerV->Add(10, 10);
	sizerV->Add(st, 0, wxALIGN_CENTER);

	//output
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Output file:",
		wxDefaultPosition, wxSize(75, 20));
	m_output_text = new wxTextCtrl(this, ID_OutputText, "");
	m_output_btn = new wxButton(this, ID_OutputBtn, "Browse...");
	sizer_1->Add(10, 10);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_output_text, 1, wxALIGN_CENTER);
	sizer_1->Add(m_output_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 10);
	//calculate
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_colocalization_btn = new wxButton(this, ID_CalcColocalizationBtn, "Colocalization",
		wxDefaultPosition, wxDefaultSize);
	sizer_2->AddStretchSpacer(1);
	sizer_2->Add(m_colocalization_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(10, 10);

	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);

	SetSizer(sizerV);
	Layout();
}

ColocalizationDlg::~ColocalizationDlg()
{
}

void ColocalizationDlg::OnOutputText(wxCommandEvent &event)
{
	m_output_file = m_output_text->GetValue();
}

void ColocalizationDlg::OnOutputBtn(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the output file",
		"", "colocalization_result.txt",
		"*.txt", wxFD_OPEN);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		m_output_file = fopendlg->GetPath();
		m_output_text->SetValue(m_output_file);
	}

	if (fopendlg)
		delete fopendlg;
}

void ColocalizationDlg::OnColocalizationBtn(wxCommandEvent &event)
{
	if (!m_group)
		return;

	int num = m_group->GetVolumeNum();
	if (num < 2)
		return;

	//result
	std::vector<std::vector<double>> rm;//result matrix
	rm.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	//fill the matrix
	size_t x = 0, y = 0;
	for (int it1 = 0; it1 < num; ++it1)
	{
		y = x;
		for (int it2 = it1; it2 < num; ++it2)
		{
			VolumeData* vd1 = m_group->GetVolumeData(it1);
			VolumeData* vd2 = m_group->GetVolumeData(it2);
			if (!vd1 || !vd2)
				continue;

			FL::ChannelCompare compare(vd1, vd2);
			//get threshold values
			double th1, th2;
			th1 = vd1->GetLeftThresh();
			th2 = vd2->GetLeftThresh();
			compare.Compare(th1, th2);
			rm[x][y] = compare.Result();
			rm[y][x] = compare.Result();
			y++;
		}
		x++;
	}

	//output
	std::wstring filename = m_output_file.ToStdWstring();
	std::ofstream outfile;
	//print names
	outfile.open(ws2s(filename), std::ofstream::out);
	outfile << "\t";
	for (int i = 0; i < num; ++i)
	{
		outfile << m_group->GetVolumeData(i)->GetName();
		if (i == num-1)
			outfile << "\n";
		else
			outfile << "\t";
	}
	//print adj matrix
	for (size_t i = 0; i < num; ++i)
	{
		outfile << m_group->GetVolumeData(i)->GetName();
		outfile << "\t";
		for (size_t j = 0; j < num; ++j)
		{
			outfile << rm[i][j];
			if (j < num - 1)
				outfile << "\t";
		}
		outfile << "\n";
	}
	outfile.close();

	//nomralize
/*	bool norm = false;
	//getValue("normalize", norm);
	if (norm)
	{
		for (size_t i = 0; i < num; ++i)
		{
			//sum
			double sum = 0;
			for (size_t j = 0; j < num; ++j)
			{
				sum += rm[i][j];
			}
			//divide
			if (sum < 1)
				continue;
			for (size_t j = 0; j < num; ++j)
			{
				rm[i][j] /= sum;
			}
		}
		//output
		std::wstring filename_markov = filename;
		if (pos != std::wstring::npos)
			filename_markov.insert(pos, L"_markov");
		else
			filename_markov += L"_markov";
		outfile.open(filename_markov, std::ofstream::out);
		for (size_t i = 0; i < num; ++i)
		{
			for (size_t j = 0; j < num; ++j)
			{
				outfile << rm[i][j];
				if (j < num - 1)
					outfile << "\t";
			}
			outfile << "\n";
		}
		outfile.close();
	}*/
}

