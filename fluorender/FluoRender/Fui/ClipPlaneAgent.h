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
#ifndef _CLIPPLANEAGENT_H_
#define _CLIPPLANEAGENT_H_

#include <Fui/InterfaceAgent.h>
#include <Scenegraph/Node.h>

namespace FUI
{
	class ClipPlanePanel;
	class ClipPlaneAgent : public InterfaceAgent
	{
	public:
		ClipPlaneAgent(ClipPlanePanel &panel);

		virtual bool isSameKindAs(const FL::Object* obj) const
		{
			return dynamic_cast<const ClipPlaneAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "ClipPlaneAgent"; }

		//observer
		virtual void objectChanging(int notify_level, void*, void* orig_node, const std::string &exp);
		virtual void objectChanged(int notify_level, void*, void* orig_node, const std::string &exp);

		virtual void setObject(FL::Node* vd);
		virtual FL::Node* getObject();

		virtual void UpdateAllSettings();

	protected:
		ClipPlanePanel &panel_;

	private:
	};
}

#endif//_CLIPPLANEAGENT_H_