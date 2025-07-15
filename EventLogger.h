////////////////////////////////////////////////////////////////////////////
// 
// Copyright(c) 2021, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Event Logger class
//
// Originator           : Lee Playford
//
// Creation Date        : 4 May 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef _EVENT_LOGGER_INCLUDED_
#define _EVENT_LOGGER_INCLUDED_

// C Includes

// C++ Includes
#include <fstream>
#include <iostream>
#include <string>
#include <mutex>

// Includes

//-------------------------------------
//
//-------------------------------------
namespace
{
    const int16_t cMaxEventMsgSize = 1024 - 32;
    const int16_t cMaxErrorMsgSize = 1024;
    const int16_t cBufferSize = 256;
}

enum  class eLogLevel
{
    None,
    Normal,
    Debug
};

//---------------------------------------------------
// class logs event from the processor to a log file
// this will be used during development and debugging
//---------------------------------------------------
class EventLogger
{
public:

    /// Constructor
    /// Detail- Default constructor
    /// Returns- Nothing
    /// Throws - n/a
    EventLogger();

    /// Default Destructor 
    ///- Details:
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    ~EventLogger();

    /// GetInstance
    ///- Details: Class is a singleton class , this method gets access to the instance
    ///
    ///- Returns:   Instance of the ErrorLogger
    ///- Throws:    n/a
    static EventLogger* GetInstance();

    /// DestroyInstance
    ///- Details: Class is a singleton class , destroys the instance
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    static void DestroyInstance() 
    { 
        if (s_pInstance != nullptr) 
        delete s_pInstance; 
    }

    /// LogEvent
    ///- Details: Standard Log event message
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    static void LogEvent
    (
        const char * p_eventData,   ///< Event Data
        ...                         ///< vargs
    );

    /// Debug
    ///- Details:   Logs a debug message if the level is set to debug
    ///             Also writes to standard out
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    static void Debug
    (
        const char * p_format,  ///< Debug Data
        ...                     ///< vargs
    );

    /// Error
    ///- Details:   Logs a Error message to the log file
    ///             add the errno() if available
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    static void Error
    (
        const char* p_format,   ///< the error message
        ...                     ///< vargs
    );

    /// SetLogLevel 
    ///- Details:   Sets the logging level
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    void SetLogLevel
    (
        eLogLevel p_level   ///< the logging level
    );

private:

    // Open the log file
    void OpenLogFile();

    // Write Log File
    void WriteLogFile(const char * p_eventMsg);

    // Close the log file
    void CloseLogFile();

    // Log file date and time
    std::string GetEventLogTimeString();
    std::string GetFileDateString();

    static EventLogger* s_pInstance;        ///!< instance of the log as a singleton
    static bool m_bVerbose;                 ///!< its in verbose mode
    std::fstream m_hLogFile;                ///!< the log file
    bool m_LogDebugData;                    ///!< Logs debug data
    char m_lastMessage[cMaxEventMsgSize];   ///!< the last message, stop duplicate messages flooding the event log
    int  m_lenLast;                         ///!< length of the last message
    std::string m_logDirectory;             ///!< Directory to log event files
    std::mutex m_lock;			            ///!< log file access lock
};

#endif