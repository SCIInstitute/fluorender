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

#include <RenderCanvasAgent.hpp>
#include <VolumeLoader.h>
#include <utility.h>
#include <Calculate/KernelExecutor.h>
#include <Calculate/VolumeCalculator.h>
#include <Selection/VolumeSelector.h>
#include <Script/ScriptProc.h>
#include <Distance/Ruler.h>
#include <Distance/RulerHandler.h>
#include <Distance/RulerRenderer.h>
#include <Selection/VolumePoint.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/MultiVolumeRenderer.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextRenderer.h>
#include <Types/Color.h>
#include <Types/BBox.h>
#include <Types/Quaternion.h>
#include <compatibility.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include <vector>
#include <stdarg.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define ID_ftrigger	ID_VRENDER_VIEW1

class VRenderFrame;
class VRenderView;
class RenderCanvas : public wxGLCanvas
{
public:
	RenderCanvas(VRenderFrame* frame,
		VRenderView* parent,
		const wxGLAttributes& attriblist,
		wxGLContext* sharedContext = 0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	~RenderCanvas();

	//for degugging, this allows inspection of the pixel format actually given.
#ifdef _WIN32
	int GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd);
#endif
	//initialization
	void Init();

	void ClearAll();
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
	fluo::VolumeData* GetAllVolumeData(int index);
	fluo::VolumeData* GetDispVolumeData(int index);
	fluo::MeshData* GetMeshData(int index);
	fluo::Object* GetLayer(int index);
	flvr::MultiVolumeRenderer* GetMultiVolumeData() { return m_mvr; };
	fluo::VolumeData* GetVolumeData(const std::string &name);
	fluo::MeshData* GetMeshData(const std::string &name);
	fluo::Annotations* GetAnnotations(const std::string &name);
	fluo::VolumeGroup* GetGroup(const std::string &name);
	fluo::VolumeGroup* GetGroup(int index);
	fluo::VolumeGroup* GetGroup(fluo::VolumeData* vd);
	fluo::MeshGroup* GetMGroup(const std::string &str);
	//add
	fluo::VolumeGroup* AddVolumeData(fluo::VolumeData* vd, const std::string &group_name = "");
	void AddMeshData(fluo::MeshData* md);
	void AddAnnotations(fluo::Annotations* ann);
	std::string AddGroup(const std::string &str, const std::string &prev_group = "");
	fluo::VolumeGroup* AddOrGetGroup();
	std::string AddMGroup(const std::string &str);
	fluo::MeshGroup* AddOrGetMGroup();
	//remove
	void RemoveVolumeData(const std::string &name);
	void RemoveVolumeDataDup(const std::string &name);//remove all duplicated data
	void ReplaceVolumeData(const std::string &name, fluo::VolumeData *dst);
	void RemoveMeshData(const std::string &name);
	void RemoveAnnotations(const std::string &name);
	void RemoveGroup(const std::string &name);
	//isolate
	void Isolate(int type, const std::string &name);
	void ShowAll();
	//move
	void MoveLayerinView(const std::string &src_name, const std::string &dst_name);
	//move volume
	void MoveLayerinGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name);
	void MoveLayertoView(const std::string &group_name, const std::string &src_name, const std::string &dst_name);
	void MoveLayertoGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name);
	void MoveLayerfromtoGroup(const std::string &src_group_name, const std::string &dst_group_name, const std::string &src_name, const std::string &dst_name);
	//move mesh
	void MoveMeshinGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name);
	void MoveMeshtoView(const std::string &group_name, const std::string &src_name, const std::string &dst_name);
	void MoveMeshtoGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name);
	void MoveMeshfromtoGroup(const std::string &src_group_name, const std::string &dst_group_name, const std::string &src_name, const std::string &dst_name);
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

	//handle camera
	void HandleProjection(int nx, int ny, bool vr = false);
	void HandleCamera(bool vr = false);
	void SetRotations(double rotx, double roty, double rotz, bool ui_update = true);
	void SetZeroRotations();
	void ResetZeroRotations(double &rotx, double &roty, double &rotz);
	void SetRadius(double r);
	void SetCenter();
	double Get121ScaleFactor();
	void SetScale121();
	void SetPinRotCenter(bool);

	//lock cam center
	void SetLockCenter(int type);
	void SetLockCenterVol();
	void SetLockCenterRuler();
	void SetLockCenterSel();

	//camera properties
	void SetAov(double aov);

	//background color
	fluo::Color GetBackgroundColor();
	fluo::Color GetTextColor();
	void SetBackgroundColor(fluo::Color &color);

	void SetFog(bool b = true);

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
	void Get4DSeqRange(int &start_frame, int &end_frame);
	void Set4DSeqFrame(int frame, int start_frame, int end_frame, bool rewind);
	void UpdateVolumeData(int frame, fluo::VolumeData* vd);
	void ReloadVolumeData(int frame);
	//3d batch file calculation
	void Get3DBatRange(int &start_frame, int &end_frame);
	void Set3DBatFrame(int frame, int start_frame, int end_frame, bool rewind);

	//frame for capturing
	void GetFrame(int &x, int &y, int &w, int &h);
	void CalcFrame();

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
	void SetVolumeA(fluo::VolumeData* vd);
	void SetVolumeB(fluo::VolumeData* vd);

	//segment volumes in current view
	void Segment();

	//brush properties
	//change display
	void ChangeBrushSize(int value);

	//set clip mode
	void SetClipMode(int mode);
	//restore clipping planes
	void RestorePlanes();
	//clipping plane rotations
	void SetClippingPlaneRotations(double rotx, double roty, double rotz);
	void GetClippingPlaneRotations(double &rotx, double &roty, double &rotz);

	//interpolation
	void SetIntp(bool mode);
	bool GetIntp();

	//force draw
	void ForceDraw();

	//run 4d script
	//start loop update
	void StartLoopUpdate();
	void HaltLoopUpdate();
	void RefreshGL(int debug_code, bool erase = false, bool start_loop = true);

	//rulers
	void DrawRulers();
	flrd::RulerList* GetRulerList();
	flrd::Ruler* GetRuler(unsigned int id);

	//draw highlighted comps
	void DrawCells();
	unsigned int DrawCellVerts(std::vector<float>& verts);
	void GetCellPoints(fluo::BBox& box,
		fluo::Point& p1, fluo::Point& p2, fluo::Point& p3, fluo::Point& p4,
		fluo::Transform& mv, fluo::Transform& p);

	//public mouse
	void OnMouse(wxMouseEvent& event);

	//traces
	flrd::Tracks* GetTraceGroup();
	void CreateTraceGroup();
	int LoadTraceGroup(wxString filename);
	int SaveTraceGroup(wxString filename);
	void ExportTrace(wxString filename, unsigned int id);
	void DrawTraces();
	void GetTraces(bool update = false);


	//read pixels
	void ReadPixels(
		int chann, bool fp32,
		int &x, int &y, int &w, int &h,
		void** image);

	//get view info for external ops
	//get size, considering enlargement
	wxSize GetGLSize();

	void UpdateClips();

