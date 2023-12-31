#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <CircularBuffer.h>

#define VERSION "v1.6"
// hardware config

#define ETHERV2
#define RESET_ZIGATE 4//13
#define FLASH_ZIGATE 33//14
#define PRODUCTION 1
#define FLASH 0

/*#ifdef ETHERV1
  #define RXD2 2 //16
  #define TXD2 4 //17
#endif*/

//#ifdef ETHERV2
  #define RXD2 17 //17
  #define TXD2 16 //16
//#endif

// ma structure config
struct ConfigSettingsStruct {
  bool enableWiFi;
  char ssid[50];
  char password[50];
  char ipAddressWiFi[18];
  char ipMaskWiFi[16];
  char ipGWWiFi[18];
  bool dhcp;
  bool connectedEther;
  char ipAddress[18];
  char ipMask[16];
  char ipGW[18];
  int serialSpeed;
  char radioType[20];
  char dataFlow[20];
  int  tcpListenPort;
  bool disableWeb;
  bool modeUSB;
  bool enableHeartBeat;
  double refreshLogs;
};

struct ZiGateInfosStruct {
  char device[8];
  char mac[8];
  char flash[8];  
};

typedef CircularBuffer<char, 1024> LogConsoleType;


#define DEBUG_ON

#ifdef DEBUG_ON
 #define DEBUG_PRINT(x)  Serial.print(x) 
 #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif
#endif