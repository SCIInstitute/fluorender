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
#include <GL/glew.h>
#include "compatibility.h"
#include <vector>
#include <boost/unordered_map.hpp>
#include <string.h>
#include <tiffio.h>
#include "FLIVR/BBox.h"
#include "FLIVR/Color.h"
#include "FLIVR/Point.h"
#include "FLIVR/MeshRenderer.h"
#include "FLIVR/VolumeRenderer.h"
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include "Formats/base_reader.h"
#include "Formats/oib_reader.h"
#include "Formats/oif_reader.h"
#include "Formats/nrrd_reader.h"
#include "Formats/tif_reader.h"
#include "Formats/nrrd_writer.h"
#include "Formats/tif_writer.h"
#include "Formats/msk_reader.h"
#include "Formats/msk_writer.h"
#include "Formats/lsm_reader.h"
#include "Formats/lbl_reader.h"
#include "Formats/pvxml_reader.h"

#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

using namespace std;
using namespace FLIVR;

#define DATA_VOLUME			1
#define DATA_MESH			2
#define DATA_ANNOTATIONS	3

#define LOAD_TYPE_NRRD		1
#define LOAD_TYPE_TIFF		2
#define LOAD_TYPE_OIB		3
#define LOAD_TYPE_OIF		4
#define LOAD_TYPE_LSM		5
#define LOAD_TYPE_PVXML		6

class TreeLayer
{
public:
	TreeLayer();
	~TreeLayer();

	int IsA()
	{
		return type;
	}
	wxString GetName()
	{
		return m_name;
	}
	void SetName(wxString name)
	{
		m_name = name;
	}

	//layer adjustment
	//gamma
	const Color GetGamma()
	{return m_gamma;}
	void SetGamma(Color gamma)
	{m_gamma = gamma;}
	//brightness
	const Color GetBrightness()
	{return m_brightness;}
	void SetBrightness(Color brightness)
	{m_brightness = brightness;}
	//hdr settings
	const Color GetHdr()
	{return m_hdr;}
	void SetHdr(Color hdr)
	{m_hdr = hdr;}
	//sync values
	bool GetSyncR()
	{return m_sync_r;}
	void SetSyncR(bool sync_r)
	{m_sync_r = sync_r;}
	bool GetSyncG()
	{return m_sync_g;}
	void SetSyncG(bool sync_g)
	{m_sync_g = sync_g;}
	bool GetSyncB()
	{return m_sync_b;}
	void SetSyncB(bool sync_b)
	{m_sync_b = sync_b;}

	//randomize color
	virtual void RandomizeColor() {}

	//soft threshold
	static void SetSoftThreshsold(double val)
	{m_sw = val;}

	//associated layer
	TreeLayer* GetAssociated()
	{return m_associated;}
	void SetAssociated(TreeLayer* layer)
	{m_associated = layer;}

protected:
	int type;//-1:invalid, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	wxString m_name;

	//layer adjustment
	Color m_gamma;
	Color m_brightness;
	Color m_hdr;
	bool m_sync_r;
	bool m_sync_g;
	bool m_sync_b;

	//soft threshold
	static double m_sw;

	//associated layer
	TreeLayer* m_associated;
};

class VolumeData : public TreeLayer
{
public:
	VolumeData();
	VolumeData(VolumeData &copy);
	virtual ~VolumeData();

	//duplication
	bool GetDup();
	//increase duplicate counter
	void IncDupCounter();

	//data related
	//reader
	void SetReader(BaseReader* reader) {m_reader = reader;}
	BaseReader* GetReader() {return m_reader;}
	//compression
	void SetCompression(bool compression);
	bool GetCompression();
	//skip brick
	void SetSkipBrick(bool skip);
	bool GetSkipBrick();
	//load
	int Load(Nrrd* data, wxString &name, wxString &path);
	int Replace(Nrrd* data, bool del_tex);
	int Replace(VolumeData* data);
	//empty data
	void AddEmptyData(int bits,
		int nx, int ny, int nz,
		double spcx, double spcy, double spcz);
	//load mask
	void LoadMask(Nrrd* mask);
	Nrrd* GetMask();
	//empty mask
	void AddEmptyMask();
	//load label
	void LoadLabel(Nrrd* label);
	//empty label
	//mode: 0-zeros;1-ordered; 2-shuffled
	void AddEmptyLabel(int mode=0);

