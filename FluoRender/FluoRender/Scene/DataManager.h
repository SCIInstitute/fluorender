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
#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <compatibility.h>
#include <BBox.h>
#include <Color.h>
#include <Point.h>
#include <Transform.h>
#include <MeshRenderer.h>
#include <VolumeRenderer.h>
#include <VertexArray.h>
#include <EntryParams.h>
#include <base_reader.h>
#include <oib_reader.h>
#include <oif_reader.h>
#include <nrrd_reader.h>
#include <tif_reader.h>
#include <nrrd_writer.h>
#include <tif_writer.h>
#include <msk_reader.h>
#include <msk_writer.h>
#include <lsm_reader.h>
#include <lbl_reader.h>
#include <pvxml_reader.h>
#include <brkxml_reader.h>
#include <imageJ_reader.h>
#include <czi_reader.h>
#include <nd2_reader.h>
#include <lif_reader.h>
#include <lof_reader.h>
#include <mpg_reader.h>
#include <TrackMap.h>
#include <Ruler.h>
#include <Progress.h>
#include <vector>
#include <string>

#define DATA_VOLUME			1
#define DATA_MESH			2
#define DATA_ANNOTATIONS	3

#define LOAD_TYPE_IMAGEJ	0
#define LOAD_TYPE_NRRD		1
#define LOAD_TYPE_TIFF		2
#define LOAD_TYPE_OIB		3
#define LOAD_TYPE_OIF		4
#define LOAD_TYPE_LSM		5
#define LOAD_TYPE_PVXML		6
#define LOAD_TYPE_BRKXML	7
#define LOAD_TYPE_CZI		8
#define LOAD_TYPE_ND2		9
#define LOAD_TYPE_LIF		10
#define LOAD_TYPE_LOF		11
#define LOAD_TYPE_MPG		12

namespace flrd
{
	class EntryParams;
}
class TreeLayer
{
public:
	TreeLayer();
	~TreeLayer();

	int IsA()
	{
		return type;
	}
	std::wstring GetName()
	{
		return m_name;
	}
	void SetName(const std::wstring& name)
	{
		m_name = name;
	}
	unsigned int Id()
	{
		return m_id;
	}
	void Id(unsigned int id)
	{
		m_id = id;
	}

	//layer adjustment
	//gamma
	const fluo::Color GetGammaColor()
	{return m_gamma;}
	void SetGammaColor(const fluo::Color &gamma)
	{m_gamma = gamma;}
	//brightness
	const fluo::Color GetBrightness()
	{return m_brightness;}
	void SetBrightness(const fluo::Color &brightness)
	{m_brightness = brightness;}
	//hdr settings
	const fluo::Color GetHdr()
	{return m_hdr;}
	void SetHdr(const fluo::Color &hdr)
	{m_hdr = hdr;}
	//sync values
	bool GetSync(int i) { if (i >= 0 && i < 3) return m_sync[i]; else return false; }
	void SetSync(int i, bool val) { if (i >= 0 && i < 3) m_sync[i] = val; }

	//randomize color
	virtual void RandomizeColor() {}

	//associated layer
	TreeLayer* GetAssociated()
	{return m_associated;}
	void SetAssociated(TreeLayer* layer)
	{m_associated = layer;}

protected:
	int type;//-1:invalid, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	std::wstring m_name;
	unsigned int m_id;

	//layer adjustment
	fluo::Color m_gamma;
	fluo::Color m_brightness;
	fluo::Color m_hdr;
	bool m_sync[3];//for rgb

	//associated layer
	TreeLayer* m_associated;
};

struct VD_Landmark
{
	std::wstring name = L"";
	double x = 0;
	double y = 0;
	double z = 0;
	double spcx = 0;
	double spcy = 0;
	double spcz = 0;
};

class VolumeData : public TreeLayer
{
public:
	VolumeData();
	VolumeData(VolumeData &copy);
	virtual ~VolumeData();

	//set viewport
	void SetViewport(GLint vp[4]);

	//set clear color
	void SetClearColor(GLfloat clear_color[4]);

