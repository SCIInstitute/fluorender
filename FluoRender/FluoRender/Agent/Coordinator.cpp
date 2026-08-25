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

#include <Coordinator.h>
#include <Agent.h>

void Coordinator::Register(Agent* agent)
{
	if (!agent)
		return;

	agents_.insert(agent);
}

void Coordinator::Unregister(Agent* agent)
{
	if (!agent)
		return;

	agents_.erase(agent);
}

void Coordinator::Dispatch(
	const UpdateRequest& request)
{
	if (request.mode == UpdateMode::None)
		return;

	for (auto agent : agents_)
	{
		if (!agent)
			continue;

		switch (request.mode)
		{
		case UpdateMode::All:
			// update all
			break;

		case UpdateMode::ExcludeSender:
			// exclude sender
			if (agent == request.sender)
				continue;
			break;

		case UpdateMode::SenderOnly:
			// sender only
			if (agent != request.sender)
				continue;
			break;

		default:
			break;
		}

		if (agent->Accept(request))
			agent->Update(request);
	}
}