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
#include <Flobject/Observer.h>

using namespace FL;

Observer::Observer()
{
}

Observer::~Observer()
{
	//delete reversely
	auto it = _observees.end();
	while (it != _observees.begin())
	{
		--it;
		if (*it)
			it = (*it)->removeObserver(this);
	}
}

bool Observer::removeObservee(Referenced* observee)
{
	for (Observees::iterator it=_observees.begin();
		it!=_observees.end(); ++it)
	{
		if ((*it) == observee)
		{
			_observees.erase(it);
			return true;
		}
	}
	return false;
}

ObserverSet::ObserverSet(const Referenced* observedObject):
	_observedObject(const_cast<Referenced*>(observedObject))
{
}

ObserverSet::~ObserverSet()
{
}

void ObserverSet::addObserver(Observer* observer)
{
	_observers.insert(observer);
}

void ObserverSet::removeObserver(Observer* observer)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		if ((*itr) == observer)
		{
			_observers.erase(itr);
			return;
		}
	}
}

Referenced* ObserverSet::addRefLock()
{
	if (!_observedObject) return 0;

	int refCount = _observedObject->ref();
	if (refCount == 1)
	{
		_observedObject->unref_nodelete();
		return 0;
	}

	return _observedObject;
}

void ObserverSet::signalObjectDeleted(void* ptr)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->objectDeleted(ptr);
	}
	_observers.clear();
	_observedObject = 0;
}

void ObserverSet::signalObjectChanging(int notify_level, void* ptr, void* orig_node, const std::string &exp)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->objectChanging(notify_level, ptr, orig_node, exp);
	}
}

void ObserverSet::signalObjectChanged(int notify_level, void* ptr, void* orig_node, const std::string &exp)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->objectChanged(notify_level, ptr, orig_node, exp);
	}
}

//scenegraph events
void ObserverSet::signalNodeAdded(void* ptr, void* parent, void* child)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->nodeAdded(ptr, parent, child);
	}
}

void ObserverSet::signalNodeRemoved(void* ptr, void* parent, void* child)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->nodeRemoved(ptr, parent, child);
	}
}

