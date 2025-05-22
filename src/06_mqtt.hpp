//======================================
// LIBRARIES
//======================================
#include <AsyncMqttClient.h>

//======================================
// VARIABLES
//======================================
#define MQTT_HOST hostIP          // MQTT BROKER IP ADDRESS (= Node red server IP)
#define MQTT_PORT 1883            // MQTT BROKER PORT
#define BROKER_USER "cimanes"     // MQTT BROKER USER
#define BROKER_PASS "Naya-1006"   // MQTT BROKER PASSWORD
#define PRINT_LEN 100             // Size limit for payload serial.print

AsyncMqttClient mqttClient;         // MQTT Client
unsigned long mqttReconnectTimerID; // Timer to reconnect to MQTT after failed
// const char* subTopics[] = { "gpio/#", "bme/interval", "bme/read", "esp/reboot", "esp/espIP", "esp/debug", "esp/wifi"  };
const char* subTopics[] = { "gpio/#", "bme/#", "esp/#" };
const byte subLen = sizeof(subTopics) / sizeof(subTopics[0]);
static char topicOut[16];  // "fb/" = 4 + null terminator


//======================================
// FUNCTIONS
//======================================
/**
 * Publish values to MQTT broker with quality of service 1 (at least once delivery).
 * @param topics Array of topic strings to publish to.
 * @param values Array of float values to publish.
 * @param numTopics Number of topics to publish to.
 */

void publishArray(const char* topic, const char* keys[], int16_t values[], byte numKeys) {
  if (Debug) Serial.println(F("Publishing..."));
  makeJsonArray(numKeys, keys, values);           // -> create jsonBuffer
  mqttClient.publish(topic, 1, true, jsonBuffer);
  if (Debug)  Serial.println(jsonBuffer);
}

void publishInt(const char* topic, int16_t value) {
  static char payload[6];  // Static buffer to reduce stack usage
  // snprintf(payload, sizeof(payload), "%d", value);  // Format integer as string
  itoa(value, payload, 10);
  mqttClient.publish(topic, 1, true, payload);
  if (Debug)  Serial.printf_P(PSTR("Pub. %s: %s \n"), topic, payload);
}

void publishBool(const char* topic, bool value) {
  static char payload[2];
  payload[0] = value ? '1' : '0';
  payload[1] = '\0';
  mqttClient.publish(topic, 1, true, payload);
  if (Debug) Serial.printf_P(PSTR("Pub. %s: %s \n"), topic, payload);
}

void publishBME() {
  readBME();
  publishArray("reading", bmeKeys, bmeValues, numBmeKeys);
}

// Operate digital outputs
void handleGPIO(const char* topic, const char* payload) {
  for (byte i = 0; i < gpioCount; i++) {
    if (strstr(topic, gpioPins[i].topic)) {
      digitalWrite(gpioPins[i].gpio, payload[0] == '1');
      sprintf(topicOut, "fb/%s", gpioPins[i].topic);
      mqttClient.publish(topicOut, 1, true, digitalRead(gpioPins[i].gpio) ? "1" : "0");
      if (Debug) Serial.printf_P(PSTR("Pub. %s: %s\n"), topicOut, digitalRead(gpioPins[i].gpio) ? "1" : "0");
      return;
    }
  }
}

void handleDebug(const char* topic, const char* payload) { 
  Debug = payload[0] == '1';
  mqttClient.publish("fb/debug", 1, true, Debug ? "1" : "0"); 
  if (Debug) Serial.printf_P(PSTR("Debug: %s\n"), Debug ? "1" : "0"); 
}

void handleInterval(const char* topic, const char* payload) {
  if (Debug) Serial.printf_P(PSTR("BME interval: %d\n"), bmeInterval);
  timer.deleteTimer(BMETimerID);
  bmeInterval = strtoul(payload, NULL, 10) * 1000;
  publishBME();
  BMETimerID = timer.setInterval(bmeInterval, publishBME);

  static char intervalStr[12];                // Enough for unsigned long (max 10 digits + null)
  ultoa(bmeInterval/1000 , intervalStr, 10);  // Convert to base-10 string
  mqttClient.publish("fb/interval", 1, true, intervalStr);
  if (Debug) Serial.printf_P(PSTR("Interval: %d\n"), bmeInterval);
}

void handleRead(const char* topic, const char* payload) {
  timer.deleteTimer(BMETimerID);
  publishBME();
  BMETimerID = timer.setInterval(bmeInterval, publishBME);
}

void handleIP(const char* topic, const char* payload) {
  if (Debug) Serial.printf_P(PSTR("Pub. espIP: %s\n"), esp_ip);
  mqttClient.publish("fb/espIP", 1, true, esp_ip);
}

void handleReboot(const char* topic, const char* payload) {
  if (Debug) Serial.println(F("Reboot requested"));
  reboot = true;
}

void handleWifi(const char* topic, const char* payload) {
    deleteFile(LittleFS, ssidPath);
    deleteFile(LittleFS, passPath);
    deleteFile(LittleFS, ipPath);
    deleteFile(LittleFS, routerPath);
    deleteFile(LittleFS, hostPath);  
    mqttClient.publish("fb/wifi", 1, true, "clear");
}