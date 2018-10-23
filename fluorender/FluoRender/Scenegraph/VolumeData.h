/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef _VOLUMEDATA_H_
#define _VOLUMEDATA_H_

#include <Scenegraph/Node.h>
#include <nrrd.h>
#include <glm/glm.hpp>
#include <vector>

namespace FLIVR
{
class VolumeRenderer;
class Texture;
class TextureBrick;
}
class BaseReader;
namespace FL
{
	class VolumeFactory;
	class VolumeData;
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
		virtual void RandomizeColor();//randomize color

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
		void AddEmptyMask(int mdoe);

		//load label
		void LoadLabel(Nrrd* label);
		Nrrd* GetLabel(bool ret);

		//empty label
		//mode: 0-zeros; 1-ordered; 2-shuffled
		void AddEmptyLabel(int mode);
		bool SearchLabel(unsigned int label);

		//save
		double GetOriginalValue(int i, int j, int k, FLIVR::TextureBrick* b = 0);
		double GetTransferValue(int i, int j, int k, FLIVR::TextureBrick* b = 0);
		void SaveData(std::wstring &filename, int mode = 0, bool bake = false, bool compress = false);
		void SaveMask(bool use_reader, long t, long c);
		void SaveLabel(bool use_reader, long t, long c);

		//volume renderer
		FLIVR::VolumeRenderer* GetRenderer();

		//texture
		FLIVR::Texture* GetTexture();
		void SetTexture();

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
		FLTYPE::Color GetColorFromColormap(double value);

	protected:
		virtual ~VolumeData();

	private:
		FLIVR::VolumeRenderer *m_vr;
		FLIVR::Texture *m_tex;
		BaseReader *m_reader;

		std::vector<VD_Landmark> m_landmarks;
		std::wstring m_metadata_id;

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
		void OnMipModeChanged();//modes
		void OnOverlayModeChanged();//overlay mode changes temporarily for rendering overlayed effects
		void OnViewportChanged();
		void OnClearColorChanged();
		void OnCurFramebufferChanged();
		void OnCompressionChanged();
		void OnInvertChanged();
		void OnMaskModeChanged();
		void OnNoiseRedctChanged();
		void On2dDmapIdChanged();
		void OnGamma3dChanged();
		void OnExtractBoundaryChanged();
		void OnSaturationChanged();
		void OnLowThresholdChanged();
		void OnHighThresholdChanged();
		void OnColorChanged();
		void OnSecColorChanged();
		void OnSecColorSetChanged();
		void OnLuminanceChanged();
		void OnAlphaChanged();
		void OnAlphaEnableChanged();
		void OnMaskThreshChanged();
		void OnUseMaskThreshChanged();
		void OnShadingEnableChanged();
		void OnMaterialChanged();
		void OnSampleRateChanged();
		void OnColormapModeChanged();
		void OnColormapValueChanged();
		void OnColormapTypeChanged();
		void OnColormapProjChanged();
		void OnSpacingChanged();
		void OnBaseSpacingChanged();
		void OnSpacingScaleChanged();
		void OnLevelChanged();
		void OnDisplayChanged();
		void OnInterpolateChanged();
		void OnDepthAttenChanged();
		void OnSkipBrickChanged();
		void OnClipPlanesChanged();
		void OnLowShadingChanged();
		void OnHighShadingChanged();
		void OnIntScaleChanged();
		void OnSyncOutputChannels();
		void OnClipX1Changed();
		void OnClipX2Changed();
		void OnClipY1Changed();
		void OnClipY2Changed();
		void OnClipZ1Changed();
		void OnClipZ2Changed();
		void OnClipRot();

		//update clipping planes
		//any value changes -> set clip values and then rotate
		void UpdateClippingPlanes();

		friend class VolumeFactory;
	};
}

#endif//_VOLUMEDATA_H_