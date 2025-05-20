//======================================
// GLOBAL LIBRARIES
//======================================
#include <Arduino.h>
#include <SimpleTimer.h>

//======================================
// GLOBAL VARIABLES
//======================================
boolean Debug = true;
boolean reboot = false;
#define HEATER_PIN 13     // Pin used for heater signal
#define BOILER_PIN 15     // Pin used for boiler signal
SimpleTimer timer;        // SimpleTimer object
unsigned int rebootTimer; // Timer for reboot

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
