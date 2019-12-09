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

#ifndef _VRENDERVIEW_H_
#define _VRENDERVIEW_H_

#include "VRenderGLView.h"

class VRenderView: public wxPanel
{
public:
	enum
	{
		ID_VolumeSeqRd = ID_VRENDER_VIEW2,
		ID_VolumeMultiRd,
		ID_VolumeCompRd,
		ID_CaptureBtn,
		ID_BgColorPicker,
		ID_BgInvBtn,
		ID_RotLinkChk,
		ID_RotResetBtn,
		ID_XRotText,
		ID_YRotText,
		ID_ZRotText,
		ID_XRotSldr,
		ID_YRotSldr,
		ID_ZRotSldr,
		ID_RotateTimer,
		ID_RotLockChk,
		ID_RotSliderType,
		ID_OrthoViewCmb,
		ID_DepthAttenChk,
		ID_DepthAttenFactorSldr,
		ID_DepthAttenResetBtn,
		ID_DepthAttenFactorText,
		ID_FullScreenBtn,
		ID_PinBtn,
		ID_CenterBtn,
		ID_Scale121Btn,
		ID_ScaleFactorSldr,
		ID_ScaleFactorText,
		ID_ScaleFactorSpin,
		ID_ScaleModeBtn,
		ID_ScaleResetBtn,
		ID_CamCtrChk,
		ID_FpsChk,
		ID_LegendChk,
		ID_ColormapChk,
		ID_ScaleBar,
		ID_ScaleText,
		ID_ScaleCmb,
		ID_DefaultBtn,
		ID_AovSldr,
		ID_AovText,
		ID_FreeChk
	};

