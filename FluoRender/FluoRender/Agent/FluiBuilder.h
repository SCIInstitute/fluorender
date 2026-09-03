/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#ifndef FluiBuilder_h
#define FluiBuilder_h

#include <memory>
#include <string>

class wxWindow;

class AnnotatPropPanel;
class AnnotData;

class BrushToolDlg;

class CalculationDlg;

class ClipPlanePanel;
class TreeLayer;

class ColocalizationDlg;

class ComponentDlg;

class ConvertDlg;

class CountingDlg;

class FpRangeDlg;

class ListPanel;

class MachineLearningDlg;

class MainFrame;

class ManipPropPanel;
class MeshData;

class MeasureDlg;

class MeshPropPanel;

class MoviePanel;

class NoiseCancellingDlg;

class OclDlg;

class OutputAdjPanel;

class RenderCanvas;
class wxGLContext;
class RenderView;

class RenderViewPanel;

class ScriptBreakDlg;

class SettingDlg;

class TrackDlg;

class TreePanel;

class VolumePropPanel;
class VolumeData;

class FluiBuilder
{
public:
	static AnnotatPropPanel* BuildAnnotatPropPanel(
		wxWindow* parent,
		const std::shared_ptr<AnnotData>& ann);

	static BrushToolDlg* BuildBrushToolDlg(
		wxWindow* parent);

	static CalculationDlg* BuildCalculationDlg(
		wxWindow* parent);

	static ClipPlanePanel* BuildClipPlanePanel(
		wxWindow* parent,
		const std::shared_ptr<TreeLayer>& layer);

	static ColocalizationDlg* BuildColocalizationDlg(
		wxWindow* parent);

	static ComponentDlg* BuildComponentDlg(
		wxWindow* parent);

	static ConvertDlg* BuildConvertDlg(
		wxWindow* parent);

	static CountingDlg* BuildCountingDlg(
		wxWindow* parent);

	static FpRangeDlg* BuildFpRangeDlg(
		wxWindow* parent);

	static ListPanel* BuildListPanel(
		wxWindow* parent);

	static MachineLearningDlg* BuildMachineLearningDlg(
		wxWindow* parent);

	static MainFrame* BuildMainFrame(
		const std::string& title,
		int x, int y,
		int w, int h,
		int reset,
		bool benchmark,
		bool fullscreen,
		bool windowed,
		bool hidepanels);

	static ManipPropPanel* BuildManipPropPanel(
		wxWindow* parent,
		const std::shared_ptr<MeshData>& md);

	static MeasureDlg* BuildMeasureDlg(
		wxWindow* parent);

	static MeshPropPanel* BuildMeshPropPanel(
		wxWindow* parent,
		const std::shared_ptr<MeshData>& md);

	static MoviePanel* BuildMoviePanel(
		wxWindow* parent);

	static NoiseCancellingDlg* BuildNoiseCancellingDlg(
		wxWindow* parent);

	static OclDlg* BuildOclDlg(
		wxWindow* parent);

	static OutputAdjPanel* BuildOutputAdjPanel(
		wxWindow* parent);

	static RenderCanvas* BuildRenderCanvas(
		wxWindow* parent,
		wxGLContext* sharedContext,
		const std::shared_ptr<RenderView>& view
	);

	static RenderViewPanel* BuildRenderViewPanel(
		wxWindow* parent,
		wxGLContext* sharedContext);

	static ScriptBreakDlg* BuildScriptBreakDlg(
		wxWindow* parent);

	static SettingDlg* BuildSettingDlg(
		wxWindow* parent);

	static TrackDlg* BuildTrackDlg(
		wxWindow* parent);

	static TreePanel* BuildTreePanel(
		wxWindow* parent);

	static VolumePropPanel* BuildVolumePropPanel(
		wxWindow* parent,
		const std::shared_ptr<VolumeData>& vd);

protected:
	template<
		class UiT,
		class AgentT,
		class... Args>
	static UiT* BuildUi(
		wxWindow* parent,
		Args&&... args);
};
#endif // FluiBuilder_h