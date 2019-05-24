# Autonomous Prospecting and Extraction Systems (Controls)

This folder contains all the software stuffs

* To setup standalone network:
    * [Follow these instructions to setup an access point](www.raspberrypi.org/documentation/configuration/wireless/access-point.md)
    * Comment out all network configs in /etc/wpa_supplicant/wpa_supplicant.conf

* To run shell only:
    * Run: ```make shell.out``` in source directory
    * Run: ```./shell.out``` in source directory

* To run server only:
    * Run: ```make server.out``` in source directory
    * Run: ```./server.out <port number>``` in source directory

* To run full system:
    * Run: ```make apes.out``` in source directory
    * Run: ```./apes.out <host number> <port number>``` in source directory
    
