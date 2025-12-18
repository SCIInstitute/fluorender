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

#ifndef _RENDERVIEW_H_
#define _RENDERVIEW_H_

#include <TreeLayer.h>
#include <Size.h>
#include <Value.hpp>
#include <glm/glm.hpp>
#include <string>
#include <memory>

#define INIT_BOUNDS  1
#define INIT_CENTER  2
#define INIT_TRANSL  4
#define INIT_ROTATE  8
#define INIT_OBJ_TRANSL  16

//information bits
#define INFO_DISP	1
#define INFO_POS	2
#define INFO_FRAME	4
#define INFO_FPS	8
#define INFO_T		16
#define INFO_X		32
#define INFO_Y		64
#define INFO_Z		128

class XboxController;
class RenderViewPanel;
class BaseState;
class IdleState;
class MouseState;
class VolumeData;
class MeshData;
class AnnotData;
class VolumeGroup;
class MeshGroup;
class TrackGroup;
class RenderMixModeGuard;
namespace flvr
{
	class MultiVolumeRenderer;
	class TextRenderer;
	enum class FaceWinding : int;
	class Framebuffer;
	enum class RenderMode : int;
	enum class ColorMode : int;
	enum class ColormapProj : int;
}
namespace flrd
{
	class CelpList;
	class RulerList;
	class Ruler;
}
namespace fluo
{
	enum class ClipPlane : int;
	class Quaternion;
}

enum class ChannelMixMode : int
{
	Layered = 1,
	Depth,
	CompositeAdd
};

enum class InteractiveMode : int
{
	None,
	Viewport,			//1-normal viewing
	BrushSelect,		//2-painting
	BrushSelectUpdate,	//4-one-time rendering update in painting mode
	Clipplane,			//3-rotate clipping planes
	Ruler,				//5-ruler mode
	EditRulerPoint,		//6-edit ruler by moving point
	BrushRuler,			//7-paint with locator
	BrushRulerUpdate,	//8-same as 4, but for paint ruler mode
	MoveRuler,			//9-move ruler
	Grow,				//10-grow, click and hold to activate
	GrowRuler,			//12-grow with ruler
	RulerDelPoint,		//14-delete ruler point
	RulerLockPoint,		//11-lock ruler point for relaxing
	Pencil,				//13-pencil with multipoint ruler
	Magnet,				//15-edit ruler by magnet
	CropFrame,			//16-edit crop frame
	CenterClick			//17-center render view on click
};
class RenderView : public TreeLayer
{
public:
	RenderView();
	RenderView(RenderView& copy);
	virtual ~RenderView();

	std::string GetBufferName(const std::string& base, int sn = -1);
	//handle
	void SetHandle(void* hWnd) { m_hWnd = hWnd; }
	//set render view panel
	void SetRenderViewPanel(RenderViewPanel* panel);

	//size
	void SetSize(int x, int y);
	Size2D GetSize() { return m_size; }
	void ResetSize();
	Size2D GetCanvasSize();
	void GetRenderSize(int& nx, int& ny);
	void SetClient(int x, int y, int w, int h) { m_client_x = x; m_client_y = y; m_client_w = w; m_client_h = h; }
	void SetDpiFactor(double factor) { m_dpi_factor = factor; }
	std::string GetOGLVersion();

	//initialization
	void Init();

	void InitOpenXR();
	void InitLookingGlass();
	//recalculate view according to object bounds
	void InitView(unsigned int type = INIT_BOUNDS);

	//Clear all layers
	void ClearAll();
	void Clear();
	void ClearVolList();
	void ClearMeshList();

	ChannelMixMode GetChannelMixMode() { return m_channel_mix_mode; }
	void SetChannelMixMode(ChannelMixMode mode) { m_channel_mix_mode = mode; }
	void UpdateChannelMixMode(ChannelMixMode mode, fluo::ValueCollection& vc);