	//set current framebuffer
	void SetCurFramebuffer(GLuint cur_framebuffer);

	//duplication
	bool GetDup();
	//increase duplicate counter
	void IncDupCounter();
	//get duplicated from
	VolumeData* GetDupData();

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
	int Load(Nrrd* data, const std::wstring &name, const std::wstring &path);
	int Replace(Nrrd* data, bool del_tex);
	int Replace(VolumeData* data);
	Nrrd* GetVolume(bool ret);
	//empty data
	void AddEmptyData(int bits,
		int nx, int ny, int nz,
		double spcx, double spcy, double spcz,
		int brick_size = 0);
	//load mask
	void LoadMask(Nrrd* mask);
	Nrrd* GetMask(bool ret);
	//empty mask
	//mode: 0-zeros; 1-255; 2-leave as is
	void AddEmptyMask(int mode, bool change=true);
	void AddMask(Nrrd* mask, int op);//op: 0-replace; 1-union; 2-exclude; 3-intersect
	void AddMask16(Nrrd* mask, int op, double scale);//16-bit data
	//load label
	void LoadLabel(Nrrd* label);
	Nrrd* GetLabel(bool ret);
	//empty label
	//mode: 0-zeros;1-ordered; 2-shuffled
	//change: whether changes label when it already exists
	void AddEmptyLabel(int mode=0, bool change=true);
	bool SearchLabel(unsigned int label);
	void SetMaskClear(bool bval = true) { m_mask_clear = bval; }
	bool GetMaskClear() { return m_mask_clear; }

	//save
	double GetOriginalValue(int i, int j, int k, flvr::TextureBrick* b = 0);
	double GetTransferedValue(int i, int j, int k, flvr::TextureBrick* b=0);
	void SetResize(int resize, int nx, int ny, int nz);
	void GetResize(bool &resize, int &nx, int &ny, int &nz);
	//mask: 0-save none; 1-save mask; 2-save label; 3-save mask and label...
	void Save(const std::wstring &filename, int mode,
		int mask, bool neg_mask,
		bool crop, int filter,
		bool bake, bool compress,
		const fluo::Point &c,
		const fluo::Quaternion &q,
		const fluo::Point &t,
		bool fix_size);
	void SaveMask(bool use_reader, int t, int c);
	void SaveLabel(bool use_reader, int t, int c);

	//volumerenderer
	flvr::VolumeRenderer *GetVR();
	//texture
	flvr::Texture* GetTexture();
	void SetTexture();

	//bounding box
	fluo::BBox GetBounds();
	fluo::BBox GetClippedBounds();
	//path
	void SetPath(const std::wstring& path);
	std::wstring GetPath();
	//multi-channel
	void SetCurChannel(int chan);
	int GetCurChannel();
	//time sequence
	void SetCurTime(int time);
	int GetCurTime();