	//save
	double GetOriginalValue(int i, int j, int k);
	double GetTransferedValue(int i, int j, int k);
	void Save(wxString &filename, int mode=0, bool bake=false, bool compress=false);

	//volumerenderer
	VolumeRenderer *GetVR();
	//texture
	Texture* GetTexture();

	//bounding box
	BBox GetBounds();
	//path
	void SetPath(wxString path);
	wxString GetPath();
	//multi-channel
	void SetCurChannel(int chan);
	int GetCurChannel();
	//time sequence
	void SetCurTime(int time);
	int GetCurTime();

	//draw volume
	void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat, glm::mat4 &tex_mat);
	void Draw(bool otho = false, bool intactive = false, double zoom = 1.0);
	void DrawBounds();
	//draw mask (create the mask)
	//type: 0-initial; 1-diffusion-based growing
	//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear
	//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
	void DrawMask(int type, int paint_mode, int hr_mode,
		double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
		double w2d, double bins, bool ortho=false, bool estimate=false);
	//draw label (create the label)
	//type: 0-initialize; 1-maximum intensity filtering
	//mode: 0-normal; 1-posterized, 2-copy values
	void DrawLabel(int type, int mode, double thresh, double gm_falloff);

	//calculation
	void Calculate(int type, VolumeData* vd_a, VolumeData* vd_b);

	//set 2d mask for segmentation
	void Set2dMask(GLuint mask);
	//set 2d weight map for segmentation
	void Set2DWeight(GLuint weight1, GLuint weight2);
	//set 2d depth map for rendering shadows
	void Set2dDmap(GLuint dmap);

	//properties
	//transfer function
	void Set3DGamma(double dVal);
	double Get3DGamma();
	void SetBoundary(double dVal);
	double GetBoundary();
	void SetOffset(double dVal);
	double GetOffset();
	void SetLeftThresh(double dVal);
	double GetLeftThresh();
	void SetRightThresh(double dVal);
	double GetRightThresh();
	void SetColor(Color &color, bool update_hsv=true);
	Color GetColor();
	void SetMaskColor(Color &color, bool set=true);
	Color GetMaskColor();
	bool GetMaskColorSet();
	void ResetMaskColorSet();
	Color SetLuminance(double dVal);
	double GetLuminance();
	void SetAlpha(double alpha);
	double GetAlpha();
	void SetEnableAlpha(bool mode);
	bool GetEnableAlpha();
	void SetHSV(double hue = -1, double sat = -1, double val = -1);
	void GetHSV(double &hue, double &sat, double &val);

	//mask threshold
	void SetMaskThreshold(double thresh);
	void SetUseMaskThreshold(bool mode);

	//shading
	void SetShading(bool bVal);
	bool GetShading();
	void SetMaterial(double amb, double diff, double spec, double shine);
	void GetMaterial(double &amb, double &diff, double &spec, double &shine);
	void SetLowShading(double dVal);
	void SetHiShading(double dVal);
	//shadow
	void SetShadow(bool bVal);
	bool GetShadow();
	void SetShadowParams(double val);
	void GetShadowParams(double &val);
	//sample rate
	void SetSampleRate(double rate);
	double GetSampleRate();

	//colormap mode
	void SetColormapMode(int mode);
	int GetColormapMode();
	void SetColormapDisp(bool disp);
	bool GetColormapDisp();
	void SetColormapValues(double low, double high);
	void GetColormapValues(double &low, double &high);
	void SetColormap(int value);
	int GetColormap();

	//resolution  scaling and spacing
	void GetResolution(int &res_x, int &res_y, int &res_z);
	void SetScalings(double sclx, double scly, double sclz);
	void GetScalings(double &sclx, double &scly, double &sclz);
	void SetSpacings(double spcx, double spcy, double spcz);
	void GetSpacings(double &spcx, double &spcy, double & spcz);
	void GetFileSpacings(double &spcx, double &spcy, double &spcz);
	//read resolutions from file
	void SetSpcFromFile(bool val=true) {m_spc_from_file = val;}
	bool GetSpcFromFile() {return m_spc_from_file;}

	//display controls
	void SetDisp(bool disp);
	bool GetDisp();
	void ToggleDisp();
	//bounding box
	void SetDrawBounds(bool draw);
	bool GetDrawBounds();
	void ToggleDrawBounds();
	//wirefraem mode
	void SetWireframe(bool val);

	//MIP & normal modes
	void SetMode(int mode);
	int GetMode();
	void RestoreMode();
	//stream modes
	void SetStreamMode(int mode) {m_stream_mode = mode;}
	int GetStreamMode() {return m_stream_mode;}

	//invert
	void SetInvert(bool mode);
	bool GetInvert();

	//mask mode
	void SetMaskMode(int mode);
	int GetMaskMode();

	//noise reduction
	void SetNR(bool val);
	bool GetNR();

	//blend mode
	void SetBlendMode(int mode);
	int GetBlendMode();

	//scalar value info
	double GetScalarScale() {return m_scalar_scale;}
	void SetScalarScale(double val) {m_scalar_scale = val; if (m_vr) m_vr->set_scalar_scale(val);}
	double GetGMScale() {return m_gm_scale;}
	void SetGMScale(double val) {m_gm_scale = val; if (m_vr) m_vr->set_gm_scale(val);}
	double GetMaxValue() {return m_max_value;}
	void SetMaxValue(double val) {m_max_value = val;}

	//clip distance
	void SetClipDistance(int distx, int disty, int distz);
	void GetClipDistance(int &distx, int &disty, int &distz);

	//randomize color
	void RandomizeColor();
	//legend
	void SetLegend(bool val);
	bool GetLegend();
	//interpolate
	void SetInterpolate(bool val);
	bool GetInterpolate();

	//number of valid bricks
	void SetBrickNum(int num) {m_brick_num = num;}
	int GetBrickNum() {return m_brick_num;}

	//estimated threshold
	double GetEstThresh() {return m_est_thresh;}

	//fog
	void SetFog(bool use_fog, double fog_intensity, double fog_start, double fog_end);

