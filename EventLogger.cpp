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
#include "EventLogger.h"

// C includes
#include <string.h>
#include <stdarg.h>
#include <time.h>	
#include <sys/stat.h>
#include <errno.h>

#ifdef __linux__
#include <unistd.h>
#else
#include <direct.h>
#endif

// C++ includes

// includes
#include "IThread.h"

namespace
{
    const char * cEventLogDirectory = "logs";
    const char * cLogFileNamePrefix = "EventLog";
    const char * cLogFileNameSuffix = ".log";
}

EventLogger* EventLogger::s_pInstance = nullptr;
bool EventLogger::m_bVerbose = (false);
//----------------------------------------------------------------
//
//----------------------------------------------------------------
EventLogger::EventLogger()
    : m_LogDebugData(false)
    , m_lenLast(0)
{
    memset(m_lastMessage, 0x0, cMaxEventMsgSize);

    // Ensure the application path exists
    char l_temp[cBufferSize];

#ifdef __linux__
    // Create the Directory Name
    std::string logDirectory;
    char * l_pHome = getenv("HOME");
    if (logDirectory.empty())
    {
        snprintf(l_temp, cBufferSize, "%s/%s", l_pHome, cEventLogDirectory);
    }
    else
    {
        snprintf(l_temp, cBufferSize, "%s/%s", l_pHome, logDirectory.c_str());

    }
    m_logDirectory = l_temp;
    mkdir(l_temp, 0777);
#else
    // Windows version
    snprintf(l_temp, cBufferSize, ".\\%s", cEventLogDirectory);
    m_logDirectory = l_temp;
    (void) _mkdir(l_temp);
#endif
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
EventLogger::~EventLogger()
{
    WriteLogFile("Closing LogFile");
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
EventLogger* EventLogger::GetInstance()
{
    if (s_pInstance == nullptr)
    {
        s_pInstance = new (std::nothrow) EventLogger();
    }
    return s_pInstance;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
void EventLogger::LogEvent(const char * p_format, ...)
{
    va_list  l_args;
    va_start(l_args, p_format);
    char l_message[cMaxEventMsgSize];
    vsnprintf(l_message, cMaxEventMsgSize, p_format, l_args);
    va_end(l_args);

    // Write to the log file
    GetInstance()->WriteLogFile(l_message);

    if (GetInstance()->m_LogDebugData)
    {
    	std::cout << l_message << std::endl;
    }
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
void EventLogger::Debug(const char* p_format, ...)
{
    if (GetInstance()->m_LogDebugData)
    {
        va_list  l_args;
        va_start(l_args, p_format);
        char l_message[cMaxEventMsgSize] = { 0 };
        vsnprintf(l_message, cMaxEventMsgSize, p_format, l_args);
        va_end(l_args);
        std::cout << l_message << std::endl;

        char l_errorMessage[cMaxErrorMsgSize] = { 0 };
        snprintf(l_errorMessage, cMaxErrorMsgSize, "#Debug# %s", l_message);

        GetInstance()->WriteLogFile(l_errorMessage);
    }
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
void EventLogger::Error(const char* p_format, ...)
{
    va_list  l_args;
    va_start(l_args, p_format);
    char l_message[cMaxEventMsgSize] = { 0 };
    vsnprintf(l_message, cMaxEventMsgSize, p_format, l_args);
    va_end(l_args);

    char l_errorMessage[cMaxErrorMsgSize] = { 0 };
#ifdef __linux__
    snprintf(l_errorMessage, cMaxErrorMsgSize, "*Error* %s %s", l_message, errno > 0 ? strerror(errno) : "");
#else
    snprintf(l_errorMessage, cMaxEventMsgSize, "*Error* %s", l_message);
#endif

    std::cout << l_errorMessage << std::endl;
    GetInstance()->WriteLogFile(l_errorMessage);
}




//----------------------------------------------------------------
//
//----------------------------------------------------------------
void EventLogger::SetLogLevel(eLogLevel p_level)
{
    if (p_level == eLogLevel::Debug)
    {
        GetInstance()->m_LogDebugData = true;
    }
    else
    {
        GetInstance()->m_LogDebugData = false;
    }
}



//--------------------------------------------
// Private Methods
//--------------------------------------------

/// OpenLogFile
///- Details:   Opens the log file on disk
///
///- Returns:   n/a
///- Throws:    n/a
void EventLogger::OpenLogFile()
{
    char l_logFileName[cBufferSize] = { 0 };

#ifdef __linux__
    snprintf(l_logFileName, cBufferSize, "%s/%s_%s%s"
        , m_logDirectory.c_str()
        , cLogFileNamePrefix
        , GetFileDateString().c_str()
        , cLogFileNameSuffix);
#else
    snprintf(l_logFileName, cBufferSize, "%s\\%s_%s%s"
        , m_logDirectory.c_str()
        , cLogFileNamePrefix
        , GetFileDateString().c_str()
        , cLogFileNameSuffix);

#endif

    m_hLogFile.open(l_logFileName, std::ios::out | std::ios::app);
    if (!m_hLogFile.is_open())
    {
        perror("Unable to open Log File");
    }
}


///  WriteLogFile
///- Details:   Writes the string in to the log file
///
///- Returns:   n/a
///- Throws:    n/a
void EventLogger::WriteLogFile
(
    const char * p_eventMsg ///!< message to write to disk
)
{
    // make sure two threads aren't writing at the same time
    Lock l_lock(m_lock);

    int l_length = static_cast<int> (strlen(p_eventMsg));

    // avoid writing the same message twice
    if (l_length != m_lenLast && memcmp(p_eventMsg, m_lastMessage, l_length) != 0 )
    {
        // copy the last l_message
        snprintf(m_lastMessage, sizeof(m_lastMessage), "%s", p_eventMsg);
        m_lenLast = l_length;

        char l_logMessage[cMaxEventMsgSize] = { 0 };
        l_length = snprintf(l_logMessage, cMaxEventMsgSize, "[%s] %s", GetEventLogTimeString().c_str(), p_eventMsg);

        // add a cr-lf if not there
        if (l_logMessage[l_length - 1] != '\n')
        {
            l_logMessage[l_length] = '\n';
        }

        // Open the Log File
        OpenLogFile();

        // if the log file is open
        if (m_hLogFile.is_open())
        {
            m_hLogFile << l_logMessage;
            m_hLogFile.flush();
        }

        // Keep the log file closed
        CloseLogFile();
    }
}

///  CloseLogFile
///- Details:   Closes the log file
///
///- Returns:   n/a
///- Throws:    n/a
void EventLogger::CloseLogFile()
{
    if (m_hLogFile.is_open())
    {
        m_hLogFile.close();
    }
}

///  GetEventLogTimeString
///- Details:   Returns a timestamp to log in the file
///
///- Returns:   std::string with the time tamp
///- Throws:    n/a
std::string EventLogger::GetEventLogTimeString()
{
    int64_t l_secondsEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    int l_mseconds = static_cast<int>  ((l_secondsEpoch) % 1000);
    l_secondsEpoch /= 1000;
    int l_seconds = static_cast<int>  ((l_secondsEpoch) % 60);
    int l_minutes = static_cast<int>  ((l_secondsEpoch / 60) % 60);
    int l_hours = static_cast<int>    ((l_secondsEpoch / 3600) % 24);
    char l_buffer[64];
    snprintf(l_buffer, sizeof(l_buffer), "%02d:%02d:%02d:%03d", l_hours, l_minutes, l_seconds, l_mseconds);
    std::string l_result = l_buffer;
    return l_result;
}

///  GetFileDateString
///- Details:   Used when creating Disk Files
///             in the format DD_MM_YYYY
///
///- Returns:   std::string with the date stamp
///- Throws:    n/a
std::string EventLogger::GetFileDateString()
{
    time_t l_rawtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
#ifdef __linux__
    // need to fix this for linux
    struct tm l_tmInfo;
    gmtime_r(&l_rawtime, &l_tmInfo);
#endif

#ifdef _WIN32
    struct tm l_tmInfo;
    gmtime_s(&l_tmInfo , &l_rawtime );
#endif

    char l_buffer[32];
    snprintf(l_buffer, sizeof(l_buffer), "%02d_%02d_%04d"
        , l_tmInfo.tm_mday
        , l_tmInfo.tm_mon + 1
        , l_tmInfo.tm_year + 1900);
    std::string l_result = l_buffer;
    return l_result;
}
