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

#ifndef _VRENDERVIEW_H_
#define _VRENDERVIEW_H_

#include "DataManager.h"
#include "VolumeLoader.h"
#include "utility.h"
#include "VolumeSelector.h"
#include "TextRenderer.h"
#include "KernelExecutor.h"
#include "Calculate/VolumeCalculator.h"

#include "FLIVR/Color.h"
#include "FLIVR/ShaderProgram.h"
#include "FLIVR/KernelProgram.h"
#include "FLIVR/BBox.h"
#include "FLIVR/MultiVolumeRenderer.h"
#include "FLIVR/Quaternion.h"
#include "FLIVR/ImgShader.h"
#include "FLIVR/VolKernel.h"
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

#ifdef _WIN32
//#include <Windows.h>
//wacom support
#include <wx/msw/private.h>
#include <MSGPACK.h>
#include <wintab.h>
#define PACKETDATA	(PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include <PKTDEF.h>
#include <Wacom/Utils.h>
#endif

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
class VRenderGLView: public wxGLCanvas
{
public:
	VRenderGLView(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		const wxGLAttributes& attriblist,
		wxGLContext* sharedContext=0,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=0);
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
	void InitView(unsigned int type=INIT_BOUNDS);

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
	MultiVolumeRenderer* GetMultiVolumeData() {return m_mvr;};
	VolumeData* GetVolumeData(wxString &name);
	MeshData* GetMeshData(wxString &name);
	Annotations* GetAnnotations(wxString &name);
	DataGroup* GetGroup(wxString &name);
	MeshGroup* GetMGroup(wxString str);
	//add
	DataGroup* AddVolumeData(VolumeData* vd, wxString group_name="");
	void AddMeshData(MeshData* md);
	void AddAnnotations(Annotations* ann);
	wxString AddGroup(wxString str, wxString prev_group="");
	DataGroup* AddOrGetGroup();
	wxString AddMGroup(wxString str);
	MeshGroup* AddOrGetMGroup();
	//remove
	void RemoveVolumeData(wxString &name);
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
	{transx = m_transx; transy = m_transy; transz = m_transz;}
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
	void SetRotations(double rotx, double roty, double rotz, bool ui_update=true);
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
	bool GetPersp() {return m_persp;}
	void SetPersp(bool persp=true);
	bool GetFree() {return m_free;}
	void SetFree(bool free=true);
	double GetAov() {return m_aov;}
	void SetAov(double aov);
	double GetNearClip() {return m_near_clip;}
	void SetNearClip(double nc) {m_near_clip = nc;}
	double GetFarClip() {return m_far_clip;}
	void SetFarClip(double fc) {m_far_clip = fc;}

	//background color
	Color GetBackgroundColor();
	Color GetTextColor();
	void SetBackgroundColor(Color &color);
	void SetGradBg(bool val);

	//disply modes
	int GetDrawType() {return m_draw_type;}
	void SetVolMethod(int method);
	int GetVolMethod() {return m_vol_method;}
	void SetFog(bool b=true) {m_use_fog = b;}
	bool GetFog() {return m_use_fog;}
	void SetFogIntensity(double i) {m_fog_intensity = i;}
	double GetFogIntensity() {return m_fog_intensity;}
	void SetPeelingLayers(int n) {m_peeling_layers = n;}
	int GetPeelingLayers() {return m_peeling_layers;}
	void SetBlendSlices(bool val) {m_blend_slices = val;}
	bool GetBlendSlices() {return m_blend_slices;}
	void SetAdaptive(bool val) {m_adaptive = val;}
	bool GetAdaptive() {return m_adaptive;}

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
	void Set4DSeqFrameVd(int frame, bool run_script,
		VolumeData* vd, VRenderFrame* vframe);
	//3d batch file calculation
	void Get3DBatFrames(int &start_frame, int &end_frame, int &cur_frame);
	void Set3DBatFrame(int offset);

