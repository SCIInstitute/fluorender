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
#ifndef _AGENTFACTORY_H_
#define _AGENTFACTORY_H_

#include <ObjectFactory.hpp>
#include <InterfaceAgent.hpp>
#include <Value.hpp>

#pragma message ("replace dummy window class")
class wxWindow
{
public:
	wxWindow() {}
	~wxWindow() {}
	void UpdateWindow(const fluo::ValueCollection &names) {}
};
#pragma message ("replace dummy dialog")
class AnnotationPropPanel : public wxWindow
{
public:
	AnnotationPropPanel() {}
	~AnnotationPropPanel() {}
};
class BrushToolDlg : public wxWindow
{
public:
	BrushToolDlg() {}
	~BrushToolDlg() {}
};
class CalculationDlg : public wxWindow
{
public:
	CalculationDlg() {}
	~CalculationDlg() {}
};
class ClipPlanePanel : public wxWindow
{
public:
	ClipPlanePanel() {}
	~ClipPlanePanel() {}
};
class ClKernelDlg : public wxWindow
{
public:
	ClKernelDlg() {}
	~ClKernelDlg() {}
};
class ColocalDlg : public wxWindow
{
public:
	ColocalDlg() {}
	~ColocalDlg() {}
};
class ComponentDlg : public wxWindow
{
public:
	ComponentDlg() {}
	~ComponentDlg() {}
};
class ConvertDlg : public wxWindow
{
public:
	ConvertDlg() {}
	~ConvertDlg() {}
};
class CountingDlg : public wxWindow
{
public:
	CountingDlg() {}
	~CountingDlg() {}
};
class ListPanel : public wxWindow
{
public:
	ListPanel() {}
	~ListPanel() {}
};
class MeasureDlg : public wxWindow
{
public:
	MeasureDlg() {}
	~MeasureDlg() {}
};
class MeshPropPanel : public wxWindow
{
public:
	MeshPropPanel() {}
	~MeshPropPanel() {}
};
class MeshTransPanel : public wxWindow
{
public:
	MeshTransPanel() {}
	~MeshTransPanel() {}
};
class MoviePanel : public wxWindow
{
public:
	MoviePanel() {}
	~MoviePanel() {}
};
class NoiseReduceDlg : public wxWindow
{
public:
	NoiseReduceDlg() {}
	~NoiseReduceDlg() {}
};
class OutAdjustPanel : public wxWindow
{
public:
	OutAdjustPanel() {}
	~OutAdjustPanel() {}
};
class RecorderDlg : public wxWindow
{
public:
	RecorderDlg() {};
	~RecorderDlg() {};
};
class RenderCanvas : public wxWindow
{
public:
	RenderCanvas() {}
	~RenderCanvas() {}
};
class RenderFrame : public wxWindow
{
public:
	RenderFrame() {}
	~RenderFrame() {}
};
class RenderviewPanel : public wxWindow
{
public:
	RenderviewPanel() {}
	~RenderviewPanel() {}
};
class SettingDlg : public wxWindow
{
public:
	SettingDlg() {}
	~SettingDlg() {}
};
class TrackDlg : public wxWindow
{
public:
	TrackDlg() {}
	~TrackDlg() {}
};
class TreePanel : public wxWindow
{
public:
	TreePanel() {}
	~TreePanel() {}
};
class VolumePropPanel : public wxWindow
{
public:
	VolumePropPanel() {}
	~VolumePropPanel() {}
};

namespace fluo
{
	class AnnotationPropAgent;
	class BrushToolAgent;
	class CalculationAgent;
	class ClipPlaneAgent;
	class ClKernelAgent;
	class ColocalAgent;
	class ComponentAgent;
	class ConvertAgent;
	class CountingAgent;
	class ListAgent;
	class MeasureAgent;
	class MeshPropAgent;
	class MeshTransAgent;
	class MovieAgent;
	class NoiseReduceAgent;
	class OutAdjustAgent;
	class RecorderAgent;
	class RenderCanvasAgent;
	class RenderFrameAgent;
	class RenderviewAgent;
	class SettingAgent;
	class TrackAgent;
	class TreeAgent;
	class VolumePropAgent;
	class AgentFactory : public ObjectFactory
	{
	public:
		AgentFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const AgentFactory*>(obj) != NULL;
		}

		virtual const char* className() const { return "AgentFactory"; }

		virtual void createDefault() {}//no default agent

		virtual InterfaceAgent* getDefault() { return 0; }//no default agent

		virtual InterfaceAgent* build(InterfaceAgent* agent = 0) { return 0; }

		virtual InterfaceAgent* clone(InterfaceAgent*) { return 0; }

		virtual InterfaceAgent* clone(const unsigned int) { return 0; }

