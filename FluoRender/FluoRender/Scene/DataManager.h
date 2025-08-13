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

#include <BBox.h>
#include <Color.h>
#include <Point.h>
#include <Progress.h>
#include <nrrd.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifndef __glew_h__
typedef unsigned int GLuint;
typedef int GLint;
typedef float GLfloat;
#endif // !__glew_h__
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
#define LOAD_TYPE_PNG		13
#define LOAD_TYPE_JPG		14

class BaseReader;
struct _GLMmodel;
typedef struct _GLMmodel GLMmodel;
namespace flvr
{
	class MeshRenderer;
	class VolumeRenderer;
	class TextureBrick;
	class Texture;
	class CacheQueue;
}
namespace flrd
{
	class Vertex;
	typedef std::shared_ptr<Vertex> Verp;
	typedef std::unordered_map<unsigned int, Verp> VertexList;
	class Ruler;
	class RulerList;
	typedef std::vector<Ruler*>::iterator RulerListIter;
	class EntryParams;
	class TrackMap;
	typedef std::shared_ptr<TrackMap> pTrackMap;
	class CelpList;
}
namespace fluo
{
	class Transform;
	class Quaternion;
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

protected:
	int type;//-1:invalid, 0:root 1: canvas, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	std::wstring m_name;
	unsigned int m_id;

	//layer adjustment
	fluo::Color m_gamma;
	fluo::Color m_brightness;
	fluo::Color m_hdr;
	bool m_sync[3];//for rgb
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

class RenderView;
class Root : public TreeLayer
{
public:
	Root();
	~Root();

	//view functions
	int GetViewNum();
	std::shared_ptr<RenderView> GetView(int i);
	std::shared_ptr<RenderView> GetView(const std::wstring& name);
	int GetView(RenderView* view);
	std::shared_ptr<RenderView> GetLastView();
	void AddView(const std::shared_ptr<RenderView>& view);
	void DeleteView(int i);
	void DeleteView(RenderView* view);
	void DeleteView(const std::wstring& name);

private:
	std::vector<std::shared_ptr<RenderView>> m_views;
};

class VolumeData : public TreeLayer, public std::enable_shared_from_this<VolumeData>
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

	//data related
	//reader
	void SetReader(const std::shared_ptr<BaseReader>& reader) {m_reader = reader;}
	std::shared_ptr<BaseReader> GetReader() {return m_reader.lock();}
	//compression
	void SetCompression(bool compression);
	bool GetCompression();
	//skip brick
	void SetSkipBrick(bool skip);
	bool GetSkipBrick();
	//load
	void ResetVolume();
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
	bool IsValidMask();//check if mask doesn't exist or it's empty
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
	double GetMaskValue(int i, int j, int k, flvr::TextureBrick* b = 0);
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
	void Draw(bool otho = false, bool intactive = false, double zoom = 1.0, double sf121 = 1.0);
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
	void SetBoundaryLow(double val, bool set_this = true);
	double GetBoundaryLow();
	double GetMlBoundaryLow();
	void SetBoundaryHigh(double val, bool set_this = true);
	double GetBoundaryHigh();
	double GetMlBoundaryHigh();
	void SetBoundaryMax(double val);
	double GetBoundaryMax();

	void SetMinMaxEnable(bool);
	bool GetMinMaxEnable();
	void SetLowOffset(double val, bool set_this = true);
	double GetLowOffset();
	double GetMlLowOffset();
	void SetHighOffset(double val, bool set_this = true);
	double GetHighOffset();
	double GetMlHighOffset();

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
	void SetHSVColor(const fluo::HSVColor& hsv);
	fluo::HSVColor GetHSVColor();

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
	void GetColormapRange(double& v1, double& v2);
	double GetColormapMin();
	double GetColormapMax();
	void GetColormapValues(double &low, double &high);
	void GetColormapDispValues(double& low, double& high);
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
	fluo::Color GetColorFromColormap(double value, bool raw = false);
	bool GetColormapData(std::vector<unsigned char>& data);
	//see if need update histogram
	bool GetHistogramDirty() { return m_hist_dirty; }
	void ComputeHistogram(bool set_prog_func);
	bool GetHistogram(std::vector<unsigned char>& data);
	//get auto threshold from histogram
	double GetAutoThreshold();
	bool IsAutoThresholdValid() { return m_auto_threshold >= 0; }

