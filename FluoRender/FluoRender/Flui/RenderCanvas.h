﻿/*
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

#ifndef _RENDERCANVAS_H_
#define _RENDERCANVAS_H_

#include <DataManager.h>
#include <VolumeLoader.h>
#include <KernelExecutor.h>
#include <VolumeCalculator.h>
#include <VolumeSelector.h>
#include <ScriptProc.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <RulerRenderer.h>
#include <VolumePoint.h>
#include <ShaderProgram.h>
#include <KernelProgram.h>
#include <MultiVolumeRenderer.h>
#include <ImgShader.h>
#include <VolKernel.h>
#include <TextRenderer.h>
#include <Color.h>
#include <BBox.h>
#include <Quaternion.h>
#include <Size.h>
#include <compatibility.h>
#include <wxBasisSlider.h>

#include <wx/wx.h>
#include <wx/clrpicker.h>
#include <wx/spinbutt.h>
#include <wx/glcanvas.h>
#include <wx/event.h>
#include <wx/timer.h>

#include <vector>
#include <stdarg.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _WIN32
//wacom support
#include <wx/msw/private.h>
#include <MSGPACK.h>
#include <wintab.h>
#define PACKETDATA	(/*PK_X | PK_Y | */PK_BUTTONS |\
		PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include <PKTDEF.h>
#include <WacomUtils.h>
#ifdef USE_XINPUT
#include <XInput/XboxController.h>
#endif
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

#define ID_ftrigger	0

