/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <AsyncTimerFactory.hpp>
#include <Names.h>

using namespace fluo;

AsyncTimerFactory::AsyncTimerFactory()
{
	m_name = gstAsyncTimerFactory;
	default_object_name_ = gstAsyncTimer;
}

AsyncTimerFactory::~AsyncTimerFactory()
{

}

void AsyncTimerFactory::createDefault()
{
	if (!getDefault())
	{
		AsyncTimer* at = new AsyncTimer();
		at->setName(default_object_name_);
		objects_.push_front(at);

		//add default values here
		at->addValue(gstTimerInterval, long(1000));
		at->addValue(gstTimerRunning, bool(false));
	}
}

AsyncTimer* AsyncTimerFactory::build(AsyncTimer* at)
{
	if (at)
		return clone(at);
	unsigned int default_id = 0;
	return clone(default_id);
}

AsyncTimer* AsyncTimerFactory::clone(AsyncTimer* at)
{
	incCounter();

	Object* new_at = at->clone(CopyOp::DEEP_COPY_ALL);
	new_at->setId(global_id_);
	std::string name = "async timer" + std::to_string(local_id_);
	new_at->setName(name);
	new_at->addRvalu(gstFactory, this);

	objects_.push_front(new_at);

	return dynamic_cast<AsyncTimer*>(new_at);
}

AsyncTimer* AsyncTimerFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		AsyncTimer* at = dynamic_cast<AsyncTimer*>(object);
		if (at)
			return clone(at);
	}
	return 0;
}

