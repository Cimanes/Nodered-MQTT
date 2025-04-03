//======================================
// LIBRARIES
//======================================
#include <AsyncMqttClient.h>

//======================================
// VARIABLES
//======================================
#define MQTT_HOST IPAddress(172, 16, 14, 193) //MQTT BROKER IP ADDRESS
#define MQTT_PORT 1883                        // MQTT BROKER PORT
#define BROKER_USER "cimanes"                 // MQTT BROKER USER
#define BROKER_PASS "Naya-1006"               // MQTT BROKER PASSWORD

AsyncMqttClient mqttClient;

// Pub Topics array
const char* pubTopics[] = {
  "esp/bme280/temperature",
  "esp/bme280/humidity",
  "esp/bme280/pressure"
};

// Sub Topics
#define MQTT_SUB_DEBUG "esp/debug"
#define MQTT_SUB_DIGITAL "esp/digital/#"
#define MQTT_SUB_INTERVAL "bme/interval"
#define MQTT_SUB_TRIGGER "bme/trigger"

//======================================
// FUNCTIONS
//======================================
void connectToMqtt() {
  if (espDebug) Serial.println(F("Connecting to MQTT..."));
  mqttClient.connect();
}

void publishMqtt(const char* topics[], float values[], int numTopics) {
  static char payload[8];  // Static buffer to reduce stack usage

  for (int i = 0; i < numTopics; i++) {
    snprintf(payload, sizeof(payload), "%.2f", values[i]);  // Format float as string
    uint16_t packetId = mqttClient.publish(topics[i], 1, true, payload);
    if (espDebug) {
      Serial.printf(PSTR("Publishing on topic %s at QoS 1, packetId: %i\n"), topics[i], packetId);
      Serial.printf(PSTR("Message: %s \n"), payload);
    }
  }
}

// Subscribe 
void onmqttConnect(bool sessionPresent) {
  if(espDebug) {
    Serial.println(F("Connected to MQTT."));
    Serial.print(F("Session present: "));
    Serial.println(sessionPresent);
  }
  const char* subTopics[] = {
    MQTT_SUB_DIGITAL,
    MQTT_SUB_INTERVAL,
    MQTT_SUB_DEBUG,
    MQTT_SUB_TRIGGER
  };
  const byte subLen = sizeof(subTopics) / sizeof(subTopics[0]);
  for (byte i = 0; i < subLen; i++) {
    uint16_t packetIdSub = mqttClient.subscribe(subTopics[i], 2);
    if(espDebug) {
      Serial.print(F("Subscribing at QoS 2, packetId: "));  
      Serial.println(packetIdSub);
    }
  }
}

void onmqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (espDebug) Serial.print(F("Disconnected from MQTT"));
  if (WiFi.isConnected()) {
    mqttReconnectTimerID = timer.setTimeout(2000, connectToMqtt); // Option: SimpleTimer
  }
}

void onmqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (espDebug) {
    Serial.println(F("Subscribe acknowledged."));
    Serial.print(F("  packetId: "));
    Serial.println(packetId);
    Serial.print(F("  qos: "));
    Serial.println(qos);
  }
}

void onmqttUnsubscribe(uint16_t packetId) {
  if (espDebug) {
    Serial.println(F("Unsubscribe acknowledged."));
    Serial.print(F("  packetId: "));
    Serial.println(packetId); 
  }
}

void onmqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (espDebug) {
    Serial.println(F("Publish received."));
    Serial.print(F("  topic: "));
    Serial.println(topic);
    Serial.print(F("  qos: "));
    Serial.println(properties.qos);
    Serial.print(F("  dup: "));
    Serial.println(properties.dup);
    Serial.print(F("  retain: "));
    Serial.println(properties.retain);
    Serial.print(F("  len: "));
    Serial.println(len);
    Serial.print(F("  index: "));
    Serial.println(index);
    Serial.print(F("  total: "));
    Serial.println(total);
  }
  // Operate digital outputs
  if (strstr(topic, "digital") != NULL) {
    // Check which GPIO we want to control
    const char* gpioStr = strstr(topic, "/digital/") + 9; // skip "/digital/"
    int gpio = atoi(gpioStr);
    if (espDebug) {
      Serial.print(F("DIGITAL GPIO: "));
      Serial.println(gpio);
    }
    pinMode(gpio, OUTPUT);
    if (!strncmp(payload, "true", len))  { digitalWrite(gpio, HIGH); }  // payload == "ture"
    else  { digitalWrite(gpio, LOW); }  // payload is not equal to "true"
    return;
  }

  // Change BME280 sensor interval
  else if (!!strstr(topic, "interval")) {  // topic contains "interval"
    interval = strtoul(payload, NULL, 10) * 1000;
    if (espDebug) {
      Serial.print(F("BME interval: "));
      Serial.println(interval);
    }
    timer.deleteTimer(BMETimerID);
    BMETimerID = timer.setInterval(interval, []() {
      readBME();
      publishMqtt(pubTopics, values, sizeof(pubTopics)/sizeof(pubTopics[0]));
    });
    return;
  }

  // Change debug level
  else if (!!strstr(topic, "debug")) {
    espDebug = (!strncmp(payload, "true", len));  // set espDebug per payload
    if (espDebug) {
      Serial.print(F("Debug level: "));
      Serial.println(espDebug);
    }
    return;
  }

  // Trigger BME reading
  else if (!!strstr(topic, "trigger")) {
    if (espDebug) { Serial.print(F("BME reading triggered")); }
    timer.deleteTimer(BMETimerID);
    readBME();
    publishMqtt(pubTopics, values, sizeof(pubTopics)/sizeof(pubTopics[0]));
    BMETimerID = timer.setInterval(interval, []() {
      readBME();
      publishMqtt(pubTopics, values, sizeof(pubTopics)/sizeof(pubTopics[0]));
    });
  }
}

void onmqttPublish(uint16_t packetId) {
  if (espDebug) {
    Serial.println(F("Publish acknowledged."));
    Serial.print(F("  packetId: "));
    Serial.println(packetId);
  }
}

void initMqtt() {         // Initialize MQTT
  mqttClient.onConnect(onmqttConnect);
  mqttClient.onDisconnect(onmqttDisconnect);
  mqttClient.onSubscribe(onmqttSubscribe);
  mqttClient.onUnsubscribe(onmqttUnsubscribe);
  mqttClient.onMessage(onmqttMessage);
  mqttClient.onPublish(onmqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(BROKER_USER, BROKER_PASS);
  if (espDebug) Serial.println(F("initMQTT done"));
}

