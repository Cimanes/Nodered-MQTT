//======================================
// LIBRARIES
//======================================
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//======================================
// VARIABLES
//======================================
// BME280 object
Adafruit_BME280 bme;
// Array with global variables to hold sensor readings
int16_t bmeValues[3];

//======================================
// FUNCTIONS
//======================================
void initBME() {      
  if (!bme.begin(0x76)) {
    Serial.println(F("BME not found, check wiring!"));
    while (1);
  }
  if (Debug) Serial.println(F("BME init OK"));
}

void readBME() {      // Read data from BME280
  if (Debug) Serial.println(F("Reading BME"));
  bmeValues[0] = round(bme.readTemperature() * 10);
  bmeValues[1] = round(bme.readHumidity() * 10);
  bmeValues[2] = round(bme.readPressure() / 10);
}