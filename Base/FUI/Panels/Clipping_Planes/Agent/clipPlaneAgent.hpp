/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef CLIP_PLANE_AGENT_HPP
#define CLIP_PLANE_AGENT_HPP

#include <AgentFactory.hpp>
#include <VolumeData.hpp>

class AgentFactory;
class ClippingPlane;
class ClipPlaneAgent : public fluo::InterfaceAgent
{
  public:
	ClipPlaneAgent(ClippingPlane &panel);

	virtual bool isSameKindAs(const fluo::Object* obj) const
	{
		return dynamic_cast<const ClipPlaneAgent*>(obj) != NULL;
	}

	virtual const char* className() const { return "ClipPlaneAgent"; }

	virtual void setObject(fluo::VolumeData* vd);
	virtual fluo::VolumeData* getObject();

	virtual void UpdateAllSettings();

	//void alignRenderViewRot(); Need to figure out how to get this to play with my renderview.

	friend class AgentFactory;

  protected:

	//update functions
	void OnClipXChanged(fluo::Event& event);
	void OnClipYChanged(fluo::Event& event);
	void OnClipZChanged(fluo::Event& event);
	void OnClipDistXChanged(fluo::Event& event);
	void OnClipDistYChanged(fluo::Event& event);
	void OnClipDistZChanged(fluo::Event& event);
	void OnClipLinkXChanged(fluo::Event& event);
	void OnClipLinkYChanged(fluo::Event& event);
	void OnClipLinkZChanged(fluo::Event& event);
	void OnClipRotXChanged(fluo::Event& event);
	void OnClipRotYChanged(fluo::Event& event);
	void OnClipRotZChanged(fluo::Event& event);
  private:
	ClippingPlane &parentPanel;

};

#endif//_CLIPPLANEAGENT_H_