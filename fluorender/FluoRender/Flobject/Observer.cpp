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
#include <Flobject/Event.h>

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
	_observedObject(const_cast<Referenced*>(observedObject)),
	_observers(ObserverComparator())
{
}

ObserverSet::~ObserverSet()
{
}

void ObserverSet::addObserver(Observer* observer)
{
	_observers.insert(observer);
}

bool ObserverSet::hasObserver(Observer* observer)
{
	auto result = _observers.find(observer);
	if (result != _observers.end())
		return true;
	return false;
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

void ObserverSet::signalObjectDeleted(Event& event)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->objectDeleted(event);
	}
	_observers.clear();
	_observedObject = 0;
}

void ObserverSet::signalObjectChanging(Event& event)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->objectChanging(event);
	}
}

void ObserverSet::signalObjectChanged(Event& event)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->objectChanged(event);
	}
}

//scenegraph events
void ObserverSet::signalNodeAdded(Event& event)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->nodeAdded(event);
	}
}

void ObserverSet::signalNodeRemoved(Event& event)
{
	for (Observers::iterator itr = _observers.begin();
		itr != _observers.end(); ++itr)
	{
		(*itr)->nodeRemoved(event);
	}
}

