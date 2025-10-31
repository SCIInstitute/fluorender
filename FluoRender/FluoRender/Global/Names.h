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
#ifndef NAMES_HPP
#define NAMES_HPP

//nothing
#define gstNull "null"

//origin
#define gstOrigin "origin"

//paths
#define gstPaths "paths"
#define gstExecutablePath "executable path"

//current object
#define gstCurrent "current"
#define gstCurrentSelect "current select"//0:root; 1:view; 2:volume; 3:mesh; 5:volume group; 6:mesh group; 7:annotations
#define gstCurrentView "current view"
#define gstCurrentVolume "current volume"
#define gstCurrentMesh "current mesh"
#define gstCurrentVolumeGroup "current volume group"
#define gstCurrentMeshGroup "current mesh group"
#define gstCurrentAnnot "current annot"
#define gstCurVolIdx "cur vol idx"
#define gstCurMshIdx "cur msh idx"
//copy source
#define gstSourceVolume "source volume"//for mask copying
#define gstSourceMode "source mode"//copy 0:data; 1:mask

//factories & defaults
#define gstDefaultFile "default file"
#define gstDefaultObject "default object"
#define gstFactoryGroup "factory group"
#define gstFactory "factory"
#define gstVolumeFactory "VolumeData factory"
#define gstDefaultVolume "default volume"
#define gstMeshFactory "MeshData factory"
#define gstDefaultMesh "default mesh"
#define gstAnnotFactory "AnnotData factory"
#define gstDefaultAnnot "default annot"
#define gstRenderviewFactory "Renderview factory"
#define gstDefaultRenderview "default renderview"
#define gstAgentFactory "InterfaceAgent factory"
#define gstRenderer2DFactory "renderer2d factory"
#define gstDefaultRenderer2D "default renderer2d"
#define gstRenderer3DFactory "renderer3d factory"
#define gstDefaultRenderer3D "default renderer3d"
#define gstProcessorNodeFactory "processor node factory"
#define gstDefaultProcessorNode "default processor node"
#define gstProcessorFactory "processor factory"
#define gstDefaultProcessor "default processor"
#define gstAsyncTimerFactory "async timer factory"
#define gstStopWatchFactory "stop watch factory"
//shader factories
#define gstVolShader "volume shader"
#define gstMeshShader "mesh shader"
#define gstSegShader "segmentation shader"
#define gstImgShader "image shader"
#define gstVolCalShader "volume calculation shader"
#define gstLightFieldShader "light field shader"

//agent names
#define gstAgentAsset "asset"
#define gstAnnotPropAgent "AnnotatPropPanel"
#define gstBrushToolAgent "BrushToolDlg"
#define gstCalculationAgent "CalculationDlg"
#define gstClipPlaneAgent "ClipPlanePanel"
#define gstClKernelAgent "ClKernelDlg"
#define gstColocalAgent "ColocalDlg"
#define gstCountingAgent "CountingDlg"
#define gstComponentAgent "ComponentDlg"
#define gstConvertAgent "ConvertDlg"
#define gstListAgent "ListPanel"
#define gstMeasureAgent "MeasureDlg"
#define gstMeshPropAgent "MeshPropPanel"
#define gstMeshTransAgent "MeshTransPanel"
#define gstMovieAgent "MoviePanel"
#define gstNoiseReduceAgent "NoiseReduceDlg"
#define gstOutAdjustAgent "OutAdjustPanel"
#define gstRenderCanvasAgent "Render View:"//plus a serial number
#define gstRenderFrameAgent "RenderFrame"
#define gstRenderviewAgent "RenderviewPanel"//plus a serial number
#define gstSettingAgent "SettingDlg"
#define gstTrackAgent "TrackDlg"
#define gstTreeAgent "TreePanel"
#define gstVolumePropAgent "VolumePropPanel"
#define gstNonObjectValues "non object values"//values not from the managed object

//panels
#define gstVolumePropPanel "volume prop panel"
#define gstMeshPropPanel "mesh prop panel"
#define gstManipPropPanel "manip prop panel"
#define gstAnnotatPropPanel "annotat prop panel"
#define gstProjPanel "proj panel"
#define gstMoviePanel "movie panel"
#define gstPropPanel "prop panel"
#define gstOutAdjPanel "out adj panel"
#define gstClipPlanePanel "clip plane panel"
#define gstMainToolbar "main toolbar"
#define gstMainFrameTitle "main frame title"
#define gstMainStatusbarText "main statusbar text"
#define gstMainStatusbarPush "main statusbar push"
#define gstMainStatusbarPop "main statusbar pop"

//root
#define gstRoot "Scene Graph"
#define gstSortValue "sort value"
#define gstSortMethod "sort method"
#define gstActivated "activated"

//renderview buffer names
#define gstRBCanvasDefault "rb canvas default"
#define gstRBViewBase "rb view base"
#define gstRBViewData "rb view data"//when scene graph only 
#define gstRBViewDataWithDepth "rb view data with depth"
#define gstRBQuilt "rb quilt"
#define gstRBQuiltView "rb quilt view"
#define gstRBPeel "rb peel"
#define gstRBVrRight "rb vr right"
#define gstRBVrLeft "rb vr left"
#define gstRBChannel "rb channel"
#define gstRBTemporary "rb temporary"
#define gstRBGradMip "rb grad mip"
#define gstRBFilter "rb filter"
#define gstRBMicroBlend "rb micro blend"
#define gstRBOverlay "rb overlay"
#define gstRBPaintBrush "rb paint brush"
#define gstRBBlendInteractive "rb blend int"
#define gstRBBlendDenoise "rb blend denoise"
#define gstRBBlendQuality "rb blend quality"
#define gstRBPick "rb pick"

