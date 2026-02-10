# LiXee-ZiBridge
Zigbee bridge for ESP32 chip

## Compatibility
This application can be used with :  
* [LiXee-ZiWifi32](https://lixee.fr/produits/41-lixee-ziwifi32-3770014375162.html)  (WiFi only)
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
  
# How to flash release
Just install esptools and run this command

## Windows

`esptool.py.exe --chip esp32 --port "COMXX" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 16MB 0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 lixee-zibridge.bin`

# Screenshots
## Status
![status](https://github.com/fairecasoimeme/LiXee-ZiBridge/blob/master/screenshots/ZiBridge_status.JPG)  
## Config
![status](https://github.com/fairecasoimeme/LiXee-ZiBridge/blob/master/screenshots/ZiBridge_config_general.JPG)  
## WiFi
![wifi](https://github.com/fairecasoimeme/LiXee-ZiBridge/blob/master/screenshots/ZiBridge_config_Wifi.JPG)
## Console
![console](https://github.com/fairecasoimeme/LiXee-ZiBridge/blob/master/screenshots/ZiBridge_consolel.JPG)


## Changelog

### V1.6
* Fix Wifi network scan
* Fix AsyncTCP capacity
* Fix mDNS problem to be compatible with ZeroConf home-assistant
* Change name of the project
* Convert arduino IDE source to PlatformIo environment

### V1.5a
* Initial source from [ZiGate-Ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet)
