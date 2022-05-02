/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#ifndef VOLUMEDATA_HPP
#define VOLUMEDATA_HPP

#include <Node.hpp>
#include <Names.hpp>
#include <Nrrd/nrrd.h>
#include <glm/glm.hpp>
#include <vector>

//output adjustments
#define gstGammaR "gamma r"
#define gstGammaG "gamma g"
#define gstGammaB "gamma b"
#define gstBrightnessR "brightness r"
#define gstBrightnessG "brightness g"
#define gstBrightnessB "brightness b"
#define gstEqualizeR "equalize r"
#define gstEqualizeG "equalize g"
#define gstEqualizeB "equalize b"
#define gstSyncR "sync r"
#define gstSyncG "sync g"
#define gstSyncB "sync b"
//bounding box
#define gstBounds "bounds"
//clipping planes
#define gstClipPlanes "clip planes"
#define gstClipBounds "clip bounds"
//clipping values
#define gstClipX1 "clip x1"
#define gstClipX2 "clip x2"
#define gstClipY1 "clip y1"
#define gstClipY2 "clip y2"
#define gstClipZ1 "clip z1"
#define gstClipZ2 "clip z2"
#define gstClipDistX "clip dist x"
#define gstClipDistY "clip dist y"
#define gstClipDistZ "clip dist z"
#define gstClipLinkX "clip link x"
#define gstClipLinkY "clip link y"
#define gstClipLinkZ "clip link z"
#define gstClipRotX "clip rot x"
#define gstClipRotY "clip rot y"
#define gstClipRotZ "clip rot z"
#define gstDrawClip "draw clip"
#define gstClipHold "clip hold"
#define gstClipMask "clip mask"
#define gstClipRenderMode "clip render mode"
//file properties
#define gstDataPath "data path"
#define gstChannel "channel"
#define gstTime "time"
//render modes
#define gstBlendMode "blend mode"
#define gstMipMode "mip mode"
#define gstOverlayMode "overlay mode"
//stream mode
#define gstStreamMode "stream mode"
#define gstMaskMode "mask mode"
#define gstUseMaskThresh "use mask thresh"
#define gstMaskThresh "mask thresh"
#define gstInvert "invert"
//trasnfer function
#define gstSoftThresh "soft thresh"
#define gstIntScale "int scale"
#define gstGmScale "gm scale"
#define gstMaxInt "max int"
#define gstMaxScale "max scale"
#define gstGamma3d "gamma 3d"
#define gstExtractBoundary "extract boundary"
#define gstSaturation "saturation"
#define gstLowThreshold "low threshold"
#define gstHighThreshold "high threshold"
#define gstAlpha "alpha"
#define gstAlphaPower "alpha power"
#define gstAlphaEnable "alpha enable"
#define gstMatAmb "mat amb"
#define gstMatDiff "mat diff"
#define gstMatSpec "mat spec"
#define gstMatShine "mat shine"
#define gstNoiseRedct "noise redct"
#define gstShadingEnable "shding enable"
#define gstLowShading "low shading"
#define gstHighShading "high shading"
#define gstShadowEnable "shadow enable"
#define gstShadowInt "shadow int"
#define gstShadowDirX "shadow dir x"
#define gstShadowDirY "shadow dir y"
#define gstSampleRate "sample rate"
//color
#define gstColor "color"
#define gstHsv "hsv"
#define gstLuminance "luminance"
#define gstSecColor "sec color"
#define gstSecColorSet "sec color set"
#define gstRandomizeColor "randomize color"
//resolution
#define gstResX "res x"
#define gstResY "res y"
#define gstResZ "res z"
#define gstBaseResX "base res x"
#define gstBaseResY "base res y"
#define gstBaseResZ "base res z"
//scaling
#define gstScaleX "scale x"
#define gstScaleY "scale y"
#define gstScaleZ "scale z"
//spacing
#define gstSpcX "spc x"
#define gstSpcY "spc y"
#define gstSpcZ "spc z"
#define gstSpcFromFile "spc from file"
#define gstBaseSpcX "base spc x"
#define gstBaseSpcY "base spc y"
#define gstBaseSpcZ "base spc z"
#define gstSpcSclX "spc scl x"
#define gstSpcSclY "spc scl y"
#define gstSpcSclZ "spc scl z"
//display
#define gstBits "bits"
#define gstDisplay "display"
#define gstDrawBounds "draw bounds"
#define gstTestWiref "test wiref"
//colormap
#define gstColormapEnable "colormap enable"
#define gstColormapMode "colormap mode"
#define gstColormapType "colormap type"
#define gstColormapLow "colormap low"
#define gstColormapHigh "colormap high"
#define gstColormapProj "colormap proj"
#define gstColormapInv "colormap inv"
//tex ids
#define gst2dMaskId "2d mask id"
#define gst2dWeight1Id "2d weight1 id"
#define gst2dWeight2Id "2d weight2 id"
#define gst2dDmapId "2d dmap id"
//compress
#define gstHardwareCompress "hardware compress"
//resize
#define gstResize "resize"
#define gstResizeX "resize x"
#define gstResizeY "resize y"
#define gstResizeZ "resize z"
//brick
#define gstSkipBrick "skip brick"
#define gstBrickNum "brick num"
//legend
#define gstLegend "legend"
//interpolate
#define gstInterpolate "interpolate"
//label
#define gstLabelMode "label mode"
//depth atten
#define gstDepthAtten "depth atten"
#define gstDaInt "da int"
#define gstDaStart "da start"
#define gstDaEnd "da end"
//estimated thresh
#define gstEstimateThresh "estimate thresh"
//others
#define gstViewport "viewport"
#define gstClearColor "clear color"
#define gstCurFramebuffer "cur framebuffer"
//multires
#define gstMultires "multires"
#define gstLevel "level"
#define gstLevelNum "level num"
//tex transform
#define gstTexTransform "tex transform"
//selection
#define gstSelected "selected"
//mask clear
#define gstMaskClear "mask clear"
//sync group
#define gstSyncGroup "sync group"
//duplicated
#define gstDuplicate "duplicate"

