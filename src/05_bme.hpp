//======================================
// LIBRARIES
//======================================
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//======================================
// VARIABLES
//======================================
Adafruit_BME280 bme;    // BME280 object
bool bmeConnected = false;
uint16_t bmeTimerID;    // Timer ID for BME periodical readings
unsigned long bmeInterval = 30000;   // BME interval 
const char* bmeKeys[] = { "temp", "hum", "pres", "ok" };
const byte numBmeKeys = sizeof(bmeKeys) / sizeof(bmeKeys[0]);
int16_t bmeValues[numBmeKeys];   // Array with BME sensor readings

//======================================
// FUNCTIONS
//======================================
void initBME() {      // Connect to sensor
  if (!bme.begin(0x76)) {
    if (Debug) Serial.println(F("BME not found!"));
    return;
  }
  bmeConnected = true;
  if (Debug) Serial.println(F("BME found"));
}

void readBME() {      // Read data from BME280
  if (Debug) Serial.println(F("Reading BME"));
  bmeValues[0] = round(bme.readTemperature() * 10);
  bmeValues[1] = round(bme.readHumidity() * 10);
  bmeValues[2] = round(bme.readPressure() / 10);
  if (isnan(bmeValues[0]) || isnan(bmeValues[1]) || isnan(bmeValues[2])) {  // Check if any reading is NaN
    if (Debug) Serial.println(F("BME lost!. Reconnecting."));
    bmeConnected = false;
    bmeValues[3] = 0;
    initBME();
  } 
  else bmeValues[3] = 1;
}