//properties
#define gstMultiFuncTips "multi func tips"
//list panel
#define gstListCtrl "list ctrl"
//tree panel
#define gstTreeIcons "tree icons"
#define gstTreeColors "tree colors"
#define gstTreeCtrl "tree ctrl"
#define gstTreeSelection "tree selection"
//output adjustments
#define gstGammaR "gamma r"
#define gstGammaG "gamma g"
#define gstGammaB "gamma b"
#define gstBrightnessR "brightness r"
#define gstBrightnessG "brightness g"
#define gstBrightnessB "brightness b"
#define gstEqualizeR "equalize r"
#define gstEqualizeG "equalize g"
#define gstEqualizeB "equalize b"
#define gstSyncR "sync r"
#define gstSyncG "sync g"
#define gstSyncB "sync b"
#define gstUpdateSync "update sync"
//bounding box
#define gstBounds "bounds"
#define gstClipPlanes "clip planes"
#define gstClipBounds "clip bounds"
//clipping values
#define gstClipX1 "clip x1"
#define gstClipX2 "clip x2"
#define gstClipY1 "clip y1"
#define gstClipY2 "clip y2"
#define gstClipZ1 "clip z1"
#define gstClipZ2 "clip z2"
#define gstClipDist "clip dist"
#define gstClipDistX "clip dist x"
#define gstClipDistY "clip dist y"
#define gstClipDistZ "clip dist z"
#define gstClipLinkX "clip link x"
#define gstClipLinkY "clip link y"
#define gstClipLinkZ "clip link z"
#define gstClipRotX "clip rot x"
#define gstClipRotY "clip rot y"
#define gstClipRotZ "clip rot z"
#define gstClipHold "clip hold"
#define gstClipMask "clip mask"
#define gstClipRenderMode "clip render mode"
//file properties
#define gstDataPath "data path"
#define gstChannel "channel"
#define gstTime "time"
#define gstBits "bits"
//render modes
#define gstBlendMode "blend mode"
#define gstRenderMode "render mode"
#define gstOverlayMode "overlay mode"
#define gstMaskMode "mask mode"
#define gstUseMaskThresh "use mask thresh"
#define gstMaskThresh "mask thresh"
#define gstInvert "invert"
#define gstTransparent "transparent"
//trasnfer function
#define gstVolumeProps "volume props"
#define gstUpdateHistogram "update histogram"
#define gstIntScale "int scale"
#define gstGmScale "gm scale"
#define gstMaxInt "max int"
#define gstMaxScale "max scale"
#define gstGammaEnable "gamma enable"
#define gstGamma3d "gamma 3d"
#define gstBoundary "boundary"
#define gstBoundaryEnable "boundary enable"
#define gstBoundaryLow "boundary low"
#define gstBoundaryHigh "boundary high"
#define gstBoundaryMax "boundary max"
#define gstMinMax "minmax"
#define gstMinMaxEnable "minmax enable"
#define gstLowOffset "low offset"
#define gstHighOffset "high offset"
#define gstThresholdEnable "threshold enable"
#define gstThreshold "threshold"
#define gstLowThreshold "low threshold"
#define gstHighThreshold "high threshold"
#define gstSoftThresh "soft thresh"
#define gstLuminanceEnable "luminance enable"
#define gstAlphaEnable "alpha enable"
#define gstAlpha "alpha"
#define gstAlphaPower "alpha power"
#define gstShading "shading"
#define gstShadingEnable "shading enable"
#define gstLowShading "low shading"
#define gstHighShading "high shading"
#define gstMatAmb "mat amb"
#define gstMatDiff "mat diff"
#define gstMatSpec "mat spec"
#define gstMatShine "mat shine"
#define gstShadow "shadow"
#define gstShadowEnable "shadow enable"
#define gstShadowInt "shadow int"
#define gstShadowDir "shdow dir"
#define gstShadowDirEnable "shadow dir enable"
#define gstShadowDirX "shadow dir x"
#define gstShadowDirY "shadow dir y"
#define gstSampleRateEnable "sample rate enable"
#define gstSampleRate "sample rate"
#define gstNoiseRedct "noise redct"
//color
#define gstColor "color"
#define gstHsv "hsv"
#define gstLuminance "luminance"
#define gstSecColor "sec color"
#define gstSecColorSet "sec color set"
#define gstRandomizeColor "randomize color"
//resolution
#define gstResX "res x"
#define gstResY "res y"
#define gstResZ "res z"
#define gstBaseResX "base res x"
#define gstBaseResY "base res y"
#define gstBaseResZ "base res z"
//scaling
#define gstScaleX "scale x"
#define gstScaleY "scale y"
#define gstScaleZ "scale z"
//spacing
#define gstSpacing "spacing"
#define gstSpcFromFile "spc from file"
#define gstSpcX "spc x"
#define gstSpcY "spc y"
#define gstSpcZ "spc z"
#define gstBaseSpcX "base spc x"
#define gstBaseSpcY "base spc y"
#define gstBaseSpcZ "base spc z"
#define gstSpcSclX "spc scl x"
#define gstSpcSclY "spc scl y"
#define gstSpcSclZ "spc scl z"
//display
#define gstDisplay "display"
#define gstDrawBounds "draw bounds"
//colormap
#define gstColormap "colormap"
#define gstColormapEnable "colormap enable"
#define gstColormapMode "colormap mode"
#define gstColormapDisp "colormap disp"
#define gstColormapLow "colormap low"
#define gstColormapHigh "colormap high"
#define gstColormapInv "colormap inv"
#define gstColormapType "colormap type"
#define gstColormapProj "colormap proj"
//tex ids
#define gst2dMaskId "2d mask id"
#define gst2dWeight1Id "2d weight1 id"
#define gst2dWeight2Id "2d weight2 id"
#define gst2dDmapId "2d dmap id"
//compress
#define gstHardwareCompress "hardware compress"
//resize
#define gstResize "resize"
#define gstResizeX "resize x"
#define gstResizeY "resize y"
#define gstResizeZ "resize z"
//brick
#define gstSkipBrick "skip brick"
#define gstBrickNum "brick num"
//legend
#define gstLegend "legend"
//interpolate
#define gstInterpolate "interpolate"
//label
#define gstLabelMode "label mode"
//depth atten
#define gstDepthAtten "depth atten"
#define gstDaInt "da int"
#define gstDaStart "da start"
#define gstDaEnd "da end"
//estimated thresh
#define gstEstimateThresh "estimate thresh"
//others
#define gstViewport "viewport"
#define gstClearColor "clear color"
#define gstCurFramebuffer "cur framebuffer"
//multires
#define gstMultires "multires"
#define gstLevel "level"
#define gstLevelNum "level num"
#define gstLevelOffset "level offset"//offset to detail level
//tex transform
#define gstTexTransform "tex transform"
//selection
#define gstSelected "selected"
//mask clear
#define gstMaskClear "mask clear"
//sync group
#define gstSyncGroup "sync group"
//duplicated
#define gstDuplicate "duplicate"
//stream mode
#define gstStreamMode "stream mode"

