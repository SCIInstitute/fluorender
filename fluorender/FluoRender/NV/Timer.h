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
#endif
// includes for other OSes will go here


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
   

private:
    //
    // Private data
    //
    
#ifdef _WIN32
    LARGE_INTEGER _nStartCount;
    LARGE_INTEGER _nStopCount;
    
    LARGE_INTEGER _nFrequency;
#endif
// Data for other OSes potentially goes here

    double _nLastPeriod;
    double _nSum;
    
    unsigned int _nBoxFilterSize;
    unsigned int _iFilterPosition;
    double *     _aIntervals;
    
    bool _bClockRuns;
};





}; // end namespace nv

#endif // NV_TIMER_H 