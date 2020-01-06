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

#ifndef _VRENDERGLVIEW_H_
#define _VRENDERGLVIEW_H_

#include "DataManager.h"
#include "VolumeLoader.h"
#include "utility.h"
#include "VolumeSelector.h"
#include "KernelExecutor.h"
#include "Calculate/VolumeCalculator.h"
#include <Distance/Ruler.h>
#include <Distance/RulerHandler.h>
#include "FLIVR/Color.h"
#include "FLIVR/ShaderProgram.h"
#include "FLIVR/KernelProgram.h"
#include "FLIVR/BBox.h"
#include "FLIVR/MultiVolumeRenderer.h"
#include "FLIVR/Quaternion.h"
#include "FLIVR/ImgShader.h"
#include "FLIVR/VolKernel.h"
#include <FLIVR/TextRenderer.h>
#include "compatibility.h"

#include <wx/wx.h>
#include <wx/clrpicker.h>
#include <wx/spinbutt.h>
#include <wx/glcanvas.h>
#include <wx/event.h>
#include <wx/timer.h>

#include <vector>
#include <stdarg.h>
#include "NV/Timer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _WIN32
//#include <Windows.h>
//wacom support
#include <wx/msw/private.h>
#include <MSGPACK.h>
#include <wintab.h>
#define PACKETDATA	(/*PK_X | PK_Y | */PK_BUTTONS |\
		PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include <PKTDEF.h>
#include <Wacom/Utils.h>
#include <XInput/XboxController.h>
#endif

#include <openvr.h>

#define VOL_METHOD_SEQ    1
#define VOL_METHOD_MULTI  2
#define VOL_METHOD_COMP    3

#define INIT_BOUNDS  1
#define INIT_CENTER  2
#define INIT_TRANSL  4
#define INIT_ROTATE  8
#define INIT_OBJ_TRANSL  16

//clipping plane mask
#define CLIP_X1  1
#define CLIP_X2  2
#define CLIP_Y1  4
#define CLIP_Y2  8
#define CLIP_Z1  16
#define CLIP_Z2  32
//clipping plane winding
#define CULL_OFF  0
#define FRONT_FACE  1
#define BACK_FACE  2

//information bits
#define INFO_DISP	1
#define INFO_POS	2
#define INFO_FRAME	4
#define INFO_FPS	8
#define INFO_T		16
#define INFO_X		32
#define INFO_Y		64
#define INFO_Z		128

#define ID_ftrigger	ID_VRENDER_VIEW1

using namespace std;

class VRenderView;
class VRenderFrame;
class VRenderGLView : public wxGLCanvas
{
public:
	VRenderGLView(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		const wxGLAttributes& attriblist,
		wxGLContext* sharedContext = 0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	~VRenderGLView();

	//for degugging, this allows inspection of the pixel format actually given.
#ifdef _WIN32
	int GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd);
#endif
	wxString GetOGLVersion();
	//initialization
	void Init();

	//Clear all layers
	void Clear();

	//recalculate view according to object bounds
	void InitView(unsigned int type = INIT_BOUNDS);

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
	MultiVolumeRenderer* GetMultiVolumeData() { return m_mvr; };
	VolumeData* GetVolumeData(wxString &name);
	MeshData* GetMeshData(wxString &name);
	Annotations* GetAnnotations(wxString &name);
	DataGroup* GetGroup(wxString &name);
	DataGroup* GetGroup(int index);
	MeshGroup* GetMGroup(wxString str);
	//add
	DataGroup* AddVolumeData(VolumeData* vd, wxString group_name = "");
	void AddMeshData(MeshData* md);
	void AddAnnotations(Annotations* ann);
	wxString AddGroup(wxString str, wxString prev_group = "");
	DataGroup* AddOrGetGroup();
	wxString AddMGroup(wxString str);
	MeshGroup* AddOrGetMGroup();
	//remove
	void RemoveVolumeData(wxString &name);
	void RemoveVolumeDataDup(wxString &name);//remove all duplicated data
	void ReplaceVolumeData(wxString &name, VolumeData *dst);
	void RemoveMeshData(wxString &name);
	void RemoveAnnotations(wxString &name);
	void RemoveGroup(wxString &name);
	//isolate
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
	//
	void PopVolumeList();
	void PopMeshList();
	//reorganize layers in view
	void OrganizeLayers();
	//randomize color
	void RandomizeColor();