namespace flvr
{
	class VolumeRenderer;
	class Texture;
	class TextureBrick;
}
class BaseReader;
namespace fluo
{
	class VolumeFactory;
	class VolumeData;
	class Quaternion;
	typedef std::vector<VolumeData*> VolumeList;
	struct VD_Landmark
	{
		std::wstring name;
		double x, y, z;
		double spcx, spcy, spcz;
	};

	class VolumeData : public Node
	{
	public:
		VolumeData();
		VolumeData(const VolumeData& data, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new VolumeData(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const VolumeData*>(obj) != NULL;
		}

		virtual const char* className() const { return "VolumeData"; }

		virtual VolumeData* asVolumeData() { return this; }
		virtual const VolumeData* asVolumeData() const { return this; }

		//functions from old class
		//reader
		void SetReader(BaseReader* reader);
		BaseReader* GetReader();

		//load
		int LoadData(Nrrd* data, const std::string &name, const std::wstring &path);
		int ReplaceData(Nrrd* data, bool del_tex);
		int ReplaceData(VolumeData* data);
		Nrrd* GetData(bool ret);

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
		void AddEmptyMask(int mode, bool change);
		void AddMask(Nrrd* mask, int op);
		void AddMask16(Nrrd* mask, int op, double scale);

		//load label
		void LoadLabel(Nrrd* label);
		Nrrd* GetLabel(bool ret);

		//empty label
		//mode: 0-zeros; 1-ordered; 2-shuffled
		void AddEmptyLabel(int mode, bool change);
		bool SearchLabel(unsigned int label);
		//save label
		void PushLabel(bool ret);
		void PopLabel();
		void LoadLabel2();

		//save
		double GetOriginalValue(int i, int j, int k, flvr::TextureBrick* b = 0);
		double GetTransferValue(int i, int j, int k, flvr::TextureBrick* b = 0);
		void SaveData(const std::wstring &filename,
			int mode, bool crop, int filter, bool bake,
			bool compress, fluo::Quaternion &q);
		void SaveMask(bool use_reader, long t, long c);
		void SaveLabel(bool use_reader, long t, long c);

		//volume renderer
		flvr::VolumeRenderer* GetRenderer();

		//texture
		flvr::Texture* GetTexture();
		void SetTexture();
		int GetAllBrickNum();

		//draw
		void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat, glm::mat4 &tex_mat);
		void Draw(bool otho = false, bool adaptive = false,
			bool intactive = false, double zoom = 1.0,
			int stream_mode = 0);
		void DrawBounds();
		//draw mask (create the mask)
		//type: 0-initial; 1-diffusion-based growing
		//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear
		//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
		//order (for updating instreaming mode): 0-no op; 1-normal order; 2-reversed order
		void DrawMask(int type, int paint_mode, int hr_mode,
			double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
			double w2d, double bins, int order, bool ortho = false, bool estimate = false);
		//draw label (create the label)
		//type: 0-initialize; 1-maximum intensity filtering
		//mode: 0-normal; 1-posterized, 2-copy values
		void DrawLabel(int type, int mode, double thresh, double gm_falloff);

