/*
  waste_bin_reminder_v1_1.ino    

  v1.1 2023-11-15 get data less often

  Use esp32 core version 2.0.14; the alpha versions of 3 have a problem with the neopixel lib

  www.weigu.lu  
  Hardware used: Lolin C3 Mini
  for UDP, listen on Linux PC (UDP_LOG_PC_IP) with netcat command:
  nc -kulw 0 5558
  more infos: www.weigu.lu/microcontroller/esptoolbox/index.html
  ---------------------------------------------------------------------------
  Copyright (C) 2023 Guy WEILER www.weigu.lu

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
  ---------------------------------------------------------------------------
*/

/*!!!!!!       Make your changes in config.h (or secrets_xxx.h)      !!!!!!*/

/*------ Comment or uncomment the following line suiting your needs -------*/
#define USE_SECRETS
#define OTA               // if Over The Air update needed (security risk!)
//#define MQTTPASSWORD    // if you want an MQTT connection with password
#define STATIC            // if static IP needed (no DHCP)

/****** Arduino libraries needed ******/
#include "ESPToolbox.h"            // ESP helper lib (more on weigu.lu)
#ifdef USE_SECRETS
  // The file "secrets_xxx.h" has to be placed in a sketchbook libraries
  // folder. Create a folder named "Secrets" in sketchbook/libraries and copy
  // the config.h file there. Rename it to secrets_xxx.h
  #include <secrets_waste_bin_reminder.h> // things you need to change are here
#else
  #include "config.h"              // or things you need to change are here
#endif // USE_SECRETS
#include <PubSubClient.h>          // for MQTT
#include <ArduinoJson.h>           // convert MQTT messages to JSON
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

/****** WiFi and network settings ******/
const char *WIFI_SSID = MY_WIFI_SSID;           // if no secrets, use config.h
const char *WIFI_PASSWORD = MY_WIFI_PASSWORD;   // if no secrets, use config.h
#ifdef STATIC
  IPAddress NET_LOCAL_IP (NET_LOCAL_IP_BYTES);  // 3x optional for static IP
  IPAddress NET_GATEWAY (NET_GATEWAY_BYTES);    // look in config.h
  IPAddress NET_MASK (NET_MASK_BYTES);
  IPAddress NET_DNS (NET_DNS_BYTES);
#endif // ifdef STATIC
#ifdef OTA                                      // Over The Air update settings
  const char *OTA_NAME = MY_OTA_NAME;
  const char *OTA_PASS_HASH = MY_OTA_PASS_HASH; // use the config.h file
#endif // ifdef OTA
IPAddress UDP_LOG_PC_IP(UDP_LOG_PC_IP_BYTES);   // UDP log if enabled in setup
WiFiClient espClient;
WiFiClientSecure Wifi_s_client;

/****** MQTT settings ******/
const short MQTT_PORT = MY_MQTT_PORT;
PubSubClient MQTT_Client(espClient);
#ifdef MQTTPASSWORD
  const char *MQTT_USER = MY_MQTT_USER;
  const char *MQTT_PASS = MY_MQTT_PASS;
#endif // MQTTPASSWORD

Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

ESPToolbox Tb;                                // Create an ESPToolbox Object

/****** SETUP ****************************************************************/

void setup() {        
  Tb.set_udp_log(true, UDP_LOG_PC_IP, UDP_LOG_PORT); 
  init_pixels();   
  show_pixels();  
  #ifdef STATIC
    Tb.set_static_ip(true,NET_LOCAL_IP, NET_GATEWAY, NET_MASK, NET_DNS);
  #endif // ifdef STATIC
  Tb.init_ntp_time();
  Tb.init_wifi_sta(WIFI_SSID, WIFI_PASSWORD, NET_MDNSNAME, NET_HOSTNAME);  
  delay(1000);
  //WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #ifdef OTA
    Tb.init_ota(OTA_NAME, OTA_PASS_HASH);
  #endif // ifdef OTA    
  delay(1000);
  MQTT_Client.setBufferSize(MQTT_MAXIMUM_PACKET_SIZE);
  MQTT_Client.setServer(MQTT_SERVER,MQTT_PORT); //open connection MQTT server  
  get_json_data();
  Tb.log_ln("Setup done!");
}

/****** LOOP *****************************************************************/

void loop() {  
  #ifdef OTA
    ArduinoOTA.handle();
  #endif // ifdef OTA
  if ((Tb.t.hour==0) && (Tb.t.minute==0) && (Tb.t.second==0)) {
    get_json_data();
    delay(1000);
  }
  if (Tb.non_blocking_delay(PUBLISH_TIME)) { // PUBLISH_TIME in config.h    
    which_bin = get_bin_from_json(json);
    Tb.log("Which bin to use: " + String(which_bin) + '\n');
    set_wastebin_color(which_bin);
    mqtt_publish();    
    long hippi = ESP.getFreeHeap();
    Tb.log("Heap: " +  String(hippi) + '\n'); 
  }
  if (WiFi.status() != WL_CONNECTED) {   // if WiFi disconnected, reconnect
    Tb.init_wifi_sta(WIFI_SSID, WIFI_PASSWORD, NET_MDNSNAME, NET_HOSTNAME);
  }
  if (!MQTT_Client.connected()) {        // reconnect mqtt client, if needed
    mqtt_connect();
  }
  MQTT_Client.loop();                    // make the MQTT live
  delay(1); // needed for the watchdog! (alt. yield())
}

