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
#include "UDPSender.h"

// C includes
#include <stdlib.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#else
#include <WS2tcpip.h>
#endif

// C++ Includes
#include <string>

// Includes
#include "../EventLogger.h"
#include "../Utils.h"
#include "../Config.h"

//----------------------------------------------------------------
//
//----------------------------------------------------------------
UDPSender::UDPSender(uint16_t p_networkPort , const std::string& p_rIpAddress, const std::string& p_rAdaptorAddress, bool p_broadcastEnable)
    : m_socket(SOCKET_ERROR) 
    , m_port(p_networkPort)
    , m_ipAddress(p_rIpAddress)
    , m_adaptorAddr(p_rAdaptorAddress)
    , m_broadcastEnable(p_broadcastEnable)
    , m_initOk(false)
{
    m_initOk = InitSocket();
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
UDPSender::~UDPSender()
{
    Close();
#ifdef __linux__
    shutdown(m_socket, SHUT_RDWR);
#else
    shutdown(m_socket, SD_BOTH);
#endif
    m_socket = SOCKET_ERROR;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
bool UDPSender::InitSocket()
{
    bool l_success = true;
    /* create what looks like an ordinary UDP socket */
    if (m_port == 0 || m_adaptorAddr.empty())
    {
        l_success = false;
        m_socket = SOCKET_ERROR;
    }
    if (l_success && (m_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        EventLogger::Error("UDPSender Init() socket open error");
        l_success = false;
        m_socket = SOCKET_ERROR;
    }
    else
    {
        /* set up destination address */
        memset(&m_addr, 0, sizeof(sockaddr_in));
        m_addr.sin_family = AF_INET;
        if (m_adaptorAddr.empty())
        {
            m_addr.sin_addr.s_addr = htonl (INADDR_ANY);
        }
        else
        {
            m_addr.sin_addr.s_addr = inet_addr(m_adaptorAddr.c_str());
        }

        if (bind(m_socket, (struct sockaddr*)&m_addr, sizeof(m_addr)) < 0)
        {
            EventLogger::Error("UDPSender() Unable to bind to address %s", m_adaptorAddr.c_str());
            l_success = false;
            Close();
            m_socket = SOCKET_ERROR;
        }

        if (l_success)
        {
            // if we want to broadcast
            if (m_broadcastEnable)
            {
                AdaptorDetails l_adaptorDetails = Utils::GetIpAddress (cDEFAULT_ADAPTOR);
                std::string l_broadcastAddress = l_adaptorDetails.m_broadcastAddress; 
                if (!l_broadcastAddress.empty())
                {
#ifdef __linux__
                    int l_broadcastEnable = 1;
#else
                    const char l_broadcastEnable = 1;
#endif
                    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &l_broadcastEnable, sizeof(l_broadcastEnable)) == SOCKET_ERROR)
                    {
                        EventLogger::Error("Error setting UDPSender() in broadcast mode");
                        l_success = false;
                    }
                    // need to calculate the network broadcast address
                    inet_pton(AF_INET, l_broadcastAddress.c_str(), &m_addr.sin_addr.s_addr);
                }
            }
            else
            {
                //use the given address
                inet_pton(AF_INET, m_ipAddress.c_str(), &m_addr.sin_addr.s_addr);
            }
            m_addr.sin_port = htons(m_port);
        }
    }

    if (l_success)
    {
        EventLogger::LogEvent("UDPSender() Opened Sender on %s:%d", m_adaptorAddr.c_str() , m_port);
    }
    else
    {
        EventLogger::LogEvent("UDPSender() Failed to open socket on %s" , m_adaptorAddr.c_str());
    }

    return l_success;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
int16_t UDPSender::SendData(const char * p_pBuffer , uint16_t p_bufferLen, int p_socket)
{
    static_cast<void> (p_socket);

    int16_t l_bytesSent (-1);
    
    if (p_pBuffer != nullptr && m_initOk && m_socket != SOCKET_ERROR)
    {
        Lock l_lock(m_sendLock);

        l_bytesSent = sendto(m_socket, p_pBuffer, p_bufferLen, 0, (struct sockaddr*) & m_addr, sizeof(m_addr));
        if ( l_bytesSent < 0)
        {
            EventLogger::Error("UDPSender::SendData() failed" );
        }
    }
    return l_bytesSent;
}



