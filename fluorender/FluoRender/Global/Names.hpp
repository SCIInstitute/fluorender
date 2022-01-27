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
#ifndef NAMES_HPP
#define NAMES_HPP

//origin
#define gstOrigin "origin"

//timer
#define gstTimer "default timer"
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
#define gstCurrentAnnotations "current annotations"
#define gstCurVolIdx "cur vol idx"
#define gstCurMshIdx "cur msh idx"


//factories & defaults
#define gstDefaultFile "default file"
#define gstDefaultObject "default object"
#define gstFactoryGroup "factory group"
#define gstVolumeFactory "volume factory"
#define gstDefaultVolume "default volume"
#define gstMeshFactory "mesh factory"
#define gstDefaultMesh "default mesh"
#define gstAnnotationFactory "annotation factory"
#define gstDefaultAnnotations "default annotations"
#define gstRenderviewFactory "renderview factory"
#define gstDefaultRenderview "default renderview"
#define gstAgentFactory "agent factory"
#define gstRenderer2DFactory "renderer2d factory"
#define gstDefaultRenderer2D "default renderer2d"
#define gstRenderer3DFactory "renderer3d factory"
#define gstDefaultRenderer3D "default renderer3d"
#define gstProcessorNodeFactory "processor node factory"
#define gstDefaultProcessorNode "default processor node"
#define gstProcessorFactory "processor factory"
#define gstDefaultProcessor "default processor"

//root
#define gstRoot "root"
#define gstSortValue "sort value"
#define gstSortMethod "sort method"
#define gstActivated "activated"

//properties
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
#define gstClipDistX "clip dist x"
#define gstClipDistY "clip dist y"
#define gstClipDistZ "clip dist z"
#define gstClipLinkX "clip link x"
#define gstClipLinkY "clip link y"
#define gstClipLinkZ "clip link z"
#define gstClipRotX "clip rot x"
#define gstClipRotY "clip rot y"
#define gstClipRotZ "clip rot z"
#define gstClipDisplay "clip display"
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
#define gstMipMode "mip mode"
#define gstOverlayMode "overlay mode"
#define gstMaskMode "mask mode"
#define gstUseMaskThresh "use mask thresh"
#define gstMaskThresh "mask thresh"
#define gstInvert "invert"
//trasnfer function
#define gstSoftThresh "soft thresh"
#define gstIntScale "int scale"
#define gstGmScale "gm scale"
#define gstMaxInt "max int"
#define gstMaxScale "max scale"
#define gstGamma3d "gamma 3d"
#define gstExtractBoundary "extract boundary"
#define gstSaturation "saturation"
#define gstLowThreshold "low threshold"
#define gstHighThreshold "high threshold"
#define gstAlpha "alpha"
#define gstAlphaPower "alpha power"
#define gstAlphaEnable "alpha enable"
#define gstMatAmb "mat amb"
#define gstMatDiff "mat diff"
#define gstMatSpec "mat spec"
#define gstMatShine "mat shine"
#define gstNoiseRedct "noise redct"
#define gstShadingEnable "shding enable"
#define gstLowShading "low shading"
#define gstHighShading "high shading"
#define gstShadowEnable "shadow enable"
#define gstShadowInt "shadow int"
#define gstShadowDirX "shadow dir x"
#define gstShadowDirY "shadow dir y"
#define gstSampleRate "sample rate"
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
#define gstColormapEnable "colormap enable"
#define gstColormapMode "colormap mode"
#define gstColormapType "colormap type"
#define gstColormapLow "colormap low"
#define gstColormapHigh "colormap high"
#define gstColormapProj "colormap proj"
#define gstColormapInv "colormap inv"
//tex ids
#define gst2dMaskId "2d mask id"
#define gst2dWeight1Id "2d weight1 id"
#define gst2dWeight2Id "2d weight2 id"
#define gst2dDmapId "2d dmap id"
//compress
#define gstCompression "compression"
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
#define gstTransX "trans x"
#define gstTransY "trans y"
#define gstTransZ "trans z"
#define gstRotX "rot x"
#define gstRotY "rot y"
#define gstRotZ "rot z"

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
#define gstSizeX "size x"
#define gstSizeY "size y"
#define gstUseDefault "use default"
#define gstClipLinkChan "clip link chan"
#define gstClipPlaneMode "clip plane mode"
#define gstInitialized "initialized"
#define gstInitView "init view"
#define gstSetGl "set gl"//set gl context
#define gstRunScript "run script"//script run
#define gstScriptFile "script file"
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
#define gstDrawAnnotations "draw annotations"
#define gstDrawCamCtr "draw cam ctr"
#define gstCamCtrSize "cam ctr size"
#define gstDrawInfo "draw info"
#define gstLoadUpdate "load update"
#define gstDrawCropFrame "draw crop frame"
#define gstTestSpeed "test speed"
#define gstDrawClip "draw clip"
#define gstDrawLegend "draw legend"
#define gstDrawColormap "draw colormap"
#define gstMouseFocus "mouse focus"
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
#define gstScaleMode "scale mode"//zoom ratio meaning: 0-view; 1-pixel; 2-data(pixel*xy spc)
#define gstAutoPinRotCtr "auto pin rot ctr"//pin rotation center
#define gstPinRotCtr "pin rot ctr"
#define gstRotCtrDirty "rot ctr dirty"//request rot ctr update
#define gstPinThresh "pin thresh"//ray casting threshold value
#define gstRotCtrPin "rot ctr pin"//rotation center point from pin
#define gstPointVolumeMode "point volume mode"//0: use view plane; 1: use max value; 2: use accumulated value
#define gstRulerUseTransf "ruler use transf"//ruler use volume transfer function
#define gstRulerTransient "ruler transient"//ruler is time dependent
#define gstLinkedRot "linked rot"
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
#define gstTextColor "text color"//text color
#define gstBgColor "background color"//bg
#define gstBgColorInv "background color inv"//inverted background color
#define gstGradBg "gradient background"
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
#define gstForceClear "force clear"//forced update
#define gstInteractive "interactive"//currently user is interacting with view
#define gstAdaptive "adaptive"//drawing quality is adaptive to speed
#define gstClearBuffer "clear buffer"
#define gstBrushState "brush state"  //sets the button state of the tree panel
						//0-not set
						//2-append
						//3-erase
						//4-diffuse
						//8-solid
