# udpRC_ESP8266-01
udpRC_ESP8266-01 Receiver (RX) firmware for ESP8266 (ESP-01)

## Overview

The ESP8266-01 SoC Board is applied as transparent interface between WLAN and the PiKoder/SSC's or PiKoder/PPM's UART.
In this setup the ESP8266 would establish an access point (ap) with the follwowing default settings: 
SSID: "PiKoder_wRX", Password: "password". Your Android(TM) smart device's PiKoder app such as e.g. "udpRC" or "udpRC4UGV" (available at the
google play store) would connect to this ap as a client.   
For more information please refer to www.pikoder.com.


## Features

The sketch provides for a command line interface for programming the SSID, password and the baud rate. If you enter '$?<cr><lf>' the controller will display the current settings (default baudrate is 9600 Bd). 

The SSID can be changed to 'new-ssid' by entering '$S=new-ssid<cr><lf>' (default: PiKoder_wRX), the password is customized thorugh '$P=new-password<cr><lf>' (default: password). To adjust the baudrate to your application enter '$B=nnnn<cr><lf>' with nnnn representing the new baudrate (e.g. 115200).    


## Build and loading

This sketch requires Arduino IDE (Version > 1.6.8). The following settings are recommended:

Add ESP board: 
File -> Preferences -> Additional Boards Manager URLs:
enter "http://arduino.esp8266.com/staging/package_esp8266com_index.json" 
and then using:
Tools -> Boards -> Boards Manager to install the esp8266

Recommended board settings:
Board: "Generic ESP8266 module"
Flash Mode: "QIO"
Flash Frequency: "40MHz"
Upload Using: "Serial"
CPU Frequency: "80MHz"
Flash Size: "1M (64K SPIFFS)"
Upload Speed "115200"
Port: "COM40" (depends on your setup)
Programmer: "Arduino Gemma"

Please refer to www.makerprojects.de and www.pikoder.de for more information.


## Copyright 2017 - 2018 Gregor Schlechtriem

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