private:
	//duplication indicator and counter
	bool m_dup;
	int m_dup_counter;

	wxString m_tex_path;
	BBox m_bounds;
	VolumeRenderer *m_vr;
	Texture *m_tex;

	int m_chan;	//channel index of the original file
	int m_time;	//time index of the original file

	//modes (MIP & normal)
	int m_mode;	//0-normal; 1-MIP; 2-white shading; 3-white mip
	//modes for streaming
	int m_stream_mode;	//0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask

	//mask mode
	int m_mask_mode;	//0-normal, 1-render with mask, 2-render with mask excluded,
						//3-random color with label, 4-random color with label+mask
	bool m_use_mask_threshold;// use mask threshold

	//volume properties
	double m_scalar_scale;
	double m_gm_scale;
	double m_max_value;
	//gamma
	double m_gamma3d;
	double m_gm_thresh;
	double m_offset;
	double m_lo_thresh;
	double m_hi_thresh;
	Color m_color;
	HSVColor m_hsv;
	double m_alpha;
	double m_sample_rate;
	double m_mat_amb;
	double m_mat_diff;
	double m_mat_spec;
	double m_mat_shine;
	//noise reduction
	bool m_noise_rd;
	//shading
	bool m_shading;
	//shadow
	bool m_shadow;
	double m_shadow_darkness;

	//resolution, scaling, spacing
	int m_res_x, m_res_y, m_res_z;
	double m_sclx, m_scly, m_sclz;
	double m_spcx, m_spcy, m_spcz;
	bool m_spc_from_file;

	//display control
	bool m_disp;
	bool m_draw_bounds;
	bool m_test_wiref;

	//color map mode
	int m_colormap_mode;	//0-normal; 1-rainbow
	bool m_colormap_disp;	//true/false
	double m_colormap_low_value;
	double m_colormap_hi_value;
	int m_colormap;//index to a colormap

	//save the mode for restoring
	int m_saved_mode;

	//blend mode
	int m_blend_mode;	//0: ignore; 1: layered; 2: depth; 3: composite

	//2d mask texture for segmentation
	GLuint m_2d_mask;
	//2d weight map for segmentation
	GLuint m_2d_weight1;	//after tone mapping
	GLuint m_2d_weight2;	//before tone mapping

	//2d depth map texture for rendering shadows
	GLuint m_2d_dmap;

	//reader
	BaseReader *m_reader;

	//clip distance
	int m_clip_dist_x;
	int m_clip_dist_y;
	int m_clip_dist_z;

	//compression
	bool m_compression;

	//brick skipping
	bool m_skip_brick;

	//shown in legend
	bool m_legend;
	//interpolate
	bool m_interpolate;

	//valid brick number
	int m_brick_num;

	//estimated threshold
	double m_est_thresh;

