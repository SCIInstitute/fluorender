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
#ifndef _RENDERVIEWAGENT_H_
#define _RENDERVIEWAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>

class RenderviewPanel;
namespace fluo
{
	class RenderviewAgent : public InterfaceAgent
	{
	public:
		//top settings
		DEFINE_ATTR(MixMethod);				//Renderview::MixMethods
		DEFINE_ATTR(DrawInfo);				//Info
		DEFINE_ATTR(DrawCamCtr);			//Axis
		DEFINE_ATTR(DrawLegend);			//Legend
		DEFINE_ATTR(DrawColormap);			//Color Map
		DEFINE_ATTR(DrawScaleBar);			//Show scale bar
		DEFINE_ATTR(DrawScaleBarText);		//Show scale bar text
		DEFINE_ATTR(BgColor);				//Background
		DEFINE_ATTR(Aov);					//Projection
		DEFINE_ATTR(Perspective);			//perspective projection enable
		DEFINE_ATTR(Free);					//free fly enable
		DEFINE_ATTR(FullScreen);			//flag only
		//left settings
		DEFINE_ATTR(DepthAtten);			//Depth Attenuation (check)
		DEFINE_ATTR(DaInt);					//Depth Attenuation
		//right settings
		DEFINE_ATTR(PinRotCtr);				//Pin
		DEFINE_ATTR(ScaleFactor121);		//Scale factor at 121
		DEFINE_ATTR(ScaleFactor);			//scale factor
		DEFINE_ATTR(ScaleMode);				//Zoom ratio mode: 0-view; 1-pixel; 2-data(pixel*xy spc)
		//bottom settings
		DEFINE_ATTR(GearedEnable);			//enable geared rotation
		DEFINE_ATTR(CamRotX);				//camera rotation
		DEFINE_ATTR(CamRotY);
		DEFINE_ATTR(CamRotZ);
		DEFINE_ATTR(CamRotQ);				//rotation in quaternion
		DEFINE_ATTR(CamRotZeroQ);			//rotation at zero setting

		RenderviewAgent(RenderviewPanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const RenderviewAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "RenderviewAgent"; }

		virtual void setObject(Renderview* view);
		virtual Renderview* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual RenderviewAgent* asRenderviewAgent() { return this; }
		virtual const RenderviewAgent* asRenderviewAgent() const { return this; }

		friend class AgentFactory;

	protected:
		RenderviewPanel &panel_;

		virtual void setupInputs();

		//void OnBoundsChanged(Event& event);
		//void OnSceneChanged(Event& event);
	};
}
#endif//_RENDERVIEWAGENT_H_