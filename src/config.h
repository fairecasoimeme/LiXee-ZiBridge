#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <CircularBuffer.h>

#define VERSION "v2.0"
// hardware config

#ifdef ESP32S3
  // ZiWiFi32 (ESP32-S3) pin mapping
  #define RESET_ZIGATE 19
  #define FLASH_ZIGATE 40
  #define RXD2 17
  #define TXD2 18
#else
  // Original ESP32 (ETH board) pin mapping
  #define ETHERV2
  #define RESET_ZIGATE 4
  #define FLASH_ZIGATE 33
  #define RXD2 17
  #define TXD2 16
#endif

#define PRODUCTION 1
#define FLASH 0

// UART pour la ZiGate : Serial1 sur ESP32-S3, Serial2 sur ESP32
#ifdef ESP32S3
  #define ZiGateSerial Serial1
#else
  #define ZiGateSerial Serial2
#endif

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