public:
	VRenderView* m_vrv;
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
	fluo::VolumeData *m_cur_vol;
	//clipping settings
	int m_clip_mask;
	int m_clip_mode;//0-normal; 1-ortho planes; 2-rot difference
					//scale bar
	bool m_disp_scale_bar;
	bool m_disp_scale_bar_text;
	double m_sb_length;
	double m_sb_x;
	double m_sb_y;
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
	int m_scale_mode;//zoom ratio meaning: 0-view; 1-pixel; 2-data(pixel*xy spc)
	//pin rotation center
	bool m_auto_update_rot_center;
	bool m_pin_rot_center;
	bool m_rot_center_dirty;
	double m_pin_pick_thresh;//ray casting threshold value
	fluo::Point m_pin_ctr;
	//mode in determining depth of volume
	int m_point_volume_mode;  //0: use view plane; 1: use max value; 2: use accumulated value
							  //ruler use volume transfer function
	bool m_ruler_use_transf;
	//ruler time dependent
	bool m_ruler_time_dep;
	//linked rotation
	static bool m_linked_rot;
	static RenderCanvas* m_master_linked_view;
	//count after paint
	bool m_paint_count;
	//colocalize after paint
	bool m_paint_colocalize;
	//relax after ruler
	bool m_ruler_autorelax;