	VRenderView(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		wxGLContext* sharedContext=0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	~VRenderView();

	//recalculate view according to object bounds
	void InitView(unsigned int type=INIT_BOUNDS);

	//clear layers
	void Clear();

	//data management
	int GetAny();
	int GetDispVolumeNum();
	int GetAllVolumeNum();
	int GetMeshNum();
	int GetGroupNum();
	int GetLayerNum();
	VolumeData* GetAllVolumeData(int index);
	VolumeData* GetDispVolumeData(int index);
	MeshData* GetMeshData(int index);
	TreeLayer* GetLayer(int index);
	MultiVolumeRenderer* GetMultiVolumeData();
	VolumeData* GetVolumeData(wxString &name);
	MeshData* GetMeshData(wxString &name);
	Annotations* GetAnnotations(wxString &name);
	DataGroup* GetGroup(wxString &name);
	DataGroup* GetGroup(int index);
	DataGroup* AddVolumeData(VolumeData* vd, wxString group_name="");
	void AddMeshData(MeshData* md);
	void AddAnnotations(Annotations* ann);
	wxString AddGroup(wxString str = "", wxString prev_group="");
	DataGroup* AddOrGetGroup();
	wxString AddMGroup(wxString str = "");
	MeshGroup* AddOrGetMGroup();
	MeshGroup* GetMGroup(wxString &name);
	void RemoveVolumeData(wxString &name);
	void RemoveVolumeDataDup(wxString &name);
	void RemoveMeshData(wxString &name);
	void RemoveAnnotations(wxString &name);
	void RemoveGroup(wxString &name);
	void Isolate(int type, wxString name);
	void ShowAll();
	//move
	void MoveLayerinView(wxString &src_name, wxString &dst_name);
	//move volume
	void MoveLayerinGroup(wxString &group_name, wxString &src_name, wxString &dst_name);
	void MoveLayertoView(wxString &group_name, wxString &src_name, wxString &dst_name);
	void MoveLayertoGroup(wxString &group_name, wxString &src_name, wxString &dst_name);
	void MoveLayerfromtoGroup(wxString &src_group_name, wxString &dst_group_name, wxString &src_name, wxString &dst_name);
	//move mesh
	void MoveMeshinGroup(wxString &group_name, wxString &src_name, wxString &dst_name);
	void MoveMeshtoView(wxString &group_name, wxString &src_name, wxString &dst_name);
	void MoveMeshtoGroup(wxString &group_name, wxString &src_name, wxString &dst_name);
	void MoveMeshfromtoGroup(wxString &src_group_name, wxString &dst_group_name, wxString &src_name, wxString &dst_name);
	//reorganize layers in view
	void OrganizeLayers();
	//randomize color
	void RandomizeColor();

	//toggle hiding/displaying
	void SetDraw(bool draw);
	void ToggleDraw();
	bool GetDraw();

	//camera operations
	void GetTranslations(double &transx, double &transy, double &transz);
	void SetTranslations(double transx, double transy, double transz);
	void GetRotations(double &rotx, double &roty, double &rotz);
	void SetRotations(double rotx, double roty, double rotz, bool ui_update=true);
	void GetCenters(double &ctrx, double &ctry, double &ctrz);
	void SetCenters(double ctrx, double ctry, double ctrz);
	double GetCenterEyeDist();
	void SetCenterEyeDist(double dist);
	double GetRadius();
	void SetRadius(double r);
	void UpdateScaleFactor(bool update_text = true);
	void SetScaleFactor(double s, bool update);
	void SetScaleMode(bool mode, bool update);

	//object operations
	void GetObjCenters(double &ctrx, double &ctry, double &ctrz);
	void SetObjCenters(double ctrx, double ctry, double ctrz);
	void GetObjTrans(double &transx, double &transy, double &transz);
	void SetObjTrans(double transx, double transy, double transz);
	void GetObjRot(double &rotx, double &roty, double &rotz);
	void SetObjRot(double rotx, double roty, double rotz);

	//camera properties
	bool GetPersp() {if (m_glview) return m_glview->GetPersp(); else return true;}
	void SetPersp(bool persp) {if (m_glview) m_glview->SetPersp(persp);}
	bool GetFree() {if (m_glview) return m_glview->GetFree(); else return false;}
	void SetFree(bool free) {if (m_glview) m_glview->SetFree(free);}
	double GetAov();
	void SetAov(double aov);
	double GetNearClip();
	void SetNearClip(double nc);
	double GetFarClip();
	void SetFarClip(double fc);

	//background color
	Color GetBackgroundColor();
	Color GetTextColor();
	void SetBackgroundColor(Color &color);
	void SetGradBg(bool val);

	//rot center anchor thresh
	void SetPinThreshold(double value);

	//point volume mode
	void SetPointVolumeMode(int mode);
	int GetPointVolumeMode();

	//ruler uses transfer function
	void SetRulerUseTransf(bool val);
	bool GetRulerUseTransf();

	//ruler time dependent
	void SetRulerTimeDep(bool val);
	bool GetRulerTimeDep();

	//disply modes
	int GetDrawType();
	void SetVolMethod(int method);
	int GetVolMethod();

	//other properties
	void SetFog(bool b=true);
	bool GetFog();
	void SetFogIntensity(double i);
	double GetFogIntensity();
	void SetPeelingLayers(int n);
	int GetPeelingLayers();
	void SetBlendSlices(bool val);
	bool GetBlendSlices();
	void SetAdaptive(bool val);
	bool GetAdaptive();

	//reset counter
	static void ResetID();

	//get rendering context
	wxGLContext* GetContext();

	//refresh glview
	void RefreshGL(bool intactive=true, bool start_loop=true);

	//movie export
	//get frame info
	//4d sequence
	void Get4DSeqFrames(int &start_frame, int &end_frame, int &cur_frame)
	{
		if (m_glview)
			m_glview->Get4DSeqFrames(start_frame, end_frame, cur_frame);
	}
	void Set4DSeqFrame(int frame, bool run_script)
	{
		if (m_glview)
			m_glview->Set4DSeqFrame(frame, run_script);
	}
	//3d batch
	void Get3DBatFrames(int &start_frame, int &end_frame, int &cur_frame)
	{
		if (m_glview)
			m_glview->Get3DBatFrames(start_frame, end_frame, cur_frame);
	}
	void Set3DBatFrame(int offset)
	{
		if (m_glview)
			m_glview->Set3DBatFrame(offset);
	}
	//set movie export
	void Set3DRotCapture(double start_angle,
		double end_angle,
		double step,
		int frames,
		int rot_axis,
		wxString &cap_file,
		bool rewind,
		int len = 4)
	{
		if (m_glview)
			m_glview->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, cap_file, rewind, len);
	}
	void Set4DSeqCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind)
	{
		if (m_glview)
			m_glview->Set4DSeqCapture(cap_file, begin_frame, end_frame, rewind);
	}
	void Set3DBatCapture(wxString &cap_file, int begin_frame, int end_frame)
	{
		if (m_glview)
			m_glview->Set3DBatCapture(cap_file, begin_frame, end_frame);
	}
	void SetParamCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind)
	{
		if (m_glview)
			m_glview->SetParamCapture(cap_file, begin_frame, end_frame, rewind);
	}
	void SetParams(double p) {
		if (m_glview)
			m_glview->SetParams(p);
	}
	//reset & stop
	void ResetMovieAngle()
	{
		if (m_glview)
			m_glview->ResetMovieAngle();
	}
	void StopMovie()
	{
		if (m_glview)
			m_glview->StopMovie();
	}

	//movie frame
	void EnableFrame()
	{
		if (m_glview) m_glview->EnableFrame();
	}
	void DisableFrame()
	{
		if (m_glview) m_glview->DisableFrame();
	}
	void SetFrame(int x, int y, int w, int h)
	{
		if (m_glview) m_glview->SetFrame(x, y, w, h);
	}
	void GetFrame(int &x, int &y, int &w, int &h)
	{
		if (m_glview) m_glview->GetFrame(x, y, w, h);
	}
	void CalcFrame()
	{
		if (m_glview) m_glview->CalcFrame();
	}
	bool GetFrameEnabled()
	{
		if (m_glview) return m_glview->m_draw_frame;
		return false;
	}
	wxSize GetGLSize()
	{
		if (m_glview) return m_glview->GetSize();
		else return wxSize(0, 0);
	}
	//scale bar
	void SetScaleBarLen(double len)
	{if (m_glview) m_glview->SetScaleBarLen(len);}

	//set dirty
	void SetVolPopDirty()
	{if (m_glview) m_glview->SetVolPopDirty();}
	void SetMeshPopDirty()
	{if (m_glview) m_glview->SetMeshPopDirty();}
	//clear
	void ClearVolList()
	{if (m_glview) m_glview->ClearVolList();}
	void ClearMeshList()
	{if (m_glview) m_glview->ClearMeshList();}

	//inteactive mode selection
	int GetIntMode()
	{if (m_glview) return m_glview->GetIntMode(); else return 1;}
	void SetIntMode(int mode = 1)
	{if (m_glview) m_glview->SetIntMode(mode);}

	//set paint use 2d results
	void SetPaintUse2d(bool use2d)
	{if (m_glview) m_glview->SetPaintUse2d(use2d);}
	bool GetPaintUse2d()
	{if (m_glview) return m_glview->GetPaintUse2d(); else return false;}

	//segmentation mode selection
	void SetPaintMode(int mode)
	{if (m_glview) m_glview->SetPaintMode(mode);}
	int GetPaintMode()
	{if (m_glview) return m_glview->GetPaintMode(); else return 0;}

	//segment volumes in current view
	void Segment()
	{if (m_glview) m_glview->Segment();}

	//calculations
	void SetVolumeA(VolumeData* vd)
	{if (m_glview) m_glview->SetVolumeA(vd);}
	void SetVolumeB(VolumeData* vd)
	{if (m_glview) m_glview->SetVolumeB(vd);}
	void Calculate(int type, wxString prev_group="")
	{if (m_glview) return m_glview->Calculate(type, prev_group);}

	//brush properties
	//use pressure
	void SetBrushUsePres(bool pres)
	{ if (m_glview) m_glview->SetBrushUsePres(pres); }
	bool GetBrushUsePres()
	{ if (m_glview) return m_glview->GetBrushUsePres(); else return false;}
	//set brush size
	void SetUseBrushSize2(bool val)
	{ if (m_glview) m_glview->SetUseBrushSize2(val); }
	bool GetUseBrushSize2()
	{ if (m_glview) return m_glview->GetUseBrushSize2(); else return false; }
	void SetBrushSize(double size1, double size2)
	{ if (m_glview) m_glview->SetBrushSize(size1, size2); }
	double GetBrushSize1()
	{ if (m_glview) return m_glview->GetBrushSize1(); else return 0.0; }
	double GetBrushSize2()
	{ if (m_glview) return m_glview->GetBrushSize2(); else return 0.0; }
	//set brush spacing
	void SetBrushSpacing(double spacing)
	{ if (m_glview) m_glview->SetBrushSpacing(spacing); }
	double GetBrushSpacing()
	{ if (m_glview) return m_glview->GetBrushSpacing(); else return 1.0; }
	//set iteration number
	void SetBrushIteration(int num)
	{ if (m_glview) m_glview->SetBrushIteration(num); }
	int GetBrushIteration()
	{ if (m_glview) return m_glview->GetBrushIteration(); else return 0; }
	//set brush size relation
	void SetBrushSizeData(bool val)
	{ if (m_glview) m_glview->SetBrushSizeData(val); }
	bool GetBrushSizeData()
	{ if (m_glview) return m_glview->GetBrushSizeData(); else return false; }
	//scalar translate
	void SetBrushSclTranslate(double val)
	{ if (m_glview) m_glview->SetBrushSclTranslate(val); }
	double GetBrushSclTranslate()
	{ if (m_glview) return m_glview->GetBrushSclTranslate(); else return 0.0; }
	//gm falloff
	void SetBrushGmFalloff(double val)
	{ if (m_glview) m_glview->SetBrushGmFalloff(val); }
	double GetBrushGmFalloff()
	{ if (m_glview) return m_glview->GetBrushGmFalloff(); else return 0.0; }
	//w2d
	void SetW2d(double val)
	{ if (m_glview) m_glview->SetW2d(val); }
	double GetW2d()
	{ if (m_glview) return m_glview->GetW2d(); else return 0.0; }
	//edge detect
	void SetEdgeDetect(bool value)
	{ if (m_glview) m_glview->SetEdgeDetect(value); }
	bool GetEdgeDetect()
	{ if (m_glview) return m_glview->GetEdgeDetect(); else return false;}
	//hidden removal
	void SetHiddenRemoval(bool value)
	{ if (m_glview) m_glview->SetHiddenRemoval(value); }
	bool GetHiddenRemoval()
	{ if (m_glview) return m_glview->GetHiddenRemoval(); else return false;}
	//select group
	void SetSelectGroup(bool value)
	{ if (m_glview) m_glview->SetSelectGroup(value); }
	bool GetSelectGroup()
	{ if (m_glview) return m_glview->GetSelectGroup(); else return false;}
	//estimate threshold
	void SetEstimateThresh(bool value)
	{ if (m_glview) m_glview->SetEstimateThresh(value);}
	bool GetEstimateThresh()
	{ if (m_glview) return m_glview->GetEstimateThresh(); else return false;}
	//brick acuracy
	void SetAccurateBricks(bool value)
	{ if (m_glview) m_glview->SetAccurateBricks(value);}
	bool GetAccurateBricks()
	{ if (m_glview) return m_glview->GetAccurateBricks(); else return false;}
	//select a and b
	void SetSelectBoth(bool value)
	{ if (m_glview) m_glview->SetSelectBoth(value); }
	bool GetSelectBoth()
	{ if (m_glview) return m_glview->GetSelectBoth(); else return false;}

	//set clip mode
	void SetClipMode(int mode)
	{
		if (m_glview) m_glview->SetClipMode(mode);
	}
	int GetClipMode()
	{
		if (m_glview) return m_glview->GetClipMode();
		return 0;
	}
	//restore clipping planes
	void RestorePlanes()
	{
		if (m_glview) m_glview->RestorePlanes();
	}
	void SetClippingPlaneRotations(double rotx, double roty, double rotz)
	{
		if (m_glview) m_glview->SetClippingPlaneRotations(rotx, roty, rotz);
	}
	void GetClippingPlaneRotations(double &rotx, double &roty, double &rotz)
	{
		if (m_glview) m_glview->GetClippingPlaneRotations(rotx, roty, rotz);
	}

	//get volume selector
	VolumeSelector* GetVolumeSelector()
	{
		if (m_glview) return m_glview->GetVolumeSelector(); else return 0;
	}
	//get volume calculator
	FL::VolumeCalculator* GetVolumeCalculator()
	{
		if (m_glview) return m_glview->GetVolumeCalculator(); else return 0;
	}
	//get kernel executor
	KernelExecutor* GetKernelExecutor()
	{
		if (m_glview) return m_glview->GetKernelExecutor(); else return 0;
	}

	//rulers
	FL::RulerList* GetRulerList()
	{
		if (m_glview) return m_glview->GetRulerList(); else return 0;
	}
	FL::Ruler* GetRuler(unsigned int id)
	{
		if (m_glview) return m_glview->GetRuler(id); else return 0;
	}
	FL::RulerHandler* GetRulerHandler()
	{
		if (m_glview) return &m_glview->m_ruler_handler; else return 0;
	}
	int RulerProfile(int index)
	{
		if (m_glview) return m_glview->RulerProfile(index); else return 0;
	}
	int RulerDistance(int index)
	{
		if (m_glview) return m_glview->RulerDistance(index); else return 0;
	}

	//traces
	void CreateTraceGroup()
	{
		if (m_glview) m_glview->CreateTraceGroup();
	}
	TraceGroup* GetTraceGroup()
	{
		if (m_glview) return m_glview->GetTraceGroup(); else return 0;
	}
	int LoadTraceGroup(wxString filename)
	{
		if (m_glview) return m_glview->LoadTraceGroup(filename); else return 0;
	}
	int SaveTraceGroup(wxString filename)
	{
		if (m_glview) return m_glview->SaveTraceGroup(filename); else return 0;
	}
	void ExportTrace(wxString filename, unsigned int id)
	{
		if (m_glview) m_glview->ExportTrace(filename, id);
	}

	//bit mask for items to save
	bool m_default_saved;
	void SaveDefault(unsigned int mask = 0xffffffff);

	//set full screen
	void SetFullScreen();

	//stereo/vr
	void InitOpenVR()
	{
#ifdef _WIN32
		if (m_glview) m_glview->InitOpenVR();
#endif
	}
	void SetStereo(bool bval)
	{
		if (m_glview) m_glview->SetStereo(bval);
	}
	void SetEyeDist(double dval)
	{
		if (m_glview) m_glview->SetEyeDist(dval);
	}

