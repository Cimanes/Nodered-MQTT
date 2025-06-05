// =============================================
// OPTIONS 
// =============================================
#define WIFI_MANAGER    // Use wifi manager with selection of SSID & IP address
#define USE_OTA         // Use OTA updates
#define TOLEDO          // Choose Wifi credentials [CIMANES, TOLEDO, TRAVEL]
#define SSID_TRAVEL "John-Rs-Foodhall_EXT"  //Enter SSID for travel wifi
#define PASS_TRAVEL "sive2017"              //Enter password for travel wifi

//======================================
// GLOBAL LIBRARIES
//======================================
#include <Arduino.h>
#include <SimpleTimer.h>

//======================================
// GLOBAL VARIABLES
//======================================
boolean Debug = true;
SimpleTimer timer;          // SimpleTimer object

// =====================================
// Setup GPIO's
// ======================================
// struct to assign GPIO pins for each topic
struct pinMap { const char* topic; const byte gpio; const bool value; }; 
const pinMap gpioPins[] = {       
  { "led", LED_BUILTIN, HIGH },
  { "heater", 13 , LOW }, 
  { "boiler", 15, LOW }
};
const byte gpioCount = sizeof(gpioPins) / sizeof(gpioPins[0]);


void initGPIO() {
  for (byte i = 0; i < gpioCount; i++) {
    pinMode(gpioPins[i].gpio, OUTPUT);
    digitalWrite(gpioPins[i].gpio, gpioPins[i].value);
  }
}