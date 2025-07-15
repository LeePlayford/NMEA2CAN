////////////////////////////////////////////////////////////////////////////
// 
// Copyright(c) 2021, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Network base class
//
// Originator           : Lee Playford
//
// Creation Date        : 24 August 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef _INETWORK_H_INCLUDED_
#define _INETWORK_H_INCLUDED_

// C includes
#include <stdint.h>

// C++ includes

// includes

//---------------------------------------
// Interface class that allows the Network classes to be passed
// around by base class.
//---------------------------------------
class INetwork
{
public:
    /// SendData
    /// Detail- Pure virtual method to Send data to the network
    /// Returns- Number of bytes Sent to the network (-1 if failure)
    /// Throws - n/a
    virtual int16_t SendData 
    (
        const char * p_pData ,  ///< pointer to the data to be sent
        uint16_t p_dataSize,    ///< size of the data to be sent
        int p_socket = -1       ///< socket to send to (-1 means broadcast)
    ) = 0;
};


#endif