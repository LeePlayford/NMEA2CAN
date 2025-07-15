////////////////////////////////////////////////////////////////////////////
//
// Copyright(c) 2021, Chelton Ltd. 
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : UDP Reader Class implementation file
//
// Originator           : Lee Playford
//
// Creation Date        : 4 May 2020
//
////////////////////////////////////////////////////////////////////////////
// File Description
// network class implementation file
#include "UDPReader.h"

// C includes
#include <stdlib.h>
#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// C++ includes
#include <thread>
#include <string>

// includes
#include "../EventLogger.h"
#include "../Handlers/MessageHandler.h"
#include "../Config.h"

namespace
{
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
    const int kRxBufferSize = 1500;
#define UDP_READER_THREAD_NAME "UDPRx  "
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
UDPReader::UDPReader(
    IMessageHandlerInterface *p_pMessageHandler,
    uint16_t p_port,
    const std::string &p_rAdaptorAddress,
    const std::string &p_rSenderAddress,
    bool l_broadcastEnable)
    : m_socket(SOCKET_ERROR), m_pMsgHandler(p_pMessageHandler), m_port(p_port), m_adaptorAddr(p_rAdaptorAddress), m_senderAddr(p_rSenderAddress), m_broadcastEnable(l_broadcastEnable), m_initOk(false)

{
    m_threadRunning = false;
    if (InitSocket() && p_pMessageHandler != nullptr)
    {
        m_threadRunning = StartThread();
    }
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
UDPReader::~UDPReader()
{
    if (m_socket != SOCKET_ERROR)
    {
        Close();
#ifdef __linux__
        shutdown(m_socket, SHUT_RDWR);
#else
        shutdown(m_socket, SD_BOTH);
#endif
        m_socket = SOCKET_ERROR;
    }
    StopThread();
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
bool UDPReader::InitSocket()
{
    bool l_success = true;
    // create an ordinary UDP socket
    if ((m_socket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
    {
        EventLogger::Error("Error opening UDP Reader socket");
        l_success = false;
    }

#ifdef __linux__
    int l_dummy = 1;
#else
    const char l_dummy(1);
#endif

    if (l_success && setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &l_dummy, sizeof(l_dummy)) == SOCKET_ERROR)
    {
        EventLogger::Error("Error setting UDP Reader socket options");
        l_success = false;
    }

    if (l_success && m_broadcastEnable)
    {
        // if we want to broadcast
        int l_broadcastEnable = 1;
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&l_broadcastEnable), sizeof(l_broadcastEnable)) == SOCKET_ERROR)
        {
            EventLogger::Error("Error setting UDP Reader in broadcast mode");
            l_success = false;
        }
    }

    // set up destination address
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(m_adaptorAddr.c_str());
    m_addr.sin_port = htons(m_port);

    // bind to the address
    if (l_success && bind(m_socket, (struct sockaddr *)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
    {
        EventLogger::Error("UDPReader() bind error");
        l_success = false;
    }

    if (l_success)
    {
        EventLogger::LogEvent("UDPReader() Opened Reader on %s:%d", m_adaptorAddr.c_str(), m_port);
    }
    else
    {
        EventLogger::LogEvent("UDPReader() Failed to open socket on %s", m_adaptorAddr.c_str());
    }
    m_initOk = l_success;
    return m_initOk;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
bool UDPReader::StartThread()
{
    bool l_success(false);
    // start the reader thread
    m_threadHandle = std::thread([=]
                                 { ReaderThread(); });
    if (m_threadHandle.joinable())
    {
        l_success = true;
    }
    return l_success;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
void UDPReader::StopThread()
{
    // stop the Socket reader thread
    if (m_threadRunning)
    {
        m_threadRunning = false;
        if (m_threadHandle.joinable())
        {
            m_threadHandle.join();
        }
    }
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
int16_t UDPReader::Read(void *p_pBuffer, uint16_t p_BufSize, timeval *p_pTimeVal)
{
    // set up a read descriptor
    fd_set l_rfds;
    FD_ZERO(&l_rfds);
    FD_SET(m_socket, &l_rfds);

    // set up a timeout

#ifdef __linux__
    struct timeval l_tv = {1, 0};
#else
    // default to 1 second
    struct timeval l_tv;
    l_tv.tv_sec = 1;
    l_tv.tv_usec = 0;
#endif

    if (p_pTimeVal != nullptr)
    {
        l_tv.tv_sec = p_pTimeVal->tv_sec;
        l_tv.tv_usec = p_pTimeVal->tv_usec;
    }
    else
    {
        // default to 1 second
        l_tv.tv_sec = 1;
        l_tv.tv_usec = 0;
    }

    int16_t l_dataSize = 0;

    // wait for data or a timeout
    int l_retval = select(m_socket + 1, &l_rfds, nullptr, nullptr, &l_tv);
    if (l_retval > 0 && FD_ISSET(m_socket, &l_rfds))
    {
        struct sockaddr_in l_addr;
        socklen_t l_addrlen = sizeof(l_addr);

        l_dataSize = recvfrom(m_socket, reinterpret_cast<char *>(p_pBuffer), p_BufSize, 0, reinterpret_cast<struct sockaddr *>(&l_addr), &l_addrlen);
        if (l_dataSize == static_cast<int16_t>(SOCKET_ERROR))
        {
            EventLogger::Error("UDPReader:recvfrom returned socket error");
        }
    }
    return l_dataSize;
}

//----------------------------------------------------------------
// Private Methods
//----------------------------------------------------------------

/// UDP Reader Thread
///- Details:   Reads the UDP link and passes any sentences to the
///             message handler
///
///- Returns:   n/a
///- Throws:    n/a
void UDPReader::ReaderThread()
{
    EventLogger::Debug("#%s Thread Started", UDP_READER_THREAD_NAME);
// Set the thread name for system debugging
#ifdef __linux__
    prctl(PR_SET_NAME, UDP_READER_THREAD_NAME, 0, 0, 0);
#endif

    // struct for received data
    struct sockaddr_in l_addr;
    socklen_t l_addrlen = sizeof(l_addr);

    // vars for the select
    fd_set l_rfds;
    struct timeval l_tv;

    FD_ZERO(&l_rfds);
    FD_SET(m_socket, &l_rfds);

    // large enough received buffer
    uint8_t l_receiveBuffer[kRxBufferSize];

    // get the senders address if set
    uint32_t l_senderAddress = 0;
    if (!m_senderAddr.empty())
    {
        l_senderAddress = inet_addr(m_senderAddr.c_str());
    }

    // get the adaptor address - used to ignore own messages
    /*uint32_t l_AdaptorAddress;
    if (!m_adaptorAddr.empty())
    {
        l_AdaptorAddress = inet_addr(m_adaptorAddr.c_str());
    }*/

    // run until the thread has been told to exit
    while (m_threadRunning)
    {
        l_tv.tv_sec = 1;
        l_tv.tv_usec = 0;
        FD_SET(m_socket, &l_rfds);

        // wait for data or a timeout
        int l_retval = select(m_socket + 1, &l_rfds, nullptr, nullptr, &l_tv);
        if (l_retval < 0)
        {
            EventLogger::Error("UDP Reader Select failure");
        }
        else if (l_retval > 0 && FD_ISSET(m_socket, &l_rfds))
        {
            // Data received
            int l_dataSize = recvfrom(m_socket, reinterpret_cast<char *>(l_receiveBuffer), kRxBufferSize, 0, reinterpret_cast<struct sockaddr *>(&l_addr), &l_addrlen);
            if (l_dataSize == SOCKET_ERROR)
            {
                EventLogger::Error("UDPReader thread():recvfrom returned socket error");
            }
            else if (l_dataSize > 0)
            {
                if (l_senderAddress != 0 && l_senderAddress != l_addr.sin_addr.s_addr)
                {
                    //continue;
                }

                // If the data has been loopback, ignore it
                /*if (p_rAdaptorAddress != 0 && p_rAdaptorAddress == l_addr.sin_addr.s_addr)
                {
                    continue;
                }*/
                l_receiveBuffer[l_dataSize] = 0;
                if (m_pMsgHandler != nullptr && m_threadRunning)
                {
                    uint16_t l_handledSize = l_dataSize;
                    m_pMsgHandler->HandleMessage(l_receiveBuffer, l_handledSize);
                }
            }
        }
    }

    EventLogger::Debug("$%s Thread Exit", UDP_READER_THREAD_NAME);
    return;
}