	//data management
	int GetAny();
	int GetDispVolumeNum();
	int GetAllVolumeNum();
	int GetMeshNum();
	int GetGroupNum();
	int GetLayerNum();
	std::shared_ptr<VolumeData> GetAllVolumeData(int index);
	std::shared_ptr<VolumeData> GetDispVolumeData(int index);
	std::shared_ptr<MeshData> GetMeshData(int index);
	std::shared_ptr<TreeLayer> GetLayer(int index);
	flvr::MultiVolumeRenderer* GetMultiVolumeData() { return m_mvr.get(); };
	std::shared_ptr<VolumeData> GetVolumeData(const std::wstring &name);
	std::shared_ptr<MeshData> GetMeshData(const std::wstring &name);
	std::shared_ptr<AnnotData> GetAnnotData(const std::wstring &name);
	std::shared_ptr<VolumeGroup> GetGroup(const std::wstring &name);
	std::shared_ptr<VolumeGroup> GetGroup(int index);
	std::shared_ptr<VolumeGroup> GetGroup(const std::shared_ptr<VolumeData>& vd);
	std::shared_ptr<MeshGroup> GetMGroup(const std::wstring &str);
	//add
	std::shared_ptr<VolumeGroup> AddVolumeData(const std::shared_ptr<VolumeData>& vd, const std::wstring &group_name = L"");
	void AddMeshData(const std::shared_ptr<MeshData>& md);
	void AddAnnotData(const std::shared_ptr<AnnotData>& ann);
	std::wstring AddGroup(const std::wstring& str, const std::wstring& prev_group = L"");
	std::shared_ptr<VolumeGroup> AddOrGetGroup();
	std::wstring AddMGroup(const std::wstring& str);
	std::shared_ptr<MeshGroup> AddOrGetMGroup();
	//remove
	void RemoveVolumeData(const std::wstring &name);
	void ReplaceVolumeData(const std::wstring &name, const std::shared_ptr<VolumeData>& dst);
	void RemoveMeshData(const std::wstring &name);
	void RemoveAnnotData(const std::wstring &name);
	void RemoveGroup(const std::wstring &name);
	//isolate
	void Isolate(int type, const std::wstring& name);
	void ShowAll();
	//move
	void MoveLayerinView(const std::wstring &src_name, const std::wstring &dst_name);
	//move volume
	void MoveLayerinGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name);
	void MoveLayertoView(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name);
	void MoveLayertoGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name);
	void MoveLayerfromtoGroup(const std::wstring &src_group_name, const std::wstring &dst_group_name, const std::wstring &src_name, const std::wstring &dst_name);
	//move mesh
	void MoveMeshinGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name);
	void MoveMeshtoView(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name);
	void MoveMeshtoGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name);
	void MoveMeshfromtoGroup(const std::wstring &src_group_name, const std::wstring &dst_group_name, const std::wstring &src_name, const std::wstring &dst_name);
	//
	void PopVolumeList();
	void PopMeshList();
	//reorganize layers in view
	void OrganizeLayers();
	//randomize color
	void RandomizeColor();

	//toggle hiding/displaying
	void SetDraw(bool draw) { m_draw_all = draw; }
	void ToggleDraw() { m_draw_all = !m_draw_all; }
	bool GetDraw() { return m_draw_all; }

	//handle camera
	void HandleProjection(int nx, int ny, bool vr = false);
	void GetCameraSettings(glm::vec3& eye, glm::vec3& center, glm::vec3& up);
	void GetCameraSettingsOrthoFligt(glm::vec3& eye, glm::vec3& center, float& dist);
	void HandleCamera(bool vr = false);
	float GetDistancePerspFlight();
	//camera operations
	fluo::Vector GetTranslations();
	void SetTranslations(const fluo::Vector& val);
	fluo::Quaternion GetZeroQuat() { return m_cam_zero_q; }
	void SetZeroQuat(const fluo::Quaternion& val) { m_cam_zero_q = val; }
	fluo::Quaternion GetRotQuat() { return m_cam_q; }
	fluo::Vector GetRotations();
	void SetRotations(const fluo::Vector& val, bool notify);
	int GetOrientation();//same as the indices in the view panel
	void SetZeroRotations() { m_cam_zero_q = m_cam_q; }
	fluo::Vector ResetZeroRotations();
	fluo::Point GetCenters();
	void SetCenters(const fluo::Point& val);
	double GetCenterEyeDist() { return m_distance; }
	void SetCenterEyeDist(double dist) { m_distance = dist; }
	double GetInitDist() { return m_init_dist; }
	void SetInitDist(double dist) { m_init_dist = dist; }
	double GetRadius() { return m_radius; }
	void SetRadius(double r) { m_radius = r; }
	void SetCenter();
	double Get121ScaleFactor();
	void SetScale121();
	void SetPinRotCenter(bool, bool);

	//object operations
	fluo::Point GetObjCenters();
	void SetObjCenters(const fluo::Point& val);
	fluo::Vector GetObjRot();
	void SetObjRot(const fluo::Vector& val);
	void SetOffset();
	fluo::Vector GetObjCtrOff();
	void SetObjCtrOff(const fluo::Vector& val);
	fluo::Vector GetObjRotCtrOff();
	void SetObjRotCtrOff(const fluo::Vector& val);
	fluo::Vector GetObjRotOff();
	void SetObjRotOff(const fluo::Vector& val);
	void SetOffsetTransform(const fluo::Transform& tf);
	fluo::Vector GetObjTrans();
	void SetObjTrans(const fluo::Vector& val);
	void SetRotLock(bool mode) { m_rot_lock = mode; }
	bool GetRotLock() { return m_rot_lock; }
	//lock cam center
	void SetLockCamObject(bool bval) { m_lock_cam_object = bval; }
	bool GetLockCamObject() { return m_lock_cam_object; }
	void SetLockCenter();
	void SetLockCenterVol();
	void SetLockCenterRuler();
	void SetLockCenterSel();

	//camera properties
	bool GetPersp() { return m_persp; }
	void SetPersp(bool persp) { m_persp = persp; }
	int GetCamMode() { return m_cam_mode; }
	void SetCamMode(int mode);
	double GetAov() { return m_aov; }
	void SetAov(double aov) { m_aov = aov; }
	double GetNearClip() { return m_near_clip; }
	void SetNearClip(double nc) { m_near_clip = nc; }
	double GetFarClip() { return m_far_clip; }
	void SetFarClip(double fc) { m_far_clip = fc; }

	//force clear
	void SetForceClear(bool val) { m_force_clear = val; }
	bool GetForceClear() { return m_force_clear; }
	//interactive
	void SetInteractive(bool val = false) { m_interactive = val; }

	//background color
	fluo::Color GetBackgroundColor() { return m_bg_color; }
	fluo::Color GetTextColor();
	void SetBackgroundColor(fluo::Color &color);

	//disply modes
	int GetDrawType() { return m_draw_type; }
	void SetFog(bool b = true) { m_use_fog = b; }
	bool GetFog() { return m_use_fog; }
	void SetFogIntensity(double i) { m_fog_intensity = i; }
	double GetFogIntensity() { return m_fog_intensity; }

	//movie capture
	void Set3DRotCapture(double start_angle,
		double end_angle,
		double step,
		int frames,
		int rot_axis,
		const std::wstring &cap_file,
		bool rewind,
		int len);
	//time sequence data
	void Set4DSeqCapture(const std::wstring &cap_file, int begin_frame, int end_frame, bool rewind);
	//batch files
	void Set3DBatCapture(const std::wstring &cap_file, int begin_frame, int end_frame);
	//parameter recording/capture
	void SetParamCapture(const std::wstring &cap_file, int begin_frame, int end_frame, bool rewind);
	//set parameters
	void SetParams(double t);
	//reset and stop
	void StopMovie();
	//4d movie frame calculation
	void Get4DSeqRange(int &start_frame, int &end_frame);
	void Set4DSeqFrame(int frame, int start_frame, int end_frame, bool rewind);
	void UpdateVolumeData(int frame, const std::shared_ptr<VolumeData>& vd);
	void UpdateMeshData(int frame, const std::shared_ptr<MeshData>& md);
	void ReloadVolumeData(int frame);
	void ReloadMeshData(int frame);
	//3d batch file calculation
	void Get3DBatRange(int &start_frame, int &end_frame);
	void Set3DBatFrame(int frame, int start_frame, int end_frame, bool rewind);

	//frame for capturing
	void EnableFrame() { m_draw_frame = true; }
	void DisableFrame() { m_draw_frame = false; };
	void CalcFrame();

	//scale bar
	void SetScalebarDisp(int disp) { m_scalebar_disp = disp; }
	void SetScaleBarLen(double len) { m_sb_length = len; }
	void SetSBText(const std::wstring text) { m_sb_text = text; }

	//gamma settings
	fluo::Color GetGammaColor() { return m_gamma; }
	void SetGammaColor(fluo::Color gamma) { m_gamma = gamma; }
	//brightness adjustment
	fluo::Color GetBrightness() { return m_brightness; }
	void SetBrightness(fluo::Color brightness) { m_brightness = brightness; }
	//hdr settings
	fluo::Color GetHdr() { return m_hdr; }
	void SetHdr(fluo::Color hdr) { m_hdr = hdr; }
	//sync values
	bool GetSync(int i) { if (i >= 0 && i < 3) return m_sync[i]; else return false; }
	void SetSync(int i, bool val) { if (i >= 0 && i < 3) m_sync[i] = val; }

	//reload volume list
	void SetVolPopDirty() { m_vd_pop_dirty = true; }
	//reload mesh list
	void SetMeshPopDirty() { m_md_pop_dirty = true; }

	//inteactive mode selection
	InteractiveMode GetIntMode() { return m_int_mode; }
	void SetIntMode(InteractiveMode val);

	//set use 2d rendering results
	void SetPaintUse2d(bool use2d);
	bool GetPaintUse2d();

	//brush properties
	//change display
	void ChangeBrushSize(int value, bool ctrl);

	//set clip mode
	void SetClipMode(int mode);
	int GetClipMode() { return m_clip_mode; }

	void ResetAllClipValues();
	//sync
	virtual void SyncClippingBoxes(const fluo::ClippingBox& cb) override;
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

	//interpolation
	void SetIntp(bool mode) { m_intp = mode; }
	bool GetIntp() { return m_intp; }

	//text renderer
	flvr::TextRenderer* GetTextRenderer();

	//entry point for drawing the view
	bool Draw();			//return if swap buffers
	void DrawDefault();		//draw the base fbo to default on canvas

	//start loop update
	void StartLoopUpdate();
	void HaltLoopUpdate();

	//sort bricks after view change
	void SetSortBricks();

	//rulers
	flrd::RulerList* GetRulerList();
	void SetCurRuler(flrd::Ruler* ruler);
	flrd::Ruler* GetCurRuler();
	flrd::Ruler* GetRuler(unsigned int id);

	//traces
	//track map file
	int GetTrackFileExist(bool save);//0:no trace group; 1:trace groups exists not saved; 2:saved
	TrackGroup* GetTrackGroup();
	std::wstring GetTrackGroupFile();
	void CreateTrackGroup();
	int LoadTrackGroup(const std::wstring& filename);
	int SaveTrackGroup(const std::wstring& filename);
	void ExportTrackGroup(const std::wstring& filename, unsigned int id);
	void GetTraces(bool update = false);

	//enlarge output image
	void SetKeepEnlarge(bool value) { m_keep_enlarge = value; }
	void SetEnlarge(bool value);
	void SetEnlargeScale(double value);
	bool GetEnlarge() { return m_enlarge; }
	double GetEnlargeScale() { return m_enlarge_scale; }

	//read pixels
	void ReadPixels(
		int chann, bool fp32,
		int &x, int &y, int &w, int &h,
		void** image);
	void ReadPixelsQuilt(
		int chann, bool fp32,
		int &x, int &y, int &w, int &h,
		void** image);

	//set cell list
	void SetCellList(flrd::CelpList& list);

	glm::mat4 GetModelView() { return m_mv_mat; }
	glm::mat4 GetProjection() { return m_proj_mat; }
	glm::mat4 GetObjectMat();
	glm::mat4 GetDrawMat();
	glm::mat4 GetInvtMat();
	glm::mat4 GetDrawWorldMat();
	fluo::Transform GetInvOffsetMat();
	fluo::Vector GetSide();

	void RotateClips();

	//mouse position
	void SetMousePos(int x, int y) { m_mouse_x = x; m_mouse_y = y; }

	//process idle
	void ProcessIdle(IdleState& state);
	//process mouse
	void ProcessMouse(MouseState& state);

	//pop list
	bool GetVolPopListEmpty() { return m_vd_pop_list.empty(); }
	std::shared_ptr<VolumeData> GetVolPopList(int index) { return m_vd_pop_list[index].lock(); }
	bool GetMeshPopListEmpty() { return m_md_pop_list.empty(); }
	std::shared_ptr<MeshData> GetMeshPopList(int index) { return m_md_pop_list[index].lock(); }

	//volume properties
	void SetGammaEnable(bool);
	void SetGamma(double val, bool set_this = true);
	void SetBoundaryEnable(bool);
	void SetBoundaryLow(double, bool set_this = true);
	void SetBoundaryHigh(double, bool set_this = true);
	void SetBoundaryMax(double val);
	void SetMinMaxEnable(bool);
	void SetLowOffset(double, bool set_this = true);
	void SetHighOffset(double, bool set_this = true);
	void SetThreshEnable(bool);
	void SetLeftThresh(double, bool set_this = true);
	void SetRightThresh(double, bool set_this = true);
	void SetLuminanceEnable(bool);
	void SetLuminance(double, bool set_this = true);
	void SetAlphaEnable(bool);
	void SetAlpha(double, bool set_this = true);
	void SetShadingEnable(bool);
	void SetShadingStrength(double);
	void SetShadingShine(double);
	void SetShadowEnable(bool);
	void SetShadowIntensity(double);
	void SetSampleRateEnable(bool);
	void SetSampleRate(double, bool set_this = true);

	void SetColorMode(flvr::ColorMode mode);
	void SetColormapDisp(bool disp);
	void SetColormapValues(double low, double high);
	void SetColormapInv(double val);
	void SetColormap(int value);
	void SetColormapProj(flvr::ColormapProj value);
	void SetRenderMode(flvr::RenderMode mode);
	void SetAlphaPower(double val);
	void SetLabelMode(int mode);
	void SetNR(bool val);
	void SetInterpolate(bool mode);
	void SetInvert(bool mode);
	void SetTransparent(bool val);

