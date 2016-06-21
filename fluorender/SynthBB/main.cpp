#include "Components/CompGenerator.h"
#include "DataManager.h"
#include "DataManager.cpp"
#include "Formats/tif_reader.h"
#include "Formats/msk_writer.h"
#include "compatibility.h"
#include "utility.cpp"
#include <wx/wx.h>
//#include <vld.h>

using namespace std;

//progress
float m_prog_bit;
float m_prog;

//initial grow
bool m_initial_grow = true;
bool m_ig_param_transition = false;
int m_ig_iterations = 50;
//translate
double m_ig_translate = 1.0;
double m_ig_translate2 = 1.0;
//scalar falloff
double m_ig_scalar_falloff = 0.15;
double m_ig_scalar_falloff2 = 0.15;
//grad falloff
double m_ig_grad_falloff = 0.1;
double m_ig_grad_falloff2 = 0.1;
//variance falloff
double m_ig_var_falloff = 0.2;
double m_ig_var_falloff2 = 0.2;
//angle falloff
double m_ig_angle_falloff = 0.2;
double m_ig_angle_falloff2 = 0.2;

//sized grow
bool m_sized_grow = true;
bool m_sg_param_transition = false;
int m_sg_iterations = 40;
//size limiter
int m_sg_size_limiter = 20;
int m_sg_size_limiter2 = 20;
//translate
double m_sg_translate = 0.5;
double m_sg_translate2 = 0.5;
//scalar falloff
double m_sg_scalar_falloff = 0.25;
double m_sg_scalar_falloff2 = 0.25;
//grad falloff
double m_sg_grad_falloff = 0.25;
double m_sg_grad_falloff2 = 0.25;
//variance falloff
double m_sg_var_falloff = 0.35;
double m_sg_var_falloff2 = 0.35;
//angle falloff
double m_sg_angle_falloff = 0.35;
double m_sg_angle_falloff2 = 0.35;

//cleanup
bool m_cleanup = true;
int m_cl_iterations = 10;
int m_cl_size_limiter = 5;

//match slices
bool m_match_slices = true;
bool m_bidir_match = false;
int m_size_thresh = 25;
double m_size_ratio = 0.6;
double m_dist_thresh = 2.5;
double m_angle_thresh = 0.7;

bool LoadSettings(wxString &filename)
{
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return false;
	wxFileConfig fconfig(is);

	bool read_value = false;

	//initial grow
	read_value = fconfig.Read("initial_grow", &m_initial_grow) || read_value;
	read_value = fconfig.Read("ig_param_transition", &m_ig_param_transition) || read_value;
	read_value = fconfig.Read("ig_iterations", &m_ig_iterations) || read_value;
	//translate
	read_value = fconfig.Read("ig_translate", &m_ig_translate) || read_value;
	read_value = fconfig.Read("ig_translate2", &m_ig_translate2) || read_value;
	//scalar falloff
	read_value = fconfig.Read("ig_scalar_falloff", &m_ig_scalar_falloff) || read_value;
	read_value = fconfig.Read("ig_scalar_falloff2", &m_ig_scalar_falloff2) || read_value;
	//grad falloff
	read_value = fconfig.Read("ig_grad_falloff", &m_ig_grad_falloff) || read_value;
	read_value = fconfig.Read("ig_grad_falloff2", &m_ig_grad_falloff2) || read_value;
	//variance falloff
	read_value = fconfig.Read("ig_var_falloff", &m_ig_var_falloff) || read_value;
	read_value = fconfig.Read("ig_var_falloff2", &m_ig_var_falloff2) || read_value;
	//angle falloff
	read_value = fconfig.Read("ig_angle_falloff", &m_ig_angle_falloff) || read_value;
	read_value = fconfig.Read("ig_angle_falloff2", &m_ig_angle_falloff2) || read_value;

	//sized grow
	read_value = fconfig.Read("sized_grow", &m_sized_grow) || read_value;
	read_value = fconfig.Read("sg_param_transition", &m_sg_param_transition) || read_value;
	read_value = fconfig.Read("sg_iterations", &m_sg_iterations) || read_value;
	//size limiter
	read_value = fconfig.Read("sg_size_limiter", &m_sg_size_limiter) || read_value;
	read_value = fconfig.Read("sg_size_limiter2", &m_sg_size_limiter2) || read_value;
	//translate
	read_value = fconfig.Read("sg_translate", &m_sg_translate) || read_value;
	read_value = fconfig.Read("sg_translate2", &m_sg_translate2) || read_value;
	//scalar falloff
	read_value = fconfig.Read("sg_scalar_falloff", &m_sg_scalar_falloff) || read_value;
	read_value = fconfig.Read("sg_scalar_falloff2", &m_sg_scalar_falloff2) || read_value;
	//grad falloff
	read_value = fconfig.Read("sg_grad_falloff", &m_sg_grad_falloff) || read_value;
	read_value = fconfig.Read("sg_grad_falloff2", &m_sg_grad_falloff2) || read_value;
	//variance falloff
	read_value = fconfig.Read("sg_var_falloff", &m_sg_var_falloff) || read_value;
	read_value = fconfig.Read("sg_var_falloff2", &m_sg_var_falloff2) || read_value;
	//angle falloff
	read_value = fconfig.Read("sg_angle_falloff", &m_sg_angle_falloff) || read_value;
	read_value = fconfig.Read("sg_angle_falloff2", &m_sg_angle_falloff2) || read_value;

	//cleanup
	read_value = fconfig.Read("cleanup", &m_cleanup) || read_value;
	read_value = fconfig.Read("cl_iterations", &m_cl_iterations) || read_value;
	read_value = fconfig.Read("cl_size_limiter", &m_cl_size_limiter) || read_value;

	//match slices
	read_value = fconfig.Read("match_slices", &m_match_slices) || read_value;
	read_value = fconfig.Read("bidir_match", &m_bidir_match) || read_value;
	read_value = fconfig.Read("size_thresh", &m_size_thresh) || read_value;
	read_value = fconfig.Read("size_ratio", &m_size_ratio) || read_value;
	read_value = fconfig.Read("dist_thresh", &m_dist_thresh) || read_value;
	read_value = fconfig.Read("angle_thresh", &m_angle_thresh) || read_value;

	if (read_value)
		cout << "Settings found: " << filename << "\n";

	return read_value;
}

