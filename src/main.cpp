//======================================
// LIBRARIES
//======================================
#include "00_globals.hpp"
#include "01_json.hpp"
#include "02_fileSys.hpp"
#include "03_wifi.hpp"
#include "04_bme.hpp"
#include "05_mqtt.hpp"
#include "06_events.hpp"

//======================================
// SETUP
//======================================
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
  bmeTimerID = timer.setInterval(1000 * bmeInterval, publishBME);
}

//======================================
// LOOP
//======================================
void loop() { timer.run(); }