	//frame for capturing
	void EnableFrame() {m_draw_frame = true;}
	void DisableFrame() {m_draw_frame = false;};
	void SetFrame(int x, int y, int w, int h) {m_frame_x = x;
	m_frame_y = y; m_frame_w = w; m_frame_h = h;}
	void GetFrame(int &x, int &y, int &w, int &h);
	void CalcFrame();

	//scale bar
	void EnableScaleBar() {m_disp_scale_bar = true;}
	void DisableScaleBar() {m_disp_scale_bar = false;}
	void EnableSBText() {m_disp_scale_bar_text = true;}
	void DisableSBText() {m_disp_scale_bar_text = false;}
	void SetScaleBarLen(double len) {m_sb_length = len;}
	void SetSBText(wxString text) {m_sb_text = text;}

	//gamma settings
	Color GetGamma() {return m_gamma;}
	void SetGamma(Color gamma) {m_gamma = gamma;}
	//brightness adjustment
	Color GetBrightness() {return m_brightness;}
	void SetBrightness(Color brightness) {m_brightness = brightness;}
	//hdr settings
	Color GetHdr() {return m_hdr;}
	void SetHdr(Color hdr) {m_hdr = hdr;}
	//sync values
	bool GetSyncR() {return m_sync_r;}
	void SetSyncR(bool sync_r) {m_sync_r = sync_r;}
	bool GetSyncG() {return m_sync_g;}
	void SetSyncG(bool sync_g) {m_sync_g = sync_g;}
	bool GetSyncB() {return m_sync_b;}
	void SetSyncB(bool sync_b) {m_sync_b = sync_b;}

	//reload volume list
	void SetVolPopDirty() {m_vd_pop_dirty = true;}
	//reload mesh list
	void SetMeshPopDirty() {m_md_pop_dirty = true;}
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
	void Calculate(int type, wxString prev_group="", bool add=true);
	void CalculateSingle(int type, wxString prev_group, bool add);

	//set 2d weights
	void Set2dWeights();
	//segment volumes in current view
	void Segment();
	//label volumes in current view
	void Label();
	//noise removal
	int CompAnalysis(double min_voxels, double max_voxels, double thresh, double falloff, bool select, bool gen_ann, bool size_map);
	void CompExport(int mode, bool select);//mode: 0-multi channels; 1-random colors
	void ShowAnnotations();
	int NoiseAnalysis(double min_voxels, double max_voxels, double thresh);
	void NoiseRemoval(int iter, double thresh);

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
	//select both
	void SetSelectBoth(bool value);
	bool GetSelectBoth();

	//set clip mode
	void SetClipMode(int mode);
	int GetClipMode() {return m_clip_mode;}
	//restore clipping planes
	void RestorePlanes();
	//clipping plane rotations
	void SetClippingPlaneRotations(double rotx, double roty, double rotz);
	void GetClippingPlaneRotations(double &rotx, double &roty, double &rotz);

	//interpolation
	void SetIntp(bool mode);
	bool GetIntp();

	//get volume selector
	VolumeSelector* GetVolumeSelector() {return &m_selector;}
	//get volume calculator
	FL::VolumeCalculator* GetVolumeCalculator() {return &m_calculator;}
	//get kernel executor
	KernelExecutor* GetKernelExecutor() { return &m_kernel_executor; }

	//force draw
	void ForceDraw();

	//run 4d script
	void SetRun4DScript(bool runscript) {m_run_script = runscript;}
	bool GetRun4DScript() {return m_run_script;}
	void Run4DScript();

	//start loop update
	void StartLoopUpdate();
	void HaltLoopUpdate();
	void RefreshGL(int debug_code, bool erase=false, bool start_loop=true);

	//rulers
	int GetRulerType();
	void SetRulerType(int type);
	void FinishRuler();
	bool GetRulerFinished();
	void AddRulerPoint(int mx, int my);
	void AddPaintRulerPoint();
	void DrawRulers();
	vector<Ruler*>* GetRulerList();
	Ruler* GetRuler(unsigned int id);
	int RulerProfile(int index);
	//public mouse

