////////////////////////////////////////////////////////////////////////////
// 
// Copyright(c) 2022, Chelton Ltd. 
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Thread base class
//
// Originator           : Lee Playford
//
// Creation Date        : 4 May 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef _ITHREAD_H_INCLUDED_
#define _ITHREAD_H_INCLUDED_

// C includes

// C++ includes
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

// includes
#include "Handlers/Wait.h"

//-------------------------------------
// typedefs
//-------------------------------------
typedef std::lock_guard<std::mutex> Lock;
typedef std::unique_lock<std::mutex> TryLock;


//-------------------------------------
// Interface class to ensure that
// the thread can be shutdown
//-------------------------------------
class IThread
{
public:
    /// Default Constructor
    /// Detail- Base class constructor
    /// Returns- n/a
    /// Throws - n/a
    IThread() 
    : m_threadRunning(false) {}
    
    /// Default Destructor
    /// Detail- virtual base class destructor
    /// Returns- n/a
    /// Throws - n/a
    virtual ~IThread() {};

    /// StartThread
    /// Detail- Pure Virtual Function used to start the thread
    /// Returns- true if the thread was started
    /// Throws - n/a
    virtual bool StartThread () = 0;

    /// StopThread
    /// Detail- Pure Virtual Function used to start the thread
    /// Returns- true if the thread was started
    /// Throws - n/a
    virtual void StopThread() = 0;

protected:
    bool m_threadRunning;	            ///< Thread close flag
    std::thread m_threadHandle;         ///< Thread handle
    Wait m_interruptTimeOut;            ///< Time out object for inter thread comms
};


#endif