	//toggle hiding/displaying
	void SetDraw(bool draw);
	void ToggleDraw();
	bool GetDraw();

	//camera operations
	void GetTranslations(double &transx, double &transy, double &transz)
	{
		transx = m_transx; transy = m_transy; transz = m_transz;
	}
	void SetTranslations(double transx, double transy, double transz)
	{
		m_transx = transx; m_transy = transy; m_transz = transz;
		m_distance = sqrt(m_transx*m_transx + m_transy*m_transy + m_transz*m_transz);
	}
	void GetRotations(double &rotx, double &roty, double &rotz)
	{
		rotx = m_rotx;
		roty = m_roty;
		rotz = m_rotz;
	}
	void SetRotations(double rotx, double roty, double rotz, bool ui_update = true);
	void SetZeroRotations();
	void ResetZeroRotations(double &rotx, double &roty, double &rotz);
	void GetCenters(double &ctrx, double &ctry, double &ctrz)
	{
		ctrx = m_ctrx; ctry = m_ctry; ctrz = m_ctrz;
	}
	void SetCenters(double ctrx, double ctry, double ctrz)
	{
		m_ctrx = ctrx; m_ctry = ctry; m_ctrz = ctrz;
	}
	double GetCenterEyeDist()
	{
		return m_distance;
	}
	void SetCenterEyeDist(double dist)
	{
		m_distance = dist;
	}
	double GetInitDist()
	{
		return m_init_dist;
	}
	void SetInitDist(double dist)
	{
		m_init_dist = dist;
	}
	double GetRadius()
	{
		return m_radius;
	}
	void SetRadius(double r);
	void SetCenter();
	double Get121ScaleFactor();
	void SetScale121();
	void SetPinRotCenter(bool);

	//object operations
	void GetObjCenters(double &ctrx, double &ctry, double &ctrz)
	{
		ctrx = m_obj_ctrx;
		ctry = m_obj_ctry;
		ctrz = m_obj_ctrz;
	}
	void SetObjCenters(double ctrx, double ctry, double ctrz)
	{
		m_obj_ctrx = ctrx;
		m_obj_ctry = ctry;
		m_obj_ctrz = ctrz;
	}
	void GetObjTrans(double &transx, double &transy, double &transz)
	{
		transx = m_obj_transx;
		transy = m_obj_transy;
		transz = m_obj_transz;
	}
	void SetObjTrans(double transx, double transy, double transz)
	{
		m_obj_transx = transx;
		m_obj_transy = transy;
		m_obj_transz = transz;
	}
	void GetObjRot(double &rotx, double &roty, double &rotz)
	{
		rotx = m_obj_rotx;
		roty = m_obj_roty;
		rotz = m_obj_rotz;
	}
	void SetObjRot(double rotx, double roty, double rotz)
	{
		m_obj_rotx = rotx;
		m_obj_roty = roty;
		m_obj_rotz = rotz;
	}
	void SetRotLock(bool mode)
	{
		m_rot_lock = mode;
	}
	bool GetRotLock()
	{
		return m_rot_lock;
	}
	//camera properties
	bool GetPersp() { return m_persp; }
	void SetPersp(bool persp = true);
	bool GetFree() { return m_free; }
	void SetFree(bool free = true);
	double GetAov() { return m_aov; }
	void SetAov(double aov);
	double GetNearClip() { return m_near_clip; }
	void SetNearClip(double nc) { m_near_clip = nc; }
	double GetFarClip() { return m_far_clip; }
	void SetFarClip(double fc) { m_far_clip = fc; }

	//background color
	Color GetBackgroundColor();
	Color GetTextColor();
	void SetBackgroundColor(Color &color);
	void SetGradBg(bool val);

	//disply modes
	int GetDrawType() { return m_draw_type; }
	void SetVolMethod(int method);
	int GetVolMethod() { return m_vol_method; }
	void SetFog(bool b = true) { m_use_fog = b; }
	bool GetFog() { return m_use_fog; }
	void SetFogIntensity(double i) { m_fog_intensity = i; }
	double GetFogIntensity() { return m_fog_intensity; }
	void SetPeelingLayers(int n) { m_peeling_layers = n; }
	int GetPeelingLayers() { return m_peeling_layers; }
	void SetBlendSlices(bool val) { m_blend_slices = val; }
	bool GetBlendSlices() { return m_blend_slices; }
	void SetAdaptive(bool val) { m_adaptive = val; }
	bool GetAdaptive() { return m_adaptive; }