	//shuffle
	void SetShuffle(int val);
	int GetShuffle();
	void IncShuffle();

	//resolution  scaling and spacing
	void GetResolution(int &res_x, int &res_y, int &res_z, int lv = -1);
	void SetScalings(double sclx, double scly, double sclz);
	void GetScalings(double &sclx, double &scly, double &sclz);
	fluo::Vector GetScalings();
	void SetSpacings(double spcx, double spcy, double spcz);
	void GetSpacings(double &spcx, double &spcy, double & spcz, int lv = -1);
	fluo::Vector GetSpacings(int lv = -1);
	void GetFileSpacings(double &spcx, double &spcy, double &spcz);
	//read resolutions from file
	void SetSpcFromFile(bool val=true) {m_spc_from_file = val;}
	bool GetSpcFromFile() {return m_spc_from_file;}

	//brkxml
	void SetBaseSpacings(double spcx, double spcy, double spcz);
	void GetBaseSpacings(double &spcx, double &spcy, double & spcz);
	fluo::Vector GetBaseSpacings();
	void SetSpacingScales(double s_spcx, double s_spcy, double s_spcz);
	void GetSpacingScales(double &s_spcx, double &s_spcy, double &s_spcz);
	fluo::Vector GetSpacingScales();
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
	void SetScalarScale(double val);
	double GetGMScale() {return m_gm_scale;}
	void SetGMScale(double val);
	double GetMinValue() {return m_min_value;}
	double GetMaxValue() {return m_max_value;}
	double GetMinValueScale();
	void SetMinMaxValue(double val1, double val2) { m_min_value = val1; m_max_value = val2; }

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

	void SetMaskCount(unsigned int sum, float wsum);
	void ResetMaskCount() { m_mask_count_dirty = true; }

private:
	std::unique_ptr<flrd::EntryParams> m_ep;
	std::unique_ptr<flvr::VolumeRenderer> m_vr;
	std::shared_ptr<flvr::Texture> m_tex;

	std::wstring m_tex_path;
	fluo::BBox m_bounds;
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
	double m_min_value;
	double m_max_value;

	//transfer function settings
	bool m_gamma_enable;
	double m_gamma;

	bool m_boundary_enable;
	double m_boundary_low;
	double m_boundary_high;
	double m_boundary_max;

	bool m_minmax_enable;
	double m_lo_offset;
	double m_hi_offset;

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
	int m_colormap;//index to a colormap
	int m_colormap_proj;//index to method of mapping
						//0-intensity; 1-z-value; 2-y-value; 3-x-value; 4-t-value;
						//5 - gradient magnitude; 6 - gradient dir; 7 - intensity delta; 8 - speed	
	double m_colormap_low_value;
	double m_colormap_hi_value;
	//the min/max values are needed because it can be mapped to values other than intensity
	double m_colormap_min_value;
	double m_colormap_max_value;

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
	std::weak_ptr<BaseReader> m_reader;

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

	//colormap of histogram
	bool m_hist_dirty;
	std::vector<unsigned int> m_hist;
	double m_auto_threshold = -1;//automatically calculated threshold. //-1 if not calculated

	//mask count
	bool m_mask_count_dirty;
	unsigned int m_mask_sum;
	float m_mask_wsum;

private:
	//label functions
	void SetOrderedID(unsigned int* val);
	void SetReverseID(unsigned int* val);
	void SetShuffledID(unsigned int* val);

	//update colormap range
	void UpdateColormapRange();
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
	void SetTranslation(const fluo::Vector& val);
	fluo::Vector GetTranslation();
	void SetRotation(double x, double y, double z);
	void GetRotation(double &x, double &y, double &z);
	void SetRotation(const fluo::Vector& val);
	fluo::Vector GetRotation();
	void SetScaling(double x, double y, double z);
	void GetScaling(double &x, double &y, double &z);
	void SetScaling(const fluo::Vector& val);
	fluo::Vector GetScaling();

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
	std::unique_ptr<GLMmodel> m_data;
	std::unique_ptr<flvr::MeshRenderer> m_mr;
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
	void SetVolume(const std::shared_ptr<VolumeData>& vd);
	std::shared_ptr<VolumeData> GetVolume();

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
	int Load(const std::wstring &filename);
	void Save(const std::wstring &filename);

	//info meaning
	std::wstring GetInfoMeaning();
	void SetInfoMeaning(const std::wstring &str);