private:
	fluo::RenderCanvasAgent* m_agent;
	//initializaion
	bool m_initialized;

	bool m_drawing;
	bool m_refresh;//own refresh command
	wxSize m_size;
	wxGLContext* m_glRC;
	bool m_sharedRC;
	VRenderFrame* m_frame;
	//temporary lists of data
	bool m_vd_pop_dirty;
	std::vector <fluo::VolumeData*> m_vd_pop_list;
	bool m_md_pop_dirty;
	std::vector <fluo::MeshData*> m_md_pop_list;
	//real data list
	std::vector<fluo::ref_ptr<fluo::Object>> m_layer_list;
	//ruler list
	flrd::RulerList m_ruler_list;
	flrd::Ruler *m_cur_ruler;
	//traces
	flrd::Tracks* m_trace_group;
	//multivolume
	flvr::MultiVolumeRenderer* m_mvr;
	//highlighted comps
	flrd::CelpList m_cell_list;
	//fisrt volume data in the depth groups
	//VolumeData* m_first_depth_vd;
	bool m_init_view;
	//bg color
	fluo::Color m_bg_color;
	fluo::Color m_bg_color_inv;
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
					 //7-paint with locator
					 //8-same as 4, but for paint ruler mode
					 //9-move ruler
					 //10-grow, click and hold to activate
					 //11-lock ruler point for relaxing
					 //12-grow with ruler
					 //13-pencil with multipoint ruler
					 //14-delete ruler point
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
	bool m_grow_on;//flag for grow is currently on for idle events
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
	fluo::Quaternion m_q;
	fluo::Quaternion m_zq;//zero rotation
	fluo::Vector m_up;
	fluo::Vector m_head;

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

	//lock on object
	bool m_lock_cam_object;
	fluo::Point m_lock_center;
	bool m_pick_lock_center;

	//object bounding box
	fluo::BBox m_bounds;
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
	int m_rot_axis; //0-X; 1-Y; 2-Z;
	int m_movie_seq;
	bool m_rewind;
	int m_stages; //0-moving to start angle; 1-moving to end; 2-rewind
	bool m_4d_rewind;

	//movie frame properties
	int m_frame_x;
	int m_frame_y;
	int m_frame_w;
	int m_frame_h;

	//post image processing
	fluo::Color m_gamma;
	fluo::Color m_brightness;
	fluo::Color m_hdr;
	bool m_sync_r;
	bool m_sync_g;
	bool m_sync_b;

	//volume color map
	//double m_value_1;
	fluo::Color m_color_1;
	double m_value_2;
	fluo::Color m_color_2;
	double m_value_3;
	fluo::Color m_color_3;
	double m_value_4;
	fluo::Color m_color_4;
	double m_value_5;
	fluo::Color m_color_5;
	double m_value_6;
	fluo::Color m_color_6;
	//double m_value_7;
	fluo::Color m_color_7;

	//clipping plane rotations
	fluo::Quaternion m_q_cl;
	fluo::Quaternion m_q_cl_zero;
	double m_rotx_cl, m_roty_cl, m_rotz_cl;

	//volume selector for segmentation
	flrd::VolumeSelector m_selector;

	//calculator
	flrd::VolumeCalculator m_calculator;

	//kernel executor
	flrd::KernelExecutor m_kernel_executor;

	//scriptor
	flrd::ScriptProc m_scriptor;

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
	flvr::TextRenderer m_text_renderer;
	static unsigned int m_tsize;

	//enlargement
	static bool m_keep_enlarge;
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

	//enable vr
	bool m_enable_vr;
	bool m_use_openvr;
	uint32_t m_vr_size[2];
	double m_vr_eye_offset;
	int m_vr_eye_idx;//0: left; 1: right
#ifdef _WIN32
	vr::IVRSystem *m_vr_system;
#ifdef USE_XINPUT
	XboxController* m_controller;
#endif
#endif

	//handle rulers
	flrd::RulerHandler m_ruler_handler;
	flrd::RulerRenderer m_ruler_renderer;
	flrd::VolumePoint m_vp;

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
		wxString name, fluo::Color color,
		double font_height, bool hilighted = false);
	void DrawFrame();
	void DrawClippingPlanes(bool border, int face_winding);
	void SetColormapColors(int colormap, fluo::Color &c, double inv);
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
	void DrawVolumesMulti(std::vector<fluo::VolumeData*> &list, int peel = 0);
	void DrawVolumesComp(std::vector<fluo::VolumeData*> &list, bool mask = false, int peel = 0);
	void DrawMIP(fluo::VolumeData* vd, int peel = 0);
	void DrawOVER(fluo::VolumeData* vd, bool mask, int peel = 0);
	//overlay passes
	void DrawOLShading(fluo::VolumeData* vd);
	void DrawOLShadows(std::vector<fluo::VolumeData*> &vlist);
	void DrawOLShadowsMesh(double darkenss);

	//get mesh shadow
	bool GetMeshShadow(double &val);

	//painting
	void DrawCircles(
		double cx, double cy,
		double r1, double r2,
		fluo::Color &color,
		glm::mat4 &matrix);
	void DrawBrush();
	void PaintStroke();
	void DisplayStroke();

	fluo::Quaternion Trackball(double dx, double dy);
	fluo::Quaternion TrackballClip(int p1x, int p1y, int p2x, int p2y);
	void Q2A();
	void A2Q();
	//sort bricks after the view has been changed
	void SetSortBricks();

	//pre draw and post draw
	void PreDraw();
	void PostDraw();
	void ResetEnlarge();

	//brush states update
	void SetBrush(int mode);
	void UpdateBrushState();

	//selection
	void Pick();
	void PickMesh();
	void PickVolume();
	void SetCompSelection(fluo::Point& p, int mode);//node: 0-exclusive; 1-add or remove

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

	void switchLevel(fluo::VolumeData *vd);

	DECLARE_EVENT_TABLE()

	friend class VRenderView;
	friend class fluo::RenderCanvasAgent;
};

inline wxSize RenderCanvas::GetGLSize()
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	wxSize size = GetSize() * dval;
	if (m_enlarge)
		size.Set(size.x * m_enlarge_scale,
			size.y * m_enlarge_scale);
	return size;
}

#endif//_VRENDERGLVIEW_H_
