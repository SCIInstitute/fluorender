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
#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <chrono>
#include <vector>     // CHANGE

namespace fluo
{
	class StopWatch
	{
	public:
		StopWatch(unsigned int nBoxFilterSize = 1);
		StopWatch(double interval);
		const char* className() const
		{
			return "StopWatch";
		}

		void start();
		void stop();
		void sample();

		// CHANGE:
		// returns last measured interval in seconds
		double time() const;

		bool check();

		// CHANGE:
		// target interval in seconds
		void interval(double);

		double average() const;

		unsigned long long count() const;

		double total_time() const;

		double total_fps() const;

		unsigned long long sys_time();

		unsigned long long get_ticks();

	protected:
		virtual ~StopWatch();

	private:
		using Clock = std::chrono::steady_clock; // CHANGE

	private:
		// CHANGE:
		// store actual time points instead of epoch counts
		Clock::time_point _startTime;
		Clock::time_point _stopTime;

		double _nLastPeriod;
		double _nSum;
		double _nTotal;
		unsigned long long _nCount;

		unsigned int _nBoxFilterSize;
		unsigned int _iFilterPosition;

		// CHANGE:
		// replaces raw array
		std::vector<double> _aIntervals;

		// CHANGE:
		// seconds
		double _fInterval;

		// CHANGE:
		// last trigger time for check()
		double _fLastTime;

		bool _bClockRuns;
	};
}

#endif