	bool InsideClippingPlanes(fluo::Point &pos);

private:
	static int m_num;
	std::vector<std::shared_ptr<AText>> m_alist;
	fluo::Transform *m_tform;
	std::weak_ptr<VolumeData> m_vd;

	bool m_disp;

	//memo
	std::wstring m_memo;
	bool m_memo_ro;//read only

	//on disk
	std::wstring m_data_path;

	//atext info meaning
	std::wstring m_info_meaning;

private:
	std::shared_ptr<AText> GetAText(const std::wstring& str);
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
	std::unique_ptr<flrd::CelpList> m_cell_list;

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
	std::shared_ptr<VolumeData> GetVolumeData(int index)
	{
		if (index>=0 && index<(int)m_vd_list.size())
			return m_vd_list[index];
		else return nullptr;
	}
	void InsertVolumeData(int index, const std::shared_ptr<VolumeData>& vd)
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
	void ReplaceVolumeData(int index, const std::shared_ptr<VolumeData>& vd)
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
	double GetGamma();
	void SetBoundaryEnable(bool);
	void SetBoundaryLow(double, bool set_this = true);
	double GetBoundaryLow();
	void SetBoundaryHigh(double, bool set_this = true);
	double GetBoundaryHigh();
	void SetBoundaryMax(double val);
	double GetBoundaryMax();
	void SetMinMaxEnable(bool);
	void SetLowOffset(double, bool set_this = true);
	double GetLowOffset();
	void SetHighOffset(double, bool set_this = true);
	double GetHighOffset();
	void SetThreshEnable(bool);
	void SetLeftThresh(double, bool set_this = true);
	double GetLeftThresh();
	void SetRightThresh(double, bool set_this = true);
	double GetRightThresh();
	double GetSoftThreshold();
	void SetLuminanceEnable(bool);
	void SetLuminance(double, bool set_this = true);
	double GetLuminance();
	void SetAlphaEnable(bool);
	void SetAlpha(double, bool set_this = true);
	double GetAlpha();
	void SetShadingEnable(bool);
	void SetLowShading(double);
	void SetHiShading(double);
	double GetLowShading();
	double GetHiShading();
	void SetShadowEnable(bool);
	void SetShadowIntensity(double);
	double GetShadowIntensity();
	void SetSampleRateEnable(bool);
	void SetSampleRate(double, bool set_this = true);
	double GetSampleRate();

	void SetColormapMode(int mode);
	void SetColormapDisp(bool disp);
	void SetColormapValues(double low, double high);
	double GetColormapLow();
	double GetColormapHigh();
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
	std::vector<std::shared_ptr<VolumeData>> m_vd_list;
	bool m_sync_volume_prop;

	bool m_disp;

	//synced values
	double m_gamma;
	double m_boundary_low;
	double m_boundary_high;
	double m_boundary_max;
	double m_lo_offset;
	double m_hi_offset;
	double m_lo_thresh;
	double m_hi_thresh;
	double m_sw;
	double m_luminance;
	double m_alpha;
	double m_mat_amb;
	double m_mat_diff;
	double m_mat_spec;
	double m_mat_shine;
	double m_shadow_intensity;
	double m_sample_rate;
	double m_colormap_low;
	double m_colormap_high;
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
	std::shared_ptr<MeshData> GetMeshData(int index)
	{
		if (index>=0 && index<(int)m_md_list.size())
			return m_md_list[index];
		else return 0;
	}
	void InsertMeshData(int index, const std::shared_ptr<MeshData>& md)
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
	std::vector<std::shared_ptr<MeshData>> m_md_list;
	bool m_sync_mesh_prop;
	bool m_disp;
};

class MainFrame;
class RenderView;
struct CurrentObjects
{
	CurrentObjects() :
		mainframe(0)
	{}

	//0:root, 1:view, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	int GetType()
	{
		if (vol_data.lock())
			return 2;
		if (mesh_data.lock())
			return 3;
		if (ann_data.lock())
			return 4;
		if (vol_group.lock())
			return 5;
		if (mesh_group.lock())
			return 6;
		if (render_view.lock())
			return 1;
		return 0;
	}
	void SetRoot()
	{
		render_view.reset();
		vol_group.reset();
		mesh_group.reset();
		vol_data.reset();
		mesh_data.reset();
		ann_data.reset();
	}
	void SetRenderView(const std::shared_ptr<RenderView>& v);
	void SetVolumeGroup(const std::shared_ptr<DataGroup>& g);
	void SetMeshGroup(const std::shared_ptr<MeshGroup>& g);
	void SetVolumeData(const std::shared_ptr<VolumeData>& vd);
	void SetMeshData(const std::shared_ptr<MeshData>& md);
	void SetAnnotation(const std::shared_ptr<Annotations>& ann);