private:
	//label functions
	void SetOrderedID(unsigned int* val);
	void SetReverseID(unsigned int* val);
	void SetShuffledID(unsigned int* val);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MESH_COLOR_AMB	1
#define MESH_COLOR_DIFF	2
#define MESH_COLOR_SPEC	3
#define MESH_FLOAT_SHN	4
#define MESH_FLOAT_ALPHA	5

class MeshData : public TreeLayer
{
public:
	MeshData();
	virtual ~MeshData();

	wxString GetPath();
	BBox GetBounds();
	GLMmodel* GetMesh();
	void SetDisp(bool disp);
	void ToggleDisp();
	bool GetDisp();
	void SetDrawBounds(bool draw);
	void ToggleDrawBounds();
	bool GetDrawBounds();

	//data management
	int Load(wxString &filename);
	int Load(GLMmodel* mesh);
	void Save(wxString &filename);

	//MR
	MeshRenderer* GetMR();

	//draw
	void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat);
	void Draw(int peel);
	void DrawBounds();
	void DrawInt(unsigned int name);

	//lighting
	void SetLighting(bool bVal);
	bool GetLighting();
	void SetFog(bool bVal, double fog_intensity, double fog_start, double fog_end);
	bool GetFog();
	void SetMaterial(Color& amb, Color& diff, Color& spec, 
		double shine = 30.0, double alpha = 1.0);
	void SetColor(Color &color, int type);
	void SetFloat(double &value, int type);
	void GetMaterial(Color& amb, Color& diff, Color& spec,
		double& shine, double& alpha);
	bool IsTransp();
	//shadow
	void SetShadow(bool bVal);
	bool GetShadow();
	void SetShadowParams(double val);
	void GetShadowParams(double &val);

	void SetTranslation(double x, double y, double z);
	void GetTranslation(double &x, double &y, double &z);
	void SetRotation(double x, double y, double z);
	void GetRotation(double &x, double &y, double &z);
	void SetScaling(double x, double y, double z);
	void GetScaling(double &x, double &y, double &z);

	//randomize color
	void RandomizeColor();

	//shown in legend
	void SetLegend(bool val);
	bool GetLegend();

	//size limiter
	void SetLimit(bool bVal);
	bool GetLimit();
	void SetLimitNumer(int val);
	int GetLimitNumber();

private:
	//wxString m_name;
	wxString m_data_path;
	GLMmodel* m_data;
	MeshRenderer *m_mr;
	BBox m_bounds;
	Point m_center;

	bool m_disp;
	bool m_draw_bounds;

	//lighting
	bool m_light;
	bool m_fog;
	Color m_mat_amb;
	Color m_mat_diff;
	Color m_mat_spec;
	double m_mat_shine;
	double m_mat_alpha;
	//shadow
	bool m_shadow;
	double m_shadow_darkness;
	//size limiter
	bool m_enable_limit;
	int m_limit;

	double m_trans[3];
	double m_rot[3];
	double m_scale[3];

	//legend
	bool m_legend;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AText
{
public:
	AText();
	AText(const string &str, const Point &pos);
	~AText();

	string GetText();
	Point GetPos();
	void SetText(string str);
	void SetPos(Point pos);
	void SetInfo(string str);

	friend class Annotations;

private:
	string m_txt;
	Point m_pos;
	string m_info;
};

class DataManager;
class Annotations : public TreeLayer
{
public:
	Annotations();
	virtual ~Annotations();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	int GetTextNum();
	string GetTextText(int index);
	Point GetTextPos(int index);
	Point GetTextTransformedPos(int index);
	string GetTextInfo(int index);
	void AddText(std::string str, Point pos, std::string info);
	void SetTransform(Transform *tform);
	void SetVolume(VolumeData* vd);
	VolumeData* GetVolume();

	void Clear();