	//movie capture
	void Set3DRotCapture(double start_angle,
		double end_angle,
		double step,
		int frames,
		int rot_axis,
		wxString &cap_file,
		bool rewind,
		int len);
	//time sequence data
	void Set4DSeqCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind);
	//batch files
	void Set3DBatCapture(wxString &cap_file, int begin_frame, int end_frame);
	//parameter recording/capture
	void SetParamCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind);
	//set parameters
	void SetParams(double t);
	//reset and stop
	void ResetMovieAngle();
	void StopMovie();
	//4d movie frame calculation
	void Get4DSeqFrames(int &start_frame, int &end_frame, int &cur_frame);
	void Set4DSeqFrame(int frame, bool run_script);
	void UpdateVolumeData(int frame, bool run_script,
		VolumeData* vd, VRenderFrame* vframe);
	//3d batch file calculation
	void Get3DBatFrames(int &start_frame, int &end_frame, int &cur_frame);
	void Set3DBatFrame(int offset);

	//frame for capturing
	void EnableFrame() { m_draw_frame = true; }
	void DisableFrame() { m_draw_frame = false; };
	void SetFrame(int x, int y, int w, int h) {
		m_frame_x = x;
		m_frame_y = y; m_frame_w = w; m_frame_h = h;
	}
	void GetFrame(int &x, int &y, int &w, int &h);
	void CalcFrame();

	//scale bar
	void EnableScaleBar() { m_disp_scale_bar = true; }
	void DisableScaleBar() { m_disp_scale_bar = false; }
	void EnableSBText() { m_disp_scale_bar_text = true; }
	void DisableSBText() { m_disp_scale_bar_text = false; }
	void SetScaleBarLen(double len) { m_sb_length = len; }
	void SetSBText(wxString text) { m_sb_text = text; }

	//gamma settings
	Color GetGamma() { return m_gamma; }
	void SetGamma(Color gamma) { m_gamma = gamma; }
	//brightness adjustment
	Color GetBrightness() { return m_brightness; }
	void SetBrightness(Color brightness) { m_brightness = brightness; }
	//hdr settings
	Color GetHdr() { return m_hdr; }
	void SetHdr(Color hdr) { m_hdr = hdr; }
	//sync values
	bool GetSyncR() { return m_sync_r; }
	void SetSyncR(bool sync_r) { m_sync_r = sync_r; }
	bool GetSyncG() { return m_sync_g; }
	void SetSyncG(bool sync_g) { m_sync_g = sync_g; }
	bool GetSyncB() { return m_sync_b; }
	void SetSyncB(bool sync_b) { m_sync_b = sync_b; }

	//reload volume list
	void SetVolPopDirty() { m_vd_pop_dirty = true; }
	//reload mesh list
	void SetMeshPopDirty() { m_md_pop_dirty = true; }
	//clear
	void ClearVolList();
	void ClearMeshList();

	//inteactive mode selection
	int GetIntMode();
	void SetIntMode(int mode);

	//set use 2d rendering results
	void SetPaintUse2d(bool use2d);
	bool GetPaintUse2d();

	//segmentation mode selection
	void SetPaintMode(int mode);
	int GetPaintMode();

	//calculations
	void SetVolumeA(VolumeData* vd);
	void SetVolumeB(VolumeData* vd);
	//1-sub;2-add;3-div;4-and;5-new;6-new inv;7-clear
	void Calculate(int type, wxString prev_group = "", bool add = true);
	void CalculateSingle(int type, wxString prev_group, bool add);

	//set 2d weights
	void Set2dWeights();
	//segment volumes in current view
	void Segment();

	//brush properties
	//load default;
	void LoadBrushSettings();
	void SaveBrushSettings();
	//use pressure
	void SetBrushUsePres(bool pres);
	bool GetBrushUsePres();
	//set brush size
	void SetUseBrushSize2(bool val);
	bool GetUseBrushSize2();
	void SetBrushSize(double size1, double size2);
	double GetBrushSize1();
	double GetBrushSize2();
	//set brush spacing
	void SetBrushSpacing(double spacing);
	double GetBrushSpacing();
	//set iteration number
	void SetBrushIteration(int num);
	int GetBrushIteration();
	//set brush size relation
	void SetBrushSizeData(bool val);
	bool GetBrushSizeData();
	//set translate
	void SetBrushSclTranslate(double val);
	double GetBrushSclTranslate();
	//set gm falloff
	void SetBrushGmFalloff(double val);
	double GetBrushGmFalloff();
	//change display
	void ChangeBrushSize(int value);
	//w2d
	void SetW2d(double val);
	double GetW2d();
	//edge detect
	void SetEdgeDetect(bool value);
	bool GetEdgeDetect();
	//hidden removal
	void SetHiddenRemoval(bool value);
	bool GetHiddenRemoval();
	//select group
	void SetSelectGroup(bool value);
	bool GetSelectGroup();
	//estimate threshold
	void SetEstimateThresh(bool value);
	bool GetEstimateThresh();
	//brick acuracy
	void SetAccurateBricks(bool value);
	bool GetAccurateBricks();
	//select both
	void SetSelectBoth(bool value);
	bool GetSelectBoth();

	//set clip mode
	void SetClipMode(int mode);
	int GetClipMode() { return m_clip_mode; }
	//restore clipping planes
	void RestorePlanes();
	//clipping plane rotations
	void SetClippingPlaneRotations(double rotx, double roty, double rotz);
	void GetClippingPlaneRotations(double &rotx, double &roty, double &rotz);

	//interpolation
	void SetIntp(bool mode);
	bool GetIntp();

	//get volume selector
	VolumeSelector* GetVolumeSelector() { return &m_selector; }
	//get volume calculator
	FL::VolumeCalculator* GetVolumeCalculator() { return &m_calculator; }
	//get kernel executor
	KernelExecutor* GetKernelExecutor() { return &m_kernel_executor; }

	//force draw
	void ForceDraw();

	//run 4d script
	void SetRun4DScript(bool runscript) { m_run_script = runscript; }
	bool GetRun4DScript() { return m_run_script; }

	//start loop update
	void StartLoopUpdate();
	void HaltLoopUpdate();
	void RefreshGL(int debug_code, bool erase = false, bool start_loop = true);

	//rulers
	void DrawRulerPoint(vector<float> &verts, int type, float px, float py, float w, Color &c);
	void DrawRulerArc(Point & ppc, Point& pp0, Point& pp1,
		Color &c, Transform& mv, Transform& p,
		vector<float> &verts, unsigned int& num);
	unsigned int DrawRulersVerts(vector<float> &verts);
	void DrawRulers();
	FL::RulerList* GetRulerList();
	FL::Ruler* GetRuler(unsigned int id);
	int RulerProfile(int index);
	int RulerDistance(int index);

	//draw highlighted comps
	void DrawCells();
	unsigned int DrawCellVerts(vector<float>& verts);
	void GetCellPoints(BBox& box,
		Point& p1, Point& p2, Point& p3, Point& p4,
		Transform& mv, Transform& p);

	//public mouse
	void OnMouse(wxMouseEvent& event);

	//traces
	TraceGroup* GetTraceGroup();
	void CreateTraceGroup();
	int LoadTraceGroup(wxString filename);
	int SaveTraceGroup(wxString filename);
	void ExportTrace(wxString filename, unsigned int id);
	void DrawTraces();
	void GetTraces(bool update = false);

	//enlarge output image
	static void SetEnlarge(bool value)
	{
		m_enlarge = value;
	}
	static void SetEnlargeScale(double value)
	{
		m_enlarge_scale = value;
		if (m_enlarge)
		{
			m_tsize = TextRenderer::text_texture_manager_.GetSize();
			TextRenderer::text_texture_manager_.SetSize(m_tsize * m_enlarge_scale);
		}
	}

	//stereo/vr
	void SetStereo(bool bval)
	{
		m_enable_vr = bval;
	}
	void SetEyeDist(double dval)
	{
		m_vr_eye_offset = dval / 2.0;
	}

	//read pixels
	void ReadPixels(
		int chann, bool fp32,
		int &x, int &y, int &w, int &h,
		void** image);

	//set cell list
	void SetCellList(FL::CellList &list)
	{
		m_cell_list = list;
	}

	//get mouse point in 3D
	//mode: 0-maximum with original value; 1-maximum with transfered value; 2-accumulated with original value; 3-accumulated with transfered value
	double GetPointVolume(Point &mp, Point &ip, double mx, double my, VolumeData* vd, int mode, bool use_transf, double thresh = 0.5);
	double GetPointVolumeBox(Point &mp, double mx, double my, VolumeData* vd, bool calc_mats = true);
	double GetPointVolumeBox2(Point &p1, Point &p2, double mx, double my);
	double GetPointPlane(Point &mp, double mx, double my, Point *planep = 0, bool calc_mats = true);

	//get view info for external ops
	//get size, considering enlargement
	wxSize GetGLSize();
	glm::mat4 GetModelView()
	{
		return m_mv_mat;
	}
	glm::mat4 GetProjection()
	{
		return m_proj_mat;
	}
	glm::mat4 GetObjectMat()
	{
		glm::mat4 obj_mat = m_mv_mat;
		//translate object
		obj_mat = glm::translate(obj_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		obj_mat = glm::translate(obj_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));
		return obj_mat;
	}
	glm::mat4 GetDrawMat()
	{
		glm::mat4 drw_mat = m_mv_mat;
		//translate object
		drw_mat = glm::translate(drw_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		drw_mat = glm::translate(drw_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));
		return drw_mat;
	}

