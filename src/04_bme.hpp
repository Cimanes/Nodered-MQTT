//======================================
// LIBRARIES
//======================================
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//======================================
// VARIABLES
//======================================
Adafruit_BME280 bme       ;  // BME280 object
uint16_t bmeTimerID       ;  // Timer ID for BME periodical readings
uint16_t bmeInterval = 900;  // Interval to publish values (s)

// Arrays with BME key-value pairs
const char* bmeKeys[] = { "temp", "hum", "pres", "err" };
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
  if (Debug) Serial.println(F("BME found"));
}

// Read data from BME280, round to 1 decimal
void readBME() {
  // Define values with *10 factor (round to one decimal)
  // Pressure is converted to mbar and then applied *10 factor
  if (Debug) Serial.println(F("Reading BME"));
  bmeValues[0] = round(bme.readTemperature() * 10);
  bmeValues[1] = round(bme.readHumidity() * 10);
  bmeValues[2] = round(bme.readPressure() / 10);

  if (isnan(bmeValues[0]) || isnan(bmeValues[1]) || isnan(bmeValues[2])) {  // Check if any reading is NaN
    if (Debug) Serial.println(F("BME lost!. Reconnecting."));
    bmeValues[3] = 1;
    initBME();
    return;
  }
  if (bmeValues[0] > 600 || bmeValues[0] < -300 || bmeValues[1] > 1000 
    || bmeValues[1] < 0 || bmeValues[2] < 8000 || bmeValues[2] > 11000)   {  // Check if any reading is NaN
    if (Debug) Serial.println(F("BME bad readings!"));
    bmeValues[3] = 2;
    return;
  } 
  else bmeValues[3] = 0;
}