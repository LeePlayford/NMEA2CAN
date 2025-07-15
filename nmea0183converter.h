
//-------------------------------------------------
//
// Project BaroGraph
// GPS Reader Header file
//
//
//
//-------------------------------------------------
#ifndef NMEA0183CONVERTER
#define NMEA0183CONVERTER

//--------------------------------------
//
//--------------------------------------
#include <string>

#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>

#include <NMEA2000/NMEA2000.h>

#include "BoatData.h"
#include "Handlers/MessageHandlerInterface.h"
#include "IThread.h"

#include "SafeQueue.h"


#define GPS_BAUD 9600
#define GPS_MAX_SENTENCE 100


struct tNMEA0183Handler {
    const char *Code;
    void (*Handler)(const tNMEA0183Msg &NMEA0183Msg); 
  };


//-------------------------------------
//
//-------------------------------------
  class NMEA0183Converter : public IMessageHandlerInterface, IThread
{
    public:
        NMEA0183Converter (tNMEA2000 & NMEA2000);
        NMEA0183Converter ();
        ~NMEA0183Converter();

        bool Init ();
        void processNMEASentence(tNMEA0183Msg& NMEA0183Msg);

        bool HandleMessage
        (
            const uint8_t *p_pMessage,  ///< pointer to the message
            uint16_t &p_size,           ///< size of the message in bytes, returns the number of bytes processed
            int p_socket = -1           ///< socket receiving the data (-1 if the socket doesn't matter)
        );

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

    private:
    
    void HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg);
    void InitNMEA0183Handlers(tNMEA2000 *_NMEA2000, tBoatData *_BoatData);
    void Thread ();
    std::string GetCurrentDate(unsigned long DaysSince1970)  ;
    std::string GetCurrentTime(double secondsSinceMidnight) const;
    std::string ConvertToDegreesMinutes(double value, bool isLatitude) const;

    tNMEA2000 * m_pNMEA2000;
    tBoatData * m_pBoatData;

    bool m_isDst;
    int m_currentYear;
    SafeQueue<tNMEA0183Msg> m_NMEA0183Queue;
    bool m_runThread;



};

#endif
