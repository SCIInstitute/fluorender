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
#ifndef _PROPPANEL_H_
#define _PROPPANEL_H_

#include <Value.hpp>
#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <string>

#define FOUND_VALUE(v) vc.find(v) != vc.end()

class MainFrame;
class wxBasisSlider;
class PropBase
{
public:
	PropBase(MainFrame* frame);
	~PropBase() {};

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {}) = 0;
	//excl_self: 0 - update all; 1 - exclude this; 2 - only this; 3 - update none
	virtual void FluoRefresh(int excl_self = 1,
		const fluo::ValueCollection& vc = {},
		const std::set<int>& views = {});

protected:
	MainFrame* m_frame;

	virtual double getDpiScaleFactor();
};

class PropPanel : public PropBase, public wxScrolledWindow
{
public:
	PropPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "PropPanel");
	~PropPanel() {};

protected:
};

class PropDialog : public PropBase, public wxDialog
{
public:
	PropDialog(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "PropPanel");
	~PropDialog() {};

protected:
};

class TabbedPanel : public PropPanel
{
public:
	TabbedPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "TabbedPanel");
	~TabbedPanel() {};
	virtual void LoadPerspective(const std::string& str);
	virtual std::string SavePerspective();

	void AddPage(wxWindow* page, const wxString& caption, bool select = false);
	int FindPage(const wxWindow* page) const;
	bool DeletePage(size_t page);
	bool DeleteAllPages();
	int SetSelection(size_t page);

protected:
	wxAuiNotebook* m_notebook = nullptr;
};
#endif//_PROPPANEL_H_
