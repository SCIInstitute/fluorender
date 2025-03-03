﻿/*
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
/*
* Adapted from the ref_ptr of the OpenSceneGraph library
*/

#ifndef FL_REF_PTR_HPP
#define FL_REF_PTR_HPP

namespace fluo
{
/** Smart pointer for handling referenced counted objects.*/
template<class T>
class ref_ptr
{
public:
	typedef T element_type;

    ref_ptr() : _ptr(nullptr) {}
	ref_ptr(T* ptr) : _ptr(ptr) { if (_ptr) _ptr->ref(); }
	ref_ptr(const ref_ptr& rp) : _ptr(rp._ptr) { if (_ptr) _ptr->ref(); }
	template<class Other> ref_ptr(const ref_ptr<Other>& rp) : _ptr(rp._ptr) { if (_ptr) _ptr->ref(); }
    //ref_ptr(observer_ptr<T>& optr) : _ptr(nullptr) { optr.lock(*this); }
	~ref_ptr()
	{
		if (_ptr)
			_ptr->unref();
        _ptr = nullptr;
	}

	ref_ptr& operator = (const ref_ptr& rp)
	{
		assign(rp);
		return *this;
	}

	template<class Other> ref_ptr& operator = (const ref_ptr<Other>& rp)
	{
		assign(rp);
		return *this;
	}

	inline ref_ptr& operator = (T* ptr)
	{
		if (_ptr==ptr) return *this;
		T* tmp_ptr = _ptr;
		_ptr = ptr;
		if (_ptr) _ptr->ref();
		// unref second to prevent any deletion of any object which might
		// be referenced by the other object. i.e rp is child of the
		// original _ptr.
		if (tmp_ptr) tmp_ptr->unref();
		return *this;
	}

	// implicit output conversion
	//operator T*() const { return _ptr; }
	// comparison operators for ref_ptr.
	bool operator == (const ref_ptr& rp) const { return (_ptr==rp._ptr); }
	bool operator == (const T* ptr) const { return (_ptr==ptr); }
	friend bool operator == (const T* ptr, const ref_ptr& rp) { return (ptr==rp._ptr); }

	bool operator != (const ref_ptr& rp) const { return (_ptr!=rp._ptr); }
	bool operator != (const T* ptr) const { return (_ptr!=ptr); }
	friend bool operator != (const T* ptr, const ref_ptr& rp) { return (ptr!=rp._ptr); }

	bool operator < (const ref_ptr& rp) const { return (_ptr<rp._ptr); }


	// follows is an implmentation of the "safe bool idiom", details can be found at:
	//   http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Safe_bool
	//   http://lists.boost.org/Archives/boost/2003/09/52856.php

private:
	typedef T* ref_ptr::*unspecified_bool_type;

public:
	// safe bool conversion
    operator unspecified_bool_type() const { return valid()? &ref_ptr::_ptr : nullptr; }

	T& operator*() const { return *_ptr; }
	T* operator->() const { return _ptr; }
	T* get() const { return _ptr; }

    bool operator!() const   { return _ptr==nullptr; } // not required
    bool valid() const       { return _ptr!=nullptr; }

    T* release() { T* tmp=_ptr; if (_ptr) _ptr->unref_nodelete(); _ptr=nullptr; return tmp; }

	void swap(ref_ptr& rp) { T* tmp=_ptr; _ptr=rp._ptr; rp._ptr=tmp; }

private:

	template<class Other> void assign(const ref_ptr<Other>& rp)
	{
		if (_ptr==rp._ptr) return;
		T* tmp_ptr = _ptr;
		_ptr = rp._ptr;
		if (_ptr) _ptr->ref();
		// unref second to prevent any deletion of any object which might
		// be referenced by the other object. i.e rp is child of the
		// original _ptr.
		if (tmp_ptr) tmp_ptr->unref();
	}

	template<class Other> friend class ref_ptr;

	T* _ptr;
};

template<class T> inline
	void swap(ref_ptr<T>& rp1, ref_ptr<T>& rp2) { rp1.swap(rp2); }

template<class T> inline
	T* get_pointer(const ref_ptr<T>& rp) { return rp.get(); }

template<class T, class Y> inline
	ref_ptr<T> static_pointer_cast(const ref_ptr<Y>& rp) { return static_cast<T*>(rp.get()); }

template<class T, class Y> inline
	ref_ptr<T> dynamic_pointer_cast(const ref_ptr<Y>& rp) { return dynamic_cast<T*>(rp.get()); }

template<class T, class Y> inline
	ref_ptr<T> const_pointer_cast(const ref_ptr<Y>& rp) { return const_cast<T*>(rp.get()); }

}

#endif
