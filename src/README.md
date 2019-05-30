# Autonomous Prospecting and Extraction Systems (Controls)

This folder contains all the software stuffs

* TODO (Software):
    * Re: Server/Client
        * [ ] ensure that client reconnects on connection failure
        * [ ] call shutdown procedure
        * [ ] nail down file reading/writing schema
    * Re: Components
        * [ ] figure out pin mapping and bus specification
        * [ ] fill out prepped encoder funcs
        * [ ] implement switches?
        * [ ] implement drill control
            * using the same motor driver?
        * [ ] test each component
            * make sure wob is calibrated
            * test that the L298N lib works
    * Re: System
        * [ ] put each sensor thread in 1 func that reads at time interval 
        * [ ] finish the set of commands that can be done in the background
             and those that can be done in the foreground
        * [ ] specify and finish implenting data logging
            * this is more a client thing since the client can graphically arrange
              data sent by server
            * if sending binary files is compatible with current system, send
              data.csv file instead of transmitting line by line
    * Re: Cleanup
        * [ ] generally make the code more streamlined
            * remove unnecessary headers/libraries

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

* To run full system:
    * Run: ```make apes.out``` in source directory
    * Run: ```./apes.out <host number> <port number>``` in source directory
   
