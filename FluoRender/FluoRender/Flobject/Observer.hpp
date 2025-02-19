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
#ifndef FL_OBSERVER_HPP
#define FL_OBSERVER_HPP

#include <Referenced.hpp>
#include <set>
#include <string>

namespace fluo
{
class Event;
class Observer
{
public:

	Observer();
	virtual ~Observer();

	virtual unsigned int getPriority() const { return 9999; }//priority number to sort observers
	virtual void objectDeleted(Event& event) {}
	virtual void processNotification(Event& event) {}

	virtual bool removeObservee(Referenced* observee);

	friend Referenced;

protected:
	Observees _observees;
};

//determines which observer to notify first
class ObserverComparator
{
public:
	bool operator() (const Observer* obsrvr1, const Observer* obsrvr2) const
	{
		if (obsrvr1->getPriority() != obsrvr2->getPriority())
			return obsrvr1->getPriority() < obsrvr2->getPriority();
		else
			return obsrvr1 < obsrvr2;
	}
};

class ObserverSet : public Referenced
{
public:

	ObserverSet(const Referenced* observedObject);

	Referenced* getObservedObject() { return _observedObject; }
	const Referenced* getObservedObject() const { return _observedObject; }

	Referenced* addRefLock();

	void addObserver(Observer* observer);
	bool hasObserver(Observer* observer);
	void removeObserver(Observer* observer);

	void signalObjectDeleted(Event& event);
	void notifyObserver(Event& event);

	typedef std::set<Observer*, ObserverComparator> Observers;
	Observers& getObservers() { return _observers; }
	const Observers& getObservers() const { return _observers; }

	virtual const char* className() const { return "ObserverSet"; }

protected:

	ObserverSet(const ObserverSet& rhs): Referenced(rhs) {}
	ObserverSet& operator = (const ObserverSet&) {return *this; }
	virtual ~ObserverSet();

	Referenced* _observedObject;
	Observers _observers;
};

}
#endif