public:
	wxWindow* m_frame;
	static int m_max_id;
	int m_id;

	//render view///////////////////////////////////////////////
	VRenderGLView *m_glview;
	wxFrame* m_full_frame;
	wxBoxSizer* m_view_sizer;

	//top bar///////////////////////////////////////////////////
	wxPanel* m_panel_1;
	wxColourPickerCtrl *m_bg_color_picker;
	wxButton* m_bg_inv_btn;
	wxSlider* m_aov_sldr;
	wxTextCtrl* m_aov_text;
	wxToolBar * m_options_toolbar;
	//wxToolBar * m_options_toolbar2;
	wxToolBar * m_left_toolbar;
	wxToolBar * m_right_toolbar2;
	wxToolBar * m_lower_toolbar;
	//scale bar
	wxTextCtrl *m_scale_text;
	wxComboBox *m_scale_cmb;

	//bottom bar///////////////////////////////////////////////////
	wxPanel* m_panel_2;
	wxScrollBar *m_x_rot_sldr;
	wxTextCtrl *m_x_rot_text;
	wxScrollBar *m_y_rot_sldr;
	wxTextCtrl *m_y_rot_text;
	wxScrollBar *m_z_rot_sldr;
	wxTextCtrl *m_z_rot_text;
	wxTimer m_timer;
	bool m_x_rotating, m_y_rotating, m_z_rotating;
	bool m_skip_thumb;
	wxToolBar *m_rot_lock_btn;
	wxComboBox *m_ortho_view_cmb;

	//left bar///////////////////////////////////////////////////
	wxPanel* m_panel_3;
	wxSlider *m_depth_atten_factor_sldr;
	wxToolBar *m_depth_atten_reset_btn;
	wxTextCtrl *m_depth_atten_factor_text;

	//right bar///////////////////////////////////////////////////
	wxPanel* m_panel_4;
	wxToolBar *m_full_screen_btn;
	wxToolBar *m_pin_btn;
	wxToolBar *m_center_btn;
	wxToolBar *m_scale_121_btn;
	wxSlider *m_scale_factor_sldr;
	wxTextCtrl *m_scale_factor_text;
	wxToolBar *m_scale_mode_btn;
	wxToolBar *m_scale_reset_btn;
	wxSpinButton* m_scale_factor_spin;

	//draw clip
	bool m_draw_clip;

	//draw scalebar
	enum SCALEBAR_STATE {
		kOn,
		kOff,
		kText
	};
	SCALEBAR_STATE m_draw_scalebar;

	double m_pin_scale_thresh;//scale factor theshold value for auto update
	//rot slider style
	bool m_rot_slider;

	bool m_use_dft_settings;
	double m_dft_x_rot;
	double m_dft_y_rot;
	double m_dft_z_rot;
	double m_dft_depth_atten_factor;
	double m_dft_scale_factor;
	bool m_dft_scale_factor_mode;

