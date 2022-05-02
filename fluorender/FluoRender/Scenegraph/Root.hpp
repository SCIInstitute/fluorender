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

#ifndef _ROOT_H_
#define _ROOT_H_

#include <Names.hpp>
#include <Group.hpp>

#define gstRoot "Active Datasets"
//current object
#define gstCurrent "current"
#define gstCurrentSelect "current select"//0:root; 1:view; 2:volume; 3:mesh; 5:volume group; 6:mesh group; 7:annotations
#define gstCurrentView "current view"
#define gstCurrentVolume "current volume"
#define gstCurrentMesh "current mesh"
#define gstCurrentVolumeGroup "current volume group"
#define gstCurrentMeshGroup "current mesh group"
#define gstCurrentAnnotations "current annotations"
#define gstCurVolIdx "cur vol idx"
#define gstCurMshIdx "cur msh idx"
//copy source
#define gstSourceVolume "source volume"//for mask copying
#define gstSourceMode "source mode"//copy 0:data; 1:mask
//
#define gstSortValue "sort value"
#define gstSortMethod "sort method"
#define gstActivated "activated"
//settigns
#define gstSaveProjectEnable "save project enable"
#define gstProjectPath "project path"
#define gstCaptureAlpha "capture alpha"
#define gstCaptureFloat "capture float"
#define gstCaptureCompress "capture compress"
#define gstHardwareCompress "hardware compress"
#define gstSkipBrick "skip brick"
#define gstTestSpeed "test speed"
#define gstTestParam "test param"
#define gstTestWiref "test wiref"
#define gstPeelNum "peel num"//peeling layer num
#define gstMicroBlendEnable "micro blend enable"//mix at slice level
#define gstShadowDirEnable "shadow dir enable"
#define gstShadowDirX "shadow dir x"
#define gstShadowDirY "shadow dir y"
#define gstAdaptive "adaptive"//drawing quality is adaptive to speed
#define gstWaveColor1 "wave color1"
#define gstWaveColor2 "wave color2"
#define gstWaveColor3 "wave color3"
#define gstWaveColor4 "wave color4"
#define gstTimeFileId "time file id"
#define gstGradBg "gradient background"
#define gstPinThresh "pin thresh"//ray casting threshold value
#define gstVrEnable "vr enable"
#define gstVrEyeOffset "vr eye offset"
#define gstOverrideVoxSpc "override vox spc"
#define gstSoftThresh "soft thresh"
#define gstRunScript "run script"//script run
#define gstScriptFile "script file"
#define gstTextSize "text size"
#define gstTextColorMode "text color mode"//text color: 0- contrast to bg; 1-same as bg; 2-volume sec color
#define gstFontFile "font file"
#define gstLineWidth "line width"//for drawing on screen lines
#define gstStreamEnable "stream enable"
#define gstGpuMemSize "gpu mem size"
#define gstLargeDataSize "large data size"
#define gstBrickSize "brick size"
#define gstResponseTime "response time"
#define gstUpdateOrder "update order"//front to back (1) or back to front (0)
#define gstLodOffset "lod offset"
#define gstPointVolumeMode "point volume mode"//0: use view plane; 1: use max value; 2: use accumulated value
#define gstRulerUseTransf "ruler use transf"//ruler use volume transfer function
#define gstRulerTransient "ruler transient"//ruler is time dependent
#define gstRulerF1 "ruler f1"
#define gstRulerInfr "ruler infr"
#define gstRulerRelaxIter "ruler relax iter"
#define gstRulerRelax "ruler relax"//compute ruler relax after drawing
#define gstRulerRelaxType "ruler relax type"
#define gstRulerDfoverf "ruler dfoverf"
#define gstRulerSizeThresh "ruler size thresh"
#define gstPvxmlFlipX "pvxml flip x"
#define gstPvxmlFlipY "pvxml flip y"
#define gstPvxmlSeqType "pvxml seq type"
#define gstApiType "api type"
#define gstOutputBitR "output bit r"
#define gstOutputBitG "output bit g"
#define gstOutputBitB "output bit b"
#define gstOutputBitA "output bit z"
#define gstOutputBitD "output bit d"
#define gstOutputSamples "output samples"
#define gstGlVersionMajor "gl version major"
#define gstGlVersionMinor "gl version minor"
#define gstGlProfileMask "gl profile mask"
#define gstClPlatformId "cl platform id"
#define gstClDeviceId "cl device id"
#define gstPaintHistory "paint history"
#define gstStayOnTop "stay on top"
#define gstShowCursor "show cursor"
#define gstLastTool "last tool"
#define gstTrackIter "track iter"
#define gstCompSizeLimit "comp size limit"
#define gstCompConsistent "comp consistent"
#define gstTryMerge "try merge"
#define gstTrySplit "try split"
#define gstContactFactor "contact factor"
#define gstSimilarity "similarity"
#define gstMaxTextureSizeEnable "max texture size enable"
#define gstMaxTextureSize "max texture size"
#define gstNoTexPack "no tex pack"
#define gstClipPlaneMode "clip plane mode"
#define gstImagejMode "imagej mode"
#define gstJvmPath "jvm path"
#define gstImagejPath "imagej path"
#define gstBioformatsPath "bioformats path"
//from render frame
#define gstOpenSlices "open slices"
#define gstOpenChanns "open channs"
#define gstOpenDigitOrder "open digit order"
#define gstOpenSeriesNum "open series num"
#define gstEmbedDataInProject "embed data in project"
#define gstLoadMask "load mask"
#define gstLoadLabel "load label"

namespace fluo
{
	class Renderview;
	class VolumeData;
	class VolumeGroup;
	class MeshData;
	class MeshGroup;
	class Annotations;

	enum SortMethod
	{
		SortNone = 0,//no sorting
		SortAscend = 1,//ascending
		SortDescend = 2//descending
	};

	class Root : public Group
	{
	public:
		Root();
		Root(const Root& root, const CopyOp& copyop = CopyOp::SHALLOW_COPY) {}

		virtual Object* clone(const CopyOp& copyop) const
		{
			return 0;
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const Root*>(obj) != NULL;
		}

		virtual const char* className() const { return "Root"; }

		virtual Root* asRoot() { return this; }
		virtual const Root* asRoot() const { return this; }

		virtual bool addChild(Node* child);
		virtual bool insertChild(size_t index, Node* child);
		virtual bool removeChildren(size_t pos, size_t num);
		virtual bool setChild(size_t i, Node* node);

		//currently highlighted
		Renderview* getCurrentRenderview();
		VolumeData* getCurrentVolumeData();
		VolumeGroup* getCurrentVolumeGroup();
		MeshData* getCurrentMeshData();
		MeshGroup* getCurrentMeshGroup();
		Annotations* getCurrentAnnotations();

	protected:
		virtual ~Root()
		{
		}
	};
}

#endif//_ROOT_H_