	void OnMouse(wxMouseEvent& event);

	//traces
	TraceGroup* GetTraceGroup();
	void CreateTraceGroup();
	int LoadTraceGroup(wxString filename);
	int SaveTraceGroup(wxString filename);
	void ExportTrace(wxString filename, unsigned int id);
	void DrawTraces();
	void GetTraces(bool update=false);

	//text renderer
	void SetTextRenderer(TextRenderer* text_renderer)
	{ m_text_renderer = text_renderer; }

	//enlarge output image
	static void SetEnlarge(bool value)
	{ m_enlarge = value; }
	static void SetEnlargeScale(double value)
	{ m_enlarge_scale = value; }

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
	bool m_updating;
	bool m_draw_annotations;
	bool m_draw_camctr;
	double m_camctr_size;
	int m_draw_info;
	bool m_load_update;
	bool m_drawing_coord;
	bool m_draw_frame;
	bool m_test_speed;
	bool m_draw_clip;
	bool m_draw_legend;
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
	bool m_pin_rot_center;
	bool m_rot_center_dirty;
	//mode in determining depth of volume
	int m_point_volume_mode;  //0: use view plane; 1: use max value; 2: use accumulated value
	//ruler use volume transfer function
	bool m_ruler_use_transf;
	//ruler time dependent
	bool m_ruler_time_dep;
	//linked rotation
	static bool m_linked_rot;
	static VRenderGLView* m_master_linked_view;

private:
	bool m_drawing;
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
	int m_ruler_type;//0: 2point ruler; 1:multi-point ruler; 2:locator
	vector <Ruler*> m_ruler_list;
	Point* m_editing_ruler_point;
	//traces
	TraceGroup* m_trace_group;
	//multivolume
	MultiVolumeRenderer* m_mvr;
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
	bool m_resize_ol1;
	bool m_resize_ol2;
	//brush tools
	bool m_draw_brush;
	bool m_paint_enable;
	bool m_paint_display;
	//2d frame buffers
	GLuint m_cur_framebuffer;
	GLuint m_fbo;
	GLuint m_tex;
	GLuint m_tex_wt2;  //use this texture instead of m_tex when the volume for segmentation is rendered
	GLuint m_fbo_final;
	GLuint m_tex_final;
	//temp buffer for large data compositing
	GLuint m_fbo_temp;
	GLuint m_tex_temp;
	//shading (shadow) overlay
	GLuint m_fbo_ol1;
	GLuint m_fbo_ol2;
	GLuint m_tex_ol1;
	GLuint m_tex_ol2;
	//paint buffer
	GLuint m_fbo_paint;
	GLuint m_tex_paint;
	bool m_clear_paint;
	//depth peeling buffers
	vector<GLuint> m_dp_fbo_list;
	vector<GLuint> m_dp_tex_list;
	//vert buffer
	GLuint m_quad_vbo, m_quad_vao;
	GLuint m_misc_vbo, m_misc_ibo, m_misc_vao;
	//mesh picking buffer
	GLuint m_fbo_pick, m_tex_pick, m_tex_pick_depth;

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
	//camera center
	double m_ctrx, m_ctry, m_ctrz;
	//saved camera center
	double m_ctrx_saved, m_ctry_saved, m_ctrz_saved;
	Quaternion m_q;
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
	static ImgShaderFactory m_img_shader_factory;
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
	bool m_use_pres;
	bool m_on_press;
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
	Quaternion m_q_cl;
	Quaternion m_q_cl_zero;
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
	HCTX m_hTab;
	LOGCONTEXTA m_lc;
#endif
	double m_pressure;

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

	//predraw in streaming mode
	bool m_pre_draw;

	//glm matrices
	glm::mat4 m_mv_mat;
	glm::mat4 m_proj_mat;
	glm::mat4 m_tex_mat;

	//text renderer
	TextRenderer* m_text_renderer;