public:
	//capture modes
	bool m_capture;
	bool m_capture_rotat;
	bool m_capture_rotate_over;
	bool m_capture_tsequ;
	bool m_capture_bat;
	bool m_capture_param;
	//begin and end frame
	int m_begin_frame;
	int m_begin_play_frame;//frame number when the play button is clicked
	int m_end_frame;
	int m_end_all_frame;//end of all frames
	//counters
	int m_tseq_cur_num;
	int m_tseq_prv_num;
	int m_frame_num_type;//0:tseq;1:param
	int m_param_cur_num;
	int m_total_frames;
	//file name for capturing
	std::wstring m_cap_file;
	//folder name for 3d batch
	std::wstring m_bat_folder;
	//hud
	bool m_retain_finalbuffer;	//sometimes we don't redraw everything,
								//just use the final buffer from last draw
	bool m_updating;
	bool m_draw_annotdata;
	bool m_draw_camctr;
	double m_camctr_size;
	int m_draw_info;
	bool m_load_update;
	bool m_draw_frame;
	bool m_draw_legend;
	int m_colormap_disp;//0-no display; 1-color map; 2-with text
	bool m_mouse_focus;
	//current volume
	std::weak_ptr<VolumeData> m_cur_vol;
	std::weak_ptr<VolumeData> m_cur_vol_save;
	//clipping settings
	int m_clip_mode;//0-normal; 1-ortho planes; 2-rot difference
	//scale bar
	int m_scalebar_disp;//0-no display; 1-length; 2-with text
	double m_sb_length;
	int m_sb_unit;
	std::wstring m_sb_text;
	std::wstring m_sb_num;
	double m_sb_height;
	//ortho size
	double m_ortho_left;
	double m_ortho_right;
	double m_ortho_bottom;
	double m_ortho_top;
	//scale factor
	double m_scale_factor;
	//scale mode
	int m_scale_mode;//zoom ratio meaning: 0-view; 1-pixel; 2-data(pixel*xy spc)
	//pin rotation center
	bool m_pin_rot_ctr;//enable auto compute of rot center based on data
	bool m_update_rot_ctr;//rot center needs update
	double m_pin_pick_thresh;//ray casting threshold value
	fluo::Point m_pin_ctr;//the center point for view rotation

