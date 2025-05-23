#include "00_globals.hpp"
#include "01_reboot.hpp"
#include "02_json.hpp"
#include "03_fileSys.hpp"
#include "04_wifi.hpp"
#include "05_bme.hpp"
#include "06_mqtt.hpp"
#include "07_events.hpp"

void setup() {
  Serial.begin(115200);
  initFS();       // Initialize File System
  initGPIO();     // Initialize GPIO  
  initBME();      // Connect to BME

  // Initialize wifi and MQTT
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onwifiDisconnect);
  wifiConnectHandler = WiFi.onStationModeGotIP(onwifiConnect);
  connectToWifi();

  // Set timers for BME-read and Reboot-check
  initReboot();
  bmeTimerID = timer.setInterval(bmeInterval, publishBME);
}

void loop() { timer.run(); }