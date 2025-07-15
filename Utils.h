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
// Creation Date        : 4 Jun 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

// C Includes


// C++ Includes
#include <vector>
#include <string>
#include <cmath>

// Includes



typedef std::vector<std::string> StringList;

typedef struct _adaptorDetails
{
    std::string m_adaptorName;
    std::string m_ipAddress;
    std::string m_netmask;
    std::string m_broadcastAddress;

} AdaptorDetails;

class Utils
{
public:
    Utils() = delete;
    ~Utils() = delete ;

    /// string_split
    ///- Details:   Splits a string into separate strings delimited by the character
    ///
    ///- Returns:   List of strings in a std::list
    ///- Throws:    n/a
    static StringList string_split
    (
        const std::string& p_rStr,  ///< string containing the string to be split
        const char p_delimiter      ///< delimiter character
    );

    /// string_split
    ///- Details:   Splits a string into separate strings delimited by the character
    ///
    ///- Returns:   List of strings in a std::list
    ///- Throws:    n/a
    static StringList binary_split
    (
        const uint8_t* p_pStr,  ///< buffer containig the string to be split
        uint16_t p_size,        ///< size og the buffer
        const char p_delimiter  ///< delimiter
    );

    /// toUpper
    ///- Details:   converts all the character to upper case
    ///
    ///- Returns:   the new string with upper case chars
    ///- Throws:    n/a
    static std::string toUpper
    (
        const std::string& p_rStr   ///< string to be converted
    );

    /// toUpper
    ///- Details:   remove any non numberic characters
    ///
    ///- Returns:   the new string containing only numbers
    ///- Throws:    n/a
    static std::string& RemoveNonNumeric
    (
        std::string & p_rItem   ///< string to remove numbers from
    );

    /// Current Timestamp seconds
    ///- Details:   returns the current timestamp in seconds
    ///
    ///- Returns:   timestamp in seconds (since epoch)
    ///- Throws:    n/a
    static uint64_t CurrentTimestampSeconds();

    /// Current Timestamp milliseconds
    ///- Details:   returns the current timestamp in milliseconds
    ///
    ///- Returns:   timestamp in milliseconds (since epoch)
    ///- Throws:    n/a
    static uint64_t CurrentTimestampMilliSeconds();

    /// Get Interface Address
    ///- Details:   return the IP address, broadcast address and netmask of the interface
    ///
    ///- Returns:   the IP Address
    ///- Throws:    n/a
    static AdaptorDetails GetIpAddress(const char* p_interface);

    /// to Float
    /// Detail- converts a uint32_t to a float
    /// Returns- float of the uint32_t
    /// Throws - n/a
    static float toFloat(uint32_t p_integer);

    /// toMemoryItem, 
    /// Detail- helper function
    /// Returns- uint32_t of the floating point number
    /// Throws - n/a
    static uint32_t toMemoryItem(float p_float);

    static void GetTimeString(char *buffer, int size);

    static bool g_DumpSequence;

#ifdef _WIN32
    static char m_localAddress[22];
    static char m_localNetmask[22];

#endif


};

//--------------------------
// Pointer deleter
//--------------------------
#ifndef SAFE_DELETE
#define SAFE_DELETE
template<class T>
void SafeDelete (T*& p_pPtr)
{
    if (p_pPtr != nullptr)
    {
        delete p_pPtr;
        p_pPtr = nullptr;
    }
}
#endif

#ifdef __linux__
#define sprintf_s sprintf
#endif

#endif