//specific to mesh
#define gstBoundsTf "bounds tf"//bounding box after transformation
#define gstCenter "center"
#define gstLimitEnable "limit enable"//size limiter
#define gstLimit "limit"
//transformation
#define gstMeshTranslation "mesh translation"
#define gstTransX "trans x"
#define gstTransY "trans y"
#define gstTransZ "trans z"
#define gstMeshRotation "mesh rotation"
#define gstRotX "rot x"
#define gstRotY "rot y"
#define gstRotZ "rot z"
#define gstMeshScale "mesh scale"

//specific to ann
#define gstVolume "volume"
#define gstTransform "transform"
#define gstMemo "memo"
#define gstMemoRo "memo ro"
#define gstInfoHeader "info header"

//specific to atext
#define gstText "text"
#define gstLocation "location"
#define gstInfo	"info"

//specific to renderview
#define gstFocus "focus"//set focus to panel
#define gstSizeX "size x"
#define gstSizeY "size y"
#define gstUseDefault "use default"
#define gstClipLinkChan "clip link chan"
#define gstClipPlaneMode "clip plane mode"
#define gstClipPlaneRanges "clip plane ranges"
#define gstClipPlaneRangeColor "clip plane range color"
#define gstInitialized "initialized"
#define gstInitView "init view"
#define gstSetGl "set gl"//set gl context
#define gstRunScript "run script"//script run
#define gstScriptFile "script file"
#define gstScriptList "script list"
#define gstScriptSelect "script select"
#define gstCapture "capture"//capture modes
#define gstCaptureRot "capture rot"
#define gstCaptureRotOver "capture rot over"
#define gstCaptureTime "capture time"
#define gstCaptureBat "capture bat"
#define gstCaptureParam "capture param"
#define gstBeginFrame "begin frame"
#define gstEndFrame "end frame"
#define gstCurrentFrame "current frame"
#define gstPreviousFrame "previous frame"
#define gstParamFrame "param frame"
#define gstTotalFrames "total frames"
#define gstCaptureFile "capture file"
#define gstCaptureAlpha "capture alpha"
#define gstCaptureFloat "capture float"
#define gstCaptureCompress "capture compress"
#define gstBatFolder "bat folder"
#define gstRetainFb "retain fb"//sometimes we don't redraw everything,
								//just use the final buffer from last draw
