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

#include "RenderviewFactory.hpp"
#include <Names.hpp>

using namespace fluo;

RenderviewFactory::RenderviewFactory()
{
	m_name = gstRenderviewFactory;
	default_object_name_ = gstDefaultRenderview;
}

RenderviewFactory::~RenderviewFactory()
{

}

void RenderviewFactory::createDefault()
{
	if (!getDefault())
	{
		Renderview* view = new Renderview();
		view->setName(default_object_name_);
		objects_.push_back(view);

		//add default values here
		//view->addValue(gstColor, Color());
		//view->addRvalu(gstVolume, (Referenced*)0);
		//view->addValue(gstTransform, Transform());
		//view->addValue(gstDisplay, bool(true));
		//view->addValue(gstMemo, std::string());
		//view->addValue(gstMemoRo, bool(true));//memo is read only
		//view->addValue(gstDataPath, std::wstring());
		//view->addValue(gstInfoHeader, std::string());
	}
}

Renderview* RenderviewFactory::build(Renderview* view)
{
	if (view)
		return clone(view);
	unsigned int default_id = 0;
	return clone(default_id);
}

Renderview* RenderviewFactory::clone(Renderview* view)
{
	incCounter();

	Object* new_view = view->clone(CopyOp::DEEP_COPY_ALL);
	new_view->setId(global_id_);
	std::string name = "renderview" + std::to_string(local_id_);
	new_view->setName(name);

	objects_.push_back(new_view);

	return dynamic_cast<Renderview*>(new_view);
}

Renderview* RenderviewFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		Renderview* view = dynamic_cast<Renderview*>(object);
		if (view)
			return clone(view);
	}
	return 0;
}

