////////////////////////////////////////////////////////////////////////////
//
// Copyright(c) 2022, Chelton Ltd. 
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Utils class header
//
// Originator           : Lee Playford
//
// Creation Date        : 4 May 2020
//
////////////////////////////////////////////////////////////////////////////
#include "Utils.h"

// C Includes
#include <memory.h>

// C++ Includes
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <ctime>

// Includes

#ifdef __linux__
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <WS2tcpip.h>
#endif


#ifdef _WIN32
char Utils::m_localAddress[22];
char Utils::m_localNetmask[22];

#endif


//--------------------------
//
//--------------------------
StringList Utils::string_split(const std::string& p_rStr, const char p_delimiter)
{
    size_t l_posStart = 0;

    size_t l_posEnd = p_rStr.find_first_of(p_delimiter);

    StringList l_output;
    std::string l_token;

    while ((l_posEnd = p_rStr.find(p_delimiter, l_posStart)) != std::string::npos)
    {
        l_token = p_rStr.substr(l_posStart, l_posEnd - l_posStart);
        l_posStart = l_posEnd + 1;
        l_output.push_back(l_token);
    }

    l_output.push_back(p_rStr.substr(l_posStart));

    return l_output;
}

//--------------------------
//
//--------------------------
StringList Utils::binary_split(const uint8_t * p_pStr, uint16_t p_size , const char p_delimiter)
{
    // loop the  string separating the parts
    StringList l_results;
    std::string l_item;
    for (int i = 0 ; i < p_size ; i++ )
    {
        if (p_pStr[i] == p_delimiter)
        {
            l_results.push_back(l_item);
            l_item.clear();
        }
        else
        {
            l_item += (p_pStr[i]);
        }
    }
    if (!l_item.empty())
    {
        l_results.push_back(l_item);
    }

    return l_results;
}

//--------------------------
//
//--------------------------
std::string Utils::toUpper(const std::string& p_rStr)
{
    std::string l_tmp = p_rStr;
    std::transform(l_tmp.begin(), l_tmp.end(), l_tmp.begin(), [](unsigned char ch)
    {
        return std::toupper(ch);
    });
    return l_tmp;
}


//--------------------------
//
//--------------------------
std::string& Utils::RemoveNonNumeric(std::string & p_rItem)
{
    p_rItem.erase(std::remove_if(p_rItem.begin(), p_rItem.end(), [](char l_ch)
    {
        return l_ch != '.' && !isdigit(l_ch);
    }), p_rItem.end());
    
    return p_rItem;
}

//--------------------------
//
//--------------------------
uint64_t Utils::CurrentTimestampSeconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

//--------------------------
//
//--------------------------
uint64_t Utils::CurrentTimestampMilliSeconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

/// toFloat
///- Details:   Converts a uint32_t to a float
///
///- Returns:   floating point number
///- Throws:    n/a
float Utils::toFloat(uint32_t p_integer)
{
    float l_result(std::nan("0"));
    memcpy(&l_result, &p_integer, sizeof(float));
    return l_result;
}

/// toMemoryItem
///- Details:   Converts a uint32_t to a float
///
///- Returns:   floating point number
///- Throws:    n/a
uint32_t Utils::toMemoryItem(float p_float)
{
    uint32_t l_memItem(0L);
    memcpy(&l_memItem, &p_float, sizeof(uint32_t));
    return l_memItem;
}

void Utils::GetTimeString(char * buffer , int size)
{
    if (size > 30)
    {
        auto now = std::chrono::system_clock::now();
        auto timenow = std::chrono::system_clock::to_time_t(now);
        int ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
        char timeBuf[32];
        std::strftime(timeBuf, 32, "%H:%M:%S", std::localtime(&timenow));
#ifdef __linux__
        sprintf_s(buffer, "%s.%03d", timeBuf , static_cast<int>(ms));
#else
        sprintf_s(buffer, 32, "%s.%03d", timeBuf , static_cast<int>(ms));
#endif
    }
}


#ifdef __linux__
//--------------------------
//
//--------------------------
AdaptorDetails Utils::GetIpAddress(const char* p_pInterface)
{
    AdaptorDetails l_adaptorDetails;

    struct ifaddrs *l_addrs = nullptr, *l_tmp = nullptr;
    if (getifaddrs (&l_addrs) == 0)
    {
        l_tmp = l_addrs;
        while (l_tmp != nullptr)
        {
            if (l_tmp->ifa_addr && l_tmp->ifa_addr->sa_family == AF_INET)
            {
                if (strncmp (l_tmp->ifa_name , p_pInterface , sizeof(*p_pInterface)) == 0)
                {
                    char l_netmask[INET_ADDRSTRLEN];
                    char l_hostIp[INET_ADDRSTRLEN];
                   
                    // found the interface
                    getnameinfo (l_tmp->ifa_addr , sizeof (struct sockaddr_in) , l_hostIp , INET_ADDRSTRLEN , nullptr , 0 , NI_NUMERICHOST);
                    l_adaptorDetails.m_adaptorName = l_tmp->ifa_name;
                    l_adaptorDetails.m_netmask = inet_ntop (AF_INET , &(((struct sockaddr_in *)l_tmp->ifa_netmask)->sin_addr), l_netmask, INET_ADDRSTRLEN);
                    l_adaptorDetails.m_ipAddress = l_hostIp;

                    // create the broadcast address 
                    struct in_addr l_host, l_mask ,l_broadcast;
                    char l_broadcastAddress[INET_ADDRSTRLEN];
                    if (inet_pton(AF_INET, l_hostIp, &l_host) == 1 &&
                        inet_pton(AF_INET, l_netmask, &l_mask) == 1)
                    {
                        l_broadcast.s_addr = l_host.s_addr | ~l_mask.s_addr;

                        if (inet_ntop(AF_INET, &l_broadcast, l_broadcastAddress, INET_ADDRSTRLEN) != NULL)
                        {
                            l_adaptorDetails.m_broadcastAddress = l_broadcastAddress;
                        }
                    }
                    break;
                }
            }
            l_tmp = l_tmp->ifa_next;
        }
        freeifaddrs(l_addrs);
    }
    return l_adaptorDetails;
}
#else

//------------------------------------------------------
// Windows variant, pass back the local address or the cmd
// line passed address
//------------------------------------------------------
AdaptorDetails Utils::GetIpAddress(const char* p_pInterface)
{
    AdaptorDetails l_adaptorDetails;
    l_adaptorDetails.m_adaptorName = "Windows";
    l_adaptorDetails.m_ipAddress = std::string (Utils::m_localAddress);
    l_adaptorDetails.m_netmask = std::string(Utils::m_localNetmask);


    struct in_addr l_host, l_mask, l_broadcast;
    char l_broadcastAddress[INET_ADDRSTRLEN];

    char l_netmask[INET_ADDRSTRLEN];
    char l_hostIp[INET_ADDRSTRLEN];

    strcpy_s(l_netmask, Utils::m_localNetmask);
    strcpy_s(l_hostIp, Utils::m_localAddress);
    
    if (inet_pton(AF_INET, l_hostIp, &l_host) == 1 &&
        inet_pton(AF_INET, l_netmask, &l_mask) == 1)
    {
        l_broadcast.s_addr = l_host.s_addr | ~l_mask.s_addr;

        if (inet_ntop(AF_INET, &l_broadcast, l_broadcastAddress, INET_ADDRSTRLEN) != NULL)
        {
            l_adaptorDetails.m_broadcastAddress = l_broadcastAddress;
        }
    }
    return l_adaptorDetails;

}
#endif