public:
	//set gl context
	bool m_set_gl;
	//script run
	bool m_run_script;
	wxString m_script_file;
	//capture modes
	bool m_capture;
	bool m_capture_rotat;
	bool m_capture_rotate_over;
	bool m_capture_tsequ;
	bool m_capture_bat;
	bool m_capture_param;
	//begin and end frame
	int m_begin_frame;
	int m_end_frame;
	//counters
	int m_tseq_cur_num;
	int m_tseq_prv_num;
	int m_param_cur_num;
	int m_total_frames;
	//file name for capturing
	wxString m_cap_file;
	//folder name for 3d batch
	wxString m_bat_folder;
	//hud
	bool m_retain_finalbuffer;	//sometimes we don't redraw everything,
								//just use the final buffer from last draw
	bool m_updating;
	bool m_draw_annotations;
	bool m_draw_camctr;
	double m_camctr_size;
	int m_draw_info;
	bool m_load_update;
	bool m_draw_frame;
	bool m_test_speed;
	bool m_draw_clip;
	bool m_draw_legend;
	bool m_draw_colormap;
	bool m_mouse_focus;
	bool m_test_wiref;
	bool m_draw_bounds;
	bool m_draw_grid;
	bool m_draw_rulers;
	//current volume
	VolumeData *m_cur_vol;
	//clipping settings
	int m_clip_mask;
	int m_clip_mode;//0-normal; 1-ortho planes; 2-rot difference
					//scale bar
	bool m_disp_scale_bar;
	bool m_disp_scale_bar_text;
	double m_sb_length;
	int m_sb_unit;
	wxString m_sb_text;
	wxString m_sb_num;
	double m_sb_height;
	//ortho size
	double m_ortho_left;
	double m_ortho_right;
	double m_ortho_bottom;
	double m_ortho_top;
	//scale factor
	double m_scale_factor;
	double m_scale_factor_saved;
	//scale mode
	bool m_scale_mode;
	//pin rotation center
	bool m_auto_update_rot_center;
	bool m_pin_rot_center;
	bool m_rot_center_dirty;
	double m_pin_pick_thresh;//ray casting threshold value
	Point m_pin_ctr;
	//mode in determining depth of volume
	int m_point_volume_mode;  //0: use view plane; 1: use max value; 2: use accumulated value
							  //ruler use volume transfer function
	bool m_ruler_use_transf;
	//ruler time dependent
	bool m_ruler_time_dep;
	//linked rotation
	static bool m_linked_rot;
	static VRenderGLView* m_master_linked_view;
	//count after paint
	bool m_paint_count;
	//colocalize after paint
	bool m_paint_colocalize;

