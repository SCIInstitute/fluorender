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

#include <StopWatchFactory.hpp>
#include <Names.hpp>

using namespace fluo;

StopWatchFactory::StopWatchFactory()
{
	m_name = gstStopWatchFactory;
	default_object_name_ = gstStopWatch;
}

StopWatchFactory::~StopWatchFactory()
{

}

void StopWatchFactory::createDefault()
{
	if (!getDefault())
	{
		StopWatch* obj = new StopWatch();
		obj->setName(default_object_name_);
		objects_.push_front(obj);
	}
}

StopWatch* StopWatchFactory::build(StopWatch* sw)
{
	if (sw)
		return clone(sw);
	unsigned int default_id = 0;
	return clone(default_id);
}

StopWatch* StopWatchFactory::clone(StopWatch* sw)
{
	incCounter();

	Object* new_sw = sw->clone(CopyOp::DEEP_COPY_ALL);
	new_sw->setId(global_id_);
	std::string name = "stop watch" + std::to_string(local_id_);
	new_sw->setName(name);
	new_sw->addRvalu(gstFactory, this);

	objects_.push_front(new_sw);

	return dynamic_cast<StopWatch*>(new_sw);
}

StopWatch* StopWatchFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		StopWatch* sw = dynamic_cast<StopWatch*>(object);
		if (sw)
			return clone(sw);
	}
	return 0;
}

