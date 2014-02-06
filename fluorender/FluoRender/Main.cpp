#include "Check.h"
#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/msw/registry.h>
#include "VRenderFrame.h"
// -- application --

using namespace std;

class VRenderApp : public wxApp
{
public:
	bool OnInit();
	void OnInitCmdLine(wxCmdLineParser& parser);
	bool OnCmdLineParsed(wxCmdLineParser& parser);

private:
	wxArrayString m_files;
	int m_file_num;
	wxString m_address;

	bool CheckAddress();
	bool CheckTime();
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{ wxCMD_LINE_PARAM, NULL, NULL, NULL,
	wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL|wxCMD_LINE_PARAM_MULTIPLE },
	{ wxCMD_LINE_NONE }
};

DECLARE_APP(VRenderApp)
IMPLEMENT_APP(VRenderApp)

bool VRenderApp::CheckAddress()
{
	string license_address;
	string str_address = GetAddress();
	m_address = str_address;
#ifdef LICENSE_ADDRESS
	license_address = LICENSE_ADDRESS;
	if (license_address.compare("-1") == 0)
		return true;
#endif
#if !_FREE_VERSION_
	istringstream ss(str_address);
	string str;
	while (getline(ss, str))
	{
		if (str.compare(license_address) == 0)
			return true;
	}
	return false;
#endif
	return true;
}

bool VRenderApp::CheckTime()
{
	bool rvalue = false;
	long cur_time = wxGetUTCTime();
	wxRegKey *key = new wxRegKey(wxRegKey::HKCU, "System\\Time_Log");
	if (key->Exists())
	{
		long first_t, last_t;
		key->QueryValue("first_t", &first_t);
		key->QueryValue("last_t", &last_t);
		if (cur_time < last_t)
			rvalue = false;
		else if (cur_time > first_t+604800)
		{
			key->SetValue("last_t", cur_time);
			rvalue = false;
		}
		else
		{
			key->SetValue("last_t", cur_time);
			rvalue = true;
		}
	}
	else
	{
		if (key->Create())
		{
			key->SetValue("first_t", cur_time);
			key->SetValue("last_t", cur_time);
		}
		rvalue = true;
	}
	delete key;
	return rvalue;
}

bool VRenderApp::OnInit()
{
	int free_version = _FREE_VERSION_;
	if (!CheckAddress())
	{
		if (!CheckTime())
		{
			wxMessageBox("This is the version with limited functions.");
			free_version = 1;
		}
	}

	//wxStandardPaths paths;
	wxString path = wxStandardPaths::Get().GetExecutablePath();
	int sep = path.Find('\\', true);
	if (sep != wxNOT_FOUND)
	{
		sep++;
		path = path.Left(sep);
	}
	::wxSetWorkingDirectory(path);

	// call default behaviour (mandatory)
	if (!wxApp::OnInit())
		return false;

	//add png handler
	wxImage::AddHandler(new wxPNGHandler);

	//the frame
	wxFrame* frame = new VRenderFrame(NULL,
		wxID_ANY,
		wxString(FLUORENDER_TITLE)+
		wxString(" ")+
		(free_version?wxString("Version"):
		wxString(FLUORENDER_TITLE_EXTRA))+
		wxString(" ")+
		wxString(VERSION_MAJOR_TAG)+
		wxString(".")+
		wxString(VERSION_MINOR_TAG),
		wxDefaultPosition,
		wxSize(1024,768),
		free_version);
	((VRenderFrame*)frame)->SetAddress(m_address);
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