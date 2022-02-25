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

#include "RenderviewFactory.hpp"
#include <Names.hpp>

using namespace fluo;

RenderviewFactory::RenderviewFactory()
{
	m_name = gstRenderviewFactory;
	default_object_name_ = gstDefaultRenderview;
}

RenderviewFactory::~RenderviewFactory()
{

}

void RenderviewFactory::createDefault()
{
	if (!getDefault())
	{
		Renderview* view = new Renderview();
		view->setName(default_object_name_);
		objects_.push_front(view);

		//add default values here
		//common
		view->addValue(gstBounds, BBox());
		view->addValue(gstDepthAtten, bool(false));
		view->addValue(gstDaInt, double(0.5));
		view->addValue(gstDaStart, double(0));
		view->addValue(gstDaEnd, double(1));
		//shadow
		view->addValue(gstShadowDirEnable, bool(false));
		view->addValue(gstShadowDirX, double(0));
		view->addValue(gstShadowDirY, double(0));
		//output (2d) adjustments
		view->addValue(gstGammaR, double(1));
		view->addValue(gstGammaG, double(1));
		view->addValue(gstGammaB, double(1));
		view->addValue(gstBrightnessR, double(1));
		view->addValue(gstBrightnessG, double(1));
		view->addValue(gstBrightnessB, double(1));
		view->addValue(gstEqualizeR, double(0));
		view->addValue(gstEqualizeG, double(0));
		view->addValue(gstEqualizeB, double(0));
		view->addValue(gstSyncR, bool(false));
		view->addValue(gstSyncG, bool(false));
		view->addValue(gstSyncB, bool(false));
		//special
		view->addValue(gstFocus, bool(false));
		view->addValue(gstSizeX, long(800));
		view->addValue(gstSizeY, long(600));
		view->addValue(gstUseDefault, bool(false));
		view->addValue(gstClipLinkChan, bool(false));
		view->addValue(gstClipPlaneMode, long(Renderview::kNormal));
		view->addValue(gstInitialized, bool(false));
		view->addValue(gstInitView, bool(false));
		view->addValue(gstCurrentSelect, long(0));//0:root; 1:view; 2:volume; 3:mesh; 5:volume group; 6:mesh group
		view->addRvalu(gstCurrentVolume, (Referenced*)(0));
		view->addValue(gstCurVolIdx, long(-1));
		view->addRvalu(gstCurrentMesh, (Referenced*)(0));
		view->addValue(gstCurMshIdx, long(-1));
		view->addValue(gstSetGl, bool(false));
		view->addValue(gstRunScript, bool(false));
		view->addValue(gstScriptFile, std::wstring(L""));
		view->addValue(gstCapture, bool(false));
		view->addValue(gstCaptureRot, bool(false));
		view->addValue(gstCaptureRotOver, bool(false));
		view->addValue(gstCaptureTime, bool(false));
		view->addValue(gstCaptureBat, bool(false));
		view->addValue(gstCaptureParam, bool(false));
		view->addValue(gstBeginFrame, long(0));
		view->addValue(gstEndFrame, long(0));
		view->addValue(gstCurrentFrame, long(0));
		view->addValue(gstPreviousFrame, long(0));
		view->addValue(gstParamFrame, long(0));
		view->addValue(gstTotalFrames, long(0));
		view->addValue(gstCaptureFile, std::wstring(L""));
		view->addValue(gstCaptureAlpha, bool(false));
		view->addValue(gstCaptureFloat, bool(false));
		view->addValue(gstCaptureCompress, bool(false));
		view->addValue(gstBatFolder, std::wstring(L""));
		view->addValue(gstRetainFb, bool(false));
		view->addValue(gstUpdateOrder, long(0));
		view->addValue(gstUpdating, bool(true));
		view->addValue(gstDrawAll, bool(true));
		view->addValue(gstDrawType, long(1));
		view->addValue(gstDrawMask, bool(true));
		view->addValue(gstMixMethod, long(Renderview::MIX_METHOD_SEQ));
		view->addValue(gstPeelNum, long(1));
		view->addValue(gstMicroBlendEnable, bool(false));
		view->addValue(gstDrawAnnotations, bool(true));
		view->addValue(gstDrawCamCtr, bool(false));
		view->addValue(gstCamCtrSize, double(2.0));
		view->addValue(gstDrawInfo, long(250));
		view->addValue(gstLoadUpdate, bool(false));
		view->addValue(gstDrawCropFrame, bool(false));
		view->addValue(gstTestSpeed, bool(false));
		view->addValue(gstDrawClip, bool(false));
		view->addValue(gstDrawLegend, bool(false));
		view->addValue(gstDrawColormap, bool(false));
		view->addValue(gstTestWiref, bool(false));
		view->addValue(gstDrawBounds, bool(false));
		view->addValue(gstDrawGrid, bool(false));
		view->addValue(gstDrawRulers, bool(false));
		view->addValue(gstClipMask, long(-1));
		view->addValue(gstClipMode, long(2));
		view->addValue(gstDrawScaleBar, bool(false));
		view->addValue(gstDrawScaleBarText, bool(false));
		view->addValue(gstScaleBarLen, double(50));
		view->addValue(gstScaleBarPosX, double(0));
		view->addValue(gstScaleBarPosY, double(0));
		view->addValue(gstScaleBarUnit, long(1));
		view->addValue(gstScaleBarText, std::wstring(L""));
		view->addValue(gstScaleBarNum, std::wstring(L"50"));
		view->addValue(gstScaleBarHeight, double(0));
		view->addValue(gstOrthoLeft, double(0));
		view->addValue(gstOrthoRight, double(1));
		view->addValue(gstOrthoBottom, double(0));
		view->addValue(gstOrthoTop, double(1));
		view->addValue(gstScaleFactor, double(1));
		view->addValue(gstScaleFactorSaved, double(1));
		view->addValue(gstScaleFactor121, double(1));
		view->addValue(gstScaleMode, long(0));
		view->addValue(gstAutoPinRotCtr, bool(false));
		view->addValue(gstPinRotCtr, bool(false));
		view->addValue(gstRotCtrDirty, bool(false));
		view->addValue(gstPinThresh, double(0.6));
		view->addValue(gstRotCtrPin, Point());
		view->addValue(gstPointVolumeMode, long(0));
		view->addValue(gstRulerUseTransf, bool(false));
		view->addValue(gstRulerTransient, bool(true));
		view->addValue(gstLinkedRot, bool(false));
		view->addValue(gstPaintCount, bool(false));
		view->addValue(gstPaintColocalize, bool(false));
		view->addValue(gstRulerRelax, bool(false));
		view->addValue(gstDrawing, bool(false));
		view->addValue(gstRefresh, bool(false));
		view->addValue(gstRefreshErase, bool(false));
		view->addValue(gstRefreshNotify, bool(false));
		view->addValue(gstWidth, long(0));
		view->addValue(gstHeight, long(0));
		view->addValue(gstVolListDirty, bool(false));
		view->addValue(gstMshListDirty, bool(false));
		view->addValue(gstFullVolListDirty, bool(false));
		view->addValue(gstFullMshListDirty, bool(false));
		view->addValue(gstTextColorMode, long(0));
		view->addValue(gstTextColor, Color());
		view->addValue(gstBgColor, Color(0.0));
		view->addValue(gstBgColorInv, Color(1.0));
		view->addValue(gstGradBg, bool(false));
		view->addValue(gstAov, double(15));
		view->addValue(gstNearClip, double(0.1));
		view->addValue(gstFarClip, double(100));
		view->addValue(gstInterpolate, bool(true));
		view->addValue(gstInterMode, long(1));
		view->addValue(gstForceClear, bool(false));
		view->addValue(gstInteractive, bool(false));
		view->addValue(gstAdaptive, bool(true));
		view->addValue(gstClearBuffer, bool(false));
		view->addValue(gstBrushState, long(0));
		view->addValue(gstGrowEnable, bool(false));
		view->addValue(gstResize, bool(false));
		view->addValue(gstDrawBrush, bool(false));
		view->addValue(gstPaintEnable, bool(false));
		view->addValue(gstPaintDisplay, bool(false));
		view->addValue(gstClearPaint, bool(true));
		view->addValue(gstPaintMode, long(0));
		view->addValue(gstCurFramebuffer, unsigned long(0));
		view->addValue(gstPerspective, bool(false));
		view->addValue(gstFree, bool(false));
		view->addValue(gstCamDist, double(10));
		view->addValue(gstCamDistIni, double(10));
		view->addValue(gstCamTransX, double(0));
		view->addValue(gstCamTransY, double(0));
		view->addValue(gstCamTransZ, double(0));
		view->addValue(gstCamTransSavedX, double(0));
		view->addValue(gstCamTransSavedY, double(0));
		view->addValue(gstCamTransSavedZ, double(0));
		view->addValue(gstCamRotX, double(0));
		view->addValue(gstCamRotY, double(0));
		view->addValue(gstCamRotZ, double(0));
		view->addValue(gstCamRotSavedX, double(0));
		view->addValue(gstCamRotSavedY, double(0));
		view->addValue(gstCamRotSavedZ, double(0));
		view->addValue(gstCamRotZeroX, double(0));
		view->addValue(gstCamRotZeroY, double(0));
		view->addValue(gstCamRotZeroZ, double(0));
		view->addValue(gstCamCtrX, double(0));
		view->addValue(gstCamCtrY, double(0));
		view->addValue(gstCamCtrZ, double(0));
		view->addValue(gstCamCtrSavedX, double(0));
		view->addValue(gstCamCtrSavedY, double(0));
		view->addValue(gstCamCtrSavedZ, double(0));
		view->addValue(gstCamRotQ, Quaternion());
		view->addValue(gstCamRotZeroQ, Quaternion());
		view->addValue(gstCamUp, Vector(0, 1, 0));
		view->addValue(gstCamHead, Vector(0, 0, -1));
		view->addValue(gstClipRotQ, Quaternion());
		view->addValue(gstClipRotZeroQ, Quaternion());
		view->addValue(gstClipRotX, double(0));
		view->addValue(gstClipRotY, double(0));
		view->addValue(gstClipRotZ, double(0));
		view->addValue(gstObjCtrX, double(0));
		view->addValue(gstObjCtrY, double(0));
		view->addValue(gstObjCtrZ, double(0));
		view->addValue(gstObjTransX, double(0));
		view->addValue(gstObjTransY, double(0));
		view->addValue(gstObjTransZ, double(0));
		view->addValue(gstObjTransSavedX, double(0));
		view->addValue(gstObjTransSavedY, double(0));
		view->addValue(gstObjTransSavedZ, double(0));
		view->addValue(gstObjRotX, double(0));
		view->addValue(gstObjRotY, double(180));
		view->addValue(gstObjRotZ, double(180));
		view->addValue(gstGearedEnable, bool(false));
		view->addValue(gstCamLockObjEnable, bool(false));
		view->addValue(gstCamLockCtr, Point());
		view->addValue(gstCamLockPick, bool(false));
		view->addValue(gstRadius, double(348));
		view->addValue(gstMovInitAng, double(0));
		view->addValue(gstMovStartAng, double(0));
		view->addValue(gstMovEndAng, double(0));
		view->addValue(gstMovCurAng, double(0));
		view->addValue(gstMovStep, double(0));
		view->addValue(gstMovRotAxis, long(1));
		view->addValue(gstMovSeqNum, long(0));
		view->addValue(gstMovRewind, bool(false));
		view->addValue(gstMovStage, long(0));
		view->addValue(gstMovRewind4d, bool(false));
		view->addValue(gstMovRunning, bool(false));
		view->addValue(gstCropX, long(-1));
		view->addValue(gstCropY, long(-1));
		view->addValue(gstCropW, long(-1));
		view->addValue(gstCropH, long(-1));
		view->addValue(gstColor1, Color(0, 0, 1));
		view->addValue(gstColor2, Color(0, 0, 1));
		view->addValue(gstColor3, Color(0, 1, 1));
		view->addValue(gstColor4, Color(0, 1, 0));
		view->addValue(gstColor5, Color(1, 1, 0));
		view->addValue(gstColor6, Color(1, 0, 0));
		view->addValue(gstColor7, Color(1, 0, 0));
		view->addValue(gstColVal1, double(0));
		view->addValue(gstColVal2, double(0));
		view->addValue(gstColVal3, double(0.25));
		view->addValue(gstColVal4, double(0.5));
		view->addValue(gstColVal5, double(0.75));
		view->addValue(gstColVal6, double(1));
		view->addValue(gstColVal7, double(1));
		view->addValue(gstPick, bool(false));
		view->addValue(gstPreDraw, bool(false));
		view->addValue(gstTextSize, double(14));
		view->addValue(gstKeepEnlarge, bool(false));
		view->addValue(gstEnlarge, bool(false));
		view->addValue(gstEnlargeScale, double(1));
		view->addValue(gstBenchmark, bool(false));
		view->addValue(gstNodrawCount, long(0));
		view->addValue(gstLoadMainThread, bool(false));
		view->addValue(gstResMode, long(1));
		view->addValue(gstFullScreen, bool(false));
		view->addValue(gstVrEnable, bool(false));
		view->addValue(gstOpenvrEnable, bool(false));
		view->addValue(gstVrSizeX, unsigned long(0));
		view->addValue(gstVrSizeY, unsigned long(0));
		view->addValue(gstVrEyeOffset, double(6));
		view->addValue(gstVrEyeIdx, long(0));
		view->addValue(gstSwapBuffers, bool(false));
		view->addValue(gstLineWidth, double(3));
		view->addValue(gstLevelOffset, long(0));
		view->addValue(gstHwnd, unsigned long long(0));
		view->addValue(gstHinstance, unsigned long long(0));
		view->addValue(gstBmRuntime, unsigned long long(0));
		view->addValue(gstBmFrames, unsigned long long(0));
		view->addValue(gstBmFps, double(0));
		view->addValue(gstSelectedMshName, std::string());
		view->addValue(gstSelectedVolName, std::string());
		view->addValue(gstSelPointVolume, Point());
		view->addValue(gstMouseX, long(-1));
		view->addValue(gstMouseY, long(-1));
		view->addValue(gstMousePrvX, long(-1));
		view->addValue(gstMousePrvY, long(-1));
		view->addValue(gstMouseClientX, long(-1));
		view->addValue(gstMouseClientY, long(-1));
		view->addValue(gstMouseIn, bool(true));
		view->addValue(gstMouseDrag, bool(false));
		view->addValue(gstMouseWheel, long(0));
		view->addValue(gstRenderviewPanelId, long(-1));
	}
}

