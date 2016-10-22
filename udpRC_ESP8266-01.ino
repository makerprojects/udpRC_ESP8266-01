// udpRC_ESP8266-01 Programm ("Firmware") für den ESP8266 (ESP-01)
//
// Das ESP8266-01 µC Board wird als Schnittstellenumsetzer zwischen dem wLAN und dem UART
// des PiKoder/SSC verwendet. Hierzu wird die WLAN-Schnittstelle des Boardes als Access Point 
// konfiguriert (SSID: "PiKoder_wRX"). In dieses WLAN bucht sich das Smartphone mit der App "udpRC" ein.

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

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

const char* ssidap = "PiKoder_wRX"; // SSID
const char *password = "PASSWORD"; // Password
unsigned int localPort = 12001; //Port zum Empfangen der Daten vom Smartphone
unsigned int remotPort = 12000;

IPAddress remoteIp;
WiFiUDP Udp;

char packetBuffer[255];

//IP-Adresse des ESP8266-01
IPAddress targetip(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress myIP;

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap,password);
  WiFi.softAPConfig(targetip,targetip,subnet);

  myIP = WiFi.softAPIP();
  
  Udp.begin(localPort);

  Serial.begin(9600); // setup connection to UART
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
    Udp.beginPacket(remoteIp, remotPort);
    while (Serial.available() > 0) {
      Udp.write(Serial.read());
    }
    Udp.endPacket();
  }  
}    
      

