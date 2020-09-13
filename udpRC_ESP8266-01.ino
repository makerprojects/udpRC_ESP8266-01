
/* 
udpRC_ESP8266-01 Receiver (RX) firmware for ESP8266 (ESP-01)

The ESP8266-01 SoC Board is applied as transparent interface between WLAN and the PiKoder/SSC's UART.
In this setup the ESP8266 would establish an access point (ap) with the follwowing default settings: 
SSID: "PiKoder_wRX", Password: "password". Your Android(TM) smart device's app "udpRC" (available at the
google play store) would connect to this ap as a client.   
*/ 

/* 
Last change:
15.08.20: Implemented backwards-compatible FW version retrieval
25.12.19: Implemented Commit and restarting of ESP to execute change in SSID and password 
25.10.18: Implemented Baudrate setting and a few minor fixes; bumped version to 1.2.
08.01.17: Changed status message to destinct between RX und TX firmware, translated comments
26.12.16: Configuration data stored in EEProm, Commands for setting network parameters
*/ 
#define FwVersion "1.3"

/* 
Copyright 2017 - 2020 Gregor Schlechtriem

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* 
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
*/ 

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// Your local default definitions
#define YourSSID "PiKoder_wRX"
#define YourPassPhrase "password"
#define DefaultBaudrate 9600

// ID of the settings block
#define ConfigHeader "PwRX"
#define ConfigVersion 1

// Settings structure
#define CONFIG_START 0

boolean debug = false;

struct StoreStruct {
  // The variables of the settings
  char ssidap[32];
  int SettingsVersion; // consumes 4 Bytes
  char password[32];
  long baudrate;
  // This is for mere detection if they are your settings
  char CHeader[8]; // it is the last variable of the struct
  // so when settings are saved, they will only be validated if
  // they are stored completely.
} settings = {
  // The default values
  YourSSID,
  ConfigVersion,
  YourPassPhrase,
  DefaultBaudrate,
  ConfigHeader
};

int settingsRetrievedOk = false;

unsigned int localPort = 12001; //local port for receiving data from smart device (client)
unsigned int remotPort = 12000;

IPAddress remoteIp;
WiFiUDP Udp;

char packetBuffer[255];

//IP-Adresse des ESP8266-01
IPAddress targetip(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress myIP;

void loadConfig() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + sizeof(settings) - sizeof(settings.CHeader) + 4) == settings.CHeader[4] && // this is '\0'
      EEPROM.read(CONFIG_START + sizeof(settings) - sizeof(settings.CHeader) + 3) == settings.CHeader[3] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - sizeof(settings.CHeader) + 2) == settings.CHeader[2] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - sizeof(settings.CHeader) + 1) == settings.CHeader[1] &&
      EEPROM.read(CONFIG_START + sizeof(settings) - sizeof(settings.CHeader)) == settings.CHeader[0])
  { // reads settings from EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++)
      *((char*)&settings + t) = EEPROM.read(CONFIG_START + t);
     settingsRetrievedOk = true; 
  } else {
    // settings aren't valid! will overwrite with default settings
    saveConfig();
  }
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(settings); t++){ // writes to EEPROM
    EEPROM.write(CONFIG_START + t, *((char*)&settings + t));
  }
  EEPROM.commit();  
  
  for (unsigned int t=0; t<sizeof(settings); t++) { // and verifies the data  
    if (EEPROM.read(CONFIG_START + t) != *((char*)&settings + t)){
      // error writing to EEPROM
    }
  }
}

