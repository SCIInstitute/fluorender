/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef _ASYNC_TIMER_H_
#define _ASYNC_TIMER_H_

#include <Names.hpp>
#include <Node.hpp>
#include <chrono>
#include <functional>
#include <thread>

namespace fluo
{
	/**
	 *  Create asynchronous timers which execute specified
	 *  functions in set time interval.
	 *
	 *  @param func		Function which sould be executed
	 *  @param interval	Interval of time in which function will be executed
	 *					(in milliseconds)
	 */
	class AsyncTimer : public Node
	{
	public:
		AsyncTimer();

		AsyncTimer(std::function<void(void)> func, const long &interval);

		AsyncTimer(const AsyncTimer& data, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new AsyncTimer(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const AsyncTimer*>(obj) != NULL;
		}

		virtual const char* className() const { return "AsyncTimer"; }

		virtual AsyncTimer* asAsyncTimer() { return this; }
		virtual const AsyncTimer* asAsyncTimer() const { return this; }

		/**
		 * Starting the timer.
		 */
		void start()
		{
			setValue(gstTimerRunning, true);

			m_thread = std::thread([&]()
			{
				bool bval;
				getValue(gstTimerRunning, bval);
				long lval;
				getValue(gstTimerInterval, lval);
				while (bval)
				{
					auto delta = std::chrono::steady_clock::now() + std::chrono::milliseconds(lval);
					m_func();
					std::this_thread::sleep_until(delta);
				}
			});
			m_thread.detach();
		}

		/*
		 *  Stopping the timer and destroys the thread.
		 */
		void stop()
		{
			setValue(gstTimerRunning, false);
			m_thread.~thread();
		}

		/*
		 *  Restarts the timer. Needed if you set a new
		 *  timer interval for example.
		 */
		void restart()
		{
			stop();
			start();
		}

		/*
		*  Set the method of the timer after
		*  initializing the timer instance.
		*
		*  @returns boolean is running
		*  @return  Timer reference of this
		*/
		AsyncTimer *setFunc(std::function<void(void)> func)
		{
			m_func = func;
			return this;
		}

	private:
		virtual ~AsyncTimer();

		// Function to be executed fater interval
		std::function<void(void)> m_func;
		// Thread timer is running into
		std::thread m_thread;
	};
}

#endif // _ASYNC_TIMER_H_ 
