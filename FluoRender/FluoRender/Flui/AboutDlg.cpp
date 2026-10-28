/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <AboutDlg.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/toolbar.h>
#include <wx/hyperlink.h>
#include <wx/datetime.h>
#include <wx/artprov.h>

// FluoRender headers
#include <UiText.h>      // AppTitle, Version*, Copyright
#include <icons.h>       // logo, logo_snow
#include <png_resource.h>

AboutDlg::AboutDlg(wxWindow* parent) :
    wxDialog(
        parent,
        wxID_ANY,
        "About FluoRender",
        wxDefaultPosition,
        wxSize(600, 200),
        wxDEFAULT_DIALOG_STYLE)
{
    wxString time = wxNow();
    const bool winterTheme =
        time.Find("Jan") != wxNOT_FOUND ||
        time.Find("Dec") != wxNOT_FOUND;

    auto* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* leftSizer = new wxBoxSizer(wxVERTICAL);
    auto* rightSizer = new wxBoxSizer(wxVERTICAL);

    //
    // Logo panel
    //
    auto* logoBar = new wxToolBar(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTB_NODIVIDER);

    wxBitmapBundle bitmap =
        winterTheme ?
        wxGetBitmap(logo_snow) :
        wxGetBitmap(logo);

    logoBar->AddTool(
        0,
        "",
        bitmap,
        "Thank you for using FluoRender");

    logoBar->SetToolLongHelp(
        0,
        "Thank you for using FluoRender");

    logoBar->Realize();

    leftSizer->Add(logoBar, 0, wxEXPAND);

    //
    // Application title
    //
    auto* txt = new wxStaticText(
        this,
        wxID_ANY,
        fluo::AppTitle);

    txt->SetFont(wxFont(
        15,
        wxFONTFAMILY_ROMAN,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));

    rightSizer->Add(txt, 0, wxEXPAND);

    //
    // Version
    //
    wxString version;
    if (fluo::VersionPatch == 0)
    {
        version = wxString::Format(
            "Version: %d.%d",
            fluo::VersionMajor,
            fluo::VersionMinor);
    }
    else
    {
        version = wxString::Format(
            "Version: %d.%d.%d",
            fluo::VersionMajor,
            fluo::VersionMinor,
            fluo::VersionPatch);
    }

    txt = new wxStaticText(this, wxID_ANY, version);
    txt->SetFont(wxFont(
        12,
        wxFONTFAMILY_ROMAN,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));

    rightSizer->Add(txt, 0, wxEXPAND);

    //
    // Copyright
    //
    txt = new wxStaticText(
        this,
        wxID_ANY,
        wxString("Copyright (c) ") + fluo::Copyright);

    txt->SetFont(wxFont(
        11,
        wxFONTFAMILY_ROMAN,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));

    rightSizer->Add(txt, 0, wxEXPAND);

    rightSizer->AddSpacer(5);

    //
    // Authors
    //
    txt = new wxStaticText(
        this,
        wxID_ANY,
        VERSION_AUTHORS);

    txt->SetFont(wxFont(
        7,
        wxFONTFAMILY_ROMAN,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));

    rightSizer->Add(txt, 0, wxEXPAND);

    //
    // Contact link
    //
    auto* contact = new wxHyperlinkCtrl(
        this,
        wxID_ANY,
        "Contact Info",
        VERSION_CONTACT);

    rightSizer->Add(contact, 0, wxEXPAND);
    rightSizer->AddStretchSpacer();

    //
    // Final layout
    //
    mainSizer->Add(leftSizer, 0, wxEXPAND | wxALL, 10);
    mainSizer->Add(rightSizer, 1, wxEXPAND | wxALL, 10);

    SetSizerAndFit(mainSizer);
    CentreOnParent();
}