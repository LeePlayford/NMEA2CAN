////////////////////////////////////////////////////////////////////////////
//
// Copyright(c) 2022, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Message Handler class p_header file
//
// Originator           : Lee Playford
//
// Creation Date        : 06 August 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef MESSAGE_HANDLER_H_INCLUDED_
#define MESSAGE_HANDLER_H_INCLUDED_

// C Includes
#include <sys/types.h>
#include <stdint.h>

// C++ Includes
#include <map>
#include <string>
#include <mutex>
#include <memory>

// Includes
#include "MessageHandlerInterface.h"
#include "Network/INetwork.h"

//-------------------------------------
// Consts & defines for class
//-------------------------------------
const int cHeaderSize = 2; // e.g. $XXX:
const char cSTART_MSG = '$';

//---------------------------------------------
// Handles all the incoming messages, dispatching them to a subscribed 
// handler
//---------------------------------------------
class MessageHandler : public IMessageHandlerInterface
{
public: // Functions
    /// Default Constructor
    /// Detail- MessageHandler constructor
    /// Returns- n/a
    /// Throws - n/a
    MessageHandler();

    /// Default Destructor
    /// Detail- class destructor
    /// Returns- n/a
    /// Throws - n/a
    ~MessageHandler();

    /// HandleMessage
    /// Detail- Handles messages received from the networking classes, overrides IMessageHandler
    /// Returns- true if the message was handled
    /// Throws - n/a
    virtual bool HandleMessage
    (
        const uint8_t *p_pMessage,  ///< pointer to the message
        uint16_t& p_size,           ///< size of the message, returns the number of bytes processed
        int p_socket = -1           ///< socket receiving the data
    ) override;

    /// SubscribeHandler
    /// Detail- Message handlers can subscribe into the message handler to
    ///         have the messages forwarded to them. It is static so the
    ///         handler does not need an instance of the message handler
    /// Returns- n/a
    /// Throws - n/a
    static void SubscribeHandler
    (
        const std::string &p_header,                  ///< indicates which message the handler is subscribing to
        IMessageHandlerInterface *p_pHandlerInterface ///< pointer to the message handler
    );

    /// UnSubscribeHandler
    /// Detail- Message handlers can unsubscribe from the message handler to
    ///         stop the messages being forwarded to them. It is static so the
    ///         handler does not need an instance of the message handler
    /// Returns- n/a
    /// Throws - n/a
    static void UnSubscribeHandler(
        IMessageHandlerInterface *p_pHandlerInterface ///< pointer to the message handler
    );

    /// SetNetwork
    /// Detail- There may be a reason to send failure messages back to the
    ///         network, so this is included to allow a network to attach.
    /// Returns- n/a
    /// Throws - n/a
    void SetNetwork
    (
        const std::shared_ptr<INetwork>& p_rNetwork ///< Reference to a Network handler
    );

private: // Functions
    void _SubscribeHandler(const std::string &p_header, IMessageHandlerInterface *p_pHandlerInterface);
    void _UnSubscribeHandler(IMessageHandlerInterface *p_pHandlerInterface);

private:                                                            
    // Private Member Variables
    std::map<IMessageHandlerInterface *, std::string> m_handlerMap; ///< map containing the subscriptions
    std::mutex m_mapLock;                                           ///< lock for the handler map
    static MessageHandler *s_pInstance;                             ///< Instance pointer of this class
    std::shared_ptr<INetwork> m_pNetwork;                           ///< Pointer to the owning network
};

#endif
