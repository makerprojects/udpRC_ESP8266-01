// udpRC_ESP8266-01 Programm ("Firmware") für den ESP8266 (ESP-01)
//
// Das ESP8266-01 µC Board wird als Schnittstellenumsetzer zwischen dem wLAN und dem UART
// des PiKoder/SSC verwendet. Hierzu wird die WLAN-Schnittstelle des Boardes als Access Point 
// konfiguriert (default SSID: "PiKoder_wRX", default Password: "password"). 
// In dieses WLAN bucht sich das Smartphone mit der App "udpRC" ein.

// Last change:
// 26.12.16: Configuration data stored in EEProm, Commands for setting network parameters
#define FwVersion "1.0"

// Dieses Programm wird mit der Arduino IDE (Version > 1.6.8) kompiliert und an den µC übertragen
// werden, wenn man hier unter Datei -> Voreinstellungen -> Additional Boards Manager URLs:
// "http://arduino.esp8266.com/staging/package_esp8266com_index.json" eingibt.
// Danach eingeben:
// Werkzeuge -> Platine -> Boards Manager -> esp8266 installieren
// Und dann einstellen:
// Platine: "Generic ESP8266 module"
// Flash Mode: "QIO"
// Flash Frequency: "40MHz"
// Upload Using: "Serial"
// CPU Frequency: "80MHz"
// Flash Size: "1M (64K SPIFFS)"
// Upload Speed "115200"
// Port: "COM40" (kann variieren)
// Programmer: "Arduino Gemma"
//
// Weitere Informationen auf www.makerprojekte.de und www.pikoder.de.

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// Your local default definitions
#define YourSSID "PiKoder_wRX"
#define YourPassPhrase "password"

// ID of the settings block
#define ConfigHeader "PwRX"
#define ConfigVersion 1

// Settings structure
#define CONFIG_START 0

struct StoreStruct {
  // The variables of the settings
  char ssidap[32];
  int SettingsVersion; // consumes 4 Bytes
  char password[32];
  // This is for mere detection if they are your settings
  char CHeader[8]; // it is the last variable of the struct
  // so when settings are saved, they will only be validated if
  // they are stored completely.
} settings = {
  // The default values
  YourSSID,
  ConfigVersion,
  YourPassPhrase,
  ConfigHeader
};

int settingsRetrievedOk = false;

unsigned int localPort = 12001; //Port zum Empfangen der Daten vom Smartphone
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
  Serial.begin(9600); // setup connection to UART
  delay(1000);
  EEPROM.begin(sizeof(settings));
  loadConfig(); 
  WiFi.mode(WIFI_AP);
  WiFi.softAP(settings.ssidap,settings.password);
  WiFi.softAPConfig(targetip,targetip,subnet);
  myIP = WiFi.softAPIP(); 
  Udp.begin(localPort);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    remoteIp = Udp.remoteIP();
    Udp.read(packetBuffer, 255);
    for (int i=0; i< packetSize; i++) {
      Serial.print(packetBuffer[i]) ;
    } 
  } 
  if (Serial.available() > 0) {
    char nChar = Serial.read();
    if (nChar == '$') {
      while (!Serial.available()); // wait for next char
      nChar = Serial.read();
      if (nChar == '?') { // dump configuration
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read(); 
        while ((nChar != 0xD && nChar != 0xA)) if (Serial.available() > 0) nChar = Serial.read(); 
        Serial.println();
        Serial.print("Configuration dump ");
        if (settingsRetrievedOk) {
          Serial.println("(retrieved from EEPROM)");         
        } else {
          Serial.println("(applied default values)");         
        }
        Serial.print("Firmware: ");
        Serial.println(FwVersion);
        Serial.print("SSID: ");
        Serial.println(settings.ssidap);
        Serial.print("Password: ");     
        Serial.println(settings.password);     
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
      if (nChar == 'P' || nChar == 'p') { // set SSID
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
      