class MainFrame;
class RenderViewPanel;
class RenderCanvas : public wxGLCanvas
{
public:
	RenderCanvas(MainFrame* frame,
		RenderViewPanel* parent,
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
	void InitOpenXR();
	void InitLookingGlass();

	std::string GetOGLVersion();
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
	VolumeData* GetAllVolumeData(int index);
	VolumeData* GetDispVolumeData(int index);
	MeshData* GetMeshData(int index);
	TreeLayer* GetLayer(int index);
	flvr::MultiVolumeRenderer* GetMultiVolumeData() { return m_mvr; };
	VolumeData* GetVolumeData(const std::wstring &name);
	MeshData* GetMeshData(const std::wstring &name);
	Annotations* GetAnnotations(const std::wstring &name);
	DataGroup* GetGroup(const std::wstring &name);
	DataGroup* GetGroup(int index);
	DataGroup* GetGroup(VolumeData* vd);
	MeshGroup* GetMGroup(const std::wstring &str);
	//add
	DataGroup* AddVolumeData(VolumeData* vd, const std::wstring &group_name = L"");
	void AddMeshData(MeshData* md);
	void AddAnnotations(Annotations* ann);
	std::wstring AddGroup(const std::wstring& str, const std::wstring& prev_group = L"");
	DataGroup* AddOrGetGroup();
	std::wstring AddMGroup(const std::wstring& str);
	MeshGroup* AddOrGetMGroup();
	//remove
	void RemoveVolumeData(const std::wstring &name);
	void RemoveVolumeDataDup(const std::wstring &name);//remove all duplicated data
	void ReplaceVolumeData(const std::wstring &name, VolumeData *dst);
	void RemoveMeshData(const std::wstring &name);
	void RemoveAnnotations(const std::wstring &name);
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
	void SetDraw(bool draw);
	void ToggleDraw();
	bool GetDraw();

	//handle camera
	void HandleProjection(int nx, int ny, bool vr = false);
	void HandleCamera(bool vr = false);
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
	fluo::Quaternion GetZeroQuat()
	{
		return m_zq;
	}
	void SetZeroQuat(double x, double y, double z, double w)
	{
		m_zq = fluo::Quaternion(x, y, z, w);
	}
	fluo::Quaternion GetRotations()
	{
		return m_q;
	}
	void GetRotations(double &rotx, double &roty, double &rotz)
	{
		rotx = m_rotx;
		roty = m_roty;
		rotz = m_rotz;
	}
	void SetRotations(double rotx, double roty, double rotz, bool notify);
	int GetOrientation();//same as the indices in the view panel
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
	void SetOffset()
	{
		if (m_obj_ctr_offx != 0.0 || m_obj_ctr_offy != 0.0 || m_obj_ctr_offz != 0.0 ||
			m_obj_rot_ctr_offx != 0.0 || m_obj_rot_ctr_offy != 0.0 || m_obj_rot_ctr_offz != 0.0 ||
			m_obj_rot_offx != 0.0 || m_obj_rot_offy != 0.0 || m_obj_rot_offz != 0.0)
			m_offset = true;
		else
			m_offset = false;
	}
	void GetObjCtrOff(double &dx, double &dy, double &dz)
	{
		dx = m_obj_ctr_offx;
		dy = m_obj_ctr_offy;
		dz = m_obj_ctr_offz;
	}
	void SetObjCtrOff(double dx, double dy, double dz)
	{
		m_obj_ctr_offx = dx;
		m_obj_ctr_offy = dy;
		m_obj_ctr_offz = dz;
		SetOffset();
	}
	void GetObjRotCtrOff(double &dx, double &dy, double &dz)
	{
		dx = m_obj_rot_ctr_offx;
		dy = m_obj_rot_ctr_offy;
		dz = m_obj_rot_ctr_offz;
	}
	void SetObjRotCtrOff(double dx, double dy, double dz)
	{
		m_obj_rot_ctr_offx = dx == 0.0 ? m_obj_ctrx : dx;
		m_obj_rot_ctr_offy = dy == 0.0 ? m_obj_ctry : dy;
		m_obj_rot_ctr_offz = dz == 0.0 ? m_obj_ctrz : dz;
		SetOffset();
	}
	void GetObjRotOff(double &dx, double &dy, double &dz)
	{
		dx = m_obj_rot_offx;
		dy = m_obj_rot_offy;
		dz = m_obj_rot_offz;
	}
	void SetObjRotOff(double dx, double dy, double dz)
	{
		m_obj_rot_offx = dx;
		m_obj_rot_offy = dy;
		m_obj_rot_offz = dz;
		SetOffset();
	}
	void SetOffsetTransform(const fluo::Transform &tf)
	{
		m_offset_tf = tf;
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
	void SetRotLock(bool mode)
	{
		m_rot_lock = mode;
	}
	bool GetRotLock()
	{
		return m_rot_lock;
	}
	//lock cam center
	void SetLockCamObject(bool bval) { m_lock_cam_object = bval; }
	bool GetLockCamObject() { return m_lock_cam_object; }
	void SetLockCenter();
	void SetLockCenterVol();
	void SetLockCenterRuler();
	void SetLockCenterSel();

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

	//intearctive
	void SetInteractive(bool val) { m_interactive = val; }
	bool GetInteractive() { return m_interactive; }

	//background color
	fluo::Color GetBackgroundColor();
	fluo::Color GetTextColor();
	void SetBackgroundColor(fluo::Color &color);

	//disply modes
	int GetDrawType() { return m_draw_type; }
	void SetVolMethod(int method);
	int GetVolMethod() { return m_vol_method; }
	void SetFog(bool b = true);
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
	void ResetMovieAngle();
	void StopMovie();
	//4d movie frame calculation
	void Get4DSeqRange(int &start_frame, int &end_frame);
	void Set4DSeqFrame(int frame, int start_frame, int end_frame, bool rewind);
	void UpdateVolumeData(int frame, VolumeData* vd);
	void ReloadVolumeData(int frame);
	//3d batch file calculation
	void Get3DBatRange(int &start_frame, int &end_frame);
	void Set3DBatFrame(int frame, int start_frame, int end_frame, bool rewind);

	//frame for capturing
	void EnableFrame() { m_draw_frame = true; }
	void DisableFrame() { m_draw_frame = false; };
	void CalcFrame();

	//scale bar
	void EnableScaleBar() { m_disp_scale_bar = true; }
	void DisableScaleBar() { m_disp_scale_bar = false; }
	void EnableSBText() { m_disp_scale_bar_text = true; }
	void DisableSBText() { m_disp_scale_bar_text = false; }
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
	//clear
	void ClearVolList();
	void ClearMeshList();

	//inteactive mode selection
	int GetIntMode();
	void SetIntMode(int mode);

	//set use 2d rendering results
	void SetPaintUse2d(bool use2d);
	bool GetPaintUse2d();

	//calculations
	void SetVolumeA(VolumeData* vd);
	void SetVolumeB(VolumeData* vd);

	//brush properties
	//change display
	void ChangeBrushSize(int value);

	//set clip mode
	void SetClipMode(int mode);
	int GetClipMode() { return m_clip_mode; }
	//restore clipping planes
	void RestorePlanes();
	//clipping plane rotations
	void ClipRotate();
	void SetClippingPlaneRotations(double rotx, double roty, double rotz);
	void GetClippingPlaneRotations(double &rotx, double &roty, double &rotz);
	void SetClipRotX(double val);
	void SetClipRotY(double val);
	void SetClipRotZ(double val);
	fluo::Quaternion GetClipRotation() { return m_q_cl; }
	//set clip values
	void SetClipValue(int i, int val);
	void SetClipValues(int i, int val1, int val2);
	void SetClipValues(const int val[6]);
	void ResetClipValues();
	void ResetClipValuesX();
	void ResetClipValuesY();
	void ResetClipValuesZ();

	//interpolation
	void SetIntp(bool mode);
	bool GetIntp();

	//text renderer
	flvr::TextRenderer* GetTextRenderer() { return &m_text_renderer; }

	//force draw
	void ForceDraw();

	//start loop update
	void StartLoopUpdate();
	void HaltLoopUpdate();
	void RefreshGL(int debug_code,
		bool erase = false,
		bool start_loop = true,
		bool lg_changed = true);

	//rulers
	void DrawRulers();
	flrd::RulerList* GetRulerList();
	void SetCurRuler(flrd::Ruler* ruler);
	flrd::Ruler* GetCurRuler();
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
	//track map file
	int GetTrackFileExist(bool save);//0:no trace group; 1:trace groups exists not saved; 2:saved
	TrackGroup* GetTrackGroup();
	std::wstring GetTrackGroupFile();
	void CreateTrackGroup();
	int LoadTrackGroup(const std::wstring& filename);
	int SaveTrackGroup(const std::wstring& filename);
	void ExportTrackGroup(const std::wstring& filename, unsigned int id);
	void DrawTraces();
	void GetTraces(bool update = false);

	//enlarge output image
	void SetKeepEnlarge(bool value)
	{
		m_keep_enlarge = value;
	}
	void SetEnlarge(bool value);
	void SetEnlargeScale(double value);
	bool GetEnlarge()
	{
		return m_enlarge;
	}
	double GetEnlargeScale()
	{
		return m_enlarge_scale;
	}

	//read pixels
	void ReadPixels(
		int chann, bool fp32,
		int &x, int &y, int &w, int &h,
		void** image);

	//set cell list
	void SetCellList(flrd::CelpList &list)
	{
		m_cell_list = list;
	}

	//get view info for external ops
	//get size, considering enlargement
	Size2D GetGLSize();
	fluo::Point GetMousePos(wxMouseEvent& e);
	bool GetMouseIn(wxPoint& p);
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
		obj_mat = glm::translate(obj_mat, glm::vec3(
			m_obj_transx,
			m_obj_transy,
			m_obj_transz));

		if (m_offset)
		{
			obj_mat = glm::translate(obj_mat, glm::vec3(
				m_obj_rot_ctr_offx - m_obj_ctrx,
				m_obj_ctry - m_obj_rot_ctr_offy,
				m_obj_rot_ctr_offz - m_obj_ctrz));
			//rotate object
			obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_roty + m_obj_rot_offy)), glm::vec3(0.0, 1.0, 0.0));
			obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotz + m_obj_rot_offz)), glm::vec3(0.0, 0.0, 1.0));
			obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotx + m_obj_rot_offx)), glm::vec3(1.0, 0.0, 0.0));
			//center object
			obj_mat = glm::translate(obj_mat, glm::vec3(
				-m_obj_rot_ctr_offx - m_obj_ctr_offx,
				-m_obj_rot_ctr_offy - m_obj_ctr_offy,
				-m_obj_rot_ctr_offz - m_obj_ctr_offz));
		}
		else
		{
			//rotate object
			obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
			obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
			obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
			//center object
			obj_mat = glm::translate(obj_mat, glm::vec3(
				-m_obj_ctrx,
				-m_obj_ctry,
				-m_obj_ctrz));
		}

		return obj_mat;
	}
	glm::mat4 GetDrawMat()
	{
		glm::mat4 drw_mat = m_mv_mat;
		//translate object
		drw_mat = glm::translate(drw_mat, glm::vec3(
			m_obj_transx,
			m_obj_transy,
			m_obj_transz));

		if (m_offset)
		{
			drw_mat = glm::translate(drw_mat, glm::vec3(
				m_obj_rot_ctr_offx - m_obj_ctrx,
				m_obj_ctry - m_obj_rot_ctr_offy,
				m_obj_rot_ctr_offz - m_obj_ctrz));
			//rotate object
			drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx + m_obj_rot_offx)), glm::vec3(1.0, 0.0, 0.0));
			drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty + m_obj_rot_offy)), glm::vec3(0.0, 1.0, 0.0));
			drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz + m_obj_rot_offz)), glm::vec3(0.0, 0.0, 1.0));
			//center object
			drw_mat = glm::translate(drw_mat, glm::vec3(
				-m_obj_rot_ctr_offx - m_obj_ctr_offx,
				-m_obj_rot_ctr_offy - m_obj_ctr_offy,
				-m_obj_rot_ctr_offz - m_obj_ctr_offz));
		}
		else
		{
			//rotate object
			drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
			drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
			drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
			//center object
			drw_mat = glm::translate(drw_mat, glm::vec3(
				-m_obj_ctrx,
				-m_obj_ctry,
				-m_obj_ctrz));
		}

		return drw_mat;
	}
	glm::mat4 GetInvtMat()
	{
		glm::mat4 inv_mat = m_mv_mat;
		//translate object
		inv_mat = glm::translate(inv_mat, glm::vec3(
			m_obj_transx,
			m_obj_transy,
			m_obj_transz));

		if (m_offset)
		{
			inv_mat = glm::translate(inv_mat, glm::vec3(
				m_obj_rot_ctr_offx - m_obj_ctrx,
				m_obj_ctry - m_obj_rot_ctr_offy,
				m_obj_rot_ctr_offz - m_obj_ctrz));
			//rotate object
			inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotz + m_obj_rot_offz)), glm::vec3(0.0, 0.0, 1.0));
			inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_roty + m_obj_rot_offy)), glm::vec3(0.0, 1.0, 0.0));
			inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotx + m_obj_rot_offx)), glm::vec3(1.0, 0.0, 0.0));
			//center object
			inv_mat = glm::translate(inv_mat, glm::vec3(
				-m_obj_rot_ctr_offx - m_obj_ctr_offx,
				-m_obj_rot_ctr_offy - m_obj_ctr_offy,
				-m_obj_rot_ctr_offz - m_obj_ctr_offz));
		}
		else
		{
			//rotate object
			inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
			inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
			inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
			//center object
			inv_mat = glm::translate(inv_mat, glm::vec3(
				-m_obj_ctrx,
				-m_obj_ctry,
				-m_obj_ctrz));
		}

		return inv_mat;
	}
	glm::mat4 GetDrawWorldMat()
	{
		glm::mat4 drw_mat = m_mv_mat;
		//translate object
		drw_mat = glm::translate(drw_mat, glm::vec3(
			m_obj_transx,
			m_obj_transy,
			m_obj_transz));

		//rotate object
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		drw_mat = glm::translate(drw_mat, glm::vec3(
			-m_obj_ctrx,
			-m_obj_ctry,
			-m_obj_ctrz));

		return drw_mat;
	}
	fluo::Transform GetInvOffsetMat()
	{
		return m_offset_tf;
	}
	fluo::Vector GetSide()
	{
		m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
		m_head.normalize();
		fluo::Vector side = Cross(m_up, m_head);
		return side;
	}

	void UpdateClips();

	void GetRenderSize(int& nx, int& ny);

	//void SetScriptBreak(bool val)
	//{
	//	m_scriptor.SetBreak(val);
	//}

	void SetFocusedSlider(wxBasisSlider* slider)
	{
		m_focused_slider = slider;
	}