	//draw volume
	void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat, glm::mat4 &tex_mat);
	void Draw(bool otho = false, bool adaptive = false, bool intactive = false, double zoom = 1.0, double sf121 = 1.0);
	void DrawBounds();
	//draw mask (create the mask)
	//type: 0-initial; 1-diffusion-based growing
	//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear
	//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
	//order (for updating instreaming mode): 0-no op; 1-normal order; 2-reversed order
	void DrawMask(int type, int paint_mode, int hr_mode,
		double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
		double w2d, double bins, int order, bool ortho=false, bool estimate=false);

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
	void SetGammaEnable(bool);
	bool GetGammaEnable();
	void SetGamma(double val, bool set_this = true);
	double GetGamma();
	double GetMlGamma();

	void SetBoundaryEnable(bool);
	bool GetBoundaryEnable();
	void SetBoundary(double val, bool set_this = true);
	double GetBoundary();
	double GetMlBoundary();

	void SetSaturationEnable(bool);
	bool GetSaturationEnable();
	void SetSaturation(double val, bool set_this = true);
	double GetSaturation();
	double GetMlSaturation();

	void SetThreshEnable(bool);
	bool GetThreshEnable();
	void SetLeftThresh(double val, bool set_this = true);
	double GetLeftThresh();
	double GetMlLeftThresh();
	void SetRightThresh(double val, bool set_this = true);
	double GetRightThresh();
	double GetMlRightThresh();
	//soft threshold
	void SetSoftThreshold(double val);
	double GetSoftThreshold();

	void SetLuminanceEnable(bool);
	bool GetLuminanceEnable();
	fluo::Color SetLuminance(double val, bool set_this = true);
	double GetLuminance();
	double GetMlLuminance();

	void SetAlphaEnable(bool mode);
	bool GetAlphaEnable();
	void SetAlpha(double val, bool set_this = true);
	double GetAlpha();
	double GetMlAlpha();

	//shading
	void SetShadingEnable(bool bVal);
	bool GetShadingEnable();
	void SetMaterial(double amb, double diff, double spec, double shine);
	void GetMaterial(double& amb, double& diff, double& spec, double& shine);
	void SetLowShading(double val);
	void SetHiShading(double val);
	double GetLowShading();
	double GetHiShading();
	double GetMlLowShading();
	double GetMlHiShading();

	//shadow
	void SetShadowEnable(bool bVal);
	bool GetShadowEnable();
	void SetShadowIntensity(double val);
	double GetShadowIntensity();
	double GetMlShadowIntensity();

	//sample rate
	void SetSampleRateEnable(bool bval);
	bool GetSampleRateEnable();
	void SetSampleRate(double val, bool set_this = true);
	double GetSampleRate();
	double GetMlSampleRate();

	//colors
	void SetColor(const fluo::Color &color, bool update_hsv=true);
	fluo::Color GetColor();
	void SetWlColor(bool bval = true);
	bool GetWlColor();
	void SetMaskColor(const fluo::Color &color, bool set=true);
	fluo::Color GetMaskColor();
	bool GetMaskColorSet();
	void ResetMaskColorSet();
	void SetHSV(double hue = -1, double sat = -1, double val = -1);
	void GetHSV(double &hue, double &sat, double &val);

	//mask threshold
	void SetMaskThreshold(double thresh);
	void SetUseMaskThreshold(bool mode);

	//colormap mode
	void SetColormapMode(int mode);
	int GetColormapMode();
	void SetColormapDisp(bool disp);
	bool GetColormapDisp();
	void SetColormapValues(double low, double high);
	void SetColormapLow(double val);
	void SetColormapHigh(double val);
	void GetColormapValues(double &low, double &high);
	double GetColormapLow();
	double GetMlColormapLow();
	double GetColormapHigh();
	double GetMlColormapHigh();
	void SetColormapInv(double val);
	double GetColormapInv();
	void SetColormap(int value);
	void SetColormapProj(int value);
	int GetColormap();
	int GetColormapProj();
	fluo::Color GetColorFromColormap(double value);

	//shuffle
	void SetShuffle(int val);
	int GetShuffle();
	void IncShuffle();

	//resolution  scaling and spacing
	void GetResolution(int &res_x, int &res_y, int &res_z, int lv = -1);
	void SetScalings(double sclx, double scly, double sclz);
	void GetScalings(double &sclx, double &scly, double &sclz);
	void SetSpacings(double spcx, double spcy, double spcz);
	void GetSpacings(double &spcx, double &spcy, double & spcz, int lv = -1);
	void GetFileSpacings(double &spcx, double &spcy, double &spcz);
	//read resolutions from file
	void SetSpcFromFile(bool val=true) {m_spc_from_file = val;}
	bool GetSpcFromFile() {return m_spc_from_file;}

	//brkxml
	void SetBaseSpacings(double spcx, double spcy, double spcz);
	void GetBaseSpacings(double &spcx, double &spcy, double & spcz);
	void SetSpacingScales(double s_spcx, double s_spcy, double s_spcz);
	void GetSpacingScales(double &s_spcx, double &s_spcy, double &s_spcz);
	void SetLevel(int lv);
	int GetLevel();
	int GetLevelNum();

	//bits
	int GetBits();

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
	//transparency
	void SetAlphaPower(double val);
	double GetAlphaPower();
	//stream modes
	void SetStreamMode(int mode) {m_stream_mode = mode;}
	int GetStreamMode() {return m_stream_mode;}

	//invert
	void SetInvert(bool mode);
	bool GetInvert();

	//mask mode
	void SetLabelMode(int mode) { m_label_mode = mode; }
	int GetLabelMode() { return m_label_mode; }
	void SetMaskMode(int mode);
	int GetMaskMode();

	//noise reduction
	void SetNR(bool val);
	bool GetNR();

	//blend mode
	void SetBlendMode(int mode);
	int GetBlendMode();

	//transparent
	void SetTransparent(bool val);
	bool GetTransparent();

	//scalar value info
	double GetMaxScale()
	{
		int bits = GetBits();
		switch (bits)
		{
		case 8:
		default:
			return 255;
		case 16:
			return 65535;
		}
	}
	double GetScalarScale() {return m_scalar_scale;}
	void SetScalarScale(double val) {m_scalar_scale = val; if (m_vr) m_vr->set_scalar_scale(val);}
	double GetGMScale() {return m_gm_scale;}
	void SetGMScale(double val) {m_gm_scale = val; if (m_vr) m_vr->set_gm_scale(val);}
	double GetMaxValue() {return m_max_value;}
	void SetMaxValue(double val) {m_max_value = val;}

	//clip size
	void GetClipValues(int &ox, int &oy, int &oz,
		int &nx, int &ny, int &nz);
	void SetClipValue(int i, int val);
	void SetClipValues(int i, int val1, int val2);
	void SetClipValues(const int val[6]);
	void ResetClipValues();
	void ResetClipValuesX();
	void ResetClipValuesY();
	void ResetClipValuesZ();

	//clip distance
	void SetClipDistX(int val) { m_clip_dist[0] = std::min(m_res_x, std::max(0, val)); }
	void SetClipDistY(int val) { m_clip_dist[1] = std::min(m_res_y, std::max(0, val)); }
	void SetClipDistZ(int val) { m_clip_dist[2] = std::min(m_res_z, std::max(0, val)); }
	int GetClipDistX() { return m_clip_dist[0]; }
	int GetClipDistY() { return m_clip_dist[1]; }
	int GetClipDistZ() { return m_clip_dist[2]; }

	//randomize color
	void RandomizeColor();
	//legend
	void SetLegend(bool val);
	bool GetLegend();
	//interpolate
	void SetInterpolate(bool val);
	bool GetInterpolate();

	//number of all bricks
	int GetAllBrickNum();
	//number of valid bricks
	void SetBrickNum(int num) {m_brick_num = num;}
	int GetBrickNum() {return m_brick_num;}

	//estimated threshold
	double GetEstThresh() {return m_est_thresh;}

	//fog
	void SetFog(bool use_fog, double fog_intensity, double fog_start, double fog_end);

	bool isBrxml();

	//save label
	void PushLabel(bool ret);
	void PopLabel();
	void LoadLabel2();

	//backgeound intensity
	bool GetBackgroundValid()
	{
		return m_bg_valid;
	}
	void SetBackgroundInt(double dval)
	{
		m_bg_int = dval;
		m_bg_valid = true;
	}
	double GetBackgroundInt()
	{
		if (m_bg_valid)
			return m_bg_int;
		else
			return 0;
	}

	//machine learning for comp gen applied
	bool GetMlCompGenApplied() { return m_ml_comp_gen_applied; }
	void SetMlCompGenApplied(bool bval) { m_ml_comp_gen_applied = bval; }

	//apply volume properties form machine learning
	void GetMlParams();
	void ApplyMlVolProp();

