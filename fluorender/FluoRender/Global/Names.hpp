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
#define gstTestWire "test wire"
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

#endif//NAMES_HPP