	//display functions
	void SetDisp(bool disp)
	{
		m_disp = disp;
	}
	void ToggleDisp()
	{
		m_disp = !m_disp;
	}
	bool GetDisp()
	{
		return m_disp;
	}

	//memo
	void SetMemo(string &memo);
	string &GetMemo();
	void SetMemoRO(bool ro);
	bool GetMemoRO();

	//save/load
	wxString GetPath();
	int Load(wxString &filename, DataManager* mgr);
	void Save(wxString &filename);

	//info meaning
	wxString GetInfoMeaning();
	void SetInfoMeaning(wxString &str);

	bool InsideClippingPlanes(Point &pos);

private:
	static int m_num;
	vector<AText*> m_alist;
	Transform *m_tform;
	VolumeData* m_vd;

	bool m_disp;

	//memo
	string m_memo;
	bool m_memo_ro;//read only

	//on disk
	wxString m_data_path;

	//atext info meaning
	wxString m_info_meaning;

private:
	AText* GetAText(wxString str);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProfileBin
{
public:
	ProfileBin():
	m_pixels(0), m_accum(0.0) {}
	~ProfileBin() {}
	int m_pixels;
	double m_accum;
};

class Ruler : public TreeLayer
{
public:
	Ruler();
	virtual ~Ruler();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	//data
	int GetNumPoint();
	Point* GetPoint(int index);
	int GetRulerType();
	void SetRulerType(int type);
	bool GetFinished();
	void SetFinished();
	double GetLength();
	double GetLengthObject(double spcx, double spcy, double spcz);
	double GetAngle();

	bool AddPoint(Point &point);
	void SetTransform(Transform *tform);

	void Clear();

	//display functions
	void SetDisp(bool disp)
	{
		m_disp = disp;
	}
	void ToggleDisp()
	{
		m_disp = !m_disp;
	}
	bool GetDisp()
	{
		return m_disp;
	}

	//time-dependent
	void SetTimeDep(bool time_dep)
	{
		m_time_dep = time_dep;
	}
	bool GetTimeDep()
	{
		return m_time_dep;
	}
	void SetTime(int time)
	{
		m_time = time;
	}
	int GetTime()
	{
		return m_time;
	}

	//extra info
	void AddInfoNames(wxString &str)
	{
		m_info_names += str;
	}
	void SetInfoNames(wxString &str)
	{
		m_info_names = str;
	}
	wxString &GetInfoNames()
	{
		return m_info_names;
	}
	void AddInfoValues(wxString &str)
	{
		m_info_values += str;
	}
	void SetInfoValues(wxString &str)
	{
		m_info_values = str;
	}
	wxString &GetInfoValues()
	{
		return m_info_values;
	}
	wxString GetDelInfoValues(wxString del=",");

	//profile
	void SetInfoProfile(wxString &str)
	{
		m_info_profile = str;
	}
	wxString &GetInfoProfile()
	{
		return m_info_profile;
	}
	vector<ProfileBin> *GetProfile()
	{
		return &m_profile;
	}
	void SaveProfile(wxString &filename);

	//color
	void SetColor(Color& color)
	{ m_color = color; m_use_color = true;}
	bool GetUseColor()
	{ return m_use_color; }
	Color &GetColor()
	{ return m_color; }

private:
	static int m_num;
	int m_ruler_type;	//0: 2 point; 1: multi point; 2:locator; 3: probe
	bool m_finished;
	vector<Point> m_ruler;
	bool m_disp;
	Transform *m_tform;
	//a profile
	wxString m_info_profile;
	vector<ProfileBin> m_profile;
	//color
	bool m_use_color;
	Color m_color;

	//time-dependent
	bool m_time_dep;
	int m_time;

	//extra info
	wxString m_info_names;
	wxString m_info_values;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//tags
#define TAG_CELL		1
#define TAG_EDGE		2
#define TAG_VERT		3
#define TAG_CMAP		4
#define TAG_FRAM		5

//label is used in render view
struct Lbl
{
	unsigned int id;
	unsigned int size;
	Point center;

	static bool cmp_id(const Lbl lbl1, const Lbl lbl2)
	{ return lbl1.id < lbl2.id; }
};

//a vertex in the map contains a cell and in/out edges
struct Vertex
{
	unsigned int id;
	unsigned int vsize;
	Point center;