#define gstUpdating "updating"
#define gstUpdateOrder "update order"//front to back (1) or back to front (0)
#define gstDrawAll "draw all"//draw settings
#define gstDrawType "draw type"
#define gstDrawMask "draw mask"
#define gstMixMethod "mix method"
#define gstPeelNum "peel num"//peeling layer num
#define gstMicroBlendEnable "micro blend enable"//mix at slice level
#define gstDrawAnnot "draw annot"
#define gstDrawCamCtr "draw cam ctr"
#define gstCamCtrSize "cam ctr size"
#define gstDrawInfo "draw info"
#define gstLoadUpdate "load update"
#define gstDrawCropFrame "draw crop frame"
#define gstDrawClip "draw clip"
#define gstDrawLegend "draw legend"
#define gstDrawColormap "draw colormap"
#define gstTestWiref "test wiref"
#define gstDrawBounds "draw bounds"
#define gstDrawGrid "draw grid"
#define gstDrawRulers "draw rulers"
#define gstClipMask "clip mask"//clipping settings
#define gstClipMode "clip mode"//0-normal; 1-ortho planes; 2-rot difference
#define gstDrawScaleBar "draw scale bar"//scale bar settings
#define gstDrawScaleBarText "draw scale bar text"
#define gstScaleBarLen "scale bar len"
#define gstScaleBarPosX "scale bar pos x"
#define gstScaleBarPosY "scale bar pos y"
#define gstScaleBarUnit "scale bar unit"
#define gstScaleBarText "scale bar text"
#define gstScaleBarNum "scale bar num"
#define gstScaleBarHeight "scale bar height"
#define gstOrthoLeft "ortho left"//orthographic view size
#define gstOrthoRight "ortho right"
#define gstOrthoBottom "ortho bottom"
#define gstOrthoTop "ortho top"
#define gstScaleFactor "scale factor"//scale factor
#define gstScaleFactorSaved "scale factor saved"
#define gstScaleFactor121 "scale factor 121"
#define gstScaleMode "scale mode"//zoom ratio meaning: 0-view; 1-pixel; 2-data(pixel*xy spc)
#define gstAutoPinRotCtr "auto pin rot ctr"//pin rotation center
#define gstPinRotCtr "pin rot ctr"
#define gstRotCtrDirty "rot ctr dirty"//request rot ctr update
#define gstPinThresh "pin thresh"//ray casting threshold value
#define gstRotCtrPin "rot ctr pin"//rotation center point from pin
#define gstRotSliderMode "rot slider mode"//slider mode for rot view
#define gstPointVolumeMode "point volume mode"//0: use view plane; 1: use max value; 2: use accumulated value
#define gstRulerUseTransf "ruler use transf"//ruler use volume transfer function
#define gstRulerTransient "ruler transient"//ruler is time dependent
#define gstSettingsRot "settings rot"//settings for rotations
#define gstLinkedRot "linked rot"//link rotation to views
#define gstMouseInt "mouse int"//reduce computations for mouse interactions
#define gstFullscreenDisplay "fullscreen display"
#define gstDisplayColorDepth "display color depth"
#define gstPaintCount "paint count"//count voxels after painting
#define gstPaintColocalize "paint colocalize"//compute colocalization after painting
#define gstRulerRelax "ruler relax"//compute ruler relax after drawing
#define gstDrawing "drawing"//is busy drawing
#define gstRefreshErase "refresh erase"//erase during refresh
#define gstRefresh "refresh"//flag to request refresh
#define gstRefreshNotify "refresh notify"//ask ui to actually refresh
#define gstWidth "width"//from m_size
#define gstHeight "height"//from m_size
#define gstVolListDirty "vol list dirty"//request vol pop list update
#define gstFullVolListDirty "full vol list dirty"
#define gstMshListDirty "msh list dirty"//request msh pop list update
#define gstFullMshListDirty "full msh list dirty"
#define gstTextColorMode "text color mode"//text color: 0- contrast to bg; 1-same as bg; 2-volume sec color
#define gstTextColor "text color"//text color
#define gstBgColor "background color"//bg
#define gstBgColorInv "background color inv"//inverted background color
#define gstGradBg "gradient background"
#define gstClearColorBg "clear color background"
#define gstAov "aov"//angle of view frustrum
#define gstNearClip "near clip"
#define gstFarClip "far clip"
#define gstInterMode "inter mode"  //interactive mode
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
#define gstFreehandToolState "freehand tool state"//combined state for brushes and rulers
#define gstForceClear "force clear"//forced update
#define gstInteractive "interactive"//currently user is interacting with view
#define gstAdaptive "adaptive"//drawing quality is adaptive to speed
#define gstClearBuffer "clear buffer"
#define gstGrowEnable "grow enable"//flag for grow is currently on for idle events
#define gstResize "resize"//request to resize
#define gstDrawBrush "draw brush"//brush settings
#define gstPaintEnable "paint enable"
#define gstPaintDisplay "paint display"
#define gstClearPaint "clear paint"
#define gstPaintMode "paint mode"
#define gstPerspective "perspective"//camera settings
#define gstCamMode "cam mode"
#define gstCamDist "cam dist"
#define gstCamDistIni "cam dist ini"
#define gstCamTransX "cam trans x"//camera translation
#define gstCamTransY "cam trans y"
#define gstCamTransZ "cam trans z"
#define gstCamTransSavedX "cam trans saved x"
#define gstCamTransSavedY "cam trans saved y"
#define gstCamTransSavedZ "cam trans saved z"
#define gstCamRotation "cam rot"
#define gstCamRotX "cam rot x"//camera rotation
#define gstCamRotY "cam rot y"
#define gstCamRotZ "cam rot z"
#define gstCamRotSavedX "cam rot saved x"
#define gstCamRotSavedY "cam rot saved y"
#define gstCamRotSavedZ "cam rot saved z"
#define gstCamRotZeroX "cam rot zero x"//camera rotation at zero setting
#define gstCamRotZeroY "cam rot zero y"
#define gstCamRotZeroZ "cam rot zero z"
#define gstCamCtrX "cam ctr x"//camera center
#define gstCamCtrY "cam ctr y"
#define gstCamCtrZ "cam ctr z"
#define gstCamCtrSavedX "cam ctr saved x"
#define gstCamCtrSavedY "cam ctr saved y"
#define gstCamCtrSavedZ "cam ctr saved z"
#define gstCamRotQ "cam rot q"//rotation in quaternion
#define gstCamRotZeroQ "cam rot zero q"//rotation at zero setting
#define gstCamUp "cam up"//camera up vector
#define gstCamHead "cam head"//camera heading vector
#define gstClipRotQ "clip rot q"//clipping plane rotation
#define gstClipRotZeroQ "clip rot zero q"
#define gstClipRotX "clip rot x"
#define gstClipRotY "clip rot y"
#define gstClipRotZ "clip rot z"
#define gstObjCtrX "obj ctr x"//object center
#define gstObjCtrY "obj ctr y"
#define gstObjCtrZ "obj ctr z"
#define gstObjTransX "obj trans x"//object translation
#define gstObjTransY "obj trans y"
#define gstObjTransZ "obj trans z"
#define gstObjTransSavedX "obj trans saved x"
#define gstObjTransSavedY "obj trans saved y"
#define gstObjTransSavedZ "obj trans saved z"
#define gstObjRotX "obj rot x"//object rotation
#define gstObjRotY "obj rot y"
#define gstObjRotZ "obj rot z"
#define gstGearedEnable "geared enable"//enable geared rotation
#define gstCamLockObjEnable "cam lock obj enable"//enable locking camera on a point
#define gstCamLockType "cam lock type"//1:volume center; 2:pick by clicking; 3:ruler center; 4:selection mask center
#define gstCamLockCtr "cam lock ctr"
#define gstCamLockPick "cam lock pick"//camera locking center by picking
#define gstRadius "radius"//scene size in terms of radius of a sphere
#define gstMovStopWatch "mov stop watch"//time keeper for running movies
#define gstMovInitAng "mov init ang"//movie properties
#define gstMovStartAng "mov start ang"
#define gstMovEndAng "mov end ang"
#define gstMovCurAng "mov cur ang"
#define gstMovStep "mov step"
#define gstMovRotAxis "mov rot axis"//0-X; 1-Y; 2-Z;
#define gstMovSeqNum "mov seq num"
#define gstMovRewind "mov rewind"
#define gstMovStage "mov stage"//0-moving to start angle; 1-moving to end; 2-rewind
#define gstMovRewind4d "mov rewind 4d"
#define gstMovRunning "mov running"//movie is currently running
#define gstCropEnable "crop enable"
#define gstCropValues "crop values"
#define gstCropX "crop x"//movie frame properties
#define gstCropY "crop y"
#define gstCropW "crop w"
#define gstCropH "crop h"
#define gstScalebarPos "scalebar pos"
#define gstColor1 "color1"//colormap settings
#define gstColor2 "color2"
#define gstColor3 "color3"
#define gstColor4 "color4"
#define gstColor5 "color5"
#define gstColor6 "color6"
#define gstColor7 "color7"
#define gstColVal1 "col val 1"
#define gstColVal2 "col val 2"
#define gstColVal3 "col val 3"
#define gstColVal4 "col val 4"
#define gstColVal5 "col val 5"
#define gstColVal6 "col val 6"
#define gstColVal7 "col val 7"
#define gstPick "pick"//for selection
#define gstPreDraw "pre draw"
#define gstTextSize "text size"
#define gstKeepEnlarge "keep enlarge"
#define gstEnlarge "enlarge"
#define gstEnlargeScale "enlarge scale"
#define gstBenchmark "benchmark"
#define gstNodrawCount "nodraw count"
#define gstLoadMainThread "load main thread"
#define gstResMode "res mode"
#define gstFullScreen "full screen"
#define gstHologramMode "hologram mode"
#define gstVrEnable "vr enable"
#define gstOpenvrEnable "openvr enable"
#define gstVrSizeX "vr size x"
#define gstVrSizeY "vr size y"
#define gstVrEyeOffset "vr eye offset"
#define gstVrEyeIdx "vr eye idx"//0-left;1-right
#define gstSwapBuffers "swap buffers"
#define gstLineWidth "line width"//for drawing on screen lines
#define gstHwnd "hwnd"//handle to window
#define gstHinstance "hinstance"//handle to instance
#define gstBmRuntime "bm runtime"//benchmark runtime in msec
#define gstBmFrames "bm frames"//benchmark frames ran
#define gstBmFps "bm fps"//benchmark fps
#define gstSelectedMshName "selected msh name"
#define gstSelectedVolName "selected vol name"
#define gstSelPointVolume "sel point volume"
#define gstMouseX "mouse x"//mouse pos from os
#define gstMouseY "mouse y"
#define gstMousePrvX "mouse prv x"
#define gstMousePrvY "mouse prv y"
#define gstMouseClientX "mouse client x"//from screentoclient()
#define gstMouseClientY "mouse client y"
#define gstMouseIn "mouse in"
#define gstRenderviewPanelId "renderview panel id"
//processor properties should be moved in the future
#define gstSelUndoRedo "sel undo redo"
#define gstSelUndo "sel undo"//selector undo
#define gstSelRedo "sel redo"
#define gstSelMask "sel mask"
#define gstSelOptions "sel options"//edge detect etc
#define gstBrushThreshold "brush threshold"
#define gstBrushGmFalloff "brush gm falloff"
#define gstBrush2dInf "brush 2d inf"
#define gstBrushSize1 "brush size1"
#define gstBrushSize2 "brush size2"
#define gstBrushIter "brush iter"
#define gstBrushSizeRel "brush size rel"
#define gstBrushHistoryEnable "brush history enable"
#define gstBrushCountResult "brush count result"
#define gstBrushCountAutoUpdate "brush count auto update"
#define gstBrushSpeedResult "brush speed result"
#define gstSettingsJava "settings java"