bool GetSettings(wxString &filename)
{
	wxString dir = filename.BeforeLast(GETSLASH(), NULL);
	wxString search_str;

	search_str = dir + GETSLASH() + "*.txt";
	wxString f = wxFindFirstFile(search_str);
	while (!f.empty())
	{
		if (LoadSettings(f))
			return true;
		f = wxFindNextFile();
	}

	search_str = dir + "*.dft";
	f = wxFindFirstFile(search_str);
	while (!f.empty())
	{
		if (LoadSettings(f))
			return true;
		f = wxFindNextFile();
	}

	return false;
}

void PrintProgress()
{
	m_prog += m_prog_bit;
	printf("%d%% finished.\n", int(m_prog));
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Wrong arguments.\n");
		return 1;
	}

	wxString filename = argv[1];

	if (!GetSettings(filename))
		cout << "Settings not found. Use default settings.\n";

	TIFReader reader;
	wstring str_w = filename.ToStdWstring();
	reader.SetFile(str_w);
	reader.SetSliceSeq(false);
	str_w = L"#$%&@*";
	reader.SetTimeId(str_w);
	reader.Preprocess();

	VolumeData* vd = new VolumeData();
	Nrrd* data = reader.Convert(0, 0, true);
	wxString name = wxString(reader.GetDataName());
	if (!vd || !vd->Load(data, name, filename))
	{
		printf("Volume loading error.\n");
		return 2;
	}

	vd->AddEmptyMask(1);
	vd->AddEmptyLabel();

	FL::ComponentGenerator cg(vd, 0);
	m_prog_bit = 97.0f / float(1 +
		(m_initial_grow ? m_ig_iterations : 0) +
		(m_sized_grow ? m_sg_iterations : 0) +
		(m_cleanup ? m_cl_iterations : 0) +
		(m_match_slices ? (m_bidir_match ? 2 : 1) : 0));
	m_prog = 0.0f;
	boost::signals2::connection connection =
		cg.m_sig_progress.connect(&PrintProgress);

	cg.OrderID_2D();

	if (m_initial_grow)
	{
		cout << "InitialGrow\t" << m_ig_iterations << "\n";
		cg.InitialGrow(m_ig_param_transition, m_ig_iterations,
			float(m_ig_translate), float(m_ig_translate2),
			float(m_ig_scalar_falloff), float(m_ig_scalar_falloff2),
			float(m_ig_grad_falloff), float(m_ig_grad_falloff2),
			float(m_ig_var_falloff), float(m_ig_var_falloff2),
			float(m_ig_angle_falloff), float(m_ig_angle_falloff2));
	}

	if (m_sized_grow)
		cg.SizedGrow(m_sg_param_transition, m_sg_iterations,
			(unsigned int)(m_sg_size_limiter), (unsigned int)(m_sg_size_limiter2),
			float(m_sg_translate), float(m_sg_translate2),
			float(m_sg_scalar_falloff), float(m_sg_scalar_falloff2),
			float(m_sg_grad_falloff), float(m_sg_grad_falloff2),
			float(m_sg_var_falloff), float(m_sg_var_falloff2),
			float(m_sg_angle_falloff), float(m_sg_angle_falloff2));

	if (m_cleanup)
		cg.Cleanup(m_cl_iterations, (unsigned int)(m_cl_size_limiter));

	if (m_match_slices)
	{
		cg.MatchSlices_CPU(false,
			(unsigned int)(m_size_thresh),
			float(m_size_ratio), float(m_dist_thresh),
			float(m_angle_thresh));
		if (m_bidir_match)
			cg.MatchSlices_CPU(true,
				(unsigned int)(m_size_thresh),
				float(m_size_ratio), float(m_dist_thresh),
				float(m_angle_thresh));
	}

	connection.disconnect();

	printf("100%% finished.\n");

	Nrrd* nrrd_label = vd->GetLabel(false);
	if (nrrd_label)
	{
		MSKWriter msk_writer;
		msk_writer.SetData(nrrd_label);
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		msk_writer.SetSpacings(spcx, spcy, spcz);
		filename = filename.Left(filename.find(".")) + ".lbl";
		msk_writer.Save(filename.ToStdWstring(), 1);
	}
	else
	{
		printf("Label not found.\n");
		return 3;
	}

	printf("All done. Quit.\n");

	return 0;
}