public:
	RenderViewPanel* m_renderview_panel;
	//set gl context
	bool m_set_gl;
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
	bool m_draw_annotations;
	bool m_draw_camctr;
	double m_camctr_size;
	int m_draw_info;
	bool m_load_update;
	bool m_draw_frame;
	bool m_draw_legend;
	bool m_draw_colormap;
	bool m_mouse_focus;
	bool m_draw_rulers;
	//current volume
	VolumeData *m_cur_vol;
	VolumeData* m_cur_vol_save;
	//clipping settings
	int m_clip_mask;
	int m_clip_mode;//0-normal; 1-ortho planes; 2-rot difference
					//scale bar
	bool m_disp_scale_bar;
	bool m_disp_scale_bar_text;
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
	double m_scale_factor_saved;
	//scale mode
	int m_scale_mode;//zoom ratio meaning: 0-view; 1-pixel; 2-data(pixel*xy spc)
	//pin rotation center
	bool m_pin_rot_ctr;//enable auto compute of rot center based on data
	bool m_update_rot_ctr;//rot center needs update
	double m_pin_pick_thresh;//ray casting threshold value
	fluo::Point m_pin_ctr;//the center point for view rotation
	//linked rotation
	static bool m_linked_rot;
	static RenderCanvas* m_master_linked_view;

