/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#ifndef RENDERVIEW_HPP
#define RENDERVIEW_HPP

#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#include <Group.hpp>
#include <Names.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <openvr.h>

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
//wacom support
//#include <wx/msw/private.h>
#include <MSGPACK.h>
#include <wintab.h>
#define PACKETDATA	(/*PK_X | PK_Y | */PK_BUTTONS |\
		PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include <PKTDEF.h>
#include <Wacom/Utils.h>
#ifdef USE_XINPUT
#include <XInput/XboxController.h>
#endif
#endif

class VolumeLoader;
class Interpolator;
namespace flrd
{
	class VolumeSelector;
	class VolumeCalculator;
	class KernelExecutor;
	class Ruler;
	class RulerList;
	class RulerHandler;
	class RulerRenderer;
	class Tracks;
	class CelpList;
	class ScriptProc;
	class VolumePoint;
}
namespace flvr
{
	class TextRenderer;
	class MultiVolumeRenderer;
}
namespace fluo
{
	class RenderviewFactory;
	class Renderview : public Group
	{
	public:
		enum MixMethods
		{
			MIX_METHOD_SEQ = 1,
			MIX_METHOD_MULTI = 2,
			MIX_METHOD_COMP = 3
		};
		enum InitMask
		{
			INIT_NONE = 0,
			INIT_BOUNDS = 1 << 0,
			INIT_CENTER = 1 << 1,
			INIT_TRANSL = 1 << 2,
			INIT_ROTATE = 1 << 3,
			INIT_OBJ_TRANSL = 1 << 4,
		};
		//plane modes
		enum ClipPlaneModes
		{
			kNormal,
			kFrame,
			kLowTrans,
			kLowTransBack,
			kNormalBack,
			kNone
		};
		//clipping plane winding
		enum ClipPlaneWinding
		{
			CULL_OFF = 0,
			FRONT_FACE = 1,
			BACK_FACE = 2
		};
		//information bits
		enum InfoEntries
		{
			INFO_NONE  = 0,
			INFO_DISP  = 1 << 0,
			INFO_POS   = 1 << 1,
			INFO_FRAME = 1 << 2,
			INFO_FPS   = 1 << 3,
			INFO_T     = 1 << 4,
			INFO_X     = 1 << 5,
			INFO_Y     = 1 << 6,
			INFO_Z     = 1 << 7
		};

		Renderview();
		Renderview(const Renderview& view, const CopyOp& copyop = CopyOp::SHALLOW_COPY);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new Renderview(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const Renderview*>(obj) != NULL;
		}

		virtual const char* className() const { return "Renderview"; }

		virtual Renderview* asRenderview() { return this; }
		virtual const Renderview* asRenderview() const { return this; }

		//migrated from the VRenderGLView class
		//initialization
		void Init();

		//Clear tex, loader
		void Clear();

		//recalculate view according to object bounds
		void InitView(unsigned int type = INIT_BOUNDS);

		//temporary lists
		void PopVolumeList();
		void PopMeshList();
		//clear
		void ClearVolList();
		void ClearMeshList();

		//handle camera
		void HandleProjection(int nx, int ny, bool vr = false);
		void HandleCamera(bool vr = false);

		//movie capture
		void Set3DRotCapture(const std::wstring &cap_file, double start_angle, double end_angle,
			double step, long frames, long rot_axis, bool rewind);
		//time sequence data
		void Set4DSeqCapture(const std::wstring &cap_file, long begin_frame, long end_frame, bool rewind);
		//batch files
		void Set3DBatCapture(const std::wstring &cap_file, long begin_frame, long end_frame);
		//parameter recording/capture
		void SetParamCapture(const std::wstring &cap_file, long begin_frame, long end_frame, bool rewind);
		//set parameters
		void SetParams(double t);
		//reset and stop
		void ResetMovieAngle();
		void StopMovie();
		//4d movie frame calculation
		void Get4DSeqRange(long &start_frame, long &end_frame);
		void Set4DSeqFrame(long frame, long start_frame, long end_frame, bool rewind);
		void UpdateVolumeData(long frame, VolumeData* vd);
		void ReloadVolumeData(int frame);
		//3d batch file calculation
		void Get3DBatRange(long &start_frame, long &end_frame);
		void Set3DBatFrame(long frame, long start_frame, long end_frame, bool rewind);
		//crop for capturing
		void CalculateCrop();