	void SetSel(const std::wstring& str);

	int GetViewId(RenderView* v = 0);

	flrd::RulerList* GetRulerList();
	flrd::Ruler* GetRuler();
	TrackGroup* GetTrackGroup();

	MainFrame* mainframe;//this is temporary before a global scenegraph is added
	std::weak_ptr<RenderView> render_view;
	std::weak_ptr<DataGroup> vol_group;
	std::weak_ptr<MeshGroup> mesh_group;
	std::weak_ptr<VolumeData> vol_data;
	std::weak_ptr<MeshData> mesh_data;
	std::weak_ptr<Annotations> ann_data;
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

	//root
	Root* GetRoot()
	{
		return m_root.get();
	}

	//load volume
	void LoadVolumes(const std::vector<std::wstring>& files, bool withImageJ);
	void StartupLoad(const std::vector<std::wstring>& files, bool run_mov, bool with_imagej);
	size_t LoadVolumeData(const std::wstring &filename, int type, bool withImageJ, int ch_num=-1, int t_num=-1);
	//set default
	void SetVolumeDefault(const std::shared_ptr<VolumeData>& vd);
	void AddVolumeData(const std::shared_ptr<VolumeData>& vd);
	std::shared_ptr<VolumeData> DuplicateVolumeData(const std::shared_ptr<VolumeData>& vd);
	void RemoveVolumeData(size_t index);
	void RemoveVolumeData(const std::wstring &name);
	size_t GetVolumeNum();
	std::shared_ptr<VolumeData> GetVolumeData(size_t index);
	std::shared_ptr<VolumeData> GetVolumeData(const std::wstring &name);
	size_t GetVolumeIndex(const std::wstring &name);
	std::shared_ptr<VolumeData> GetLastVolumeData();

	//load mesh
	void LoadMeshes(const std::vector<std::wstring>& files);
	bool LoadMeshData(const std::wstring &filename);
	bool LoadMeshData(GLMmodel* mesh);
	void AddMeshData(const std::shared_ptr<MeshData>& md);
	size_t GetMeshNum();
	std::shared_ptr<MeshData> GetMeshData(size_t index);
	std::shared_ptr<MeshData> GetMeshData(const std::wstring &name);
	size_t GetMeshIndex(const std::wstring &name);
	std::shared_ptr<MeshData> GetLastMeshData();
	void RemoveMeshData(size_t index);
	void ClearMeshSelection();

	//annotations
	bool LoadAnnotations(const std::wstring &filename);
	void AddAnnotations(const std::shared_ptr<Annotations>& ann);
	void RemoveAnnotations(size_t index);
	size_t GetAnnotationNum();
	std::shared_ptr<Annotations> GetAnnotations(size_t index);
	std::shared_ptr<Annotations> GetAnnotations(const std::wstring &name);
	size_t GetAnnotationIndex(const std::wstring &name);
	std::shared_ptr<Annotations> GetLastAnnotations();

	bool CheckNames(const std::wstring &str);

	//wavelength to color
	fluo::Color GetWavelengthColor(double wavelength);
	fluo::Color GetColor(int);

	//get vol cache queue
	flvr::CacheQueue* GetCacheQueue(VolumeData* vd);

	//update stream rendering mode
	void UpdateStreamMode(double data_size);//input is a newly loaded data size in mb

private:
	MainFrame* m_frame;
	std::unique_ptr<Root> m_root;// root of the scene graph
	std::vector<std::shared_ptr<VolumeData>> m_vd_list;
	std::vector<std::shared_ptr<MeshData>> m_md_list;
	std::vector<std::shared_ptr<BaseReader>> m_reader_list;
	std::vector<std::shared_ptr<Annotations>> m_annotation_list;

	//4d cache for volume data
	std::unordered_map<VolumeData*, std::shared_ptr<flvr::CacheQueue>> m_vd_cache_queue;
	//project path
	std::wstring m_prj_path;
	std::wstring m_prj_file;

	//for reading files and channels
	size_t m_cur_file;
	size_t m_file_num;
};

#endif//_DATAMANAGER_H_