private:
	//duplication indicator and counter
	bool m_dup;
	int m_dup_counter;
	VolumeData* m_dup_data;//duplicated from

	flrd::EntryParams m_ep;

	std::wstring m_tex_path;
	fluo::BBox m_bounds;
	flvr::VolumeRenderer *m_vr;
	flvr::Texture *m_tex;
	//save label
	void* m_label_save;

	int m_chan;	//channel index of the original file
	int m_time;	//time index of the original file

	//machine learning applied
	bool m_ml_comp_gen_applied;

	//modes (MIP & normal)
	int m_mode;	//0-normal; 1-MIP; 2-white shading; 3-white mip
	//modes for streaming
	int m_stream_mode;	//0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask

	//mask mode
	int m_label_mode;	//0-not used, 1-show label
	int m_mask_mode;	//0-normal, 1-render with mask, 2-render with mask excluded,
						//3-random color with label, 4-random color with label+mask
	bool m_use_mask_threshold;// use mask threshold

	//volume properties
	double m_scalar_scale;
	double m_gm_scale;
	double m_max_value;

	//transfer function settings
	bool m_gamma_enable;
	double m_gamma;

	bool m_boundary_enable;
	double m_boundary;

	bool m_saturation_enable;
	double m_saturation;

	bool m_thresh_enable;
	double m_lo_thresh;
	double m_hi_thresh;
	//soft threshold
	double m_sw;

	bool m_luminance_enable;
	double m_luminance;

	bool m_alpha_enable;
	double m_alpha;

	//shading
	bool m_shading_enable;
	double m_mat_amb;
	double m_mat_diff;
	double m_mat_spec;
	double m_mat_shine;

	//shadow
	bool m_shadow_enable;
	double m_shadow_intensity;

	bool m_sample_rate_enable;
	double m_sample_rate;

	fluo::Color m_color;
	bool m_wl_color;//if color has been set by wavelength
	fluo::HSVColor m_hsv;
	//noise reduction
	bool m_noise_rd;

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
	double m_colormap_inv;
	int m_colormap_mode;	//0-normal; 1-rainbow
	bool m_colormap_disp;	//true/false
	double m_colormap_low_value;
	double m_colormap_hi_value;
	int m_colormap;//index to a colormap
	int m_colormap_proj;//index to a way of projection

	//transparent
	bool m_transparent;

	//save the mode for restoring
	int m_saved_mode;

	//blend mode
	int m_blend_mode;	//0: ignore; 1: layered; 2: depth; 3: composite

	//inverted
	bool m_invert;

	//2d mask texture for segmentation
	GLuint m_2d_mask;
	//2d weight map for segmentation
	GLuint m_2d_weight1;	//after tone mapping
	GLuint m_2d_weight2;	//before tone mapping

	//2d depth map texture for rendering shadows
	GLuint m_2d_dmap;

	//reader
	BaseReader *m_reader;

	//compression
	bool m_compression;
	//resize
	bool m_resize;
	int m_rnx, m_rny, m_rnz;

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

	std::vector<VD_Landmark> m_landmarks;
	std::wstring m_metadata_id;

	//mask cleared
	bool m_mask_clear;

	//background intensity
	bool m_bg_valid;
	double m_bg_int;

	//clip distance
	int m_clip_dist[3];

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

	//set viewport
	void SetViewport(GLint vp[4]);

	std::wstring GetPath();
	fluo::BBox GetBounds();
	GLMmodel* GetMesh();
	void SetDisp(bool disp);
	void ToggleDisp();
	bool GetDisp();
	void SetDrawBounds(bool draw);
	void ToggleDrawBounds();
	bool GetDrawBounds();

	//data management
	int Load(const std::wstring &filename);
	int Load(GLMmodel* mesh);
	void Save(const std::wstring &filename);

	//MR
	flvr::MeshRenderer* GetMR();

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
	void SetMaterial(fluo::Color& amb, fluo::Color& diff, fluo::Color& spec,
		double shine = 30.0, double alpha = 1.0);
	void SetColor(fluo::Color &color, int type);
	fluo::Color GetColor();
	void SetFloat(double &value, int type);
	void GetMaterial(fluo::Color& amb, fluo::Color& diff, fluo::Color& spec,
		double& shine, double& alpha);
	bool IsTransp();
	//shadow
	void SetShadowEnable(bool bVal);
	bool GetShadowEnable();
	void SetShadowIntensity(double val);
	double GetShadowIntensity();

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
	std::wstring m_data_path;
	GLMmodel* m_data;
	flvr::MeshRenderer *m_mr;
	fluo::BBox m_bounds;
	fluo::Point m_center;

	bool m_disp;
	bool m_draw_bounds;

	//lighting
	bool m_light;
	bool m_fog;
	fluo::Color m_mat_amb;
	fluo::Color m_mat_diff;
	fluo::Color m_mat_spec;
	double m_mat_shine;
	double m_mat_alpha;
	//shadow
	bool m_shadow_enable;
	double m_shadow_intensity;
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
class Annotations;
class AText
{
public:
	AText();
	AText(const std::wstring &str, const fluo::Point &pos);
	~AText();