		inline virtual InterfaceAgent* get(size_t i)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::get(i));
		}

		inline virtual const InterfaceAgent* get(size_t i) const
		{
			return dynamic_cast<InterfaceAgent*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual InterfaceAgent* find(const unsigned int id)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::find(id));
		}

		inline virtual InterfaceAgent* findFirst(const std::string &name)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::findFirst(name));
		}

		inline virtual InterfaceAgent* findLast(const std::string &name)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::findLast(name));
		}

		//each agent type has a function
		//add
		AnnotationPropAgent* addAnnotationPropAgent(const std::string &name, wxWindow &window);
		BrushToolAgent* addBrushToolAgent(const std::string &name, wxWindow &window, wxWindow &window2);
		CalculationAgent* addCalculationAgent(const std::string &name, wxWindow &window);
		ClipPlaneAgent* addClipPlaneAgent(const std::string &name, wxWindow &window);
		ClKernelAgent* addClKernelAgent(const std::string &name, wxWindow &window);
		ColocalAgent* addColocalAgent(const std::string &name, wxWindow &window);
		ComponentAgent* addComponentAgent(const std::string &name, wxWindow &window);
		ConvertAgent* addConvertAgent(const std::string &name, wxWindow &window);
		CountingAgent* addCountingAgent(const std::string &name, wxWindow &window);
		ListAgent* addListAgent(const std::string &name, wxWindow &window);
		MeasureAgent* addMeasureAgent(const std::string &name, wxWindow &window);
		MeshPropAgent* addMeshPropAgent(const std::string &name, wxWindow &window);
		MeshTransAgent* addMeshTransAgent(const std::string &name, wxWindow &window);
		MovieAgent* addMovieAgent(const std::string &name, wxWindow &window);
		NoiseReduceAgent* addNoiseReduceAgent(const std::string &name, wxWindow &window);
		OutAdjustAgent* addOutAdjustAgent(const std::string &name, wxWindow &window);
		RecorderAgent* addRecorderAgent(const std::string &name, wxWindow &window);
		RenderCanvasAgent* addRenderCanvasAgent(const std::string &name, wxWindow &window);
		RenderFrameAgent* addRenderFrameAgent(const std::string &name, wxWindow &window);
		RenderviewAgent* addRenderviewAgent(const std::string &name, wxWindow &window);
		SettingAgent* addSettingAgent(const std::string &name, wxWindow &window);
		TrackAgent* addTrackAgent(const std::string &name, wxWindow &window);
		TreeAgent* addTreeAgent(const std::string &name, wxWindow &window);
		VolumePropAgent* addVolumePropAgent(const std::string &name, wxWindow &window);
		//get
		AnnotationPropAgent* getAnnotationPropAgent(const std::string &name = gstAnnotationPropAgent);
		BrushToolAgent* getBrushToolAgent(const std::string &name = gstBrushToolAgent);
		CalculationAgent* getCalculationAgent(const std::string &name = gstCalculationAgent);
		ClipPlaneAgent* getClipPlaneAgent(const std::string &name = gstClipPlaneAgent);
		ClKernelAgent* getClKernelAgent(const std::string &name = gstClKernelAgent);
		ColocalAgent* getColocalAgent(const std::string &name = gstColocalAgent);
		ComponentAgent* getComponentAgent(const std::string &name = gstComponentAgent);
		ConvertAgent* getConvertAgent(const std::string &name = gstConvertAgent);
		CountingAgent* getCountingAgent(const std::string &name = gstCountingAgent);
		ListAgent* getListAgent(const std::string &name = gstListAgent);
		MeasureAgent* getMeasureAgent(const std::string &name = gstMeasureAgent);
		MeshPropAgent* getMeshPropAgent(const std::string &name = gstMeshPropAgent);
		MeshTransAgent* getMeshTransAgent(const std::string &name = gstMeshTransAgent);
		MovieAgent* getMovieAgent(const std::string &name = gstMovieAgent);
		NoiseReduceAgent* getNoiseReduceAgent(const std::string &name = gstNoiseReduceAgent);
		OutAdjustAgent* getOutAdjustAgent(const std::string &name = gstOutAdjustAgent);
		RecorderAgent* getRecorderAgent(const std::string &name = gstRecorderAgent);
		RenderCanvasAgent* getRenderCanvasAgent(const std::string &name = gstRenderCanvasAgent);
		RenderFrameAgent* getRenderFrameAgent(const std::string &name = gstRenderFrameAgent);
		RenderviewAgent* getRenderviewAgent(const std::string &name = gstRenderviewAgent);
		SettingAgent* getSettingAgent(const std::string &name = gstSettingAgent);
		TrackAgent* getTrackAgent(const std::string &name = gstTrackAgent);
		TreeAgent* getTreeAgent(const std::string &name = gstTreeAgent);
		VolumePropAgent* getVolumePropAgent(const std::string &name = gstVolumePropAgent);

	protected:
		virtual ~AgentFactory();
	};
}

#endif//_AGENTFACTORY_H_
