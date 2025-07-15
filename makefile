INC=-I./ \
	-I./Network \
	-I./Handlers \
	-I./NMEA0183 \
	-I./NMEA2000 \
	-I./NMEA2000_socketCAN 
LIB=-pthread 

all:
	g++ -g \
	$(INC) $(LIB) nmea2can.cpp \
	ssd1306.cpp \
	Network/UDPReader.cpp \
	Network/UDPSender.cpp \
	Handlers/MessageHandler.cpp \
	Handlers/MessageHandlerInterface.cpp \
	EventLogger.cpp \
	Utils.cpp \
	nmea0183converter.cpp \
	NMEA0183/NMEA0183Msg.cpp \
	NMEA0183/NMEA0183Messages.cpp \
	NMEA0183/NMEA0183Stream.cpp \
	NMEA2000/N2kMsg.cpp \
	NMEA2000/N2kMessages.cpp \
	NMEA2000/NMEA2000.cpp \
	NMEA2000/N2kTimer.cpp \
	NMEA2000/N2kStream.cpp \
	NMEA2000/N2kGroupFunction.cpp \
	NMEA2000/N2kGroupFunctionDefaultHandlers.cpp \
	NMEA2000_socketCAN/NMEA2000_SocketCAN.cpp \
	-o nmea2can -std=c++14
	

clean:
	rm -f nmea2can

