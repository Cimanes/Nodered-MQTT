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
#define PRINT_LEN 100               // Size limit for payload serial.print

AsyncMqttClient mqttClient;         // MQTT Client
unsigned long mqttReconnectTimerID; // Timer to reconnect to MQTT after failed

// Subscribe topic List: Following variables are defined inside function "onMqttConnect"
// static const char* subTopics[] = { "gpio/#", "bme/interval", "bme/read", "esp/reboot", "esp/espIP", "esp/debug", "esp/wifi"  };
// const byte subLen = sizeof(subTopics) / sizeof(subTopics[0]);


//======================================
// FUNCTIONS
//======================================
void connectToMqtt() {
  if (Debug) Serial.println(F("MQTT Connecting..."));
  mqttClient.connect();
}

/**
 * Publish values to MQTT broker with quality of service 1 (at least once delivery).
 * @param topics Array of topic strings to publish to.
 * @param values Array of float values to publish.
 * @param numTopics Number of topics to publish to.
 */

void publishArray(const char* topic, const char* keys[], int16_t values[], byte numKeys) {
  if (Debug) Serial.println(F("Publishing..."));
  const char* payload = makeJsonArray(numKeys, keys, values);           // -> create jsonBuffer
  mqttClient.publish(topic, 1, true, payload);
  if (Debug) { Serial.println(payload); }
}

void publishInt(const char* topic, uint16_t value) {
  static char payload[6];  // Static buffer to reduce stack usage
  // snprintf(payload, sizeof(payload), "%d", value);  // Format integer as string
  itoa(value, payload, 10);
  mqttClient.publish(topic, 1, true, payload);
  if (Debug) {
    Serial.printf("Pub. %s. Msg: %s \n", topic, payload);
  }
}

void publishBool(const char* topic, bool value) {
  static char payload[2];
  payload[0] = value ? '1' : '0';
  payload[1] = '\0';

  mqttClient.publish(topic, 1, true, payload);
  if (Debug) {
    Serial.printf("Pub. %s. Msg: %s \n", topic, payload);
  }
}

void publishBME() {
  readBME();
  publishArray("bme", bmeKeys, bmeValues, numBmeKeys);    
}

// Subscribe 
void onMqttConnect(bool sessionPresent) {
  if(Debug) { Serial.println(F("Subscribing:")); }
  
  static const char* subTopics[] = { "gpio/#", "bme/interval", "bme/read", "esp/reboot", "esp/espIP", "esp/debug", "esp/wifi"  };
  static const byte subLen = sizeof(subTopics) / sizeof(subTopics[0]);

  for (byte i = 0; i < subLen; i++) {
    mqttClient.subscribe(subTopics[i], 2);
    if(Debug) { Serial.println(subTopics[i]); }
  }
}

void onmqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (Debug) { Serial.printf("MQTT Disconnected: %i \n", (int)reason); }
  
  if (WiFi.isConnected()) {
    mqttReconnectTimerID = timer.setTimeout(3000, connectToMqtt);
  }
}

void onmqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (Debug) { Serial.println(F("Sub. acknowledged")); }
}

void onmqttUnsubscribe(uint16_t packetId) {
  if (Debug) { Serial.println(F("Unsub. acknowledged")); }
}

/**
 * Process MQTT messages and performs specific actions based on the topic. 
 * It can control GPIO pins, adjust the BME280 sensor interval, 
 * change the debug level, and trigger BME readings.
 * 
 * @param topic The topic of the incoming MQTT message.
 * @param payload The payload of the MQTT message.
 * @param properties Properties of the MQTT message, such as QoS, duplicated flag, retain flag, etc.
 * @param len The length of the payload.
 * @param index The index of the current message chunk.
 * @param total The total size of the message.
 */