//dialog agent common
#define gstUseSelection "use selection"//use mask selection
#define gstHoldHistory "hold history"
#define gstTestSpeed "test speed"
#define gstUseTransferFunc "use transfer func"
#define gstUseMachineLearning "use machine learning"
//colocal agent
#define gstColocalMethod "colocal method"//0:dot product; 1:min value; 2:threshold
#define gstIntWeighted "int weighted"
#define gstGetRatio "get ratio"
#define gstPhysSize "phys size"
#define gstColocalColormap "colocal colormap"
#define gstColocalResult "colocal result"
#define gstColocalAutoUpdate "colocal auto update"
//component agent
#define gstCompAutoUpdate "comp auto update"//automatically gen comps
#define gstRecordCmd "record cmd"//record command for comps
#define gstRunCmd "run cmd"//run command
#define gstIteration "iteration"//number of iterations
#define gstCompThreshold "comp threshold"
#define gstUseDiffusion "use diffusion"//diffusion settings
#define gstDiffusionFalloff "diffusion falloff"
#define gstUseDensityField "use density field"//density field settings
#define gstDensityFieldThresh "density field thresh"
#define gstDensityVarThresh "density var thresh"
#define gstDensityWindowSize "density window size"
#define gstDensityStatsSize "density stats size"
#define gstUseDistField "use dist field"//dist field settings
#define gstDistFieldStrength "dist field strength"
#define gstDistFieldFilterSize "dist field filter size"
#define gstMaxDist "max dist"
#define gstDistFieldThresh "dist field thresh"
#define gstFixateEnable "fixate enable"//fixate settings
#define gstGrowFixed "grow fixed"//Continue on Fixiated Regions
#define gstFixateSize "fixate size"
#define gstCleanEnable "clean enable"//clean settings
#define gstCleanIteration "clean iteration"
#define gstCleanSize "clean size"
#define gstClusterMethod "cluster method"//0:k-means; 1:em; 2:dbscan
#define gstClusterNum "cluster num"
#define gstClusterMaxIter "cluster max iter"
#define gstClusterTol "cluster tol"
#define gstClusterSize "cluster size"
#define gstClusterEps "cluster eps"
#define gstUseMin "use min"//analysis settings
#define gstMinValue "min value"
#define gstUseMax "use max"
#define gstMaxValue "max value"
#define gstCompId "comp id"
#define gstCompIdStr "comp id str"
#define gstCompIdColor "comp id color"
#define gstCompSizeLimit "comp size limit"
#define gstCompConsistent "comp consistent"
#define gstCompColocal "comp colocal"
#define gstCompOutputType "comp output type"//1-multi; 2-rgb;
#define gstCompGenOutput "comp gen output"
#define gstCompAnalysisResult "comp analysis result"
#define gstCompListSelection "comp list selection"
#define gstDistAllChan "dist all chan"
#define gstDistNeighbor "dist neighbor"
#define gstDistNeighborValue "dist neighbor value"
#define gstAlignAxisType "align axis type"//xyz:0; yxz:1; zxy:2; xzy:3; yzx:4; zyx:5;
#define gstAlignCenter "align center"
#define gstCompCommandFile "comp command file"
#define gstCompDefaultFile "comp default file"
//conver agent
#define gstVolMeshThresh "vol mesh thresh"
#define gstVolMeshDownXY "vol mesh down xy"
#define gstVolMeshDownZ "vol mesh down z"
#define gstVolMeshWeld "vol mesh weld"
#define gstVolMeshInfo "vol mesh info"
#define gstVolMeshSimplify "vol mesh simplify"
#define gstVolMeshSmooth "vol mesh smooth"
#define gstConvVolMeshUpdate "conv vol mesh update"
#define gstConvVolMeshUpdateTransf "conv vol mesh update transf"
//counting agent
#define gstCountMinValue "count min value"
#define gstCountMaxValue "count max value"
#define gstCountUseMax "count use max"
#define gstCountResult "count result"
//fp range
#define gstFpRangeMin "fp range min"
#define gstFpRangeMax "fp range max"
//help
#define gstHelpUrl "help url"
//machine learning
#define gstMlAutoStart "ml auto start"
#define gstMlAutoLoadTable "ml auto load table"
#define gstMlTopList "ml top list"
#define gstMlCgAutoStart "ml cg auto start"
#define gstMlVpAutoStart "ml vp auto start"
#define gstMlVpAutoApply "ml vp auto apply"
//measure agent
#define gstRulerEdited "ruler edited"
#define gstRulerAutoRelax "ruler auto relax"
#define gstRulerRelaxIter "ruler relax iter"
#define gstRulerInfr "ruler infr"
#define gstRulerF1 "ruler f1"
#define gstRulerRelaxType "ruler relax type"
#define gstRulerDfoverf "ruler dfoverf"
#define gstRulerDisp "ruler disp"
#define gstRulerList "ruler list"
#define gstRulerListCur "ruler list cur"
#define gstRulerListDisp "ruler list disp"
#define gstRulerListSel "ruler list sel"
#define gstRulerGroupSel "ruler group sel"
#define gstRulerInterpolation "ruler interpolation"
#define gstRulerProfile "ruler profile"
#define gstRulerMethod "ruler method"
#define gstRulerFile "ruler file"
//movie agent
#define gstMovPlay "mov play"
#define gstMovLoop "mov loop"
#define gstMovViewList "mov view list"
#define gstMovViewIndex "mov view index"
#define gstMovSliderStyle "mov slider style"
#define gstMovProgSlider "mov prog slider"
#define gstMovTimerName "mov timer name"
#define gstMovRotEnable "mov rot enable"
#define gstMovTimeSeqEnable "mov time seq enable"
#define gstMovSeqMode "mov seq mode"//0:none; 1:4d; 2:bat
#define gstMovRotAng "mov rot ang"
#define gstMovIntrpMode "mov intrp mode"//0-linear; 1-smooth
#define gstMovLength "mov length"//length in sec
#define gstMovCurTime "mov cur time"//time in sec
#define gstMovFps "mov fps"
#define gstMovBitrate "mov bitrate"//megabits per sec
#define gstLastFrame "last frame"//last frame nunmber to save
#define gstMovRecord "mov record"
#define gstMovDelayedStop "mov delayed stop"
#define gstMovTimerState "mov timer state"
#define gstMovFilename "mov file name"
#define gstMovFileType "mov file type"
#define gstMovSliderRange "mov slider range"
#define gstMovCurrentPage "mov current page"
#define gstAutoKeyIndex "auto key index"
#define gstParamList "param list"
#define gstParamListSelect "param list sel"
#define gstParamKeyDuration "param key duration"
//noise reduct agent
#define gstNrThresh "nr thresh"
#define gstNrSize "nr size"
#define gstNrPreview "nr preview"
#define gstNrHdr "nr hdr"
#define gstNrHdrR "nr hdr r"
#define gstNrHdrG "nr hdr g"
#define gstNrHdrB "nr hdr b"
//ocl kernel
#define gstKernelList "kernel list"
#define gstKernelListSelect "kernel list select"
//script break
#define gstScriptBreakTitle "script break title"
#define gstScriptBreakInfo "script break info"
//recorder agent
#define gstSelectedKey "selected key"
//setting agent/root
#define gstConfigFile "config file"
#define gstConfigFileType "config file type"
#define gstProjectFile "project file"
#define gstSaveProjectEnable "save project enable"
#define gstProjectPath "project path"
#define gstTestParam "test param"
#define gstWavelengthColors "wavelengthe colors"
#define gstWaveColor1 "wave color1"
#define gstWaveColor2 "wave color2"
#define gstWaveColor3 "wave color3"
#define gstWaveColor4 "wave color4"
#define gstTimeFileId "time file id"
#define gstOverrideVoxSpc "override vox spc"
#define gstSettingsFont "settings font"
#define gstFontFile "font file"
#define gstStreamEnable "stream enable"
#define gstGpuMemSize "gpu mem size"
#define gstLargeDataSize "large data size"
#define gstBrickSize "brick size"
#define gstResponseTime "response time"
#define gstLodOffset "lod offset"
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
#define gstPencilDist "pencil dist"
#define gstStayOnTop "stay on top"
#define gstShowCursor "show cursor"
#define gstLastTool "last tool"
#define gstMaxTextureSizeEnable "max texture size enable"
#define gstMaxTextureSize "max texture size"
#define gstNoTexPack "no tex pack"
#define gstImagejMode "imagej mode"
#define gstJvmPath "jvm path"
#define gstImagejPath "imagej path"
#define gstBioformatsPath "bioformats path"
#define gstDeviceTree "device tree"
#define gstMaxTextureSize "max texture size"
#define gstAutomate "automate"
//from render frame
#define gstOpenSlices "open slices"
#define gstOpenChanns "open channs"
#define gstOpenDigitOrder "open digit order"
#define gstOpenSeriesNum "open series num"
#define gstEmbedDataInProject "embed data in project"
#define gstLoadMask "load mask"
#define gstLoadLabel "load label"
//track agent
#define gstTrackFile "track file"
#define gstTrackIter "track iter"
#define gstTrackSize "track size"
#define gstTrackSimilarity "track similarity"
#define gstTrackContactFactor "track contact factor"
#define gstTrackConsistent "track consistent"
#define gstTrackMerge "track merge"
#define gstTrackSplit "track split"
#define gstTrackCompId "track comp id"
#define gstTrackCellSize "track cell size"
#define gstTrackUncertainLow "track uncertain low"
#define gstTrackNewCompId "track new comp id"
#define gstTrackClusterNum "track cluster num"
#define gstGhostNum "ghost num"
#define gstGhostEnable "ghost enable"
#define gstGhostTailEnable "ghost tail enable"
#define gstGhostLeadEnable "ghost lead enable"
#define gstTrackList "track list"