	vector<unsigned int> out_ids;
	vector<unsigned int> in_ids;

	int Read(ifstream &ifs);
	int Write(ofstream &ofs);
};

typedef boost::unordered_map<unsigned int, Vertex> CellMap;
typedef boost::unordered_map<unsigned int, Vertex>::iterator CellMapIter;

//a frame contains a cell map
struct Frame
{
	unsigned int id;
	CellMap cell_map;

	//reading
	int Read(ifstream &ifs);
	int ReadCellMap(ifstream &ifs);
	//writing
	int Write(ofstream &ofs);
	int WriteCellMap(ofstream &ofs);
};

//frame list
typedef boost::unordered_map<unsigned int, Frame> FrameList;
typedef boost::unordered_map<unsigned int, Frame>::iterator FrameIter;
//id list
typedef boost::unordered_map<unsigned int, Lbl> IDMap;
typedef boost::unordered_map<unsigned int, Lbl>::iterator IDMapIter;

class TraceGroup : public TreeLayer
{
public:
	TraceGroup();
	virtual ~TraceGroup();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	wxString GetPath() {return m_data_path;}
	void SetCurTime(int time);
	int GetCurTime();
	void SetPrvTime(int time);
	int GetPrvTime();
	//ghost num
	void SetGhostNum(int num) {m_ghost_num = num;}
	int GetGhostNum() {return m_ghost_num;}
	void SetDrawTail(bool draw) {m_draw_tail = draw;}
	bool GetDrawTail() {return m_draw_tail;}
	void SetDrawLead(bool draw) {m_draw_lead = draw;}
	bool GetDrawLead() {return m_draw_lead;}
	//cells size filter
	void SetCellSize(int size) {m_cell_size = size;}
	int GetSizeSize() {return m_cell_size;}

	//for selective drawing
	void ClearIDMap();
	void AddID(Lbl lbl);
	void SetIDMap(boost::unordered_map<unsigned int, Lbl> &sel_labels);
	IDMap *GetIDMap();
	bool FindID(unsigned int id);
	//in frame list
	bool FindIDInFrame(unsigned int id, int time, Vertex &vertex);

	//modifications
	bool LinkVertices(unsigned int id1, int time1,
					  unsigned int id2, int time2,
					  bool exclusive=false);
	bool UnlinkVertices(unsigned int id1, int time1,
						unsigned int id2, int time2);
	bool AddVertex(int time, unsigned int id,
		unsigned int vsize, Point& center);
	Frame* AddFrame(int time);

	//i/o
	int Load(wxString &filename);
	int Save(wxString &filename);

	//draw
	unsigned int Draw(vector<float> &verts);

	//pattern search
	typedef struct
	{
		int div;
		int conv;
	} Patterns;
	//type: 1-diamond; 2-branching
	bool FindPattern(int type, unsigned int id, int time);

private:
	static int m_num;
	wxString m_data_path;
	//for selective drawing
	int m_cur_time;
	int m_prv_time;
	int m_ghost_num;
	bool m_draw_tail;
	bool m_draw_lead;
	int m_cell_size;

