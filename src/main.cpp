#include "00_globals.hpp"
// #include "01_json.hpp"
#include "02_fileSys.hpp"
#include "03_wifi.hpp"
#include "04_bme.hpp"
#include "05_mqtt.hpp"
#include "06_reboot.hpp"

//=======================================
// GLOBAL FUNCTIONS
//=======================================
void onwifiConnect(const WiFiEventStationModeGotIP& event) {
  initMqtt();
  connectToMqtt();
}

void onwifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  if (Debug)   Serial.println(F("Wifi disconnected."));
  timer.deleteTimer(mqttReconnectTimerID);  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  timer.setTimeout(2000, connectToWifi); // attempt to reconnect to WiFi
}


void setup() {
  Serial.begin(115200);
  initReboot();   // Initialize Reboot
  initFS();       // Initialize File System
  initGPIO();     // Initialize GPIO  
  initBME();
  // Initialize wifi and MQTT
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onwifiDisconnect);
  wifiConnectHandler = WiFi.onStationModeGotIP(onwifiConnect);
  connectToWifi();
  BMETimerID = timer.setInterval(bmeInterval, publishBME);
}

void loop() { timer.run(); }