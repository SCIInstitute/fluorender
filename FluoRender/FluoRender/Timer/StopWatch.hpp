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
#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <Flobject/Node.hpp>

#define gstStopWatch "default stop watch"

namespace fluo
{
	class StopWatch : public Node
	{
	public:
		// 
		// Construction and destruction
		//

		// Default constructor
		//
		StopWatch(unsigned int nBoxFilterSize = 1);
		StopWatch(double interval);
		StopWatch(const StopWatch& data, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new StopWatch(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const StopWatch*>(obj) != NULL;
		}

		virtual const char* className() const { return "StopWatch"; }

		virtual StopWatch* asStopWatch() { return this; }
		virtual const StopWatch* asStopWatch() const { return this; }

		//
		// Public methods
		//

		// start
		//
		void start();

		// stop
		//
		void stop();

		// sample
		//
		void sample();

		// time
		//
		// Description:
		//      Time interval in ms
		//
		double time() const;

		bool check();

		// average
		//
		// Description:
		//      Average time interval of the last events in ms.
		//          Box filter size determines how many events get
		//      tracked. If not at least filter-size events were timed
		//      the result is undetermined.
		//
		double
			average()
			const;

		unsigned long long
			count() const;

		double
			total_time() const;

		double
			total_fps() const;

		unsigned long long sys_time();

		unsigned long long get_ticks();

	protected:
		// Destructor
		//
		virtual ~StopWatch();

	private:
		//
		// Private data
		//

		unsigned long long _nStartCount;
		unsigned long long _nStopCount;
		unsigned long long _nFrequency;

		double _nLastPeriod;
		double _nSum;
		double _nTotal;
		unsigned long long _nCount;

		unsigned int _nBoxFilterSize;
		unsigned int _iFilterPosition;
		double *     _aIntervals;
		double _fInterval;//target interval
		double _fLastTime;

		bool _bClockRuns;
	};
}
#endif // _STOPWATCH_H_ 
