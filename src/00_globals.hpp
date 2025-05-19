//======================================
// LIBRARIES
//======================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SimpleTimer.h>

//======================================
// VARIABLES
//======================================
// Define wifi credentials
#define wifiManager
boolean Debug = true;
boolean reboot = false;

#define HEATER_PIN 13             // Pin used for heater signal
#define BOILER_PIN 15             // Pin used for boiler signal

// Define WiFi event handlers and Ticker/SimpleTimer object to manage reconnection
SimpleTimer timer;
unsigned long BMETimerID;

// Global variables to control timers
unsigned long bmeInterval = 60000;   // Interval at which to read BME and publish values

// =====================================
// Setup GPIO's
//======================================
void initGPIO() {
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(BOILER_PIN, OUTPUT);
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW); 
  digitalWrite(BOILER_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
}
