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
#include <AsyncTimer.hpp>

using namespace fluo;

AsyncTimer::AsyncTimer()
{
}

AsyncTimer::AsyncTimer(
	std::function<void(void)> func,
	const long& interval)
	: m_func(std::move(func)),
	interval_(interval)
{
}

AsyncTimer::~AsyncTimer()
{
	stop();
}

void AsyncTimer::start(long interval)
{
	stop(); // CHANGE:
	// prevent multiple worker threads

	interval_ = interval;
	run_ = true;

	m_thread = std::thread([this]()
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			while (run_)
			{
				// CHANGE:
				// wait first, then execute.
				if (m_cv.wait_for(
					lock,
					std::chrono::milliseconds(interval_),
					[&] {
					return !run_;
				}))
				{
					break;
				}

				auto func = m_func;

				lock.unlock();

				try
				{
					if (func)
						func();
				}
				catch (...)
				{
					// CHANGE:
					// keep timer thread alive.
				}

				lock.lock();
			}
		});
}

void AsyncTimer::stop()
{
	if (!run_)
		return;

	run_ = false;

	// CHANGE:
	// wake worker immediately.
	m_cv.notify_all();

	// CHANGE:
	// safely wait for worker exit.
	if (m_thread.joinable())
		m_thread.join();
}