//#include "Check.h"
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
{/*
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
char cpath[FILENAME_MAX];
GetCurrentDir(cpath, sizeof(cpath));
std::cout << cpath << "\n\n\n" << std::endl;
   //std::wcout << wxGetCwd().ToStdWstring() << "\n\n\n" << std::endl;
   ::wxSetWorkingDirectory(wxString(s2ws(std::string(cpath))));
   std::cout << "TEST!  " << std::endl;
  */
   // call default behaviour (mandatory)
   if (!wxApp::OnInit())
      return false;

   std::cout << "TEST2!  " << std::endl;
   //add png handler
   wxImage::AddHandler(new wxPNGHandler);

   //the frame
   std::string title =  std::string(FLUORENDER_TITLE) + std::string(" ") +
      std::string(VERSION_MAJOR_TAG) +  std::string(".") +
      std::string(VERSION_MINOR_TAG);
    std::cout << "TEST3!  " << title << std::endl;
    //MyFrame *frame0 = new MyFrame((wxFrame *) NULL,
   //                              wxString("Text wxWidgets sample"), 50, 50, 700, 550);
   wxFrame* frame = new VRenderFrame(
         (wxFrame*) NULL,
         wxString(title),
         50,50,1024,768);
   std::cout << "TEST4!  " << std::endl;
   SetTopWindow(frame);
   std::cout << "TEST5!  " << std::endl;
   frame->Show();
   std::cout << "TEST6!  " << std::endl;
   if (m_file_num>0)
      ((VRenderFrame*)frame)->StartupLoad(m_files);
   std::cout << "TEST7!  " << std::endl;

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
