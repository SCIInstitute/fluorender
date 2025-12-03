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
#ifndef _VOLUME_DATA_H_
#define _VOLUME_DATA_H_

#include <TreeLayer.h>
#include <Point.h>
#include <Quaternion.h>
#include <BBox.h>
#include <Vector4i.h>
#include <Vector4f.h>
#include <nrrd.h>
#include <glm/glm.hpp>
#include <array>

namespace flvr
{
	class VolumeRenderer;
	class TextureBrick;
	class Texture;
	enum class RenderMode : int;
	enum class ColorMode : int;
	enum class ColormapProj : int;
}
namespace flrd
{
	class EntryParams;
}
namespace fluo
{
	enum class ClipPlane : int;
}
class BaseVolReader;
enum class ChannelMixMode : int;

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

class VolumeData : public TreeLayer, public std::enable_shared_from_this<VolumeData>
{
public:
	VolumeData();
	VolumeData(VolumeData &copy);
	virtual ~VolumeData();

	//set viewport
	void SetViewport(const fluo::Vector4i& vp);

	//set clear color
	void SetClearColor(const fluo::Vector4f& cc);

	//data related
	//reader
	void SetReader(const std::shared_ptr<BaseVolReader>& reader) {m_reader = reader;}
	std::shared_ptr<BaseVolReader> GetReader() {return m_reader.lock();}
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
		const fluo::Vector& res,
		const fluo::Vector& spc,
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
	double GetOriginalValue(const fluo::Point& p, flvr::TextureBrick* b = 0);
	double GetTransferedValue(const fluo::Point& p, flvr::TextureBrick* b=0);
	double GetMaskValue(const fluo::Point& p, flvr::TextureBrick* b = 0);
	void SetResample(bool resample) { m_resample = resample; }
	void SetResampledSize(const fluo::Vector& size) { m_resampled_size = size; }
	bool GetResample() { return m_resample; }
	fluo::Vector GetResampledSize() { return m_resampled_size; }
	//mask: 0-save none; 1-save mask; 2-save label; 3-save mask and label...
	void Save(const std::wstring &filename, int mode,
		int mask, bool neg_mask,
		bool crop, int filter,
		bool bake, bool compress,
		const fluo::Point &c,
		const fluo::Quaternion &q,
		const fluo::Vector &t,
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
	void Set2dMask(unsigned int mask);
	//set 2d weight map for segmentation
	void Set2DWeight(unsigned int weight1, unsigned int weight2);
	//set 2d depth map for rendering shadows
	void Set2dDmap(unsigned int dmap);

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
	void SetLuminance(double val, bool set_this = true);
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
	void SetShadingStrength(double val);
	double GetShadingStrength();
	double GetMlShadingStrength();
	void SetShadingShine(double val);
	double GetShadingShine();
	double GetMlShadingShine();

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
	void SetColor(const fluo::Color &color, bool set_this = true);
	virtual fluo::Color GetColor() override;
	void SetWlColor(bool bval = true);
	bool GetWlColor();
	void SetMaskColor(const fluo::Color &color, bool set=true);
	fluo::Color GetMaskColor();
	bool GetMaskColorSet();
	void ResetMaskColorSet();

	//mask threshold
	void SetMaskThreshold(double thresh);
	void SetUseMaskThreshold(bool mode);

	//colormap mode
	void SetColorMode(flvr::ColorMode);
	flvr::ColorMode GetColorMode();
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
	int GetColormap();
	void SetColormapProj(flvr::ColormapProj);
	flvr::ColormapProj GetColormapProj();
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
	fluo::Vector GetResolution(int lv = -1);
	void SetScaling(const fluo::Vector& scaling);
	fluo::Vector GetScaling();
	void SetSpacing(const fluo::Vector& spacing);
	fluo::Vector GetSpacing(int lv = -1);
	//read resolutions from file
	void SetSpcFromFile(bool val=true) {m_spc_from_file = val;}
	bool GetSpcFromFile() {return m_spc_from_file;}

	//brkxml
	void SetBaseSpacing(const fluo::Vector& spacing);
	fluo::Vector GetBaseSpacing();
	void SetSpacingScale(const fluo::Vector& scaling);
	fluo::Vector GetSpacingScale();

	void SetLevel(int lv);
	int GetLevel();
	int GetLevelNum();

	//bits
	int GetBits();
	//voxel count
	uint64_t GetVoxelCount() { return m_size.get_size_xyz(); }

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
	void SetRenderMode(flvr::RenderMode mode);
	flvr::RenderMode GetRenderMode() { return m_render_mode; }
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
	void SetChannelMixMode(ChannelMixMode mode) { m_channel_mix_mode = mode; }
	ChannelMixMode GetChannelMixMode() { return m_channel_mix_mode; }

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

	virtual void SetClippingBox(const fluo::ClippingBox& box) override;
	//clip size
	virtual void SetClipValue(fluo::ClipPlane i, int val) override;
	virtual void SetClipValues(fluo::ClipPlane i, int val1, int val2) override;
	virtual void SetClipValues(const std::array<int, 6>& vals) override;
	virtual void ResetClipValues() override;
	virtual void ResetClipValues(fluo::ClipPlane i) override;
	//clip rotation
	virtual void SetClipRotation(int i, double val) override;
	virtual void SetClipRotation(const fluo::Vector& euler) override;
	virtual void SetClipRotation(const fluo::Quaternion& q) override;
	//clip distance
	virtual void SetLink(fluo::ClipPlane i, bool link) override;
	virtual void ResetLink() override;
	virtual void SetLinkedDist(fluo::ClipPlane i, int val) override;

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
	flvr::RenderMode m_render_mode;	//0-normal; 1-MIP; 2-white shading; 3-white mip
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
	double m_shading_strength;
	double m_shading_shine;

	//shadow
	bool m_shadow_enable;
	double m_shadow_intensity;

	bool m_sample_rate_enable;
	double m_sample_rate;

	fluo::Color m_color;
	bool m_wl_color;//if color has been set by wavelength

	//noise reduction
	bool m_noise_rd;

	//resolution, scaling, spacing
	fluo::Vector m_size;
	fluo::Vector m_scaling;
	fluo::Vector m_spacing;
	bool m_spc_from_file;
	//resample
	bool m_resample;
	fluo::Vector m_resampled_size;

	//display control
	bool m_disp;
	bool m_draw_bounds;
	bool m_test_wiref;

	//color map mode
	double m_colormap_inv;
	flvr::ColorMode m_color_mode;	//0-normal; 1-rainbow
	bool m_colormap_disp;	//true/false
	int m_colormap;//index to a colormap
	flvr::ColormapProj m_colormap_proj;//index to method of mapping
						//0-intensity; 1-z-value; 2-y-value; 3-x-value; 4-t-value;
						//5 - gradient magnitude; 6 - gradient dir; 7 - intensity delta; 8 - speed	
	double m_colormap_low_value;
	double m_colormap_hi_value;
	//the min/max values are needed because it can be mapped to values other than intensity
	double m_colormap_min_value;
	double m_colormap_max_value;

	//transparent
	bool m_transparent;

	//blend mode
	ChannelMixMode m_channel_mix_mode;

	//inverted
	bool m_invert;

	//2d mask texture for segmentation
	unsigned int m_2d_mask;
	//2d weight map for segmentation
	unsigned int m_2d_weight1;	//after tone mapping
	unsigned int m_2d_weight2;	//before tone mapping

	//2d depth map texture for rendering shadows
	unsigned int m_2d_dmap;

	//reader
	std::weak_ptr<BaseVolReader> m_reader;

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

	std::vector<VD_Landmark> m_landmarks;
	std::wstring m_metadata_id;

	//mask cleared
	bool m_mask_clear;

	//background intensity
	bool m_bg_valid;
	double m_bg_int;

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

#endif//_VOLUME_DATA_H_