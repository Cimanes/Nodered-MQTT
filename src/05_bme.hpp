//======================================
// LIBRARIES
//======================================
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//======================================
// VARIABLES
//======================================
Adafruit_BME280 bme;    // BME280 object
int16_t bmeValues[3];   // Array with global variables to hold sensor readings
uint16_t BMETimerID;    // Timer ID for BME periodical readings
unsigned long bmeInterval = 30000;   // BME interval 
// Array with BME topics 
const char* bmeKeys[] = { "temp", "hum", "pres" };
const byte numBmeKeys = sizeof(bmeKeys) / sizeof(bmeKeys[0]);

//======================================
// FUNCTIONS
//======================================
void initBME() {      // Connect to sensor
  if (!bme.begin(0x76)) {
    Serial.println(F("BME not found, check wiring!"));
    while (1);
  }
  if (Debug) Serial.println(F("BME found"));
}

void readBME() {      // Read data from BME280
  if (Debug) Serial.println(F("Reading BME"));
  bmeValues[0] = round(bme.readTemperature() * 10);
  bmeValues[1] = round(bme.readHumidity() * 10);
  bmeValues[2] = round(bme.readPressure() / 10);
}