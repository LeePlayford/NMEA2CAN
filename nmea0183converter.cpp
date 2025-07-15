//-------------------------------------
//
//-------------------------------------
#include "nmea0183converter.h"


#include <N2kTypes.h>
#include <N2kMessages.h>

#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>

#include "Handlers/MessageHandler.h"

#include "EventLogger.h"

#include <sstream>
#include <iomanip>
#include <complex>

//-------------------------------------
// Handler vars
//-------------------------------------
tBoatData* pBD;
tNMEA2000* pNMEA2000;

const double cDegToRads = M_PI / 180.0;
const double cRadsToDeg = 180.0 / M_PI;
const double M_2PI = 2 * M_PI;



void HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg);

// Predefinition for functions to make it possible for constant definition for NMEA0183Handlers
void HandleRMC(const tNMEA0183Msg &NMEA0183Msg);
void HandleGGA(const tNMEA0183Msg &NMEA0183Msg);
void HandleHDT(const tNMEA0183Msg &NMEA0183Msg);
void HandleVTG(const tNMEA0183Msg &NMEA0183Msg);
void HandleGSV(const tNMEA0183Msg &NMEA0183Msg);
void HandleMWV(const tNMEA0183Msg &NMEA0183Msg);
void HandleVHW(const tNMEA0183Msg &NMEA0183Msg);
void HandleDPT(const tNMEA0183Msg &NMEA0183Msg);
void HandleGLL(const tNMEA0183Msg &NMEA0183Msg);
void HandleZDA(const tNMEA0183Msg &NMEA0183Msg);
void HandleRSA(const tNMEA0183Msg &NMEA0183Msg);




tNMEA0183Handler NMEA0183Handlers[] = {
    {"GGA", &HandleGGA},
    {"HDT", &HandleHDT},
    {"VTG", &HandleVTG},
    {"RMC", &HandleRMC},
    {"GSV", &HandleGSV},
    {"MWV", &HandleMWV},
    {"VHW", &HandleVHW},
    {"DPT", &HandleDPT},
    {"GLL", &HandleGLL},
    {"ZDA", &HandleZDA},
    {"RSA", &HandleRSA},
    {0, 0}};


//-------------------------------------
//
//-------------------------------------
NMEA0183Converter::NMEA0183Converter ()
{
    m_pBoatData = new tBoatData;
    m_pNMEA2000 = (nullptr);
    pBD = m_pBoatData;
    pNMEA2000 = m_pNMEA2000;
    m_isDst = false;
    m_currentYear = 0;

}

//-------------------------------------
//
//-------------------------------------
NMEA0183Converter::NMEA0183Converter (tNMEA2000 & p_NMEA2000) : m_pNMEA2000 (&p_NMEA2000)
{

    m_pBoatData = new tBoatData;
    pBD = m_pBoatData;
    pNMEA2000 = m_pNMEA2000;
    m_isDst = false;
    m_currentYear = 0;
}

//-------------------------------------
//
//-------------------------------------
NMEA0183Converter::~NMEA0183Converter()
{
}

