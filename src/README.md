# Autonomous Prospecting and Extraction Systems (Controls)

This folder contains all the software stuffs

* To setup standalone network:
    * [Follow these instructions to setup an access point](https://www.raspberrypi.org/documentation/configuration/wireless/access-point.md)
    * Comment out all network configs in /etc/wpa_supplicant/wpa_supplicant.conf

* To run shell only:
    * Comment out all references to APES.h in APESShell.cpp and APESShell.h
    * Run: ```make shell.out``` in source directory
    * Run: ```./shell.out``` in source directory

* To run server only:
    * Run: ```make server.out``` in source directory
    * Run: ```./server.out <port number>``` in source directory

* To run component test only:
    * Run: ```make comp.out``` in source directory
    * Run: ```./comp.out``` in source directory

* To run full system:
    * Run: ```make apes.out``` in source directory
    * Run: ```./apes.out <port number>``` in source directory
   
