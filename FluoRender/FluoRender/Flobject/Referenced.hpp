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
#ifndef FL_REFERENCED_HPP
#define FL_REFERENCED_HPP

#include <string>
#include <set>

namespace fluo
{
class Event;
class Observer;
class ObserverSet;
class Referenced;
typedef std::set<Referenced*> Observees;
typedef std::set<Referenced*>::iterator ObserveeIter;
class Referenced
{
public:

	Referenced();

	Referenced(const Referenced&);

	inline Referenced& operator = (const Referenced&) { return *this; }

	inline int ref() const;

	inline int unref() const;

	int unref_nodelete() const;

	inline int referenceCount() const { return _refCount; }

	ObserverSet* getObserverSet() const
	{
		return static_cast<ObserverSet*>(_observerSet);
	}

	ObserverSet* getOrCreateObserverSet() const;

	void addObserver(Observer* observer) const;

	bool hasObserver(Observer* observer) const;

	ObserveeIter removeObserver(Observer* observer) const;

	void holdoffObserverNotification()
	{
		_hold = true;
	}
	void resumeObserverNotification()
	{
		_hold = false;
	}

	virtual const char* className() const { return "Referenced";}

	const std::string& getRefStr() const { return _refStr; }

protected:

	virtual ~Referenced();

	void signalObserversAndDelete(bool signalDelete, bool doDelete) const;

	void notifyObservers(Event& event) const;

	std::string _refStr;

	mutable int _refCount;

	mutable void* _observerSet;

	bool _hold;
};

inline int Referenced::ref() const
{
	return ++_refCount;
}

inline int Referenced::unref() const
{
	int newRef;
	newRef = --_refCount;
	bool needDelete = (newRef == 0);

	if (needDelete)
	{
		signalObserversAndDelete(true, true);
	}

	return newRef;
}

}

#endif