private:
	void* m_hWnd;
	RenderViewPanel* m_render_view_panel;
	bool m_drawing;
	bool m_refresh;//own refresh command
	Size2D m_canvas_size;//size from wxwidgets window
	Size2D m_size;//size with dpi factor
	Size2D m_gl_size;//actual render size
	double m_dpi_factor;
	std::string m_GLversion;

	ChannelMixMode m_channel_mix_mode;

	//populated lists of data
	bool m_vd_pop_dirty;
	std::vector<std::weak_ptr<VolumeData>> m_vd_pop_list;
	bool m_md_pop_dirty;
	std::vector<std::weak_ptr<MeshData>> m_md_pop_list;
	//real data list
	std::vector<std::shared_ptr<TreeLayer>> m_layer_list;
	//ruler list
	std::unique_ptr<flrd::RulerList> m_ruler_list;
	flrd::Ruler* m_cur_ruler;
	//traces
	std::unique_ptr<TrackGroup> m_track_group;
	//multivolume
	std::unique_ptr<flvr::MultiVolumeRenderer> m_mvr;
	//highlighted comps
	std::unique_ptr<flrd::CelpList> m_cell_list;
	//initializaion
	bool m_initialized;
	bool m_init_view;
	//bg color
	fluo::Color m_bg_color;
	fluo::Color m_bg_color_inv;
	//frustrum
	double m_aov;
	double m_near_clip;
	double m_far_clip;
	//interpolation
	bool m_intp;

	//interactive modes
	InteractiveMode m_int_mode;
	int m_crop_type;
	bool m_force_clear;
	bool m_grow_on;//flag for grow is currently on for idle events
	//resizing
	bool m_resize;
	//brush tools
	bool m_draw_brush;
	bool m_paint_enable;
	bool m_paint_display;
	//paint buffer
	bool m_clear_paint;

	//interactive state control
	bool m_interactive;//main control

	//camera controls
	bool m_persp;
	int m_cam_mode;//changed free to camera operation mode: 0-global; 1-flight;
	//camera distance
	double m_distance;
	double m_init_dist;
	//camera translation
	fluo::Vector m_cam_trans;
	//camera rotation
	fluo::Vector m_cam_rot;
	//zero camera rotation
	fluo::Vector m_cam_rot_zero;
	//double m_zrotx, m_zroty, m_zrotz;
	//camera center
	fluo::Point m_cam_ctr;
	//double m_ctrx, m_ctry, m_ctrz;
	fluo::Quaternion m_cam_q;
	fluo::Quaternion m_cam_zero_q;//zero rotation
	fluo::Vector m_cam_up;
	fluo::Vector m_cam_head;

	//object center
	fluo::Point m_obj_ctr;
	//object rotation
	fluo::Vector m_obj_rot = fluo::Vector(180.0, 0.0, 0.0);
	//flag for using offset values
	bool m_offset;
	//obj center offset (for registration currently)
	fluo::Vector m_obj_ctr_off;
	//obj rotation center offset
	fluo::Vector m_obj_rot_ctr_off;
	//obj rotation offset (for registration currently)
	fluo::Vector m_obj_rot_off;
	//offset transform
	fluo::Transform m_offset_tf;
	//object translation
	fluo::Vector m_obj_trans;
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

	//post image processing
	fluo::Color m_gamma;
	fluo::Color m_brightness;
	fluo::Color m_hdr;
	bool m_sync[3];//for rgb

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
	glm::mat4 m_tex_mat = glm::mat4(1.0);

	//text renderer
	std::unique_ptr<flvr::TextRenderer> m_text_renderer;

	//enlargement
	bool m_keep_enlarge;
	bool m_enlarge;
	double m_enlarge_scale;

	//nodraw count
	int m_nodraw_count;

	//VolumeLoader m_loader;
	bool m_load_in_main_thread;

	int m_res_mode;

	//enable vr
	bool m_use_openxr;
	bool m_lg_initiated;
	int m_vr_eye_idx;//0: left; 1: right

	//mouse position (client)
	int m_mouse_x;
	int m_mouse_y;
	//client
	int m_client_x;
	int m_client_y;
	int m_client_w;
	int m_client_h;

