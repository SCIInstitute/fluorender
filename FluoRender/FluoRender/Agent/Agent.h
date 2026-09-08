/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#ifndef Agent_h
#define Agent_h

#include <Value.hpp>
#include <Names.h>
#include <set>
#include <string>

class wxWindow;
class Agent;

#define FOUND_VALUE(v) request.values.find(v) != request.values.end()

enum class UpdateMode : int
{
	All,
	ExcludeSender,
	SenderOnly,
	None
};

enum class UpdateDir : int
{
	Any,
	UItoData,
	DataToUI,
	View
};

struct UpdateRequest
{
	UpdateRequest(
		const fluo::ValueCollection& vals = {},
		Agent* snd = nullptr,
		UpdateMode md = UpdateMode::ExcludeSender,
		const std::string& r = "")
		:
		values(vals),
		sender(snd),
		mode(md),
		reason(r)
	{
	}

	// what changed
	fluo::ValueCollection values;

	// source agent
	Agent* sender = nullptr;

	UpdateMode mode = UpdateMode::ExcludeSender;

	UpdateDir dir = UpdateDir::Any;

	// empty = use UpdateMode
	// non-empty = override UpdateMode
	std::set<Agent*> targets;

	// optional debug string
	std::string reason;

	static UpdateRequest UIToData(
		const fluo::ValueCollection& vals,
		Agent* sender = nullptr,
		UpdateMode mode = UpdateMode::SenderOnly,
		const std::string& reason = "")
	{
		UpdateRequest r(vals, sender, mode, reason);
		r.dir = UpdateDir::UItoData;
		return r;
	}

	static UpdateRequest DataToUI(
		const fluo::ValueCollection& vals,
		Agent* sender = nullptr,
		UpdateMode mode = UpdateMode::ExcludeSender,
		const std::string& reason = "")
	{
		UpdateRequest r(vals, sender, mode, reason);
		r.dir = UpdateDir::DataToUI;
		return r;
	}

	//default to update all views
	static UpdateRequest ViewUpdate(
		const fluo::ValueCollection& vals,
		Agent* sender = nullptr,
		UpdateMode mode = UpdateMode::All,
		const std::string& reason = "")
	{
		fluo::ValueCollection view_vals = vals;
		view_vals.insert(gstRenderView);

		UpdateRequest r(view_vals, sender, mode, reason);
		r.dir = UpdateDir::View;
		return r;
	}

	//update specific views
	static UpdateRequest ViewUpdate(
		const fluo::ValueCollection& vals,
		Agent* sender = nullptr,
		const std::set<Agent*>& targets,
		const std::string& reason = "")
	{
		fluo::ValueCollection view_vals = vals;
		view_vals.insert(gstRenderView);

		UpdateRequest r(
			view_vals,
			sender,
			UpdateMode::None,
			reason);

		r.dir = UpdateDir::View;
		r.targets = targets;
		return r;
	}
};

class Agent
{
public:
	explicit Agent(wxWindow* window) :
		window_(window)
	{
	}

	virtual ~Agent() = default;

	wxWindow* GetWindow() const
	{
		return window_;
	}

	virtual bool Accept(const UpdateRequest& request) const
	{
		return true;
	}

	virtual void Update(const UpdateRequest& request) = 0;

	template<class T>
	T* As()
	{
		return dynamic_cast<T*>(this);
	}

	template<class T>
	const T* As() const
	{
		return dynamic_cast<const T*>(this);
	}

protected:
	void Notify(const UpdateRequest& request);

	void NotifyUIToData(
		const fluo::ValueCollection& vals,
		UpdateMode mode = UpdateMode::SenderOnly,
		const std::string& reason = "")
	{
		Notify(UpdateRequest::UIToData(
			vals, this, mode, reason));
	}

	void NotifyDataToUI(
		const fluo::ValueCollection& vals,
		UpdateMode mode = UpdateMode::ExcludeSender,
		const std::string& reason = "")
	{
		Notify(UpdateRequest::DataToUI(
			vals, this, mode, reason));
	}

	void NotifyViewUpdate(
		const fluo::ValueCollection& vals,
		UpdateMode mode = UpdateMode::All,
		const std::string& reason = "")
	{
		Notify(UpdateRequest::ViewUpdate(
			vals, this, mode, reason));
	}

	void NotifyViewUpdate(
		const fluo::ValueCollection& vals,
		const std::set<Agent*>& targets,
		const std::string& reason = "")
	{
		Notify(UpdateRequest::ViewUpdate(
			vals, this, targets, reason));
	}

private:
	wxWindow* window_ = nullptr;
};

#endif//Agent_h