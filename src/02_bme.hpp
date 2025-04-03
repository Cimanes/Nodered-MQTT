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
float values[3];

//======================================
// FUNCTIONS
//======================================
void initBME() {      // Initialize BME280
  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find BME280 sensor, check wiring!"));
    while (1);
  }
}

void readBME() {      // Read data from BME280
    values[0]= bme.readTemperature();
    values[1]= bme.readHumidity();
    values[2]= bme.readPressure()/100.0F;
    if (espDebug) Serial.println(values[0] + values[1] + values[2]);
}