void onmqttMessage(const char* topic, const char* payload, const AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (Debug) {
    static const byte printLen = min(len, size_t(PRINT_LEN));
    char payloadPrint[printLen + 1];
    memcpy(payloadPrint, payload, len);
    payloadPrint[printLen] = '\0';      // Ensure it's a valid C-string
 
    Serial.print(F("Received "));
    Serial.printf("topic: %s, payload: %s\n", topic, payloadPrint);
    // More properties available:
    // Serial.printf("Pub received. \n topic: %s, qos: %i, dup: %i, retain: %i, len: %i, index: %i, total: %i \n", topic, properties.qos, properties.dup, properties.retain, len, index, total);
  }
  // Operate digital outputs
  if (strstr(topic, "gpio")) {
    // Check which GPIO we want to control
    const char* gpioStr = strstr(topic, "gpio/") + 5; // skip "gpio/"
    int gpio = atoi(gpioStr);
    bool out = !strncmp(payload, "1", len);
    digitalWrite(gpio, out);
    mqttClient.publish("fb/gpio", 1, true, out ? "1" : "0");
    return;
  }

  // Change BME280 sensor interval
  else if (strstr(topic, "interval")) {  // topic contains "interval"
    bmeInterval = strtoul(payload, NULL, 10) * 1000;
    if (Debug) {
      Serial.print(F("BME interval: "));
      Serial.println(bmeInterval);
    }
    timer.deleteTimer(BMETimerID);
    publishBME();
    BMETimerID = timer.setInterval(bmeInterval, publishBME);
    return;
  }

  // Change debug level
  else if (strstr(topic, "debug")) {
    Debug = (!strncmp(payload, "1", len));  // set Debug per payload
    mqttClient.publish("fb/debug", 1, true, Debug ? "1" : "0");
    if (Debug) {
      Serial.print(F("Debug: "));
      Serial.println(Debug);
    }
    return;
  }

  // Trigger BME reading
  else if (strstr(topic, "read")) {
    if (Debug) { Serial.println(F("Read BME")); }
    timer.deleteTimer(BMETimerID);
    publishBME();
    BMETimerID = timer.setInterval(bmeInterval, publishBME);
  }

  // Publish ESP IP
  else if (strstr(topic, "espIP")) {
    if (Debug) { Serial.println(F("Pub. ESP IP")); }
    mqttClient.publish("fb/espIP", 1, true, esp_ip);
  }

  // Re-define wifi credentials
  else if (strstr(topic, "wifi")) {
    deleteFile(LittleFS, ssidPath);
    deleteFile(LittleFS, passPath);
    deleteFile(LittleFS, ipPath);
    deleteFile(LittleFS, routerPath);
    deleteFile(LittleFS, hostPath);
    mqttClient.publish("fb/wifi", 1, true, "wifi clear");
  }

  // Reboot the ESP
  else if (strstr(topic, "reboot")) {
    if (Debug) { Serial.print(F("Rebooting")); }    
    reboot = true;
  }
}






// struct Handler {            // Handler structure to manage handlers
//   const char* topic;
//   void (*handler)(StaticJsonDocument<100>&);
// };

// const Handler handlers[] = {
//   { "heater", handleHeater },
//   { "boiler", handleBoiler },
//   { "debug", handleDebug },
//   { "led", handleLED },
//   { "interval", handleInterval },
//   { "espIP", handleIP},
//   { "read", handleRead },
//   #ifdef WIFI_MANAGER
//   { "wifi", handleWifi },
//   #endif
//   { "reboot", handleReboot }
// };
// const byte handlerCount = sizeof(handlers) / sizeof(handlers[0]);

// void processMessage(uint8_t* wsMessage) {
//   // check for error
//   if (deserializeJson(jsonDoc, wsMessage)) {
//     if (Debug) Serial.println(F("Invalid JSON"));
//     return;
//   }
//   const char* topic = jsonDoc["topic"];
//   if (!topic) return;
//   for (byte i = 0; i < handlerCount; i++) {
//     if (strcmp(topic, handlers[i].topic) == 0) {
//       handlers[i].handler(jsonDoc);
//       return;
//     }
//   }
// }








void onmqttPublish(uint16_t packetId) {
  if (Debug) {
    Serial.print(F("Pub OK. #"));
    Serial.println(packetId);
  }
}

// Initialize MQTT
void initMqtt() {         
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onmqttDisconnect);
  mqttClient.onSubscribe(onmqttSubscribe);
  mqttClient.onUnsubscribe(onmqttUnsubscribe);
  mqttClient.onMessage(onmqttMessage);
  mqttClient.onPublish(onmqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(BROKER_USER, BROKER_PASS);
  if (Debug) Serial.println(F("init MQTT done"));
}