	std::wstring GetText();
	fluo::Point GetPos();
	void SetText(const std::wstring& str);
	void SetPos(fluo::Point pos);
	void SetInfo(const std::wstring& str);

	friend class Annotations;

private:
	std::wstring m_txt;
	fluo::Point m_pos;
	std::wstring m_info;
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
	std::wstring GetTextText(int index);
	fluo::Point GetTextPos(int index);
	fluo::Point GetTextTransformedPos(int index);
	std::wstring GetTextInfo(int index);
	void AddText(const std::wstring& str, fluo::Point pos, const std::wstring& info);
	void SetTransform(fluo::Transform *tform);
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
	void SetMemo(const std::wstring &memo);
	std::wstring GetMemo();
	void SetMemoRO(bool ro);
	bool GetMemoRO();

	//save/load
	std::wstring GetPath();
	int Load(const std::wstring &filename, DataManager* mgr);
	void Save(const std::wstring &filename);

	//info meaning
	std::wstring GetInfoMeaning();
	void SetInfoMeaning(const std::wstring &str);

	bool InsideClippingPlanes(fluo::Point &pos);

private:
	static int m_num;
	std::vector<AText*> m_alist;
	fluo::Transform *m_tform;
	VolumeData* m_vd;

	bool m_disp;

	//memo
	std::wstring m_memo;
	bool m_memo_ro;//read only