		//segment volumes in current view
		void Segment();

		//get volume selector
		flrd::VolumeSelector* GetVolumeSelector() { return m_selector; }
		//get volume calculator
		flrd::VolumeCalculator* GetVolumeCalculator() { return m_calculator; }
		//get kernel executor
		flrd::KernelExecutor* GetKernelExecutor() { return m_kernel_executor; }
		//text renderer
		flvr::TextRenderer* GetTextRenderer() { return m_text_renderer; }

		//force draw
		void ForceDraw();

		//start loop update
		void StartLoopUpdate();
		void HaltLoopUpdate();
		void RefreshGL(int debug_code, bool erase = false, bool start_loop = true);

		//rulers
		void DrawRulers();
		flrd::Ruler* GetRuler(unsigned int id);
		flrd::RulerList* GetRulerList() { return m_ruler_list; }
		flrd::RulerHandler* GetRulerHandler() { return m_ruler_handler; }
		flrd::RulerRenderer* GetRulerRenderer() { return m_ruler_renderer; }

		//draw highlighted comps
		void DrawCells();
		unsigned int DrawCellVerts(std::vector<float>& verts);
		void GetCellPoints(BBox& box,
			Point& p1, Point& p2, Point& p3, Point& p4,
			Transform& mv, Transform& p);

		//traces
		flrd::Tracks* GetTraceGroup() { return m_trace_group; }
		void CreateTraceGroup();
		int LoadTraceGroup(const std::wstring &filename);
		int SaveTraceGroup(const std::wstring &filename);
		void DrawTraces();
		void GetTraces(bool update = false);

		//read pixels
		void ReadPixels(long chann, bool fp32,
			long &x, long &y, long &w, long &h, void** image);

		//set cell list
		void SetCellList(flrd::CelpList *list) { m_cell_list = list; }

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
			double dx, dy, dz;
			glm::mat4 obj_mat = m_mv_mat;
			//translate object
			getValue(gstObjTransX, dx); getValue(gstObjTransY, dy); getValue(gstObjTransZ, dz);
			obj_mat = glm::translate(obj_mat, glm::vec3(dx, dy, dz));
			//rotate object
			getValue(gstObjRotX, dx); getValue(gstObjRotY, dy); getValue(gstObjRotZ, dz);
			obj_mat = glm::rotate(obj_mat, float(glm::radians(dy)), glm::vec3(0.0, 1.0, 0.0));
			obj_mat = glm::rotate(obj_mat, float(glm::radians(dz)), glm::vec3(0.0, 0.0, 1.0));
			obj_mat = glm::rotate(obj_mat, float(glm::radians(dx)), glm::vec3(1.0, 0.0, 0.0));
			//center object
			getValue(gstObjCtrX, dx); getValue(gstObjCtrY, dy); getValue(gstObjCtrZ, dz);
			obj_mat = glm::translate(obj_mat, glm::vec3(-dx, -dy, -dz));
			return obj_mat;
		}
		glm::mat4 GetDrawMat()
		{
			double dx, dy, dz;
			glm::mat4 drw_mat = m_mv_mat;
			//translate object
			getValue(gstObjTransX, dx); getValue(gstObjTransY, dy); getValue(gstObjTransZ, dz);
			drw_mat = glm::translate(drw_mat, glm::vec3(dx, dy, dz));
			//rotate object
			getValue(gstObjRotX, dx); getValue(gstObjRotY, dy); getValue(gstObjRotZ, dz);
			drw_mat = glm::rotate(drw_mat, float(glm::radians(dx)), glm::vec3(1.0, 0.0, 0.0));
			drw_mat = glm::rotate(drw_mat, float(glm::radians(dy)), glm::vec3(0.0, 1.0, 0.0));
			drw_mat = glm::rotate(drw_mat, float(glm::radians(dz)), glm::vec3(0.0, 0.0, 1.0));
			//center object
			getValue(gstObjCtrX, dx); getValue(gstObjCtrY, dy); getValue(gstObjCtrZ, dz);
			drw_mat = glm::translate(drw_mat, glm::vec3(-dx, -dy, -dz));
			return drw_mat;
		}
		glm::mat4 GetInvtMat()
		{
			double dx, dy, dz;
			glm::mat4 inv_mat = m_mv_mat;
			//translate object
			getValue(gstObjTransX, dx); getValue(gstObjTransY, dy); getValue(gstObjTransZ, dz);
			inv_mat = glm::translate(inv_mat, glm::vec3(dx, dy, dz));
			//rotate object
			getValue(gstObjRotX, dx); getValue(gstObjRotY, dy); getValue(gstObjRotZ, dz);
			inv_mat = glm::rotate(inv_mat, float(glm::radians(dz)), glm::vec3(0.0, 0.0, 1.0));
			inv_mat = glm::rotate(inv_mat, float(glm::radians(dy)), glm::vec3(0.0, 1.0, 0.0));
			inv_mat = glm::rotate(inv_mat, float(glm::radians(dx)), glm::vec3(1.0, 0.0, 0.0));
			//center object
			getValue(gstObjCtrX, dx); getValue(gstObjCtrY, dy); getValue(gstObjCtrZ, dz);
			inv_mat = glm::translate(inv_mat, glm::vec3(-dx, -dy, -dz));
			return inv_mat;
		}

