////////////////////////////////////////////////////////////////////////////
// 
// Copyright(c) 2021, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : UDP Reader header file
//
// Originator           : Lee Playford
//
// Creation Date        : 30 July 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef UDP_READER_H_INCLUDED
#define UDP_READER_H_INCLUDED

// C includes
#ifdef __linux__
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#else
#include <WinSock2.h>
#endif

// C++ includes
#include <string>

// includes
#include "../IThread.h"

// forward declarations
class IMessageHandlerInterface;

//----------------------------------------------
// UDP Reader class reads MMH messages from the network
//----------------------------------------------
class UDPReader : public IThread
{
public:
    /// Default Constructor
    /// Detail- UDP Reader constructor
    /// Returns- n/a
    /// Throws - n/a
    UDPReader
    (
        IMessageHandlerInterface* p_pMessageHandler,    ///< pointer to the message handler
        uint16_t p_port,                                ///< UDP port number
        const std::string& p_rAdaptorAddress,           ///< adaptor address to receive the UDP data
        const std::string& p_rSenderAddress,            ///< the UDP Sender
        bool broadcastEnable                            ///< if true, then the received data will be from a broadcast address
    );

    /// Default Destructor
    /// Detail- UDP Reader destructor
    /// Returns- n/a
    /// Throws - n/a
    virtual ~UDPReader();

    /// InitSocket
    /// Detail- Initialise and open the UDP reader socket
    /// Returns- true if the socket was opened and configured
    /// Throws - n/a
    bool InitSocket();

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

    /// Start Thread 
    ///- Details:   Method to Start the Thread, overrides the method in the base class
    ///
    ///- Returns:   true if the thread started OK
    ///- Throws:    n/a
    bool StartThread() override;

    /// Stop Thread 
    ///- Details:   Method to Stop the Thread, overrides the method in the base class
    ///
    ///- Returns:   n/a
    ///- Throws:    n/a
    void StopThread() override;

    /// Read
    ///- Details:   read the UDP Stream - outside of the reader thread
    ///
    ///- Returns:   number of bytes read from the UDP socket (-1 if there was an error)
    ///- Throws:    n/a
    int16_t Read 
    (
        void* p_pBuffer ,       ///< buffer to the received the data
        uint16_t p_BufSize,     ///< size of the buffer
        timeval* p_pTimeVal = 0 ///< time val
    );

    /// IsOpen
    ///- Details:   Returns the state of the link
    ///
    ///- Returns:   true if the link is open
    ///- Throws:    n/a
    bool IsOpen() { return m_initOk; }

private:
    // The Reader Thread function
    void ReaderThread();

    int m_socket;					///!< the receive socket
    struct sockaddr_in m_addr;		///!< End point address

    IMessageHandlerInterface* m_pMsgHandler; ///!< The message handler instance
    uint16_t    m_port;             ///!< UDP Port address
    std::string m_adaptorAddr;      ///!< adaptor address
    std::string m_senderAddr;       ///!< the UDP sender address, used to filter received packets
    bool m_broadcastEnable;         ///!< true if broadcast is enabled
    bool m_initOk;                  ///!< true if the Reader is running OK
};

#endif