#if defined(_WIN32) && defined(USE_XINPUT)
	std::unique_ptr<XboxController> m_controller;
	bool m_control_connected;
#endif

private:
	//framebuffer for final output including vr and lg
	void BindViewBaseFramebuffer();
	void ClearViewBaseFramebuffer();
	//draw out the framebuffer after composition
	std::shared_ptr<flvr::Framebuffer> PrepareDataFramebuffer(int nx, int ny);
	std::shared_ptr<flvr::Framebuffer> GetDataFramebuffer();
	void DrawDataFramebuffer();
	//vr buffers
	void PrepareHologramFramebuffer();
	void DrawHologramFramebuffer();
	//pre draw and post draw actions
	void PreDraw();
	void PostDraw();

	//draw quad for framebuffer texture
	void DrawViewQuad();

	//draw calls used internally
	//different modes
	void DrawData();		//draw volumes only
	void DrawDataPeel();	//draw volumes and meshes with depth peeling
	//mesh and volume
	void DrawMesh(int peel = 0);//0: no dp
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
	//different volume drawing modes
	void DrawVolumesStandardDepth(const std::vector<std::weak_ptr<VolumeData>> &list, int peel = 0);
	void DrawVolumesMipDepth(const std::vector<std::weak_ptr<VolumeData>> &list, int peel = 0);
	void DrawVolumesComp(const std::vector<std::weak_ptr<VolumeData>> &list, bool mask = false, int peel = 0);
	void DrawVolumeCompMip(const std::weak_ptr<VolumeData>& vd, int peel = 0);
	void DrawVolumeCompStandard(const std::weak_ptr<VolumeData>& vd, bool mask, int peel = 0);
	//effect overlays
	void DrawOverlayShadingVolume(const std::vector<std::weak_ptr<VolumeData>>& list);
	void DrawOverlayScatteringVolume(const std::vector<std::weak_ptr<VolumeData>>& list);
	void DrawOverlayShadowVolume(const std::vector<std::weak_ptr<VolumeData>>& list);
	void DrawOverlayScatteringMesh();
	void DrawOverlayShadowMesh(double darkenss);
	//get mesh effects and return strength/darkness
	bool CheckMeshShadowExist(double &val);

	//other data types
	void DrawAnnots();
	//rulers
	void DrawRulers();
	//cells
	void DrawCells();
	//draw highlighted comps
	void GetCellPoints(fluo::BBox& box,
		fluo::Point& p1, fluo::Point& p2, fluo::Point& p3, fluo::Point& p4,
		fluo::Transform& mv, fluo::Transform& p);
	unsigned int GenerateCellVerts(std::vector<float>& verts);
	//tracks
	void DrawTracks();
	//clipping planes
	void DrawClippingPlanes(flvr::FaceWinding face_winding);
	//data elements
	void DrawBounds();
	void DrawGrid();
	void DrawCamCtr();

	//interactive tools
	//painting
	void DrawBrushOutlines();
	void DrawBrushCircles(
		double cx, double cy,
		double r1, double r2,
		fluo::Color &color,
		glm::mat4 &matrix);
	void GenerateBrushStrokes();
	void DrawBrushStrokes();

	//other hud elements
	void DrawScaleBar();
	void DrawLegend();
	void DrawText(double x, double y, int nx, int ny,
		const std::wstring& name, const fluo::Color& color,
		double font_height, bool hilighted = false);
	void SetColormapColors(int colormap, const fluo::Color &c1, const fluo::Color& c2, double inv);
	void DrawColormap();
	void DrawGradBg();

	//info and frame not saved by capture
	void DrawInfo(int nx, int ny, bool intactive);
	void DrawFrame();

	//reset enlargement for capture
	void ResetEnlarge();

	//depth
	double CalcZ(double z);
	void CalcFogRange();

	fluo::Quaternion Trackball(double dx, double dy);
	void Q2A();
	void A2Q();

	//brush states update
	bool UpdateBrushState(IdleState& state);

	//selection
	void Pick(BaseState& state);
	bool PickMesh(BaseState& state);
	bool PickVolume(BaseState& state);
	void SetCompSelection(fluo::Point& p, int mode);//node: 0-exclusive; 1-add or remove

	//find crop frame
	int HitCropFrame(fluo::Point& mp);
	void ChangeCropFrame(fluo::Point& mp);

	void switchLevel(VolumeData *vd);

	//controller interactions
	void ControllerMoveHorizontal(double dval, int nx, int ny);
	void ControllerZoomDolly(double dval, int nx, int ny);
	void ControllerRotate(double dx, double dy, int nx, int ny);
	void ControllerPan(double dx, double dy, int nx, int ny);
	void GrabRotate(const glm::mat4& pose);

	//determine if point in current view
	bool PointInView(int x, int y)
	{
		if (x < m_client_x || x > m_client_x + m_client_w * m_dpi_factor ||
			y < m_client_y || y > m_client_y + m_client_h * m_dpi_factor)
			return false;
		else
			return true;
	}
};

class ChannelMixModeGuard
{
public:
	ChannelMixModeGuard(RenderView& view)
		: m_view(view)
	{
		m_old_mode = m_view.GetChannelMixMode();
	}
	~ChannelMixModeGuard()
	{
		m_view.SetChannelMixMode(m_old_mode);
	}

private:
	RenderView& m_view;
	ChannelMixMode m_old_mode;
};

#endif//_RENDERVIEW_H_