	//starting frame for 4d script
	bool m_sf_script;

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

private:
#ifdef _WIN32
	//wacom tablet
	HCTX TabletInit(HWND hWnd, HINSTANCE hInst);
#endif

	void DrawBounds();
	void DrawGrid();
	void DrawCamCtr();
	void DrawScaleBar();
	void DrawLegend();
	void DrawName(double x, double y, int nx, int ny,
		wxString name, Color color,
		double font_height, bool hilighted=false);
	char* wxStringToChar(wxString input);
	void DrawFrame();
	void DrawClippingPlanes(bool border, int face_winding);
	void SetColormapColors(int colormap);
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
	void DrawMeshes(int peel=0);//0: no dp
	//1: draw depth after 15 (15)
	//2: draw mesh after 14 (14, 15)
	//3: draw mesh after 13 and before 15 (13, 14, 15)
	//4: draw mesh before 15 (at 14) (14, 15)
	//5: draw mesh at 15 (15)
	void DrawVolumes(int peel=0);//0: no dep
	//1: draw volume befoew 15 (15)
	//2: draw volume after 15 (14, 15)
	//3: draw volume after 14 and before 15 (13, 14, 15)
	//4: same as 3 (14, 15)
	//5: same as 2 (15)
	//annotation layer
	void DrawAnnotations();
	//draw out the framebuffer after composition
	void PrepFinalBuffer();
	void ClearFinalBuffer();
	void DrawFinalBuffer();
	//different volume drawing modes
	void DrawVolumesMulti(vector<VolumeData*> &list, int peel=0);
	void DrawVolumesComp(vector<VolumeData*> &list, bool mask=false, int peel=0);
	void DrawMIP(VolumeData* vd, GLuint tex, int peel=0);
	void DrawOVER(VolumeData* vd, GLuint tex, bool mask, int peel=0);
	//overlay passes
	void DrawOLShading(VolumeData* vd);
	void DrawOLShadows(vector<VolumeData*> &vlist, GLuint tex);
	void DrawOLShadowsMesh(GLuint tex_depth, double darkenss);

	//get mesh shadow
	bool GetMeshShadow(double &val);

	//painting
	void DrawCircle(double cx, double cy,
		double radius, Color &color, glm::mat4 &matrix);
	void DrawBrush();
	void PaintStroke();
	void DisplayStroke();

	//handle camera
	void HandleProjection(int nx, int ny);
	void HandleCamera();
	Quaternion Trackball(int p1x, int p1y, int p2x, int p2y);
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
	void Run4DScript(wxString &scriptname, VolumeData* vd=0);
	void RunNoiseReduction(wxFileConfig &fconfig);
	void RunSelectionTracking(wxFileConfig &fconfig);
	void RunSparseTracking(wxFileConfig &fconfig);
	void RunRandomColors(wxFileConfig &fconfig);
	void RunSeparateChannels(wxFileConfig &fconfig);
	void RunExternalExe(wxFileConfig &fconfig);
	void RunFetchMask(wxFileConfig &fconfig);
	void RunSaveMask(wxFileConfig &fconfig);
	void RunSaveVolume(wxFileConfig &fconfig);
	void RunCalculation(wxFileConfig &fconfig);
	void RunOpenCL(wxFileConfig &fconfig);
	void RunCompAnalysis(wxFileConfig &fconfig);
	void RunGenerateComp(wxFileConfig &fconfig);
	void RunRulerProfile(wxFileConfig &fconfig);

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

	//get mouse point in 3D
	//mode: 0-maximum with original value; 1-maximum with transfered value; 2-accumulated with original value; 3-accumulated with transfered value
	double GetPointVolume(Point &mp, double mx, double my, VolumeData* vd, int mode, bool use_transf, double thresh = 0.5);
	double GetPointVolumeBox(Point &mp, double mx, double my, VolumeData* vd, bool calc_mats=true);
	double GetPointVolumeBox2(Point &p1, Point &p2, double mx, double my, VolumeData* vd);
	double GetPointPlane(Point &mp, double mx, double my, Point *planep=0, bool calc_mats=true);
	Point* GetEditingRulerPoint(double mx, double my);

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

