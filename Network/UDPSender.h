////////////////////////////////////////////////////////////////////////////
// 
// Copyright(c) 2021, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : UDP Sender header file
//
// Originator           : Lee Playford
//
// Creation Date        : 30 July 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef UDP_SENDER_H_INCLUDED
#define UDP_SENDER_H_INCLUDED

// C Includes
#ifdef __linux__
#include <netinet/in.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <stdint.h>

// C++ Includes
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

// Includes

#include "../IThread.h"
#include "../Network/INetwork.h"

// File Description
// network class implementation file

namespace
{
    const uint16_t cMAX_MESSAGE_SIZE = 1024;

#ifndef SOCKET_ERROR
    const int SOCKET_ERROR = -1;
#endif
}

//----------------------------------------------
// Class creates a UDP socket and writes data through
// the socket onto the network
//----------------------------------------------
class UDPSender : public INetwork
{
public:
    /// Default Constructor
    /// Detail- UDP Sender constructor
    /// Returns- n/a
    /// Throws - n/a
    explicit UDPSender
    (
        uint16_t p_networkPort ,            ///< UDP Port to send data to 
        const std::string& p_rIpAddress,    ///< IP Address to send data to 
        const std::string& p_rAdaptorAddress, ///< The adaptor to use
        bool p_broadcastEnable);  ///< adaptor to send the data through

    /// Default Destructor
    /// Detail- UDP Sender destructor
    /// Returns- n/a
    /// Throws - n/a
    ~UDPSender();

    /// Default InitSocket
    /// Detail- Initialise and open the UDP Sender socket
    /// Returns- true if the socket was opened and configured
    /// Throws - n/a
    bool InitSocket();

    /// Read
    ///- Details:   Send the UDP data to the Network
    ///             Overides sendData in INetwork
    ///
    ///- Returns:   number of bytes sent to the UDP socket
    ///- Throws:    n/a
    virtual int16_t SendData
    (
        const char* p_pBuffer,  ///< buffer to containing the data to send
        uint16_t p_bufferLen,   ///< length of the buffer
        int p_socket = -1       ///< socket to send to
    ) override;

    /// Close the socket
    /// Detail- Closes the open UDP socket
    /// Returns- true if the socket was opened and configured
    /// Throws - n/a
    void Close()
    {
#ifdef __linux__
        close(m_socket);
#else
        closesocket(m_socket);
#endif
    }


private:
    struct sockaddr_in m_addr;	///!< Endpoint address
    int m_socket;				///!< Send socket
    uint16_t    m_port;         ///!< the send UDP Port
    std::string m_ipAddress;    ///!< the send IP Address
    std::string m_adaptorAddr;  ///!< adaptor address to send the data 
    bool m_broadcastEnable;     ///!< if true then broadcast the data
    bool m_initOk;              ///!< if True then sender is open OK
    std::mutex  m_sendLock;     ///!< general interthread lock


   
};


#endif // !UDP_SENDER_H_INCLUDED


