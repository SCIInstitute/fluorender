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
#include <StopWatch.hpp>
#include <chrono>

using namespace fluo;

// Default constructor
//
StopWatch::StopWatch(unsigned int nBoxFilterSize) :
	_nStartCount(0),
	_nStopCount(0),
	_nFrequency(0),
	_nLastPeriod(0.0),
	_nSum(0.0),
	_nTotal(0.0),
	_nCount(0),
	_nBoxFilterSize(nBoxFilterSize),
	_iFilterPosition(0),
	_aIntervals(0),
	_bClockRuns(false)
{
	_nFrequency = 1000;
	// create array to store timing results
	_aIntervals = new double[_nBoxFilterSize];

	// initialize inverals with 0
	for (unsigned int iInterval = 0; iInterval < _nBoxFilterSize; ++iInterval)
		_aIntervals[iInterval] = 0.0;
}

StopWatch::StopWatch(const StopWatch& data, const CopyOp& copyop, bool copy_values) :
	_nStartCount(data._nStartCount),
	_nStopCount(data._nStopCount),
	_nFrequency(data._nFrequency),
	_nLastPeriod(data._nLastPeriod),
	_nSum(data._nSum),
	_nTotal(data._nTotal),
	_nCount(data._nCount),
	_nBoxFilterSize(data._nBoxFilterSize),
	_iFilterPosition(data._iFilterPosition),
	_aIntervals(data._aIntervals),
	_bClockRuns(data._bClockRuns)
{
	// create array to store timing results
	_aIntervals = new double[_nBoxFilterSize];

	// initialize inverals with 0
	for (unsigned int iInterval = 0; iInterval < _nBoxFilterSize; ++iInterval)
		_aIntervals[iInterval] = 0.0;
}

// Destructor
//
StopWatch::~StopWatch()
{
	stop();
	delete[] _aIntervals;
}

//
// Public methods
//

// start
//
void
StopWatch::start()
{
	if (_bClockRuns) return;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	_nStartCount = std::chrono::time_point_cast<std::chrono::milliseconds>(t1).time_since_epoch().count();
	_bClockRuns = true;
}

// stop
//
void
StopWatch::stop()
{
	if (!_bClockRuns) return;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	_nStopCount = std::chrono::time_point_cast<std::chrono::milliseconds>(t1).time_since_epoch().count();
	_nLastPeriod = static_cast<double>(_nStopCount - _nStartCount)
		/ static_cast<double>(_nFrequency);

	_nSum -= _aIntervals[_iFilterPosition];
	_nSum += _nLastPeriod;
	_aIntervals[_iFilterPosition] = _nLastPeriod;
	_iFilterPosition++;
	_iFilterPosition %= _nBoxFilterSize;
}

// sample
//
void
StopWatch::sample()
{
	if (!_bClockRuns) return;
	unsigned long long nCurrentCount;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	nCurrentCount = std::chrono::time_point_cast<std::chrono::milliseconds>(t1).time_since_epoch().count();
	_nLastPeriod = static_cast<double>(nCurrentCount - _nStartCount)
		/ static_cast<double>(_nFrequency);
	_nStartCount = nCurrentCount;

	_nSum -= _aIntervals[_iFilterPosition];
	_nSum += _nLastPeriod;
	_aIntervals[_iFilterPosition] = _nLastPeriod;
	_iFilterPosition++;
	_iFilterPosition %= _nBoxFilterSize;

	//total
	_nCount++;
	if (_nCount >= _nBoxFilterSize)
		_nTotal += _nLastPeriod;
}

// time
//
// Description:
//      Time interval in ms
//
double
StopWatch::time()
const
{
	return _nLastPeriod;
}

// average
//
// Description:
//      Average time interval of the last events in ms.
//          Box filter size determines how many events get
//      tracked. If not at least filter-size events were timed
//      the result is undetermined.
//
double
StopWatch::average()
const
{
	return _nSum / _nBoxFilterSize;
}

unsigned long long
StopWatch::count() const
{
	if (_nCount >= _nBoxFilterSize)
		return _nCount - _nBoxFilterSize;
	else
		return 0;
}

double
StopWatch::total_time() const
{
	return _nTotal;
}

double
StopWatch::total_fps() const
{
	if (_nCount >= _nBoxFilterSize)
		return static_cast<double>(_nCount) / _nTotal;
	else
		return 0;
}

unsigned long long StopWatch::sys_time()
{
	return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}

unsigned long long StopWatch::get_ticks()
{
	return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
}
