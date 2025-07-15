#ifndef _BoatData_H_
#define _BoatData_H_

struct tBoatData
{
    unsigned long DaysSince1970; // Days since 1970-01-01

    double TrueHeading, SOG, COG, Variation,
        GPSTime, // Secs since midnight,
        Latitude, Longitude, Altitude, HDOP, GeoidalSeparation, DGPSAge,
        TWD, TWA, TWS, AWA, AWS, BSP , RudderAngle;
    int GPSQualityIndicator, SatelliteCount, DGPSReferenceStationID;
    bool MOBActivated;

public:
    tBoatData()
    {
        TrueHeading = 0;
        SOG = 0;
        COG = 0;
        Variation = 1.0;
        GPSTime = 0;
        Altitude = 0;
        HDOP = 100000;
        DGPSAge = 100000;
        DaysSince1970 = 0;
        MOBActivated = false;
        SatelliteCount = 0;
        DGPSReferenceStationID = 0;
        TWD = 0; // True Wind Direction
        TWA = 0; // True Wind Angle
        TWS = 0; // True Wind Speed
        AWA = 0; // Apparent Wind Angle
        AWS = 0; // Apparent Wind Speed
        BSP = 0; // Boat Speed
        RudderAngle = 0; // Rudder angle
    };
};

#endif // _BoatData_H_

