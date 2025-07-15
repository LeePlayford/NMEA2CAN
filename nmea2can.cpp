#include "ssd1306.h"

#include "EventLogger.h"
#include "Network/UDPReader.h"
#include "Network/UDPSender.h"
#include "Handlers/MessageHandler.h"
#include "nmea0183converter.h"
#include "Config.h"

#include "NMEA2000/NMEA2000.h"
#include <NMEA2000_CAN.h>


//-------------------------------------
//
//-------------------------------------
int main( int argc, char * argv [] ) {

	EventLogger::GetInstance()->SetLogLevel(eLogLevel::Debug);
	
	SSD1306 myDisplay;
	myDisplay.initDisplay();
	myDisplay.clearDisplay();
	sleep(1);
 
	//	SSD1306 myDisplay;
	myDisplay.setDisplayMode(SSD1306::Mode::SCROLL);
	myDisplay.setWordWrap(TRUE);
	myDisplay.textDisplay("NMEA 2 CAN");

	// STart a Message Handler
	MessageHandler msgHandler;

	// Start a NMEA0183 convertor
	NMEA0183Converter nmeaConverter (NMEA2000);

	// Start a NMEA2000 instance
	NMEA2000.SetMode(tNMEA2000::N2km_ListenAndSend , 45);
	NMEA2000.EnableForward(false);
	if (NMEA2000.Open())
	{
		NMEA2000.SetProductInformation("NMEA2CAN", 0x1234, "NMEA2CAN Model", "1.0", "1.0", 1, 2101, 0);
		// Set device information
    	NMEA2000.SetDeviceInformation(10101010, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
		NMEA2000.SendProductInformation(0x0);
		nmeaConverter.Init();
	}
	else
	{
		myDisplay.textDisplay("NMEA2000 Open failed");
		EventLogger::Error ("NMEA2000 Open failed");
		return -1;
	}



	std::string adaptor = "0.0.0.0";//cDEFAULT_ADAPTOR;
	std::string address = "";
	UDPReader udpReader (&msgHandler , 2031 ,adaptor , address , false);
	while (udpReader.IsOpen()) {
		sleep(1);
	}
	


}