#define ADD_BEFORE_EVENT(obj, name, funct) \
	obj->setValueChangingFunction(name, std::bind(&Renderview::funct, obj, std::placeholders::_1))

#define ADD_AFTER_EVENT(obj, name, funct) \
	obj->setValueChangedFunction(name, std::bind(&Renderview::funct, obj, std::placeholders::_1))

void RenderviewFactory::setEventHandler(Renderview* view)
{
	//handle before events
	//ADD_BEFORE_EVENT(view, gstMipMode, OnMipModeChanging);

	//handle after events
	ADD_AFTER_EVENT(view, gstSizeX, OnSizeXChanged);
	ADD_AFTER_EVENT(view, gstSizeY, OnSizeYChanged);
	ADD_AFTER_EVENT(view, gstEnlargeScale, OnEnlargeScaleChanged);
	ADD_AFTER_EVENT(view, gstCamRotX, OnCamRotChanged);
	ADD_AFTER_EVENT(view, gstCamRotY, OnCamRotChanged);
	ADD_AFTER_EVENT(view, gstCamRotZ, OnCamRotChanged);
	ADD_AFTER_EVENT(view, gstPerspective, OnPerspectiveChanged);
	ADD_AFTER_EVENT(view, gstVolListDirty, OnVolListDirtyChanged);
	ADD_AFTER_EVENT(view, gstMshListDirty, OnMshListDirtyChanged);
	ADD_AFTER_EVENT(view, gstFullVolListDirty, OnFullVolListDirtyChanged);
	ADD_AFTER_EVENT(view, gstFullMshListDirty, OnFullMshListDirtyChanged);
	ADD_AFTER_EVENT(view, gstCurrentVolume, OnCurrentVolumeChanged);
	ADD_AFTER_EVENT(view, gstCurrentMesh, OnCurrentMeshChanged);
	ADD_AFTER_EVENT(view, gstTextColorMode, OnTextColorModeChanged);
	ADD_AFTER_EVENT(view, gstInterMode, OnInterModeChanged);
	ADD_AFTER_EVENT(view, gstPaintMode, OnPaintModeChanged);
}

Renderview* RenderviewFactory::build(Renderview* view)
{
	if (view)
		return clone(view);
	unsigned int default_id = 0;
	return clone(default_id);
}

Renderview* RenderviewFactory::clone(Renderview* view)
{
	incCounter();

	Object* new_view = view->clone(CopyOp::DEEP_COPY_ALL);
	new_view->setId(global_id_);
	std::string name = "renderview" + std::to_string(local_id_);
	new_view->setName(name);
	new_view->addRvalu(gstFactory, this);

	objects_.push_front(new_view);

	return dynamic_cast<Renderview*>(new_view);
}

Renderview* RenderviewFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		Renderview* view = dynamic_cast<Renderview*>(object);
		if (view)
			return clone(view);
	}
	return 0;
}

