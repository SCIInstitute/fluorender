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

#include "Timer.h"
#include "../compatibility.h"

#include <assert.h>


//
// Namespaces
//

using namespace nv;


//
// Global consts
//

#ifdef _WIN32
const LARGE_INTEGER gcLargeIntZero = {{0, 0}};
#endif
// ----------------------------------------------------------------------------
// Timer class
//

// 
// Construction and destruction
//

// Default constructor
//
Timer::Timer(unsigned int nBoxFilterSize) :
#ifdef _WIN32
  _nStartCount(gcLargeIntZero)
, _nStopCount(gcLargeIntZero)
, _nFrequency(gcLargeIntZero) ,
#endif
  _nLastPeriod(0.0)
, _nSum(0.0)
, _nBoxFilterSize(nBoxFilterSize)
, _iFilterPosition(0)
, _aIntervals(0)
, _bClockRuns(false)
{
#ifdef _WIN32
	QueryPerformanceFrequency(&_nFrequency);
#endif
	// create array to store timing results
	_aIntervals = new double[_nBoxFilterSize];

	// initialize inverals with 0
	for (unsigned int iInterval = 0; iInterval < _nBoxFilterSize; ++iInterval)
		_aIntervals[iInterval] = 0.0;
}

// Destructor
//
Timer::~Timer()
{
	delete[] _aIntervals;
}

//
// Public methods
//

// start
//
void
Timer::start()
{
	if (_bClockRuns) return;

#ifdef _WIN32
	QueryPerformanceCounter(&_nStartCount);
#endif
	_bClockRuns = true;
}

// stop
//
void
Timer::stop()
{
	if (!_bClockRuns) return;

#ifdef _WIN32
	QueryPerformanceCounter(&_nStopCount);
	_nLastPeriod = static_cast<double>(_nStopCount.QuadPart - _nStartCount.QuadPart) 
		/ static_cast<double>(_nFrequency.QuadPart);
#endif
	_nSum -= _aIntervals[_iFilterPosition];
	_nSum += _nLastPeriod;
	_aIntervals[_iFilterPosition] = _nLastPeriod;
	_iFilterPosition++;
	_iFilterPosition %= _nBoxFilterSize;
}

// sample
//
void
Timer::sample()
{
	if (!_bClockRuns) return;

#ifdef _WIN32
	LARGE_INTEGER nCurrentCount;
	QueryPerformanceCounter(&nCurrentCount);
	_nLastPeriod = static_cast<double>(nCurrentCount.QuadPart - _nStartCount.QuadPart) 
		/ static_cast<double>(_nFrequency.QuadPart);
	_nStartCount = nCurrentCount;
#endif
	_nSum -= _aIntervals[_iFilterPosition];
	_nSum += _nLastPeriod;
	_aIntervals[_iFilterPosition] = _nLastPeriod;
	_iFilterPosition++;
	_iFilterPosition %= _nBoxFilterSize;
}

// time
//
// Description:
//      Time interval in ms
//
double
Timer::time()
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
Timer::average()
const
{
	return _nSum/_nBoxFilterSize;
}



