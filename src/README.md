# Autonomous Prospecting and Extraction Systems (Controls)

This folder contains all the software stuffs

* To setup standalone network:
    * [Follow these instructions to setup an access point](www.raspberrypi.org/documentation/configuration/wireless/access-point.md)
    * Comment out all network configs in /etc/wpa_supplicant/wpa_supplicant.conf

* To run server only:
    * Go into the server directory
    * Comment out all references to robot in server.cpp
    * Run: ```make```
    * Run: ```./server <port number>```

* To run full system:
    * Go into the src directory
    * Run: ```make```
    * Run: ```./apes <host number> <port number>```
    