//input
//mouse
#define gstMouseLeft "mouse left"
#define gstMouseRight "mouse right"
#define gstMouseMiddle "mouse middle"
#define gstMouseLeftDown "mouse left down"
#define gstMouseRightDown "mouse right down"
#define gstMouseMiddleDown "mouse middle down"
#define gstMouseLeftUp "mouse left up"
#define gstMouseRightUp "mouse right up"
#define gstMouseMiddleUp "mouse middle up"
#define gstMouseLeftHold "mouse left hold"
#define gstMouseRightHold "mouse right hold"
#define gstMouseMiddleHold "mouse middle hold"
#define gstMouseDrag "mouse drag"
#define gstMouseWheel "mouse wheel"
//keys
//fn
#define gstKbF5 "kb f5"//F5
#define gstKbF5Down "kb f5 down"
#define gstKbF5Up "kb f5 up"
#define gstKbF5Hold "kb f5 hold"
//control
#define gstKbAlt "kb alt"//Alt
#define gstKbAltDown "kb alt down"
#define gstKbAltUp "kb alt up"
#define gstKbAltHold "kb alt hold"
#define gstKbCtrl "kb ctrl"//Ctrl
#define gstKbCtrlDown "kb ctrl down"
#define gstKbCtrlUp "kb ctrl up"
#define gstKbCtrlHold "kb ctrl hold"
#define gstKbShift "kb shift"//Shift
#define gstKbShiftDown "kb shift down"
#define gstKbShiftUp "kb shift up"
#define gstKbShiftHold "kb shift Hold"
#define gstKbReturn "kb return"//Return
#define gstKbReturnDown "kb return down"
#define gstKbReturnUp "kb return up"
#define gstKbReturnHold "kb return hold"
#define gstKbSpace "kb space"//space
#define gstKbSpaceDown "kb space down"
#define gstKbSpaceUp "kb space up"
#define gstKbSpaceHold "kb space hold"
//arrow
#define gstKbLeft "kb left"
#define gstKbLeftDown "kb left down"
#define gstKbLeftUp "kb left up"
#define gstKbLeftHold "kb left hold"
#define gstKbRight "kb right"
#define gstKbRightDown "kb right down"
#define gstKbRightUp "kb right up"
#define gstKbRightHold "kb right hold"
#define gstKbUp "kb up"
#define gstKbUpDown "kb up down"
#define gstKbUpUp "kb up up"
#define gstKbUpHold "kb up hold"
#define gstKbDown "kb down"
#define gstKbDownDown "kb down down"
#define gstKbDownUp "kb down up"
#define gstKbDownHold "kb down hold"
//alphabet
#define gstKbA "kb a"//A
#define gstKbADown "kb a down"
#define gstKbAUp "kb a up"
#define gstKbAHold "kb a hold"
#define gstKbC "kb c"//C
#define gstKbCDown "kb c down"
#define gstKbCUp "kb c up"
#define gstKbCHold "kb c hold"
#define gstKbD "kb d"//D
#define gstKbDDown "kb d down"
#define gstKbDUp "kb d up"
#define gstKbDHold "kb d hold"
#define gstKbF "kb f"//F
#define gstKbFDown "kb f down"
#define gstKbFUp "kb f up"
#define gstKbFHold "kb f hold"
#define gstKbL "kb l"//L
#define gstKbLDown "kb l down"
#define gstKbLUp "kb l up"
#define gstKbLHold "kb l hold"
#define gstKbM "kb m"//M
#define gstKbMDown "kb m down"
#define gstKbMUp "kb m up"
#define gstKbMHold "kb m hold"
#define gstKbN "kb n"//N
#define gstKbNDown "kb n down"
#define gstKbNUp "kb n up"
#define gstKbNHold "kb n hold"
#define gstKbR "kb r"//R
#define gstKbRDown "kb r down"
#define gstKbRUp "kb r up"
#define gstKbRHold "kb r hold"
#define gstKbS "kd s"//S
#define gstKbSDown "kd s down"
#define gstKbSUp "kd s up"
#define gstKbSHold "kd s hold"
#define gstKbV "kb v"//V
#define gstKbVDown "kb v down"
#define gstKbVUp "kb v up"
#define gstKbVHold "kb v hold"
#define gstKbW "kb w"//W
#define gstKbWDown "kb w down"
#define gstKbWUp "kb w up"
#define gstKbWHold "kb w hold"
#define gstKbX "kb x"//X
#define gstKbXDown "kb x down"
#define gstKbXUp "kb x up"
#define gstKbXHold "kb x hold"
#define gstKbZ "kb z"//Z
#define gstKbZDown "kb z down"
#define gstKbZUp "kb z up"
#define gstKbZHold "kb z hold"
//digit
//punctuation mark
#define gstKbLbrkt "kb kbrkt"//[
#define gstKbLbrktDown "kb kbrkt down"
#define gstKbLbrktUp "kb kbrkt up"
#define gstKbLbrktHold "kb kbrkt hold"
#define gstKbRbrkt "kb rbrkt"//]
#define gstKbRbrktDown "kb rbrkt down"
#define gstKbRbrktUp "kb rbrkt up"
#define gstKbRbrktHold "kb rbrkt hold"
#define gstKbBslsh "kb bslsh"//backslash
#define gstKbBslshDown "kb bslsh down"
#define gstKbBslshUp "kb bslsh up"
#define gstKbBslshHold "kb bslsh hold"

//calculation agent
#define gstVolumeA "volume a"
#define gstVolumeB "volume b"

#endif//NAMES_HPP