//-------------------------------------
//
//-------------------------------------
bool NMEA0183Converter::Init ()
{
    bool l_success (false);
        // subscribe to get updates
    MessageHandler::SubscribeHandler("$", this);
    l_success = StartThread();

    memset (m_pBoatData , 0x0 , sizeof(tBoatData));
    
    return l_success;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
bool NMEA0183Converter::StartThread()
{
    bool l_success(false);
    // start the reader thread
    m_threadRunning = true;
    m_threadHandle = std::thread([=]
                                 { Thread(); });
    if (m_threadHandle.joinable())
    {
        l_success = true;
    }
    return l_success;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------
void NMEA0183Converter::StopThread()
{
    // stop the Socket reader thread
    if (m_threadRunning)
    {
        m_threadRunning = false;
        if (m_threadHandle.joinable())
        {
            m_threadHandle.join();
        }
    }
}


//-------------------------------------
//
//-------------------------------------
bool NMEA0183Converter::HandleMessage        (
            const uint8_t *p_pMessage,  ///< pointer to the message
            uint16_t &p_size,           ///< size of the message in bytes, returns the number of bytes processed
            int p_socket           ///< socket receiving the data (-1 if the socket doesn't matter)
        )
{
    if (p_pMessage == nullptr || p_size == 0)
    {
        return false; // No message to process
    }

    tNMEA0183Msg NMEA0183Msg;
    static char msgBuffer[128];
    strncpy(msgBuffer, reinterpret_cast<const char*>(p_pMessage), sizeof(msgBuffer) - 1);
    msgBuffer[sizeof(msgBuffer) - 1] = '\0'; // Ensure null

    if (NMEA0183Msg.SetMessage(msgBuffer))
    {
        m_NMEA0183Queue.enqueue(NMEA0183Msg); // Add the message to the queue
        return true; // Message processed successfully
    } 
    else
    {
        // Handle parsing error
        return false; // Parsing failed
    }

}


//-------------------------------------
//
//-------------------------------------
void NMEA0183Converter::Thread ()
{
    while (m_threadRunning)
    {
        if (!m_NMEA0183Queue.isEmpty())
        {
            tNMEA0183Msg NMEA0183Msg = m_NMEA0183Queue.dequeue(); // Get the next message from the queue

            //EventLogger::Debug ("Qs=%d", m_NMEA0183Queue.size());
            // Process the NMEA0183 message
            processNMEASentence(NMEA0183Msg);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Sleep to avoid busy waiting
        }
    }
}


//-------------------------------------
//
//-------------------------------------
void NMEA0183Converter::processNMEASentence(tNMEA0183Msg& NMEA0183Msg)
{
    HandleNMEA0183Msg(NMEA0183Msg);
}
    

//-------------------------------------
//
//-------------------------------------
std::string NMEA0183Converter::GetCurrentDate(unsigned long DaysSince1970)
{
    time_t rawtime = DaysSince1970 * 86400; // Convert days to seconds
    struct tm * timeinfo = gmtime(&rawtime);
    mktime(timeinfo); // Normalize the time structure
    m_currentYear = timeinfo->tm_year + 1900; // tm_year is years since 1900
    if (timeinfo->tm_year > 01) 
    {
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%d %b %Y", timeinfo);
        return std::string(buffer);
    }
    return std::string ("");
}

//-------------------------------------
//
//-------------------------------------
std::string NMEA0183Converter::GetCurrentTime(double secondsSinceMidnight) const
{
    int hours = static_cast<int>(secondsSinceMidnight / 3600);
    int minutes = static_cast<int>((secondsSinceMidnight - (hours * 3600)) / 60);
    int seconds = static_cast<int>(secondsSinceMidnight) % 60;

    if (m_isDst)
    {
        hours = (hours + 1) % 24; // Adjust for DST
    }
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;

    return oss.str();
}

//-------------------------------------
//
//-------------------------------------
std::string NMEA0183Converter::ConvertToDegreesMinutes(double value, bool isLatitude) const
{
    int degrees = static_cast<int>(value);
    double minutes = (value - degrees) * 60.0;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << std::abs(degrees) << "°"
        << std::fixed << std::setprecision(3) << std::abs(minutes) << "'"
        << (isLatitude ? (value >= 0 ? "N" : "S") : (value >= 0 ? "E" : "W"));

    return oss.str();
}


/*
  
  // Internal variables
  Stream* NMEA0183HandlersDebugStream=0;
  
  
  
  void DebugNMEA0183Handlers(Stream* _stream) {
    NMEA0183HandlersDebugStream=_stream;
  }
  */
  tN2kGNSSmethod GNSMethofNMEA0183ToN2k(int Method) {
    switch (Method) {
      case 0: return N2kGNSSm_noGNSS;
      case 1: return N2kGNSSm_GNSSfix;
      case 2: return N2kGNSSm_DGNSS;
      default: return N2kGNSSm_noGNSS;  
    }
  }
  
  //-------------------------------------
  //
  //-------------------------------------
  void NMEA0183Converter::HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg) {
    int iHandler;
    // Find handler
    for (iHandler=0; NMEA0183Handlers[iHandler].Code!=0 && !NMEA0183Msg.IsMessageCode(NMEA0183Handlers[iHandler].Code); iHandler++);
    if (NMEA0183Handlers[iHandler].Code!=0) 
    {
      NMEA0183Handlers[iHandler].Handler(NMEA0183Msg); 
      return;
    }
    // If no handler found, then just print the message
    EventLogger::Debug ("Missed NMEA %s" , NMEA0183Msg.MessageCode());
  }

  // NMEA0183 message Handler functions
  //-------------------------------------
  //
  //-------------------------------------
  void HandleRMC(const tNMEA0183Msg &NMEA0183Msg)
  {
      if (pBD == 0)
          return;

      if (NMEA0183ParseRMC_nc(NMEA0183Msg, pBD->GPSTime, pBD->Latitude, pBD->Longitude, pBD->COG, pBD->SOG, pBD->DaysSince1970, pBD->Variation))
      {
          if (pNMEA2000 != 0)
          {
              tN2kMsg N2kMsg;
              SetN2kGNSS(N2kMsg, 1, pBD->DaysSince1970, pBD->GPSTime, pBD->Latitude, pBD->Longitude, 0,
                         N2kGNSSt_GPS, GNSMethofNMEA0183ToN2k(pBD->GPSQualityIndicator), pBD->SatelliteCount, pBD->HDOP, 0,
                         0, 1, N2kGNSSt_GPS, pBD->DGPSReferenceStationID, pBD->DGPSAge);
              pNMEA2000->SendMsg(N2kMsg);
          }
      }
  }

//-------------------------------------
//
//-------------------------------------
void HandleGSV(const tNMEA0183Msg &NMEA0183Msg) 
  {
    if (pBD==0) return;

    int totMsg , thisMsg;
    struct tGSV gsv[4];
    if (NMEA0183ParseGSV_nc(NMEA0183Msg,totMsg , thisMsg , pBD->SatelliteCount,gsv[0] , gsv[1] , gsv[2] , gsv[3]))
    {
    }
    //else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse GSV");}
} 

//-------------------------------------
//
//-------------------------------------
void HandleGGA(const tNMEA0183Msg &NMEA0183Msg) 
{
    if (pBD==0) return;
    
    double time;
    int satcount;
    if (NMEA0183ParseGGA_nc(NMEA0183Msg,time,pBD->Latitude,pBD->Longitude,
                     pBD->GPSQualityIndicator,satcount,pBD->HDOP,pBD->Altitude,pBD->GeoidalSeparation,
                     pBD->DGPSAge,pBD->DGPSReferenceStationID)) {
      if (pNMEA2000!=0) {
        tN2kMsg N2kMsg;
        SetN2kGNSS(N2kMsg,1,pBD->DaysSince1970,pBD->GPSTime,pBD->Latitude,pBD->Longitude,pBD->Altitude,
                  N2kGNSSt_GPS,GNSMethofNMEA0183ToN2k(pBD->GPSQualityIndicator),pBD->SatelliteCount,pBD->HDOP,0,
                  pBD->GeoidalSeparation,1,N2kGNSSt_GPS,pBD->DGPSReferenceStationID,pBD->DGPSAge
                  );
        pNMEA2000->SendMsg(N2kMsg); 
      }
  
     /* if (NMEA0183HandlersDebugStream!=0) {
        NMEA0183HandlersDebugStream->print("Time="); NMEA0183HandlersDebugStream->println(pBD->GPSTime);
        NMEA0183HandlersDebugStream->print("Latitude="); NMEA0183HandlersDebugStream->println(pBD->Latitude,5);
        NMEA0183HandlersDebugStream->print("Longitude="); NMEA0183HandlersDebugStream->println(pBD->Longitude,5);
        NMEA0183HandlersDebugStream->print("Altitude="); NMEA0183HandlersDebugStream->println(pBD->Altitude,1);
        NMEA0183HandlersDebugStream->print("GPSQualityIndicator="); NMEA0183HandlersDebugStream->println(pBD->GPSQualityIndicator);
        NMEA0183HandlersDebugStream->print("SatelliteCount="); NMEA0183HandlersDebugStream->println(pBD->SatelliteCount);
        NMEA0183HandlersDebugStream->print("HDOP="); NMEA0183HandlersDebugStream->println(pBD->HDOP);
        NMEA0183HandlersDebugStream->print("GeoidalSeparation="); NMEA0183HandlersDebugStream->println(pBD->GeoidalSeparation);
        NMEA0183HandlersDebugStream->print("DGPSAge="); NMEA0183HandlersDebugStream->println(pBD->DGPSAge);
        NMEA0183HandlersDebugStream->print("DGPSReferenceStationID="); NMEA0183HandlersDebugStream->println(pBD->DGPSReferenceStationID);
      }
    } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse GGA"); }*/
    }
  }
  
  #define PI_2 6.283185307179586476925286766559

  //-------------------------------------
  //
  //-------------------------------------
  void HandleHDT(const tNMEA0183Msg &NMEA0183Msg)
  {
      if (pBD == 0)
          return;

      if (NMEA0183ParseHDT_nc(NMEA0183Msg, pBD->TrueHeading))
      {
          if (pNMEA2000 != 0)
          {
              tN2kMsg N2kMsg;
              double MHeading = pBD->TrueHeading - pBD->Variation;
              while (MHeading < 0)
                  MHeading += PI_2;
              while (MHeading >= PI_2)
                  MHeading -= PI_2;
              // Stupid Raymarine can not use true heading
              SetN2kMagneticHeading(N2kMsg, 1, MHeading, 0, pBD->Variation);
              pNMEA2000->SendMsg(N2kMsg);

              SetN2kTrueHeading(N2kMsg, 1, pBD->TrueHeading);
              pNMEA2000->SendMsg(N2kMsg);
              // EventLogger::Debug("NMEA0183Converter::HandleHDT: HDG=%f", pBD->TrueHeading);
          }
      }
  }

  //-------------------------------------
  //
  //-------------------------------------
  void HandleVTG(const tNMEA0183Msg &NMEA0183Msg)
  {
      double MagneticCOG(0.0);
      if (pBD == 0)
          return;

      if (NMEA0183ParseVTG_nc(NMEA0183Msg, pBD->COG, MagneticCOG, pBD->SOG))
      {
          MagneticCOG = 0.0;
          pBD->Variation = pBD->COG - MagneticCOG; // Save variation for Magnetic heading
          if (pNMEA2000 != 0)
          {
              tN2kMsg N2kMsg;
              SetN2kCOGSOGRapid(N2kMsg, 1, N2khr_true, pBD->COG, pBD->SOG);
              pNMEA2000->SendMsg(N2kMsg);
              SetN2kBoatSpeed(N2kMsg, 1, pBD->SOG);
              pNMEA2000->SendMsg(N2kMsg);
              // EventLogger::Debug("NMEA0183Converter::HandleVTG: COG=%f, SOG=%f", pBD->COG, pBD->SOG);
          }
      }
  }

  //-------------------------------------
  //
  //-------------------------------------
  void HandleMWV(const tNMEA0183Msg &NMEA0183Msg)
  {
      if (pBD == 0)
          return;

      double WindAngle, WindSpeed;
      tNMEA0183WindReference WindReference = tNMEA0183WindReference::NMEA0183Wind_True; // Default to True wind
      if (NMEA0183ParseMWV_nc(NMEA0183Msg, WindAngle, WindReference, WindSpeed))
      {
          if (pNMEA2000 != 0)
          {
              tN2kWindReference N2KWindReference = tN2kWindReference::N2kWind_Apparent; // Default to True wind
              if (WindReference == tNMEA0183WindReference::NMEA0183Wind_True)
              {
                  N2KWindReference = tN2kWindReference::N2kWind_True_North;
              }
              pBD->AWS = WindSpeed; // Apparent Wind Speed
              pBD->AWA = WindAngle * cDegToRads; // Apparent Wind Angle

              tN2kMsg N2kMsg;
              SetN2kWindSpeed(N2kMsg, 1, pBD->AWS, pBD->AWA, N2KWindReference);
              pNMEA2000->SendMsg(N2kMsg);

              // Calculate the TWS and TWA
              if (WindReference == tNMEA0183WindReference::NMEA0183Wind_Apparent)
              {
                  // Calculate True Wind Speed and Angle based on apparent wind and boat speed
                  pBD->TWD = pBD->TrueHeading + pBD->AWA; // True Wind Direction
                  if (pBD->TWD >= M_2PI)
                      pBD->TWD -= M_2PI; // Normalize to 0 to 360 degrees

                  // Calculate TWS and TWA using vector addition
                  {
                      std::complex<double> windVector = std::polar(pBD->AWS, pBD->AWA);
                      std::complex<double> boatVector = std::polar(pBD->SOG, 0.0);
                      std::complex<double> apparentWind = windVector - boatVector;
                      pBD->TWS = std::abs(apparentWind);
                      pBD->TWA = std::arg(apparentWind);
                      if (pBD->TWA > M_2PI)
                          pBD->TWA -= M_2PI; // Normalize to 0 to 360 degrees
                      if (pBD->TWA < 0.0)
                          pBD->TWA += M_2PI; // Normalize to 0 to 360 degrees


                      SetN2kWindSpeed(N2kMsg, 1, pBD->TWS, pBD->TWA, tN2kWindReference::N2kWind_True_boat);
                      pNMEA2000->SendMsg(N2kMsg);
                  }
              }
          }
      }
  }

//-------------------------------------
//
//-------------------------------------
void HandleVHW(const tNMEA0183Msg &NMEA0183Msg)
{
    if (pBD == 0)
        return;

    double WaterSpeed, WaterDirectionMag , WaterDirectionTrue;
    if (NMEA0183ParseVHW_nc(NMEA0183Msg,WaterDirectionTrue , WaterDirectionMag , WaterSpeed))
    {
        if (pNMEA2000 != 0)
        {
            tN2kMsg N2kMsg;
            SetN2kPGN128259(N2kMsg, 1, WaterSpeed, 0.0, tN2kSpeedWaterReferenceType::N2kSWRT_Paddle_wheel);
            pNMEA2000->SendMsg(N2kMsg);
            SetN2kPGN127250(N2kMsg, 1, WaterDirectionMag, 0.0,0.0,tN2kHeadingReference::N2khr_magnetic);
            pNMEA2000->SendMsg(N2kMsg);
        }
    }
}

//-------------------------------------
//
//-------------------------------------
void HandleDPT(const tNMEA0183Msg &NMEA0183Msg)
{
    if (pBD == 0)
        return;
    double DepthBelowTransducer, Offset , Range;
    if (NMEA0183ParseDPT_nc(NMEA0183Msg, DepthBelowTransducer, Offset, Range))
    {
        if (pNMEA2000 != 0)
        {
            tN2kMsg N2kMsg;
            SetN2kPGN128267(N2kMsg, 1, DepthBelowTransducer, Offset, Range);
            pNMEA2000->SendMsg(N2kMsg);
        }
    }
}

//-------------------------------------
//
//-------------------------------------
void HandleGLL(const tNMEA0183Msg &NMEA0183Msg)
{
    if (pBD == 0)
        return;

    tGLL GLL;
    if (NMEA0183ParseGLL_nc(NMEA0183Msg,GLL))
    {
        pBD->GPSTime = GLL.GPSTime;
        pBD->Latitude = GLL.latitude;
        pBD->Longitude = GLL.longitude;

        if (GLL.status == 'A') // 'A' = OK
        {
            if (pNMEA2000 != 0)
            {
                tN2kMsg N2kMsg;
                SetN2kGNSS(N2kMsg, 1, pBD->DaysSince1970, pBD->GPSTime, pBD->Latitude, pBD->Longitude,
                           0.0, tN2kGNSStype::N2kGNSSt_integrated, tN2kGNSSmethod::N2kGNSSm_DGNSS, 0, 0.0);
                pNMEA2000->SendMsg(N2kMsg);
            }
        }
    }
}
//-------------------------------------
//
//-------------------------------------
void HandleZDA(const tNMEA0183Msg &NMEA0183Msg)
{
    if (pBD == 0)
        return;

    int day, month, year;
    double GPSTime;
    tZDA zda;
    if (NMEA0183ParseZDA(NMEA0183Msg, zda))
    {
        //time_t lDT;
        tm lDT;
        lDT.tm_year = zda.GPSYear - 1900; // tm_year is years since 1900
        lDT.tm_mon = zda.GPSMonth - 1; // tm_mon is months since January
        lDT.tm_mday = zda.GPSDay;
        lDT.tm_hour = 12;//static_cast<int>(zda.GPSTime / 3600);
        lDT.tm_min = 0;//static_cast<int>((zda.GPSTime - (lDT.tm_hour * 3600)) / 60);
        lDT.tm_sec = 0;//static_cast<int>(zda.GPSTime) % 60;
        lDT.tm_isdst = 1; // No DST adjustment
        lDT.tm_zone = "UTC"; // Set
        time_t rawtime = mktime(&lDT); // Convert to time_t
        if ( !NMEA0183IsTimeNA(rawtime) ) 
        {
            pBD->DaysSince1970=rawtime / 86400.0;// tNMEA0183Msg::elapsedDaysSince1970(rawtime);
        }
        pBD->MOBActivated = false; // Reset MOB status on ZDA message

        if (pNMEA2000 != 0)
        {
            tN2kMsg N2kMsg;
            SetN2kPGN126992 (N2kMsg , 1 , pBD->DaysSince1970, zda.GPSTime,tN2kTimeSource::N2ktimes_GPS);
            pNMEA2000->SendMsg(N2kMsg);
        }

    }
}

//-------------------------------------
//
//-------------------------------------
void HandleRSA(const tNMEA0183Msg &NMEA0183Msg)
{

   if (pBD == 0)
        return;
    
    
    if (NMEA0183Msg.FieldCount() > 2 )
    {
    
        double rudderAngle = std::atof(NMEA0183Msg.Field(0));    // Convert rudder angle to radians
        rudderAngle *= cDegToRads; // Convert degrees to radians
        pBD->RudderAngle = rudderAngle; // Store the rudder angle in radians

        if (pNMEA2000 != 0)
        {
            tN2kMsg N2kMsg;
            SetN2kRudder(N2kMsg, rudderAngle );
            pNMEA2000->SendMsg(N2kMsg);
        }
    }
}
