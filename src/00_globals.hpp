//======================================
// GLOBAL LIBRARIES
//======================================
#include <Arduino.h>
#include <SimpleTimer.h>

//======================================
// GLOBAL VARIABLES
//======================================
SimpleTimer timer;        // SimpleTimer object
boolean Debug = true;

// struct to assign GPIO pins for each topic
struct pinMap { const char* topic; const byte gpio; const bool value; }; 
const pinMap gpioPins[] = {       
  { "led", LED_BUILTIN, true },
  { "heater", 13 , false}, 
  { "boiler", 15, false }
};
const byte gpioCount = sizeof(gpioPins) / sizeof(gpioPins[0]);

// =====================================
// Setup GPIO's
// //======================================
void initGPIO() {
  for (byte i = 0; i < gpioCount; i++) {
    pinMode(gpioPins[i].gpio, OUTPUT);
    digitalWrite(gpioPins[i].gpio, gpioPins[i].value);
  }
}