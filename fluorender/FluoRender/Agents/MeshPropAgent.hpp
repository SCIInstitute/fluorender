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
#ifndef _MESHPROPAGENT_H_
#define _MESHPROPAGENT_H_

#include <InterfaceAgent.hpp>
#include <MeshData.hpp>

#define gstAlpha "alpha"					//Transparency
#define gstShadowEnable "shadow enable"		//Shadow (check)
#define gstShadowInt "shadow int"			//Shadow
#define gstShadingEnable "shding enable"	//Lighting (check)
#define gstColor "color"					//Color (linked to material colors)
#define gstMatAmb "mat amb"					//Diffuse Color
#define gstMatSpec "mat spec"				//Specular Color
#define gstMatShine "mat shine"				//Shininess
//scaling
#define gstScaleX "scale x"
#define gstScaleY "scale y"
#define gstScaleZ "scale z"
//size limiter
#define gstLimitEnable "limit enable"		//size limit (check)
#define gstLimit "limit"					//Size Limit

class MeshPropPanel;
namespace fluo
{
	class MeshPropAgent : public InterfaceAgent
	{
	public:
		MeshPropAgent(MeshPropPanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const MeshPropAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "MeshPropAgent"; }

		virtual void setObject(MeshData* vd);
		virtual MeshData* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual MeshPropAgent* asMeshPropAgent() { return this; }
		virtual const MeshPropAgent* asMeshPropAgent() const { return this; }

	protected:
		MeshPropPanel &panel_;

	private:
	};
}

#endif//_MESHPROPAGENT_H_