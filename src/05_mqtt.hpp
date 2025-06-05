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
#define MQTT_PING_INTERVAL 30     // MQTT "Keep-Alive" interval (s)

AsyncMqttClient mqttClient;       // MQTT Client
const char* subTopics[] = { "gpio/#", "bme/#", "esp/#" };

const byte subLen = sizeof(subTopics) / sizeof(subTopics[0]);
static char topicOut[16];  // "fb/" = 4 + null terminator
unsigned long mqttReconnectTimerID      ; // Timer to reconnect to MQTT after failed

//======================================
// FUNCTIONS
//======================================

// void publishArray(const char* topic, const char* keys[], int16_t values[], byte numKeys) {
//   if (Debug) Serial.println(F("Publishing..."));
//   makeJsonArray(numKeys, keys, values);           // -> create strBuffer
//   mqttClient.publish(topic, 1, true, strBuffer);
//   if (Debug)  Serial.println(strBuffer);
// }

// void publishInt(const char* topic, int16_t value) {
//   static char payload[6];  // Static buffer to reduce stack usage
//   // snprintf(payload, sizeof(payload), "%d", value);  // Format integer as string
//   itoa(value, payload, 10);
//   mqttClient.publish(topic, 1, true, payload);
//   if (Debug)  Serial.printf_P(PSTR("Pub. %s: %s \n"), topic, payload);
// }

// void publishBool(const char* topic, bool value) { 
//   const char* payload = value ? "1" : "0";
//   mqttClient.publish(topic, 1, true, payload);
//   if (Debug) Serial.printf_P(PSTR("Pub. %s: %s \n"), topic, payload);
// }

void publishBME() {
  readBME();
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  makeJsonArray(numBmeKeys, bmeKeys, bmeValues);   // -> create strBuffer
  mqttClient.publish("fb/bme", 1, false, strBuffer);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//======================================
// HANDLERS: process incoming messages
//======================================
// Operate digital outputs
void handleGPIO(const char* topic, const char* payload) {
  for (byte i = 0; i < gpioCount; i++) {
    if (strstr(topic, gpioPins[i].topic)) {
      digitalWrite(gpioPins[i].gpio, payload[0] == '1');
      sprintf(topicOut, "fb/%s", gpioPins[i].topic);
      mqttClient.publish(topicOut, 1, true, digitalRead(gpioPins[i].gpio) ? "1" : "0");
      if (Debug) Serial.printf_P(PSTR("[MQTT]> %s: %s\n"), topicOut, digitalRead(gpioPins[i].gpio) ? "1" : "0");
      return;
    }
  }
}

void handleRead(const char* topic, const char* payload) {
  timer.deleteTimer(bmeTimerID);
  publishBME();
  bmeTimerID = timer.setInterval(1000 * bmeInterval, publishBME);
}

void handleInterval(const char* topic, const char* payload) {
  bmeInterval = atoi(payload);
  timer.deleteTimer(bmeTimerID);
  publishBME();
  bmeTimerID = timer.setInterval(1000 * bmeInterval, publishBME);

  static char intervalStr[6];           // Length for uint16_t (max 5 digits + null)
  itoa(bmeInterval , intervalStr, 10);  // Convert to base-10 string
  mqttClient.publish("fb/interval", 1, false, intervalStr);
  if (Debug) Serial.printf_P(PSTR("[MQTT]> Interval: %d\n"), bmeInterval);
}

void handleIP(const char* topic, const char* payload) {
  if (Debug) Serial.printf_P(PSTR("[MQTT]> espIP: %s\n"), esp_ip);
  mqttClient.publish("fb/espIP", 1, false, esp_ip);
}

void handleDebug(const char* topic, const char* payload) { 
  Debug = payload[0] == '1';
  mqttClient.publish("fb/debug", 1, false, Debug ? "1" : "0"); 
  if (Debug) Serial.printf_P(PSTR("[MQTT]> Debug: %s\n"), Debug ? "1" : "0"); 
}

void handleReboot(const char* topic, const char* payload) {
  if (Debug) Serial.println(F("Rebooting"));
  mqttClient.disconnect();
  #if defined(ESP32)  
    timer.setTimeout(3000, []() { esp_restart(); } );
  #elif defined(ESP8266)
    timer.setTimeout(3000, []() { ESP.restart(); } );
  #endif
}

#ifdef WIFI_MANAGER
  void handleWifi(const char* topic, const char* payload) {
      deleteFile(LittleFS, ssidPath);
      deleteFile(LittleFS, passPath);
      deleteFile(LittleFS, ipPath);
      deleteFile(LittleFS, routerPath);
      deleteFile(LittleFS, hostPath);  
      mqttClient.publish("fb/wifi", 1, false, "clear");
  }
#endif

#ifdef USE_OTA
  void handleOTA(const char* topic, const char* payload) {
    if(Debug) Serial.println(F("OTA requested"));
    timer.deleteTimer(mqttReconnectTimerID);  // avoid reconnect to MQTT while reconnecting to Wi-Fi
    mqttClient.disconnect();
    startOTAServer();
  }
#endif