private:
	bool m_drawing;
	bool m_refresh;//own refresh command
	Size2D m_size;
	std::string m_GLversion;
	wxGLContext* m_glRC;
	bool m_sharedRC;
	MainFrame* m_frame;
	//populated lists of data
	bool m_vd_pop_dirty;
	std::vector <VolumeData*> m_vd_pop_list;
	bool m_md_pop_dirty;
	std::vector <MeshData*> m_md_pop_list;
	//real data list
	std::vector <TreeLayer*> m_layer_list;
	//ruler list
	flrd::RulerList m_ruler_list;
	flrd::Ruler *m_cur_ruler;
	//traces
	TrackGroup* m_track_group;
	//multivolume
	flvr::MultiVolumeRenderer* m_mvr;
	//highlighted comps
	flrd::CelpList m_cell_list;
	//fisrt volume data in the depth groups
	//VolumeData* m_first_depth_vd;
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
	//previous focus before brush
	wxWindow* m_prev_focus;

	//interactive modes
	int m_int_mode;  //interactive mode
					 //1-normal viewing
					 //2-painting
					 //3-rotate clipping planes
					 //4-one-time rendering update in painting mode
					 //5-ruler mode
					 //6-edit ruler by moving point
					 //7-paint with locator
					 //8-same as 4, but for paint ruler mode
					 //9-move ruler
					 //10-grow, click and hold to activate
					 //11-lock ruler point for relaxing
					 //12-grow with ruler
					 //13-pencil with multipoint ruler
					 //14-delete ruler point
					 //15-edit ruler by magnet
					 //16-edit crop frame
	int m_crop_type;
	bool m_force_clear;
	bool m_interactive;
	bool m_clear_buffer;
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
	//object rotation
	double m_obj_rotx, m_obj_roty, m_obj_rotz;
	//flag for using offset values
	bool m_offset;
	//obj center offset (for registration currently)
	double m_obj_ctr_offx, m_obj_ctr_offy, m_obj_ctr_offz;
	//obj rotation center offset
	double m_obj_rot_ctr_offx, m_obj_rot_ctr_offy, m_obj_rot_ctr_offz;
	//obj rotation offset (for registration currently)
	double m_obj_rot_offx, m_obj_rot_offy, m_obj_rot_offz;
	//offset transform
	fluo::Transform m_offset_tf;
	//object translation
	double m_obj_transx, m_obj_transy, m_obj_transz;
	//saved translation for free flight
	double m_obj_transx_saved, m_obj_transy_saved, m_obj_transz_saved;
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

	//clipping plane rotations
	fluo::Quaternion m_q_cl;
	fluo::Quaternion m_q_cl_zero;
	double m_rotx_cl, m_roty_cl, m_rotz_cl;

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
	glm::mat4 m_tex_mat = glm::mat4(1.0);

	//text renderer
	flvr::TextRenderer m_text_renderer;

	//enlargement
	bool m_keep_enlarge;
	bool m_enlarge;
	double m_enlarge_scale;

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
	bool m_use_openxr;
	bool m_lg_initiated;
	int m_vr_eye_idx;//0: left; 1: right
