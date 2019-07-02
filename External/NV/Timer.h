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
#ifndef NV_TIMER_H
#define NV_TIMER_H
// ----------------------------------------------------------------------------
// 
// Content:
//      Timer class
//
// Description:
//      A high precision timer with built in filtering (averaging) 
//      capabilities.
//
// Author: Frank Jargstorff (03/08/04)
//
// Note:
//      Copyright (C) 2004 NVIDIA Crop. 
//
// ----------------------------------------------------------------------------


//
// Includes
//

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>

// includes for other OSes will go here
#else
#include <time.h>
#include <sys/time.h>
#endif

//
// Namespace
//

namespace nv {


	// ----------------------------------------------------------------------------
	// Timer class
	//
	class Timer
	{
	public:
		// 
		// Construction and destruction
		//

		// Default constructor
		//
		Timer(unsigned int nBoxFilterSize = 1);

		// Destructor
		//
		~Timer();


		//
		// Public methods
		//

		// start
		//
		void
			start();

		// stop
		//
		void
			stop();

		// sample
		//
		void
			sample();

		// time
		//
		// Description:
		//      Time interval in ms
		//
		double
			time()
			const;

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

	private:
		//
		// Private data
		//

#ifdef _WIN32
		LARGE_INTEGER _nStartCount;
		LARGE_INTEGER _nStopCount;
		LARGE_INTEGER _nFrequency;
		// Data for other OSes potentially goes here
#else
		unsigned long long _nStartCount;
		unsigned long long _nStopCount;
		unsigned long long _nFrequency;
#endif

		double _nLastPeriod;
		double _nSum;
		double _nTotal;
		unsigned long long _nCount;

		unsigned int _nBoxFilterSize;
		unsigned int _iFilterPosition;
		double *     _aIntervals;

		bool _bClockRuns;
	};
}; // end namespace nv

#endif // NV_TIMER_H 
