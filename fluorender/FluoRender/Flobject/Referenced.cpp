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
#include <Flobject/Referenced.h>
#include <Flobject/Observer.h>
#include <algorithm>

using namespace FL;

Referenced::Referenced():
  _observerSet(0),
  _refCount(0),
  _refStr(""),
  _hold(false)
{
}

Referenced::Referenced(const Referenced&):
  _observerSet(0),
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
	getOrCreateObserverSet()->addObserver(observer);
	observer->_observees.push_back(const_cast<Referenced*>(this));
}

void Referenced::removeObserver(Observer* observer) const
{
	getOrCreateObserverSet()->removeObserver(observer);
	auto it = std::find(observer->_observees.begin(),
		observer->_observees.end(), const_cast<Referenced*>(this));
	if (it != observer->_observees.end())
		observer->_observees.erase(it);
}

void Referenced::signalObserversAndDelete(bool signalDelete, bool doDelete) const
{
	ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet);

	if (observerSet && signalDelete)
	{
		observerSet->signalObjectDeleted(const_cast<Referenced*>(this));
	}

	if (doDelete)
	{
		delete this;
	}
}

void Referenced::notifyObserversOfChange(const std::string &exp) const
{
	ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet);

	if (observerSet && !_hold)
	{
		observerSet->signalObjectChanged(const_cast<Referenced*>(this), exp);
	}

}

int Referenced::unref_nodelete() const
{
	return --_refCount;
}