#ifdef _WIN32
#ifdef USE_XINPUT
	XboxController* m_controller;
	bool m_control_connected;
#endif
#endif

	//focused slider
	wxBasisSlider* m_focused_slider;

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
		const std::wstring& name, fluo::Color color,
		double font_height, bool hilighted = false);
	void DrawFrame();
	void DrawClippingPlanes(int face_winding);
	void SetColormapColors(int colormap, fluo::Color &c, double inv);
	void DrawColormap();
	void DrawGradBg();
	void DrawInfo(int nx, int ny, bool intactive);

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
	//draw out the framebuffer after composition
	void PrepFinalBuffer();
	void ClearFinalBuffer();
	void DrawFinalBuffer();
	//vr buffers
	void PrepVRBuffer();
	void ClearVRBuffer();
	void DrawVRBuffer();
	//different volume drawing modes
	void DrawVolumesMulti(std::vector<VolumeData*> &list, int peel = 0);
	void DrawVolumesComp(std::vector<VolumeData*> &list, bool mask = false, int peel = 0);
	void DrawMIP(VolumeData* vd, int peel = 0);
	void DrawOVER(VolumeData* vd, bool mask, int peel = 0);
	//overlay passes
	void DrawOLShading(VolumeData* vd);
	void DrawOLShadows(std::vector<VolumeData*> &vlist);
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
	bool UpdateBrushState();

	//selection
	void Pick();
	bool PickMesh();
	bool PickVolume();
	void SetCompSelection(fluo::Point& p, int mode);//node: 0-exclusive; 1-add or remove

	//system call
	void OnDraw(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnQuitFscreen(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);
#ifdef _WIN32
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

	//draw quad
	void DrawViewQuad();

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

	friend class RenderViewPanel;
};

inline Size2D RenderCanvas::GetGLSize()
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	wxSize size = GetSize() * dval;
	if (m_enlarge)
		size.Set(size.x * m_enlarge_scale,
			size.y * m_enlarge_scale);
	return Size2D(size.x, size.y);
}

inline fluo::Point RenderCanvas::GetMousePos(wxMouseEvent& e)
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	fluo::Point pnt(e.GetPosition().x, e.GetPosition().y, 0);
	pnt *= dval;
	if (m_enlarge)
		pnt = pnt * m_enlarge_scale;
	return pnt;
}

inline bool RenderCanvas::GetMouseIn(wxPoint& p)
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	wxRect rect = GetClientRect();
	rect.SetSize(rect.GetSize() * dval);
	return rect.Contains(p);
}
#endif//_RENDERCANVAS_H_
