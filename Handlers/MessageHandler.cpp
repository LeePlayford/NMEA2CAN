////////////////////////////////////////////////////////////////////////////
//
// Copyright(c) 2022, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Message Handler class
//
// Originator           : Lee Playford
//
// Creation Date        : 06 August 2020
//
////////////////////////////////////////////////////////////////////////////
#include "MessageHandler.h"

// C Includes
#include <memory.h>

// C++ Includes
#include <string>

// Includes
#include "EventLogger.h"

constexpr char cERROR_MESSAGE[] = {"Error No Handler Configured"};

//--------------------------------------
// Static Initialisers
//--------------------------------------
MessageHandler *MessageHandler::s_pInstance = nullptr;

//--------------------------------------
// Support for Windows build
//--------------------------------------
#ifdef _WIN32
#define strncpy(a,b,c) strncpy_s(a,b,c)
#endif

//---------------------------------------------------
// This class takes all the messages and routes them
// to the appropriate message handler
//---------------------------------------------------

//------------------------------------------
//
//------------------------------------------
MessageHandler::MessageHandler()
{
    s_pInstance = this;
}

//--------------------------------------- ---
//
//------------------------------------------
MessageHandler::~MessageHandler()
{
    m_handlerMap.clear();
    s_pInstance = nullptr;
}

//------------------------------------------
//
//------------------------------------------
bool MessageHandler::HandleMessage(const uint8_t *p_pMessage, uint16_t& p_size , int p_socket)
{
    static_cast<void>(p_socket);

    uint16_t l_bytesHandled(0); // bytes used by the handlers. Should be 0 when all messages processed
    bool l_handled(true);      // Indicates if message was handled

    // lock the map while it gets searched for handlers
    m_mapLock.lock();

    // The current plan is that the message should start with a $ symbol
    while ((l_bytesHandled < p_size) && *(p_pMessage + l_bytesHandled) == cSTART_MSG)
    {
        char l_header[cHeaderSize] = {0};
        strncpy(l_header, reinterpret_cast<const char *>(p_pMessage), cHeaderSize - 1);
        
        // pass the remainder of the message to the handler
        uint16_t l_processed (p_size - l_bytesHandled); 
        
        // look through msg handler list and find the classes that can handle the message
        for (auto &l_item : m_handlerMap)
        {
            // look at the message header and call the appropiate handler
            if (l_item.second == l_header)
            {
                l_handled &= l_item.first->HandleMessage(p_pMessage + l_bytesHandled, l_processed , p_socket);
            }
        }
        // if it hasn't been processed or there is nothing left to process
        if (!l_handled || l_processed == 0)
        {
            // message not handled
            break;
        }
        
        // Move the data pointer on
        l_bytesHandled += l_processed;
    }

    // if the message was not handled, tell the client
    if ((!l_handled || l_bytesHandled == 0) && m_pNetwork != nullptr)
    {
        m_pNetwork->SendData(cERROR_MESSAGE, sizeof(cERROR_MESSAGE));
    }
    m_mapLock.unlock();
    
    // set the number of bytes handled
    p_size = l_bytesHandled;
    return l_handled;
}

//------------------------------------------
//
//------------------------------------------
void MessageHandler::SetNetwork(const std::shared_ptr<INetwork> & p_rNetwork)
{
    m_pNetwork = p_rNetwork;
}

//------------------------------------------
//
//------------------------------------------
void MessageHandler::SubscribeHandler(const std::string &l_header, IMessageHandlerInterface *p_pHandlerInterface)
{
    if (s_pInstance != nullptr)
    {
        s_pInstance->_SubscribeHandler(l_header, p_pHandlerInterface);
    }
}

//------------------------------------------
//
//------------------------------------------
void MessageHandler::UnSubscribeHandler(IMessageHandlerInterface *p_pHandlerInterface)
{
    if (s_pInstance != nullptr)
    {
        s_pInstance->_UnSubscribeHandler(p_pHandlerInterface);
    }
}

//------------------------------------------
// Private Methods
//------------------------------------------

/// SubscribeHandler
/// Detail- Message handlers can subscribe into the message handler to
///         have the messages forwarded to them.
/// Returns- n/a
/// Throws - n/a
void MessageHandler::_SubscribeHandler
(
    const std::string &l_header,                    ///< header to subscribe to
    IMessageHandlerInterface *p_pHandlerInterface   ///< pointer to handler
) 
{
    m_mapLock.lock();
    m_handlerMap[p_pHandlerInterface] = l_header;
    m_mapLock.unlock();
    EventLogger::Debug("Handler for %s subscribed", l_header.c_str());
}

/// SubscribeHandler
/// Detail- Message handlers can subscribe into the message handler to
///         have the messages forwarded to them.
/// Returns- n/a
/// Throws - n/a
void MessageHandler::_UnSubscribeHandler
(
    IMessageHandlerInterface *p_pHandlerInterface ///< pointer to the handler to unsubscribe
)
{
    m_mapLock.lock();
    EventLogger::Debug("Handler for %s unsubscribed", m_handlerMap[p_pHandlerInterface].c_str());
    m_handlerMap.erase(p_pHandlerInterface);
    m_mapLock.unlock();
}
