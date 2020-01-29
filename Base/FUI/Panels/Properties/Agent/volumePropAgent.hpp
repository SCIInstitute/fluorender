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
#ifndef VOLUME_PROP_AGENT_HPP
#define VOLUME_PROP_AGENT_HPP

#include <Panels/Base_Agent/InterfaceAgent.hpp>
#include <VolumeData/VolumeData.hpp>

#include <QString>
#include <QColor>

namespace FluoUI
{
    class AgentFactory;
    class PropertiesPanel;
    class VolumePropAgent : public InterfaceAgent
	{
	public:
        VolumePropAgent(PropertiesPanel &panel);
/*
        virtual bool isSameKindAs(const fluo::Object* obj) const
		{
            return dynamic_cast<const PropertiesPanel*>(obj) != nullptr;
		}
*/
		virtual const char* className() const { return "VolumePropAgent"; }

        virtual void setObject(fluo::VolumeData* vd);
        virtual fluo::VolumeData* getObject();

        virtual void UpdateAllSettings();
        void updateGammaSettings();
        void updateBoundSettings();
        void updateSaturSettings();
        void updateLThreSettings();
        void updateHThreSettings();
        void updateLuminSettings();
        void updateShadoSettings();
        void updateAlphaSettings();
        void updateSamplSettings();
        void updateShadeSettings();
        void updateLColMSettings();
        void updateHColMSettings();

        friend class AgentFactory;

	protected:

		//update functions
        void OnLuminanceChanged(fluo::Event& event);
        void OnColorChanged(fluo::Event& event);

	private:
        PropertiesPanel &parentPanel;
    };
}

#endif//_VOLUMEPROPAGENT_H_
