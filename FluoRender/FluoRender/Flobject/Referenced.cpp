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
#include <Referenced.hpp>
#include <Observer.hpp>
#include <Event.hpp>
#include <algorithm>

using namespace fluo;

Referenced::Referenced():
  _observerSet(nullptr),
  _refCount(0),
  _refStr(""),
  _hold(false)
{
}

Referenced::Referenced(const Referenced&):
  _observerSet(nullptr),
  _refCount(0),
  _refStr(""),
  _hold(false)
{
}

Referenced::~Referenced()
{
	signalObserversAndDelete(true, false);

	if (_observerSet)
        static_cast<ObserverSet*>(_observerSet)->unref();
}

ObserverSet* Referenced::getOrCreateObserverSet() const
{
	if (!_observerSet)
	{
        _observerSet = new ObserverSet(this);
        static_cast<ObserverSet*>(_observerSet)->ref();
	}
    return static_cast<ObserverSet*>(_observerSet);
}

void Referenced::addObserver(Observer* observer) const
{
    if (const_cast<Referenced*>(this) ==
        dynamic_cast<Referenced*>(observer))
		return;
	getOrCreateObserverSet()->addObserver(observer);
	observer->_observees.insert(const_cast<Referenced*>(this));
}

bool Referenced::hasObserver(Observer* observer) const
{
	if (!_observerSet)
		return false;
    return static_cast<ObserverSet*>(_observerSet)->hasObserver(observer);
}

ObserveeIter Referenced::removeObserver(Observer* observer) const
{
	getOrCreateObserverSet()->removeObserver(observer);
	auto it = std::find(observer->_observees.begin(),
        observer->_observees.end(), const_cast<Referenced*>(this));
	if (it != observer->_observees.end())
		return observer->_observees.erase(it);
	else
		return it;
}

void Referenced::signalObserversAndDelete(bool signalDelete, bool doDelete) const
{
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet);

	if (observerSet && signalDelete)
	{
		Event event;
		event.init(Event::EVENT_DELETED,
            const_cast<Referenced*>(this), true);
		observerSet->signalObjectDeleted(event);
	}

	if (doDelete)
	{
		delete this;
	}
}

void Referenced::notifyObservers(Event& event) const
{
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet);

	if (observerSet && !_hold &&
        event.pass(const_cast<Referenced*>(this)))
	{
        event.push(const_cast<Referenced*>(this));
		observerSet->notifyObserver(event);
		event.pop();
	}
}

int Referenced::unref_nodelete() const
{
	return --_refCount;
}