		void UpdateClips();

		void DrawBounds();
		void DrawGrid();
		void DrawCamCtr();
		void DrawScaleBar();
		void DrawLegend();
		void DrawName(double x, double y, int nx, int ny,
			const std::string &name, Color color,
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
		void GetRenderSize(long &nx, long &ny);
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
			Color &color,
			glm::mat4 &matrix);
		void DrawBrush();
		void PaintStroke();
		void DisplayStroke();

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

		//brush states update
		void SetBrush(int mode);
		void UpdateBrushState();

		//selection
		void Pick();
		void PickMesh();
		void PickVolume();
		void SetCompSelection(Point& p, int mode);//node: 0-exclusive; 1-add or remove

		//draw quad
		void DrawViewQuad();

		void switchLevel(VolumeData *vd);

#ifdef _WIN32
		//wacom tablet
		HCTX TabletInit(HWND hWnd, HINSTANCE hInst);
		void InitOpenVR();
#endif

		friend class RenderviewFactory;

	protected:
		virtual ~Renderview();

	private:
		//temporary lists of data
		std::vector<ref_ptr<VolumeData>> m_vol_list;
		std::vector<ref_ptr<MeshData>> m_msh_list;
		//some of these may be put on the scenegraph later
		//ruler list
		flrd::RulerList *m_ruler_list;
		flrd::Ruler *m_cur_ruler;
		//traces
		flrd::Tracks* m_trace_group;
		//multivolume
		flvr::MultiVolumeRenderer* m_mvr;
		//highlighted comps
		flrd::CelpList *m_cell_list;
		//volume selector for segmentation
		flrd::VolumeSelector *m_selector;
		//calculator
		flrd::VolumeCalculator *m_calculator;
		//kernel executor
		flrd::KernelExecutor *m_kernel_executor;
		//scriptor
		flrd::ScriptProc *m_scriptor;
		//handle rulers
		flrd::RulerHandler *m_ruler_handler;
		flrd::RulerRenderer *m_ruler_renderer;
		flrd::VolumePoint *m_vp;
		VolumeLoader *m_loader;
		Interpolator* m_interpolator;

		//glm matrices
		glm::mat4 m_mv_mat;
		glm::mat4 m_proj_mat;
		glm::mat4 m_tex_mat;

		//text renderer
		flvr::TextRenderer *m_text_renderer;

#ifdef _WIN32
		//wacom support
		static HCTX m_hTab;
		static LOGCONTEXTA m_lc;
		decltype(GetPointerInfo)* GetPI;
		vr::IVRSystem *m_vr_system;
#ifdef USE_XINPUT
		XboxController* m_controller;
#endif
#endif

		private:
			void OnCamRotChanged(Event& event);//update rotation
			void OnPerspectiveChanged(Event& event);
			void OnVolListDirtyChanged(Event& event);
			void OnMshListDirtyChanged(Event& event);
	};
}

#endif//RENDERVIEW_HPP