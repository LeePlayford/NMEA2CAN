////////////////////////////////////////////////////////////////////////////
//
// Copyright(c) 2022, Chelton Ltd
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : 
//
// Originator           : Lee Playford
//
// Creation Date        : 07 December 2021
//
////////////////////////////////////////////////////////////////////////////
#ifndef WAIT_H_INCLUDED
#define WAIT_H_INCLUDED

// C Includes


// C++ Includes
#include <condition_variable>
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
using namespace std::chrono_literals;

// Includes

//------------------------------------------------
//
//------------------------------------------------
class Wait
{
public:
    /// Constructor
    /// Detail- Takes a timeout in milliseconds
    /// Returns- n/a
    /// Throws - n/a
    explicit Wait(std::chrono::milliseconds p_timeout)
    : m_timeout (p_timeout)
    , m_signalled (false)
    {
    }

    /// Constructor
    /// Detail- Takes a timeout in milliseconds
    /// Returns- n/a
    /// Throws - n/a
    explicit Wait(uint32_t p_timeout) 
    {
        Wait(std::chrono::milliseconds(p_timeout));
    }

    /// Default Constructor
    /// Detail- 
    /// Returns- n/a
    /// Throws - n/a
    Wait()
    {
        Wait(0);
    }

    /// Destructor
    /// Detail- signal the condition variable
    /// Returns- n/a
    /// Throws - n/a
    ~Wait()
    {
        m_conditionVariable.notify_one();
    }

    /// WaitFor
    /// Detail- Waits for either a signalled event or a timeout 
    /// Returns- true if the Signal was received
    /// Returns- true if the timeout was received, any for local debug
    /// Throws - n/a
    bool Wait_For ()
    {
        // The wait has already been signalled
        if (m_signalled)
        {
            // negate it and return
            ClearSignalled();
            return true;
        }
        std::unique_lock<std::mutex> l_lock (m_waitMutex);
#ifdef ZYNQ
        return m_conditionVariable.wait_for(l_lock, m_timeout) == std::cv_status::no_timeout;
#else
        // in simulator mode, return true as it will just timeout
        m_conditionVariable.wait_for(l_lock, m_timeout);
        return true;
#endif
    }

    /// WaitFor
    /// Detail- 
    /// Returns- true if the condition was met, false if the wait timed out
    /// Throws - n/a
    bool Wait_For (std::chrono::milliseconds p_timeout )
    {
        m_timeout = p_timeout;
        return Wait_For();
    }

    /// Notify
    /// Detail- Notifies the waiting condition. 
    /// Returns- n/a
    /// Throws - n/a
    void Notify()
    {
        // Set the thread as already signalled
        SetSignalled();
        std::unique_lock<std::mutex> l_lock (m_waitMutex);
        m_conditionVariable.notify_all();
    }

    void SetSignalled() { m_signalled.store (true); }
    void ClearSignalled() { m_signalled.store(false); }
    bool IsSignalled() { return m_signalled; }

private:
    std::condition_variable m_conditionVariable;    ///< Inter thread condition wait
    std::mutex m_waitMutex;                         ///< Wait mutex for the lock
    std::chrono::milliseconds m_timeout;            ///< Time out
    std::atomic<bool> m_signalled;                  ///< bool to indicate signalled

};




#endif