		//calculation
		void Calculate(int type, VolumeData* vd_a, VolumeData* vd_b);

		//color map
		fluo::Color GetColorFromColormap(double value);

		void SetShuffle(int val);
		int GetShuffle();
		void IncShuffle();

		friend class VolumeFactory;

	protected:
		virtual ~VolumeData();

	private:
		flvr::VolumeRenderer *m_vr;
		flvr::Texture *m_tex;
		BaseReader *m_reader;

		std::vector<VD_Landmark> m_landmarks;
		std::wstring m_metadata_id;
		//save label
		void* m_label_save;

	private:
		//label functions
		void SetOrderedID(unsigned int* val);
		void SetReverseID(unsigned int* val);
		void SetShuffledID(unsigned int* val);

		void Initialize();//called after renderer is set
		//handle observer notifications
		//most are for setting values in the renderer
		//this will be changed when rendering pipeline is restructured
		//void OnMipModeChanging();
		void OnMipModeChanged(Event& event);//modes
		void OnOverlayModeChanged(Event& event);//overlay mode changes temporarily for rendering overlayed effects
		void OnViewportChanged(Event& event);
		void OnClearColorChanged(Event& event);
		void OnCurFramebufferChanged(Event& event);
		void OnCompressionChanged(Event& event);
		void OnInvertChanged(Event& event);
		void OnMaskModeChanged(Event& event);
		void OnNoiseRedctChanged(Event& event);
		void On2dDmapIdChanged(Event& event);
		void OnGamma3dChanged(Event& event);
		void OnExtractBoundaryChanged(Event& event);
		void OnSaturationChanged(Event& event);
		void OnLowThresholdChanged(Event& event);
		void OnHighThresholdChanged(Event& event);
		void OnColorChanged(Event& event);
		void OnSecColorChanged(Event& event);
		void OnSecColorSetChanged(Event& event);
		void OnLuminanceChanged(Event& event);
		void OnAlphaChanged(Event& event);
		void OnAlphaPowerChanged(Event& event);
		void OnAlphaEnableChanged(Event& event);
		void OnMaskThreshChanged(Event& event);
		void OnUseMaskThreshChanged(Event& event);
		void OnShadingEnableChanged(Event& event);
		void OnMaterialChanged(Event& event);
		void OnSampleRateChanged(Event& event);
		void OnColormapModeChanged(Event& event);
		void OnColormapValueChanged(Event& event);
		void OnColormapTypeChanged(Event& event);
		void OnColormapProjChanged(Event& event);
		void OnSpacingChanged(Event& event);
		void OnBaseSpacingChanged(Event& event);
		void OnSpacingScaleChanged(Event& event);
		void OnLevelChanged(Event& event);
		void OnDisplayChanged(Event& event);
		void OnInterpolateChanged(Event& event);
		void OnLabelModeChanged(Event& event);
		void OnDepthAttenChanged(Event& event);
		void OnSkipBrickChanged(Event& event);
		void OnClipPlanesChanged(Event& event);
		void OnLowShadingChanged(Event& event);
		void OnHighShadingChanged(Event& event);
		void OnIntScaleChanged(Event& event);
		void OnSyncOutputChannels(Event& event);
		void OnClipX1Changed(Event& event);
		void OnClipX2Changed(Event& event);
		void OnClipY1Changed(Event& event);
		void OnClipY2Changed(Event& event);
		void OnClipZ1Changed(Event& event);
		void OnClipZ2Changed(Event& event);
		void OnClipRot(Event& event);
		void OnRandomizeColor(Event& event);//randomize color

		//update clipping planes
		//any value changes -> set clip values and then rotate
		void UpdateClippingPlanes(Event& event);

	};
}

#endif//_VOLUMEDATA_H_