private:
	bool m_drawing;
	bool m_refresh;//own refresh command
	wxSize m_size;
	wxString m_GLversion;
	wxGLContext* m_glRC;
	bool m_sharedRC;
	wxWindow* m_frame;
	VRenderView* m_vrv;
	//populated lists of data
	bool m_vd_pop_dirty;
	vector <VolumeData*> m_vd_pop_list;
	bool m_md_pop_dirty;
	vector <MeshData*> m_md_pop_list;
	//real data list
	vector <TreeLayer*> m_layer_list;
	//ruler list
	FL::RulerList m_ruler_list;
	//grow point
	Point m_grow_point;
	//traces
	TraceGroup* m_trace_group;
	//multivolume
	MultiVolumeRenderer* m_mvr;
	//highlighted comps
	FL::CellList m_cell_list;
	//fisrt volume data in the depth groups
	//VolumeData* m_first_depth_vd;
	//initializaion
	bool m_initialized;
	bool m_init_view;
	//bg color
	Color m_bg_color;
	Color m_bg_color_inv;
	bool m_grad_bg;
	//frustrum
	double m_aov;
	double m_near_clip;
	double m_far_clip;
	//interpolation
	bool m_intp;
	//previous focus before brush
	wxWindow* m_prev_focus;

	//interactive modes
	int m_int_mode;  //interactive mode
					 //1-normal viewing
					 //2-painting
					 //3-rotate clipping planes
					 //4-one-time rendering update in painting mode
					 //5-ruler mode
					 //6-edit ruler
					 //7-paint ruler mode
					 //8-same as 4, but for paint ruler mode
					 //9-move ruler
					 //10-grow, click and hold to activate
					 //11-lock ruler point for relaxing
	bool m_force_clear;
	bool m_interactive;
	bool m_clear_buffer;
	bool m_adaptive;
	int m_brush_state;  //sets the button state of the tree panel
						//0-not set
						//2-append
						//3-erase
						//4-diffuse
						//8-solid

						//resizing
	bool m_resize;
	//brush tools
	bool m_draw_brush;
	bool m_paint_enable;
	bool m_paint_display;
	//2d frame buffers
	GLuint m_cur_framebuffer;
	//paint buffer
	bool m_clear_paint;

	//camera controls
	bool m_persp;
	bool m_free;
	//camera distance
	double m_distance;
	double m_init_dist;
	//camera translation
	double m_transx, m_transy, m_transz;
	//saved camera trans
	double m_transx_saved, m_transy_saved, m_transz_saved;
	//camera rotation
	double m_rotx, m_roty, m_rotz;
	//saved camera rotation
	double m_rotx_saved, m_roty_saved, m_rotz_saved;
	//zero camera rotation
	double m_zrotx, m_zroty, m_zrotz;
	//camera center
	double m_ctrx, m_ctry, m_ctrz;
	//saved camera center
	double m_ctrx_saved, m_ctry_saved, m_ctrz_saved;
	FLIVR::Quaternion m_q;
	FLIVR::Quaternion m_zq;//zero rotation
	Vector m_up;
	Vector m_head;

	//object center
	double m_obj_ctrx, m_obj_ctry, m_obj_ctrz;
	//object translation
	double m_obj_transx, m_obj_transy, m_obj_transz;
	//saved translation for free flight
	double m_obj_transx_saved, m_obj_transy_saved, m_obj_transz_saved;
	//object rotation
	double m_obj_rotx, m_obj_roty, m_obj_rotz;
	//rotation lock
	bool m_rot_lock;

	//object bounding box
	BBox m_bounds;
	double m_radius;

	//mouse position
	long old_mouse_X, old_mouse_Y;
	long prv_mouse_X, prv_mouse_Y;

	//draw controls
	bool m_draw_all;
	int m_draw_type;
	int m_vol_method;
	int m_peeling_layers;
	bool m_blend_slices;

	//fog
	bool m_use_fog;
	double m_fog_intensity;
	double m_fog_start;
	double m_fog_end;

	//movie properties
	double m_init_angle;
	double m_start_angle;
	double m_end_angle;
	double m_cur_angle;
	double m_step;
	int m_rot_axis; //1-X; 2-Y; 3-Z;
	int m_movie_seq;
	bool m_rewind;
	int m_fr_length;
	int m_stages; //0-moving to start angle; 1-moving to end; 2-rewind
	bool m_4d_rewind;

	//movie frame properties
	int m_frame_x;
	int m_frame_y;
	int m_frame_w;
	int m_frame_h;

	//post image processing
	Color m_gamma;
	Color m_brightness;
	Color m_hdr;
	bool m_sync_r;
	bool m_sync_g;
	bool m_sync_b;

	//volume color map
	//double m_value_1;
	Color m_color_1;
	double m_value_2;
	Color m_color_2;
	double m_value_3;
	Color m_color_3;
	double m_value_4;
	Color m_color_4;
	double m_value_5;
	Color m_color_5;
	double m_value_6;
	Color m_color_6;
	//double m_value_7;
	Color m_color_7;

	//paint brush use pressure
	bool m_use_press;
	bool m_on_press;
	double m_pressure;
	double m_press_peak;
	double m_press_nmax;
	double m_press_tmax;
	//air brush
	double m_air_press;
	//paint stroke radius
	double m_brush_radius1;
	double m_brush_radius2;
	bool m_use_brush_radius2;
	//radius settings for individual brush types
	typedef struct
	{
		int type;//brush type
		double radius1;//radius 1
		double radius2;//radius 2
		bool use_radius2;//use radius 2
	} BrushRadiusSet;
	vector<BrushRadiusSet> m_brush_radius_sets;
	int m_brush_sets_index;
	//paint stroke spacing
	double m_brush_spacing;
	//brush size relation
	bool m_brush_size_data;

	//clipping plane rotations
	FLIVR::Quaternion m_q_cl;
	FLIVR::Quaternion m_q_cl_zero;
	double m_rotx_cl, m_roty_cl, m_rotz_cl;

	//volume selector for segmentation
	VolumeSelector m_selector;

	//calculator
	FL::VolumeCalculator m_calculator;

	//kernel executor
	KernelExecutor m_kernel_executor;

	//timer
	nv::Timer *m_timer;

	//timer for full screen
	wxTimer m_fullscreen_trigger;

	//wacom support
