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
#include <cstdio>
#include <iostream>
#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include "VRenderFrame.h"
#include "compatibility.h"
// -- application --

class VRenderApp : public wxApp
{
   public:
      virtual bool OnInit();
      void OnInitCmdLine(wxCmdLineParser& parser);
      bool OnCmdLineParsed(wxCmdLineParser& parser);

   private:
      wxArrayString m_files;
      int m_file_num;
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
   { wxCMD_LINE_PARAM, NULL, NULL, NULL,
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL|wxCMD_LINE_PARAM_MULTIPLE },
   { wxCMD_LINE_NONE }
};

DECLARE_APP(VRenderApp)
IMPLEMENT_APP(VRenderApp)

bool VRenderApp::OnInit()
{
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
   char cpath[FILENAME_MAX];
   GetCurrentDir(cpath, sizeof(cpath));
   ::wxSetWorkingDirectory(wxString(s2ws(std::string(cpath))));
   // call default behaviour (mandatory)
   if (!wxApp::OnInit())
      return false;
   //add png handler
   wxImage::AddHandler(new wxPNGHandler);
   //the frame
   std::string title =  std::string(FLUORENDER_TITLE) + std::string(" ") +
      std::string(VERSION_MAJOR_TAG) +  std::string(".") +
      std::string(VERSION_MINOR_TAG);
   wxFrame* frame = new VRenderFrame(
         (wxFrame*) NULL,
         wxString(title),
         50,50,1024,768);
   SetTopWindow(frame);
   frame->Show();
   if (m_file_num>0)
      ((VRenderFrame*)frame)->StartupLoad(m_files);
   return true;
}

void VRenderApp::OnInitCmdLine(wxCmdLineParser& parser)
{
   parser.SetDesc (g_cmdLineDesc);
}

bool VRenderApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
   int i=0;
   for (i = 0; i < (int)parser.GetParamCount(); i++)
   {
      wxString file = parser.GetParam(i);
      m_files.Add(file);
   }

   m_file_num = i;

   return true;
}
