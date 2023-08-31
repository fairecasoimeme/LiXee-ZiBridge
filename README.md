# LiXee-ZiBridge
Zigbee bridge for ESP32

## Compatibility
This application can be used with :  
* LiXee-ZiWifi32  (WiFi only)
    * The zigbee radio is a ZiGate.  
* LiXee-Ethernet32  (WiFi / Ethernet)
    * The zigbee radio can be : ZiGate, ZNP, Conbee or EZSP.
      
Obviously, you can use this code with ESP32 chip but depend on your board pins connexions. 

## Features
The main feature is to connect a smarthome box (WiFi / Ethernet) to Zigbee coordinator.  

You can :  
* Configure WiFi in STA mode or AP mode. You can scan the network WiFi to be a client.  
* Configure Ethernet mode (if you board get an ethernet port).  
* Configure your zigbee coordinator (ZiGate, ZNP, Conbee or EZSP) with serial speed and type.  
* Configure some options to optimise connexion
* Supervise dialog with your zigbee coordinator
* Superviser connexion state

  

