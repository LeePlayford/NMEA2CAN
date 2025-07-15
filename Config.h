////////////////////////////////////////////////////////////////////////////
// 
// Copyright(c) 2022, Chelton Ltd.
//
////////////////////////////////////////////////////////////////////////////
//
// Description          : Configuration Header file
//
// Originator           : Lee Playford
//
// Creation Date        : 4 Aug 2020
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_


// Header file just contains configuration data

//----------------------------------
// Consts
//----------------------------------

// Port numbers
const uint16_t cDEFAULT_TCP_PORT        = 14414;
const uint16_t cDEFAULT_UDP_PORT        = 14100;
const uint16_t cDEFAULT_GUI_LINK_PORT   = 14101;
const uint16_t cDEFAULT_AUDIO_STREAM_INPUT_PORT = 14102;
const uint16_t cDEFAULT_AUDIO_STREAM_OUTPUT_PORT = 14103;
const uint16_t cDEFAULT_IQ_STREAM_OUTPUT_PORT = 14105;


const uint16_t cDEFAULT_NMEA_PORT       = 15001;

// Addressesq
const char cDEFAULT_ADAPTOR_ADDRESS[] = {"0.0.0.0"};
const char cADDRESS_ANY[] = {"192.168.1.21"};

// Devices
const char cSERIAL_DEVICE[] = {"/dev/ttyUSB0"};
#ifdef ZYNQ
const char cDEFAULT_ADAPTOR[] = {"eth0"};
const char cDEFAULT_ADAPTOR_ALT[] = {"eth0"};
#else
const char cDEFAULT_ADAPTOR_ALT[] = {"enp0s3"};
const char cDEFAULT_ADAPTOR[] = {"wlan0"};
//const char cDEFAULT_ADAPTOR_ALT[] = {"wlan0"};
#endif

// Timeouts
const uint16_t cGUI_LINK_TIMEOUT_MS = 5000;
const uint16_t cNMEA_TIME_OUT_MS = 5000;

#endif

