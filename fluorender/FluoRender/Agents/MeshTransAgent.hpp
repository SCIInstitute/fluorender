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
#ifndef _MESHTRANAGENT_H_
#define _MESHTRANAGENT_H_

#include <InterfaceAgent.hpp>
#include <MeshData.hpp>

//transformation
#define gstTransX "trans x"
#define gstTransY "trans y"
#define gstTransZ "trans z"
#define gstRotX "rot x"
#define gstRotY "rot y"
#define gstRotZ "rot z"
#define gstScaleX "scale x"
#define gstScaleY "scale y"
#define gstScaleZ "scale z"

class MeshTransPanel;
namespace fluo
{
	class MeshTransAgent : public InterfaceAgent
	{
	public:
		MeshTransAgent(MeshTransPanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const MeshTransAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "MeshTransAgent"; }

		virtual void setObject(MeshData* an);
		virtual MeshData* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual MeshTransAgent* asMeshTransAgent() { return this; }
		virtual const MeshTransAgent* asMeshTransAgent() const { return this; }

		friend class AgentFactory;

	protected:
		MeshTransPanel &panel_;

	private:
	};
}

#endif//_MESHTRANAGENT_H_