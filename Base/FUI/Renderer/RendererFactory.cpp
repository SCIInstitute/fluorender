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

#include "RendererFactory.hpp"

using namespace fluo;

RendererFactory::RendererFactory()
{
	m_name = "renderer factory";
	default_object_name_ = "default renderer";
}

RendererFactory::~RendererFactory()
{

}

//ClipPlaneRenderer* ProcessorFactory::getOrAddClipPlaneRenderer(const std::string &name)
//{
//	Processor* result = findFirst(name);
//	if (result)
//		return dynamic_cast<ClipPlaneRenderer*>(result);
//
//	//not found
//	ClipPlaneRenderer* renderer = new FLR::ClipPlaneRenderer();
//	if (renderer)
//	{
//		renderer->setName(name);
//		objects_.push_front(renderer);
//	}
//	return renderer;
//}
