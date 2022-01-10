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
		objects_.push_back(view);

		//add default values here
		view->addValue(gstInitialized, bool(false));
		view->addValue(gstInitView, bool(false));
		view->addRvalu(gstCurrentVolume, (Referenced*)(0));
		view->addValue(gstSetGl, bool(false));
		view->addValue(gstRunScript, bool(false));
		view->addValue(gstScriptFile, std::string(""));
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
		view->addValue(gstCaptureFile, std::string(""));
		view->addValue(gstBatFolder, std::string(""));
		view->addValue(gstRetainFb, bool(false));
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
		view->addValue(gstDrawFrame, bool(false));
		view->addValue(gstTestSpeed, bool(false));
		view->addValue(gstDrawClip, bool(false));
		view->addValue(gstDrawLegend, bool(false));
		view->addValue(gstDrawColormap, bool(false));
		view->addValue(gstMouseFocus, bool(false));
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
		view->addValue(gstScaleBarText, std::string(""));
		view->addValue(gstScaleBarNum, std::string(""));
		view->addValue(gstScaleBarHeight, double(0));
		view->addValue(gstOrthoLeft, double(0));
		view->addValue(gstOrthoRight, double(1));
		view->addValue(gstOrthoBottom, double(0));
		view->addValue(gstOrthoTop, double(1));
		view->addValue(gstScaleFactor, double(1));
		view->addValue(gstScaleFactorSaved, double(1));
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
		view->addValue(gstWidth, long(0));
		view->addValue(gstHeight, long(0));
		view->addValue(gstVolListDirty, bool(true));
		view->addValue(gstMshListDirty, bool(false));
		view->addValue(gstBgColor, Color(0.0));
		view->addValue(gstBgColorInv, Color(1.0));
		view->addValue(gstGradBg, bool(false));
		view->addValue(gstAov, double(15));
		view->addValue(gstNearClip, double(0.1));
		view->addValue(gstFarClip, double(100));
		view->addValue(gstInterpolation, bool(true));
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
		view->addValue(gstCurrentFb, unsigned long(0));
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
		view->addValue(gstMouseX, long(-1));
		view->addValue(gstMouseY, long(-1));
		view->addValue(gstMousePrvX, long(-1));
		view->addValue(gstMousePrvY, long(-1));
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
		view->addValue(gstTouchEnable, bool(false));
		view->addValue(gstPtr1Id, long(-1));
		view->addValue(gstPtr1X, long(0));
		view->addValue(gstPtr1Y, long(0));
		view->addValue(gstPtr1XSave, long(0));
		view->addValue(gstPtr1YSave, long(0));
		view->addValue(gstPtr2Id, long(-1));
		view->addValue(gstPtr2X, long(0));
		view->addValue(gstPtr2Y, long(0));
		view->addValue(gstPtr2XSave, long(0));
		view->addValue(gstPtr2YSave, long(0));
		view->addValue(gstFullScreen, bool(false));
		view->addValue(gstVrEnable, bool(false));
		view->addValue(gstOpenvrEnable, bool(false));
		view->addValue(gstVrSizeX, unsigned long(0));
		view->addValue(gstVrSizeY, unsigned long(0));
		view->addValue(gstVrEyeOffset, double(6));
		view->addValue(gstVrEyeIdx, long(0));
	}
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

	objects_.push_back(new_view);

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