private:
	//called when updated from bars
	void CreateBar();
	//load default settings from file
	void LoadSettings();

	//update
	void UpdateView(bool ui_update=true);

	//bar top
	void OnVolumeMethodCheck(wxCommandEvent& event);
	void OnCh1Check(wxCommandEvent &event);
	void OnChAlphaCheck(wxCommandEvent &event);
	void OnChFloatCheck(wxCommandEvent &event);
	void OnChEmbedCheck(wxCommandEvent &event);
	void OnChEnlargeCheck(wxCommandEvent &event);
	void OnSlEnlargeScroll(wxScrollEvent &event);
	void OnTxEnlargeText(wxCommandEvent &event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
	void OnCapture(wxCommandEvent& event);
	void OnBgColorChange(wxColourPickerEvent& event);
	void OnBgInvBtn(wxCommandEvent& event);
	void OnCamCtrCheck(wxCommandEvent& event);
	void OnFpsCheck(wxCommandEvent& event);
	void OnLegendCheck(wxCommandEvent& event);
	void OnColormapCheck(wxCommandEvent& event);
	void OnScaleBar(wxCommandEvent& event);
	void OnScaleTextEditing(wxCommandEvent& event);
	void OnScaleUnitSelected(wxCommandEvent& event);
	void OnAovSldrIdle(wxIdleEvent& event);
	void OnAovChange(wxScrollEvent& event);
	void OnAovText(wxCommandEvent &event);
	void OnFreeChk(wxCommandEvent& event);
	void OnFullScreen(wxCommandEvent& event);
	//bar left
	void OnDepthAttenCheck(wxCommandEvent& event);
	void OnDepthAttenFactorChange(wxScrollEvent& event);
	void OnDepthAttenFactorEdit(wxCommandEvent& event);
	void OnDepthAttenReset(wxCommandEvent &event);
	//bar right
	void OnPin(wxCommandEvent &event);
	void OnCenter(wxCommandEvent &event);
	void OnScale121(wxCommandEvent &event);
	void OnScaleFactorChange(wxScrollEvent& event);
	void OnScaleFactorEdit(wxCommandEvent& event);
	void OnScaleMode(wxCommandEvent& event);
	void OnScaleReset(wxCommandEvent &event);
	void OnScaleFactorSpinUp(wxSpinEvent& event);
	void OnScaleFactorSpinDown(wxSpinEvent& event);
	//bar bottom
	void OnRotReset(wxCommandEvent &event);
	void OnValueEdit(wxCommandEvent& event);
	void OnXRotScroll(wxScrollEvent &event);
	void OnYRotScroll(wxScrollEvent &event);
	void OnZRotScroll(wxScrollEvent &event);
	void OnRotLockCheck(wxCommandEvent& event);
	void OnRotSliderType(wxCommandEvent& event);
	void OnOrthoViewSelected(wxCommandEvent& event);

	void OnSaveDefault(wxCommandEvent &event);

	void OnKeyDown(wxKeyEvent& event);
	//idle
	void OnTimer(wxTimerEvent& event);

	DECLARE_EVENT_TABLE()

};

#endif//_VRENDERVIEW_H_