#ifdef _WIN32
	static HCTX m_hTab;
	static LOGCONTEXTA m_lc;
#endif

	//for selection
	bool m_pick;

	//draw mask
	bool m_draw_mask;
	bool m_clear_mask;
	bool m_save_mask;

	//move view
	bool m_move_left;
	bool m_move_right;
	bool m_move_up;
	bool m_move_down;
	//move time
	bool m_tseq_forward;
	bool m_tseq_backward;
	//move clip
	bool m_clip_up;
	bool m_clip_down;
	//full cell
	bool m_cell_full;
	//link cell
	bool m_cell_link;
	//new cell id
	bool m_cell_new_id;
	//comp include
	bool m_comp_include;
	//comp exclude
	bool m_comp_exclude;
	//relax ruler
	bool m_ruler_relax;

	//predraw in streaming mode
	bool m_pre_draw;

	//glm matrices
	glm::mat4 m_mv_mat;
	glm::mat4 m_proj_mat;
	glm::mat4 m_tex_mat;

	//text renderer
	FLIVR::TextRenderer m_text_renderer;
	static unsigned int m_tsize;

	//enlargement
	static bool m_enlarge;
	static double m_enlarge_scale;

	//for benchmark
	bool m_benchmark;

	//nodraw count
	int m_nodraw_count;

	VolumeLoader m_loader;
	bool m_load_in_main_thread;

	int m_res_mode;

	//touch pointer ids
	bool m_enable_touch;