#define gstGrowEnable "grow enable"//flag for grow is currently on for idle events
#define gstResize "resize"//request to resize
#define gstDrawBrush "draw brush"//brush settings
#define gstPaintEnable "paint enable"
#define gstPaintDisplay "paint display"
#define gstClearPaint "clear paint"
#define gstPerspective "perspective"//camera settings
#define gstFree "free"
#define gstCamDist "cam dist"
#define gstCamDistIni "cam dist ini"
#define gstCamTransX "cam trans x"//camera translation
#define gstCamTransY "cam trans y"
#define gstCamTransZ "cam trans z"
#define gstCamTransSavedX "cam trans saved x"
#define gstCamTransSavedY "cam trans saved y"
#define gstCamTransSavedZ "cam trans saved z"
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
#define gstCamLockCtr "cam lock ctr"
#define gstCamLockPick "cam lock pick"//camera locking center by picking
#define gstRadius "radius"//scene size in terms of radius of a sphere
#define gstMouseX "mouse x"//mouse pos
#define gstMouseY "mouse y"
#define gstMousePrvX "mouse prv x"
#define gstMousePrvY "mouse prv y"
#define gstMouseClientX "mouse client x"//from screentoclient()
#define gstMouseClientY "mouse client y"
#define gstMouseIn "mouse in"
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
#define gstCropX "crop x"//movie frame properties
#define gstCropY "crop y"
#define gstCropW "crop w"
#define gstCropH "crop h"
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
#define gstTouchEnable "touch enable"
#define gstPtr1Id "ptr1 id"
#define gstPtr1X "ptr1 x"
#define gstPtr1Y "ptr1 y"
#define gstPtr1XSave "ptr1 x save"
#define gstPtr1YSave "ptr1 y save"
#define gstPtr2Id "ptr2 id"
#define gstPtr2X "ptr2 x"
#define gstPtr2Y "ptr2 y"
#define gstPtr2XSave "ptr2 x save"
#define gstPtr2YSave "ptr2 y save"
#define gstFullScreen "full screen"
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

#endif//NAMES_HPP
