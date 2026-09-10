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
#include <StopWatch.hpp>

#include <chrono>

using namespace fluo;

//--------------------------------------------------
// Constructor
//--------------------------------------------------

StopWatch::StopWatch(unsigned int nBoxFilterSize) :
	_nLastPeriod(0.0),
	_nSum(0.0),
	_nTotal(0.0),
	_nCount(0),
	_nBoxFilterSize(nBoxFilterSize),
	_iFilterPosition(0),
	_aIntervals(nBoxFilterSize, 0.0), // CHANGE
	_fInterval(0.1),                  // CHANGE: 100 ms = 0.1 sec
	_fLastTime(0.0),                  // CHANGE
	_bClockRuns(false)
{
}

StopWatch::StopWatch(double interval) :
	StopWatch()
{
	_fInterval = interval;
}

//--------------------------------------------------
// Destructor
//--------------------------------------------------

StopWatch::~StopWatch()
{
	stop();
}

//--------------------------------------------------
// Start
//--------------------------------------------------

void StopWatch::start()
{
	if (_bClockRuns)
		return;

	_startTime = Clock::now();

	_bClockRuns = true;
	_fLastTime = 0.0;
}

//--------------------------------------------------
// Stop
//--------------------------------------------------

void StopWatch::stop()
{
	if (!_bClockRuns)
		return;

	_stopTime = Clock::now();

	_nLastPeriod =
		std::chrono::duration<double>(
			_stopTime - _startTime).count();

	_nSum -= _aIntervals[_iFilterPosition];
	_nSum += _nLastPeriod;

	_aIntervals[_iFilterPosition] = _nLastPeriod;

	_iFilterPosition++;
	_iFilterPosition %= _nBoxFilterSize;

	_bClockRuns = false; // CHANGE
}

//--------------------------------------------------
// Sample
//--------------------------------------------------

void StopWatch::sample()
{
	if (!_bClockRuns)
		return;

	auto current = Clock::now();

	_nLastPeriod =
		std::chrono::duration<double>(
			current - _startTime).count();

	_startTime = current;

	_nSum -= _aIntervals[_iFilterPosition];
	_nSum += _nLastPeriod;

	_aIntervals[_iFilterPosition] = _nLastPeriod;

	_iFilterPosition++;
	_iFilterPosition %= _nBoxFilterSize;

	_nCount++;

	if (_nCount >= _nBoxFilterSize)
		_nTotal += _nLastPeriod;
}

//--------------------------------------------------
// Last interval in seconds
//--------------------------------------------------

double StopWatch::time() const
{
	// CHANGE:
	// much more intuitive
	return _nLastPeriod;
}

//--------------------------------------------------
// Interval check
//--------------------------------------------------

bool StopWatch::check()
{
	if (!_bClockRuns)
		return false;

	double cur_time =
		std::chrono::duration<double>(
			Clock::now() - _startTime).count();

	if (cur_time - _fLastTime >= _fInterval)
	{
		_fLastTime = cur_time;
		return true;
	}

	return false;
}

//--------------------------------------------------

void StopWatch::interval(double val)
{
	_fInterval = val;
}

//--------------------------------------------------

double StopWatch::average() const
{
	return _nBoxFilterSize ?
		_nSum / _nBoxFilterSize : 0.0;
}

//--------------------------------------------------

unsigned long long StopWatch::count() const
{
	if (_nCount >= _nBoxFilterSize)
		return _nCount - _nBoxFilterSize;

	return 0;
}

//--------------------------------------------------

double StopWatch::total_time() const
{
	return _nTotal;
}

//--------------------------------------------------

double StopWatch::total_fps() const
{
	if (_nCount < _nBoxFilterSize || _nTotal <= 0.0)
		return 0.0;

	return static_cast<double>(_nCount) / _nTotal;
}

//--------------------------------------------------

unsigned long long StopWatch::sys_time()
{
	return static_cast<unsigned long long>(
		std::chrono::time_point_cast<std::chrono::seconds>(
			std::chrono::system_clock::now())
		.time_since_epoch().count());
}

//--------------------------------------------------

unsigned long long StopWatch::get_ticks()
{
	return static_cast<unsigned long long>(
		std::chrono::time_point_cast<std::chrono::milliseconds>(
			Clock::now())
		.time_since_epoch().count());
}