////////////////////////////////////////////////////////////////////////////
//
// Copyright(c) 2022, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Message Handler class header file
//
// Originator           : Lee Playford
//
// Creation Date        : 06 August 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef MESSAGE_HANDLER_INTERFACE_H_INCLUDED_
#define MESSAGE_HANDLER_INTERFACE_H_INCLUDED_

// C Includes
#include <sys/types.h>
#include <stdint.h>

// C++ Includes
#include <memory>

// Includes
#include "Network/INetwork.h"

// forward declarations
class MMHMessage;

//---------------------------------------
// class - Interface class for the Message 
// Handlers.
//----------------------------------------
class IMessageHandlerInterface
{
public:
    /// Default Constructor
    /// Detail- IMessageMessageHandler constructor
    /// Returns- n/a
    /// Throws - n/a
    IMessageHandlerInterface() 
    {
        
    }

    /// Default Constructor
    /// Detail- IMessageMessageHandler constructor
    /// Returns- n/a
    /// Throws - n/a
    IMessageHandlerInterface(const std::shared_ptr<INetwork>& p_rNetwork) 
    : m_rTCPNetwork(p_rNetwork)
    {
    }

    /// Default Destructor
    /// Detail- virtual base class destructor
    /// Returns- n/a
    /// Throws - n/a
    virtual ~IMessageHandlerInterface(){};

    /// HandleMessage
    /// Detail- Pure virtual base class method
    /// Returns- True if the message was handled
    /// Throws - n/a
    virtual bool HandleMessage
    (
        const uint8_t *p_pMessage,  ///< pointer to the message
        uint16_t &p_size,           ///< size of the message in bytes, returns the number of bytes processed
        int p_socket = -1           ///< socket receiving the data (-1 if the socket doesn't matter)
    ) = 0;


private:
    std::shared_ptr<INetwork> m_rTCPNetwork; ///< holds the ref to the network object


};

#endif