void setup() {
  EEPROM.begin(sizeof(settings));
  loadConfig(); 
  Serial.begin(settings.baudrate); // setup connection to UART
  delay(1000);
  
  // ap setup
  WiFi.mode(WIFI_AP);
  WiFi.softAP(settings.ssidap,settings.password);
  WiFi.softAPConfig(targetip,targetip,subnet);
  myIP = WiFi.softAPIP();

  // start udp 
  Udp.begin(localPort);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    if (debug) {
      Serial.println();
      Serial.print("Start of package (size " + String(packetSize) + "): ");
    }
    remoteIp = Udp.remoteIP();
    Udp.read(packetBuffer, 255);
    if (packetBuffer[0] != '$')   // assume that ESP programming commands are transmitted in individual packets     
    { // sent command directly to PiKoder
      for (int i=0; i< packetSize; i++) {
        Serial.print(packetBuffer[i]);
      }
    } else {  // handle radio command
      if (packetSize == 1) { // request fw version
        Udp.beginPacket(remoteIp, remotPort);
        Udp.write(FwVersion);
        Udp.endPacket();     
      } else {        
        if (packetBuffer[1] == '?') { // transmit configuration in simplified format          
          Udp.beginPacket(remoteIp, remotPort);
          int i = 0;
          while (settings.ssidap[i] != '\0') Udp.write(settings.ssidap[i++]);
          Udp.write('\0');
          i = 0;
          while (settings.password[i] != '\0') Udp.write(settings.password[i++]);
          Udp.write('\0');
          Udp.endPacket();     
        } 
        if (packetBuffer[1] == 'S' || packetBuffer[1] == 's') {  // set SSID - string ending '\0'        
          for (int i=3; i < packetSize; i++) {
            settings.ssidap[i-3] = packetBuffer[i];
          } 
          saveConfig();
        }
        if (packetBuffer[1] == 'P' || packetBuffer[1] == 'p') {  // set password - string ending '\0'       
          for (int i=3; i < packetSize; i++) {
            settings.password[i-3] = packetBuffer[i];
          }
          saveConfig();
        }
      }
    }
  }
     
  if (Serial.available() > 0) {
    char nChar = Serial.read();
    if (nChar == '$') {
      
      while (!Serial.available()); // wait for next char
      nChar = Serial.read();
      if (nChar == '?') { // dump configuration
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read();  // read both (cr & lf) 
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read(); 
        Serial.println();
        Serial.print("Configuration dump ");
        if (settingsRetrievedOk) {
          Serial.println("(retrieved from EEPROM)");         
        } else {
          Serial.println("(applied default values)");         
        }
        Serial.print("RX version: ");
        Serial.println(FwVersion);
        Serial.print("SSID: ");
        Serial.println(settings.ssidap);
        Serial.print("Password: ");     
        Serial.println(settings.password);     
        Serial.print("Baudrate: ");     
        Serial.print(String(settings.baudrate,DEC));           
      } 
      if (nChar == 'S' || nChar == 's') { // set SSID
        int i=0;       
        while ((nChar != 0xD && nChar != 0xA)) {
          if (Serial.available() > 0) {
            nChar = Serial.read();
            if ((!i) && (nChar == '=')) {
            } else settings.ssidap[i++] = nChar;        
          }
        }    
        settings.ssidap[--i] = '\0';
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read(); 
        Serial.println();
        Serial.print("Setting SSID: ");
        Serial.println();
        Serial.print("New SSID: ");
        Serial.print(settings.ssidap);           
        saveConfig();
        Serial.println(" ... reset controller to apply new settings");  
      }
      if (nChar == 'P' || nChar == 'p') { // set password
        int i=0;       
        while ((nChar != 0xD && nChar != 0xA)) {
          if (Serial.available() > 0) {
            nChar = Serial.read();
            if ((!i) && (nChar == '=')) {
            } else settings.password[i++] = nChar;        
          }
        }    
        settings.password[--i] = '\0';
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read(); 
        Serial.println();
        Serial.print("Setting Password: ");
        Serial.println();
        Serial.print("New Password: ");
        Serial.print(settings.password);           
        saveConfig();
        Serial.println(" ... reset controller to apply new settings");  
      }
      if (nChar == 'B' || nChar == 'b') { // set new baud rate
        int i=0;
        long newBaudrate = 0;       
        while ((nChar != 0xD && nChar != 0xA)) {
          if (Serial.available() > 0) {
            nChar = Serial.read();
            if ((!i) && (nChar == '=')) {
            } else if (isDigit(nChar)) {
              newBaudrate = newBaudrate * 10 + nChar - '0';        
            } else if ((nChar != 0xD && nChar != 0xA)) {
              Serial.println();
              Serial.println();
              Serial.println("Found non numerical input (" + String(nChar) + ") - please re-enter.");
              newBaudrate = 0;
              break;
            }
          }
        }    
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read(); 
        if (newBaudrate > 0) {
          Serial.println();
          Serial.println();
          Serial.print("Setting Baudrate");
          Serial.println();
          Serial.print("New Baudrate: ");
          Serial.print(String(newBaudrate,DEC));           
          settings.baudrate = newBaudrate;
          saveConfig();
          Serial.println(" ... reset controller to apply new settings");
        }
      }          
      if (nChar == 'R' || nChar == 'r') { // initiate reset
        ESP.restart();
      }
    } else {
      Udp.beginPacket(remoteIp, remotPort);
      Udp.write(nChar);
      while (Serial.available() > 0) {
        Udp.write(Serial.read());
      }
      Udp.endPacket();
    }
  }  
}    
      