	FrameList m_frame_list;
	IDMap m_id_map;

private:
	//reading
	unsigned char ReadTag(ifstream &ifs);
	//writing
	void WriteTag(ofstream& ofs, unsigned char tag);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataGroup : public TreeLayer
{
public:
	DataGroup();
	virtual ~DataGroup();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	int GetVolumeNum()
	{
		return m_vd_list.size();
	}
	VolumeData* GetVolumeData(int index)
	{
		if (index>=0 && index<(int)m_vd_list.size())
			return m_vd_list[index];
		else return 0;
	}
	void InsertVolumeData(int index, VolumeData* vd)
	{
		if (!m_vd_list.empty())
		{
			if (index>-1 && index<(int)m_vd_list.size())
				m_vd_list.insert(m_vd_list.begin()+(index+1), vd);
			else if (index == -1)
				m_vd_list.insert(m_vd_list.begin()+0, vd);
		}
		else
		{
			m_vd_list.push_back(vd);
		}
	}
	void RemoveVolumeData(int index)
	{
		if (index>=0 && index<(int)m_vd_list.size())
			m_vd_list.erase(m_vd_list.begin()+index);
		ResetSync();
	}

	//display functions
	void SetDisp(bool disp)
	{
		m_disp = disp;
	}
	void ToggleDisp()
	{
		m_disp = !m_disp;
	}
	bool GetDisp()
	{
		return m_disp;
	}

	//group blend mode
	int GetBlendMode();

	//set gamma to all
	void SetGammaAll(Color &gamma);
	//set brightness to all
	void SetBrightnessAll(Color &brightness);
	//set hdr to all
	void SetHdrAll(Color &hdr);
	//set sync to all
	void SetSyncRAll(bool sync_r);
	void SetSyncGAll(bool sync_g);
	void SetSyncBAll(bool sync_b);
	//reset sync
	void ResetSync();

	//volume properties
	void SetEnableAlpha(bool mode);
	void SetAlpha(double dVal);
	void SetSampleRate(double dVal);
	void SetBoundary(double dVal);
	void Set3DGamma(double dVal);
	void SetOffset(double dVal);
	void SetLeftThresh(double dVal);
	void SetRightThresh(double dVal);
	void SetLowShading(double dVal);
	void SetHiShading(double dVal);
	void SetLuminance(double dVal);
	void SetColormapMode(int mode);
	void SetColormapDisp(bool disp);
	void SetColormapValues(double low, double high);
	void SetColormap(int value);
	void SetShading(bool shading);
	void SetShadow(bool shadow);
	void SetShadowParams(double val);
	void SetMode(int mode);
	void SetNR(bool val);
    void SetInterpolate(bool mode);
	void SetInvert(bool mode);

	//blend mode
	void SetBlendMode(int mode);

	//sync prop
	void SetVolumeSyncProp(bool bVal)
	{
		m_sync_volume_prop = bVal;
	}
	bool GetVolumeSyncProp()
	{
		return m_sync_volume_prop;
	}

	//randomize color
	void RandomizeColor();

private:
	static int m_num;
	//wxString m_name;
	vector <VolumeData*> m_vd_list;
	bool m_sync_volume_prop;

	bool m_disp;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MeshGroup : public TreeLayer
{
public:
	MeshGroup();
	virtual ~MeshGroup();

	//counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	//data
	int GetMeshNum()
	{
		return (int)m_md_list.size();
	}
	MeshData* GetMeshData(int index)
	{
		if (index>=0 && index<(int)m_md_list.size())
			return m_md_list[index];
		else return 0;
	}
	void InsertMeshData(int index, MeshData* md)
	{
		if (m_md_list.size() > 0)
		{
			if (index>-1 && index<(int)m_md_list.size())
				m_md_list.insert(m_md_list.begin()+(index+1), md);
			else if (index == -1)
				m_md_list.insert(m_md_list.begin()+0, md);
		}
		else
		{
			m_md_list.push_back(md);
		}
	}
	void RemoveMeshData(int index)
	{
		if (index>=0 && index<(int)m_md_list.size())
			m_md_list.erase(m_md_list.begin()+index);
	}

	//display functions
	void SetDisp(bool disp)
	{
		m_disp = disp;
	}
	void ToggleDisp()
	{
		m_disp = !m_disp;
	}
	bool GetDisp()
	{
		return m_disp;
	}

	//sync prop
	void SetMeshSyncProp(bool bVal)
	{
		m_sync_mesh_prop = bVal;
	}
	bool GetMeshSyncProp()
	{
		return m_sync_mesh_prop;
	}

	//randomize color
	void RandomizeColor();

private:
	static int m_num;
	vector<MeshData*> m_md_list;
	bool m_sync_mesh_prop;
	bool m_disp;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataManager
{
public:
	DataManager();
	~DataManager();

	void ClearAll();

	//set project path
	//when data and project are moved, use project file's path
	//if data's directory doesn't exist
	void SetProjectPath(wxString path);
	wxString SearchProjectPath(wxString &filename);

	//load volume
	int LoadVolumeData(wxString &filename, int type, int ch_num=-1, int t_num=-1);
	//set default
	void SetVolumeDefault(VolumeData* vd);
	//load volume options
	void SetSliceSequence(bool ss) {m_sliceSequence = ss;}
	void SetCompression(bool compression) {m_compression = compression;}
	void SetSkipBrick(bool skip) {m_skip_brick = skip;}
	void SetTimeId(wxString str) {m_timeId = str;}
	void SetLoadMask(bool load_mask) {m_load_mask = load_mask;}
	void AddVolumeData(VolumeData* vd);
	VolumeData* DuplicateVolumeData(VolumeData* vd);
	void RemoveVolumeData(int index);
	int GetVolumeNum();
	VolumeData* GetVolumeData(int index);
	VolumeData* GetVolumeData(wxString &name);
	int GetVolumeIndex(wxString &name);
	VolumeData* GetLastVolumeData()
	{
		int num = m_vd_list.size();
		if (num)
			return m_vd_list[num-1];
		else
			return 0;
	};

	//load mesh
	int LoadMeshData(wxString &filename);
	int LoadMeshData(GLMmodel* mesh);
	int GetMeshNum();
	MeshData* GetMeshData(int index);
	MeshData* GetMeshData(wxString &name);
	int GetMeshIndex(wxString &name);
	MeshData* GetLastMeshData()
	{
		int num = m_md_list.size();
		if (num)
			return m_md_list[num-1];
		else
			return 0;
	};
	void RemoveMeshData(int index);

	//annotations
	int LoadAnnotations(wxString &filename);
	void AddAnnotations(Annotations* ann);
	void RemoveAnnotations(int index);
	int GetAnnotationNum();
	Annotations* GetAnnotations(int index);
	Annotations* GetAnnotations(wxString &name);
	int GetAnnotationIndex(wxString &name);
	Annotations* GetLastAnnotations()
	{
		int num = m_annotation_list.size();
		if (num)
			return m_annotation_list[num-1];
		else
			return 0;
	}

	bool CheckNames(wxString &str);

	//wavelength to color
	void SetWavelengthColor(int c1, int c2, int c3, int c4);
	Color GetWavelengthColor(double wavelength);

	//override voxel size
	void SetOverrideVox(bool val)
	{ m_override_vox = val; }
	bool GetOverrideVox()
	{ return m_override_vox; }

	//flags for pvxml flipping
	void SetPvxmlFlipX(bool flip) {m_pvxml_flip_x = flip;}
	bool GetPvxmlFlipX() {return m_pvxml_flip_x;}
	void SetPvxmlFlipY(bool flip) {m_pvxml_flip_y = flip;}
	bool GetPvxmlFlipY() {return m_pvxml_flip_y;}
public:
	//default values
	//volume
	double m_vol_exb;	//extract_boundary
	double m_vol_gam;	//gamma
	double m_vol_of1;	//offset1
	double m_vol_of2;	//offset2
	double m_vol_lth;	//low_thresholding
	double m_vol_hth;	//high_thresholding
	double m_vol_lsh;	//low_shading
	double m_vol_hsh;	//high_shading
	double m_vol_alf;	//alpha
	double m_vol_spr;	//sample_rate
	double m_vol_xsp;	//x_spacing
	double m_vol_ysp;	//y_spacing
	double m_vol_zsp;	//z_spacing
	double m_vol_lum;	//luminance
	double m_vol_lcm;	//colormap low value
	double m_vol_hcm;	//colormap high value
	bool m_vol_eap;		//enable alpha
	bool m_vol_esh;		//enable_shading
	bool m_vol_interp;	//enable interpolation
	bool m_vol_inv;		//enable inversion
	bool m_vol_mip;		//enable_mip
	bool m_vol_nrd;		//noise reduction
	bool m_vol_shw;		//enable shadow
	double m_vol_swi;	//shadow intensity

	bool m_vol_test_wiref;		//wireframe mode

	//wavelength to color table
	Color m_vol_wav[4];

private:
	vector <VolumeData*> m_vd_list;
	vector <MeshData*> m_md_list;
	vector <BaseReader*> m_reader_list;
	vector <Annotations*> m_annotation_list;

	bool m_use_defaults;

	//slice sequence
	bool m_sliceSequence;
	//compression
	bool m_compression;
	//skip brick
	bool m_skip_brick;
	//time sequence identifier
	wxString m_timeId;
	//load volume mask
	bool m_load_mask;
	//project path
	wxString m_prj_path;
	//override voxel size
	bool m_override_vox;
	//flgs for pvxml flipping
	bool m_pvxml_flip_x;
	bool m_pvxml_flip_y;
};

#endif//_DATAMANAGER_H_