	//on disk
	std::wstring m_data_path;

	//atext info meaning
	std::wstring m_info_meaning;

private:
	AText* GetAText(const std::wstring& str);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackGroup : public TreeLayer
{
public:
	TrackGroup();
	virtual ~TrackGroup();

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

	flrd::pTrackMap GetTrackMap()
	{
		return m_track_map;
	}

	std::wstring GetPath() {return m_data_path;}
	void SetCurTime(int time);
	int GetCurTime();
	void SetPrvTime(int time);
	int GetPrvTime();
	//ghost num
	void SetGhostNum(int num);
	int GetGhostNum() {return m_ghost_num;}
	void SetDrawTail(bool draw);
	bool GetDrawTail() {return m_draw_tail;}
	void SetDrawLead(bool draw);
	bool GetDrawLead() {return m_draw_lead;}
	//cells size filter
	void SetCellSize(int size) {m_cell_size = size;}
	int GetSizeSize() {return m_cell_size;}
	//uncertainty filter
	void SetUncertainLow(int value) { m_uncertain_low = value; }
	int GetUncertainLow() { return m_uncertain_low; }

	//get information
	void GetLinkLists(size_t frame,
		flrd::VertexList &in_orphan_list,
		flrd::VertexList &out_orphan_list,
		flrd::VertexList &in_multi_list,
		flrd::VertexList &out_multi_list);

	//for selective drawing
	void ClearCellList();
	void UpdateCellList(flrd::CelpList &cur_sel_list);
	flrd::CelpList &GetCellList();
	bool FindCell(unsigned int id);

	//rulers
	bool GetMappedRulers(flrd::RulerList &rulers);

	//i/o
	void Clear();
	bool Load(const std::wstring &filename);
	bool Save(const std::wstring &filename);

	//draw
	unsigned int Draw(std::vector<float> &verts, int shuffle);

	//pattern search
/*	typedef struct
	{
		int div;
		int conv;
	} Patterns;
	//type: 1-diamond; 2-branching
	bool FindPattern(int type, unsigned int id, int time);*/

private:
	static int m_num;
	std::wstring m_data_path;
	//for selective drawing
	int m_cur_time;
	int m_prv_time;
	int m_ghost_num;
	bool m_draw_tail;
	bool m_draw_lead;
	int m_cell_size;
	int m_uncertain_low;

	flrd::pTrackMap m_track_map;
	flrd::CelpList m_cell_list;

	//edges (in a vector of drawable)
	unsigned int GetMappedEdges(
		flrd::CelpList &sel_list1, flrd::CelpList &sel_list2,
		std::vector<float> &verts,
		size_t frame1, size_t frame2,
		int shuffle=0);
	//rulers
	bool GetMappedRulers(
		flrd::CelpList &sel_list1, flrd::CelpList &sel_list2,
		flrd::RulerList &rulers,
		size_t frame1, size_t frame2);
	flrd::RulerListIter FindRulerFromList(unsigned int id, flrd::RulerList &list);
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
		return static_cast<int>(m_vd_list.size());
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
	void ReplaceVolumeData(int index, VolumeData *vd)
	{
		if (index >= 0 && index<(int)m_vd_list.size())
			m_vd_list[index] = vd;
		ResetSync();
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
	void SetGammaAll(const fluo::Color &gamma);
	//set brightness to all
	void SetBrightnessAll(const fluo::Color &brightness);
	//set hdr to all
	void SetHdrAll(const fluo::Color &hdr);
	//set sync to all
	void SetSyncAll(int i, bool val);
	//reset sync
	void ResetSync();

	//volume properties
	void SetGammaEnable(bool);
	void SetGamma(double val, bool set_this = true);
	void SetBoundaryEnable(bool);
	void SetBoundary(double, bool set_this = true);
	void SetSaturationEnable(bool);
	void SetSaturation(double, bool set_this = true);
	void SetThreshEnable(bool);
	void SetLeftThresh(double, bool set_this = true);
	void SetRightThresh(double, bool set_this = true);
	void SetLuminanceEnable(bool);
	void SetLuminance(double, bool set_this = true);
	void SetAlphaEnable(bool);
	void SetAlpha(double, bool set_this = true);
	void SetShadingEnable(bool);
	void SetLowShading(double);
	void SetHiShading(double);
	void SetShadowEnable(bool);
	void SetShadowIntensity(double);
	void SetSampleRateEnable(bool);
	void SetSampleRate(double, bool set_this = true);

	void SetColormapMode(int mode);
	void SetColormapDisp(bool disp);
	void SetColormapValues(double low, double high);
	void SetColormapInv(double val);
	void SetColormap(int value);
	void SetColormapProj(int value);
	void SetMode(int mode);
	void SetAlphaPower(double val);
	void SetLabelMode(int mode);
	void SetNR(bool val);
	void SetInterpolate(bool mode);
	void SetInvert(bool mode);
	void SetTransparent(bool val);

	//use ml
	void ApplyMlVolProp();

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

	void AddMask(Nrrd* mask, int op);//op: 0-replace; 1-union; 2-exclude; 3-intersect

private:
	static int m_num;
	std::vector <VolumeData*> m_vd_list;
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
		if (index >= 0 && index < (int)m_md_list.size())
			m_md_list.erase(m_md_list.begin() + index);
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
	std::vector<MeshData*> m_md_list;
	bool m_sync_mesh_prop;
	bool m_disp;
};

class MainFrame;
class RenderCanvas;
struct CurrentObjects
{
	CurrentObjects() :
		mainframe(0),
		canvas(0),
		vol_group(0),
		mesh_group(0),
		vol_data(0),
		mesh_data(0),
		ann_data(0)
	{}

	//0:root, 1:view, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	int GetType()
	{
		if (vol_data)
			return 2;
		if (mesh_data)
			return 3;
		if (ann_data)
			return 4;
		if (vol_group)
			return 5;
		if (mesh_group)
			return 6;
		if (canvas)
			return 1;
		return 0;
	}
	void SetRoot()
	{
		canvas = 0;
		vol_group = 0;
		mesh_group = 0;
		vol_data = 0;
		mesh_data = 0;
		ann_data = 0;
	}
	void SetCanvas(RenderCanvas* cnvs);
	void SetVolumeGroup(DataGroup* g);
	void SetMeshGroup(MeshGroup* g);
	void SetVolumeData(VolumeData* vd);
	void SetMeshData(MeshData* md);
	void SetAnnotation(Annotations* ann);
	void SetSel(const std::wstring& str);

	flrd::RulerList* GetRulerList();
	flrd::Ruler* GetRuler();
	TrackGroup* GetTrackGroup();

	MainFrame* mainframe;//this is temporary before a global scenegraph is added
	RenderCanvas* canvas;
	DataGroup* vol_group;
	MeshGroup* mesh_group;
	VolumeData* vol_data;
	MeshData* mesh_data;
	Annotations* ann_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataManager : public Progress
{
public:
	DataManager();
	~DataManager();

	void SetFrame(MainFrame* frame);

	void ClearAll();

	//set project path
	//when data and project are moved, use project file's path
	//if data's directory doesn't exist
	void SetProjectPath(const std::wstring& path);
	std::wstring SearchProjectPath(const std::wstring &filename);
	std::wstring GetProjectFile();

	//load volume
	void LoadVolumes(const std::vector<std::wstring>& files, bool withImageJ);
	void StartupLoad(const std::vector<std::wstring>& files, bool run_mov, bool with_imagej);
	size_t LoadVolumeData(const std::wstring &filename, int type, bool withImageJ, int ch_num=-1, int t_num=-1);
	//set default
	void SetVolumeDefault(VolumeData* vd);
	void AddVolumeData(VolumeData* vd);
	VolumeData* DuplicateVolumeData(VolumeData* vd);
	void RemoveVolumeData(size_t index);
	void RemoveVolumeData(const std::wstring &name);
	size_t GetVolumeNum();
	VolumeData* GetVolumeData(size_t index);
	VolumeData* GetVolumeData(const std::wstring &name);
	size_t GetVolumeIndex(const std::wstring &name);
	VolumeData* GetLastVolumeData()
	{
		size_t num = m_vd_list.size();
		if (num)
			return m_vd_list[num-1];
		else
			return 0;
	};

	//load mesh
	void LoadMeshes(const std::vector<std::wstring>& files);
	bool LoadMeshData(const std::wstring &filename);
	bool LoadMeshData(GLMmodel* mesh);
	size_t GetMeshNum();
	MeshData* GetMeshData(size_t index);
	MeshData* GetMeshData(const std::wstring &name);
	size_t GetMeshIndex(const std::wstring &name);
	MeshData* GetLastMeshData()
	{
		size_t num = m_md_list.size();
		if (num)
			return m_md_list[num-1];
		else
			return 0;
	};
	void RemoveMeshData(size_t index);
	void ClearMeshSelection();

	//annotations
	bool LoadAnnotations(const std::wstring &filename);
	void AddAnnotations(Annotations* ann);
	void RemoveAnnotations(size_t index);
	size_t GetAnnotationNum();
	Annotations* GetAnnotations(size_t index);
	Annotations* GetAnnotations(const std::wstring &name);
	size_t GetAnnotationIndex(const std::wstring &name);
	Annotations* GetLastAnnotations()
	{
		size_t num = m_annotation_list.size();
		if (num)
			return m_annotation_list[num-1];
		else
			return 0;
	}

	bool CheckNames(const std::wstring &str);

	//wavelength to color
	fluo::Color GetWavelengthColor(double wavelength);
	fluo::Color GetColor(int);

private:
	MainFrame* m_frame;
	std::vector <VolumeData*> m_vd_list;
	std::vector <MeshData*> m_md_list;
	std::vector <BaseReader*> m_reader_list;
	std::vector <Annotations*> m_annotation_list;

	//project path
	std::wstring m_prj_path;
	std::wstring m_prj_file;

	//for reading files and channels
	size_t m_cur_file;
	size_t m_file_num;
};

#endif//_DATAMANAGER_H_