#ifdef _WIN32
	decltype(GetPointerInfo)* GetPI;
#endif
	int m_ptr_id1;
	int m_ptr1_x;
	int m_ptr1_y;
	int m_ptr1_x_save;
	int m_ptr1_y_save;
	int m_ptr_id2;
	int m_ptr2_x;
	int m_ptr2_y;
	int m_ptr2_x_save;
	int m_ptr2_y_save;

	//is full screen
	bool m_full_screen;

	//file path for script
	wxString m_script_output;
	//selected labels
	FL::CellList m_sel_labels;

	//enable vr
	bool m_enable_vr;
	bool m_use_openvr;
	uint32_t m_vr_size[2];
	double m_vr_eye_offset;
	int m_vr_eye_idx;//0: left; 1: right
#ifdef _WIN32
	vr::IVRSystem *m_vr_system;
	XboxController* m_controller;
#endif

	//handle rulers
	FL::RulerHandler m_ruler_handler;

private:
#ifdef _WIN32
	//wacom tablet
	HCTX TabletInit(HWND hWnd, HINSTANCE hInst);
	void InitOpenVR();
#endif

	void DrawBounds();
	void DrawGrid();
	void DrawCamCtr();
	void DrawScaleBar();
	void DrawLegend();
	void DrawName(double x, double y, int nx, int ny,
		wxString name, Color color,
		double font_height, bool hilighted = false);
	void DrawFrame();
	void DrawClippingPlanes(bool border, int face_winding);
	void SetColormapColors(int colormap, Color &c, double inv);
	void DrawColormap();
	void DrawGradBg();
	void DrawInfo(int nx, int ny);

	//depth
	double CalcZ(double z);
	void CalcFogRange();
	//different modes
	void Draw();//draw volumes only
	void DrawDP();//draw volumes and meshes with depth peeling
				  //mesh and volume
	void DrawMeshes(int peel = 0);//0: no dp
								  //1: draw depth after 15 (15)
								  //2: draw mesh after 14 (14, 15)
								  //3: draw mesh after 13 and before 15 (13, 14, 15)
								  //4: draw mesh before 15 (at 14) (14, 15)
								  //5: draw mesh at 15 (15)
	void DrawVolumes(int peel = 0);//0: no dep
								   //1: draw volume befoew 15 (15)
								   //2: draw volume after 15 (14, 15)
								   //3: draw volume after 14 and before 15 (13, 14, 15)
								   //4: same as 3 (14, 15)
								   //5: same as 2 (15)
								   //annotation layer
	void DrawAnnotations();
	//framebuffer
	void BindRenderBuffer();
	void GetRenderSize(int &nx, int &ny);
	//draw out the framebuffer after composition
	void PrepFinalBuffer();
	void ClearFinalBuffer();
	void DrawFinalBuffer();
	//vr buffers
	void PrepVRBuffer();
	void ClearVRBuffer();
	void DrawVRBuffer();
	//different volume drawing modes
	void DrawVolumesMulti(vector<VolumeData*> &list, int peel = 0);
	void DrawVolumesComp(vector<VolumeData*> &list, bool mask = false, int peel = 0);
	void DrawMIP(VolumeData* vd, int peel = 0);
	void DrawOVER(VolumeData* vd, bool mask, int peel = 0);
	//overlay passes
	void DrawOLShading(VolumeData* vd);
	void DrawOLShadows(vector<VolumeData*> &vlist);
	void DrawOLShadowsMesh(double darkenss);

	//get mesh shadow
	bool GetMeshShadow(double &val);

	//painting
	void DrawCircles(
		double cx, double cy,
		double r1, double r2,
		Color &color,
		glm::mat4 &matrix);
	void DrawBrush();
	void PaintStroke();
	void DisplayStroke();

	//handle camera
	void HandleProjection(int nx, int ny, bool vr = false);
	void HandleCamera(bool vr = false);
	Quaternion Trackball(double dx, double dy);
	Quaternion TrackballClip(int p1x, int p1y, int p2x, int p2y);
	void Q2A();
	void A2Q();
	//sort bricks after the view has been changed
	void SetSortBricks();

	//pre draw and post draw
	void PreDraw();
	void PostDraw();
	void ResetEnlarge();

	//run 4d script
	//index: 0-pre-change; 1-post-change
	void Run4DScript(int index, wxString &scriptname);
	void RunNoiseReduction(int index, wxFileConfig &fconfig);
	void RunSelectionTracking(int index, wxFileConfig &fconfig);
	void RunSparseTracking(int index, wxFileConfig &fconfig);
	void RunRandomColors(int index, wxFileConfig &fconfig);
	void RunFetchMask(int index, wxFileConfig &fconfig);
	void RunSaveMask(int index, wxFileConfig &fconfig);
	void RunSaveVolume(int index, wxFileConfig &fconfig);
	void RunCalculate(int index, wxFileConfig &fconfig);
	void RunOpenCL(int index, wxFileConfig &fconfig);
	void RunCompAnalysis(int index, wxFileConfig &fconfig);
	void RunGenerateComp(int index, wxFileConfig &fconfig);
	void RunRulerProfile(int index, wxFileConfig &fconfig);
	void RunAddCells(int index, wxFileConfig &fconfig);
	void RunLinkCells(int index, wxFileConfig &fconfig);
	void RunUnlinkCells(int index, wxFileConfig &fconfig);

	//read/delete volume cache
	//for sparse tracking
	void ReadVolCache(FL::VolCache& vol_cache);
	void DelVolCache(FL::VolCache& vol_cache);

	//brush states update
	void SetBrush(int mode);
	void UpdateBrushState();

	//selection
	void Pick();
	void PickMesh();
	void PickVolume();
	void SetCompSelection(VolumeData* vd, Point& p, int mode);//node: 0-exclusive; 1-add or remove

	//brush sets
	void ChangeBrushSetsIndex();

	//system call
	void OnDraw(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnQuitFscreen(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);
#ifdef _WIN32
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

	//draw quad
	void DrawViewQuad();

	void switchLevel(VolumeData *vd);

	DECLARE_EVENT_TABLE()

	friend class VRenderView;
};

inline wxSize VRenderGLView::GetGLSize()
{
	wxSize size = GetSize();
	if (m_enlarge)
		size.Set(size.x * m_enlarge_scale,
			size.y * m_enlarge_scale);
	return size;
}

#endif//_VRENDERGLVIEW_H_