	//get size, considering enlargement
	wxSize GetGLSize();
	void ResizeFramebuffers();

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	DataGroup* AddVolumeData(VolumeData* vd, wxString group_name="");
	void AddMeshData(MeshData* md);
	void AddAnnotations(Annotations* ann);
	wxString AddGroup(wxString str = "", wxString prev_group="");
	DataGroup* AddOrGetGroup();
	wxString AddMGroup(wxString str = "");
	MeshGroup* AddOrGetMGroup();
	MeshGroup* GetMGroup(wxString &name);
	void RemoveVolumeData(wxString &name);
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
	void RefreshGL(bool intactive=false, bool start_loop=true);

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
	//label the volume in current view
	void Label()
	{if (m_glview) m_glview->Label();}
	//remove noise

	int CompAnalysis(double min_voxels, double max_voxels, double thresh, 
		double falloff, bool select, bool gen_ann, bool size_map)
	{
		if (m_glview) 
			return m_glview->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, gen_ann, size_map); 
		else 
			return 0;
	}
	void CompExport(int mode, bool select)
	{if (m_glview) m_glview->CompExport(mode, select);}
	void ShowAnnotations()
	{if (m_glview) m_glview->ShowAnnotations();}
	int NoiseAnalysis(double min_voxels, double max_voxels, double thresh)
	{if (m_glview) return m_glview->NoiseAnalysis(min_voxels, max_voxels, thresh); else return 0;}
	void NoiseRemoval(int iter, double thresh)
	{if (m_glview) m_glview->NoiseRemoval(iter, thresh);}

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

	//set ruler type
	int GetRulerType()
	{
		if (m_glview) return m_glview->GetRulerType();
		else return 0;
	}
	void SetRulerType(int type)
	{
		if (m_glview) m_glview->SetRulerType(type);
	}
	void FinishRuler()
	{
		if (m_glview) m_glview->FinishRuler();
	}
	vector<Ruler*>* GetRulerList()
	{
		if (m_glview) return m_glview->GetRulerList(); else return 0;
	}
	Ruler* GetRuler(unsigned int id)
	{
		if (m_glview) return m_glview->GetRuler(id); else return 0;
	}
	int RulerProfile(int index)
	{
		if (m_glview) return m_glview->RulerProfile(index); else return 0;
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

	//text renderer
	void SetTextRenderer(TextRenderer* text_renderer)
	{ if (m_glview) m_glview->SetTextRenderer(text_renderer); }

	//set full screen
	void SetFullScreen();

public:
	wxWindow* m_frame;
	static int m_id;

	//render view///////////////////////////////////////////////
	VRenderGLView *m_glview;
	wxFrame* m_full_frame;
	wxBoxSizer* m_view_sizer;

	//top bar///////////////////////////////////////////////////
	wxPanel* m_panel_1;
	wxColourPickerCtrl *m_bg_color_picker;
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
	void OnChEmbedCheck(wxCommandEvent &event);
	void OnChEnlargeCheck(wxCommandEvent &event);
	void OnSlEnlargeScroll(wxScrollEvent &event);
	void OnTxEnlargeText(wxCommandEvent &event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
	void OnCapture(wxCommandEvent& event);
	void OnBgColorChange(wxColourPickerEvent& event);
	void OnCamCtrCheck(wxCommandEvent& event);
	void OnFpsCheck(wxCommandEvent& event);
	void OnLegendCheck(wxCommandEvent& event);
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

	void OnSaveDefault(wxCommandEvent &event);

	void OnKeyDown(wxKeyEvent& event);
	//idle
	void OnTimer(wxTimerEvent& event);

	DECLARE_EVENT_TABLE()

};

#endif//_VRENDERVIEW_H_