/********** MQTT functions ***************************************************/

// connect to MQTT server
void mqtt_connect() {
  while (!MQTT_Client.connected()) { // Loop until we're reconnected
    Tb.log("Attempting MQTT connection...");
    #ifdef MQTTPASSWORD
      if (MQTT_Client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
    #else
      if (MQTT_Client.connect(MQTT_CLIENT_ID)) { // Attempt to connect
    #endif // ifdef MQTTPASSWORD
      Tb.log_ln("MQTT connected");
      MQTT_Client.subscribe(MQTT_TOPIC_IN.c_str());
    }
    else {
      Tb.log("MQTT connection failed, rc=");
      Tb.log(String(MQTT_Client.state()));
      Tb.log_ln(" try again in 5 seconds");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

// MQTT get the time, relay flags ant temperature an publish the data
void mqtt_publish() {
  DynamicJsonDocument doc_out(1024);
  String mqtt_msg, we_msg;
  Tb.get_time();
  doc_out["datetime"] = Tb.t.datetime;    
  //doc_out["date"] = Tb.t.date;    
  //doc_out["tomorrow"] = Tb.t.tomorrow;  
  doc_out["waste_bin"] = which_bin;  
  serializeJson(doc_out, mqtt_msg);
  MQTT_Client.publish(MQTT_TOPIC_OUT.c_str(),mqtt_msg.c_str());
  Tb.log("MQTT published at ");
  Tb.log_ln(Tb.t.time);
}

/********** HTTPS functions **************************************************/

void get_json_data() {
  while (json=="nothing") {
    json = get_json_from_server();
    Tb.log_ln(json);
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); //RED
    pixels.show();
    delay(2000);
  }  
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // clear
  pixels.show();
}  

String get_json_from_server() {
  Serial.println("\nStarting connection to server...");
  int http_code;
  String json_payload;    
  Wifi_s_client.setInsecure();   // set secure client without certificate
  HTTPClient Https;               //create an HTTPClient instance
  if (Https.begin(Wifi_s_client, server_name)) {  // HTTPS      
    http_code = Https.GET(); // start connection and send HTTPS header
    Tb.log("[HTTPS] GET... code: " + String(http_code) + '\n');
    if (http_code > 0) { // httpCode will be negative on error              
      if (http_code == HTTP_CODE_OK || http_code == HTTP_CODE_MOVED_PERMANENTLY) {
        json_payload = Https.getString(); // file found 
        //Tb.log(json_payload);          
      }
    }               
    else {
      Tb.log("[HTTPS] GET... failed, error: " +  String(http_code) + '\n');
      json_payload = "nothing";
    }
    Wifi_s_client.stop();
    Https.end();  
  }       
  else {
    Tb.log("Https.begin failed\n");
    json_payload = "nothing";
  } 
  return json_payload;
}  

String get_bin_from_json(String json) {
  DynamicJsonDocument http_doc(4096);    
  deserializeJson(http_doc, json);  
  if (Tb.t.hour > 11) {       
    String tommorow = http_doc[Tb.t.tomorrow];
    return tommorow;
  }
  else {
    String today = http_doc[Tb.t.date];
    return today;
  }          
}  

/****** NeoPixel functions ***************************************************/

void init_pixels() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)  
  pixels.clear(); // Set all pixel colors to 'off'  
}  

void show_pixels() {
  unsigned int my_delay = 250;   
  pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // white
  pixels.show();
  delay (my_delay);
  pixels.setPixelColor(0, pixels.Color(165, 22, 22)); // brown
  pixels.show();
  delay (my_delay);
  pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // green
  pixels.show();
  delay (my_delay);
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // blue
  pixels.show();               
  delay (my_delay);
  pixels.setPixelColor(0, pixels.Color(125, 0, 125)); // light blue
  pixels.show();               
  delay (my_delay);
  pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // yellow
  pixels.show();               
  delay (my_delay);
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // off  
  pixels.show();               
  delay (my_delay);  
}

void set_wastebin_color(String which_bin) {
  if (which_bin == "waste") {
      pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // white      
  }
  else if (which_bin == "biomass") {
      pixels.setPixelColor(0, pixels.Color(165, 22, 22)); // brown      
  }
  else if (which_bin == "glas") {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // green      
  }
  else if (which_bin == "paper") {
      pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // blue      
  }
  else if (which_bin == "valorlux") {
      pixels.setPixelColor(0, pixels.Color(125, 0, 125)); // light blue      
  }
  else if (which_bin == "bulky_waste") {
      pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // yellow      
  }
  else if (which_bin == "null") {      
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));      
  }
  pixels.show();
}
