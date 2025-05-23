//======================================================
// VARIABLES
//======================================================
struct Handler {            // Handler structure to manage handlers
  const char* topic;
  void (*handler)(const char* topic, const char* payload);
};
const Handler handlers[] = {
  { "gpio/", handleGPIO },        // gpio/#
  { "debug", handleDebug },       // esp/debug
  { "interval", handleInterval }, // bme/interval
  { "read", handleRead },         // bme/read
  { "espIP", handleIP},           // esp/espIP
  { "reboot", handleReboot },     // esp/reboot
  #ifdef WIFI_MANAGER
  { "wifi", handleWifi }          // esp/wifi
  #endif
};
const byte handlerCount = sizeof(handlers) / sizeof(handlers[0]);
unsigned long mqttReconnectTimerID; // Timer to reconnect to MQTT after failed

//======================================================
// MQTT EVENT FUNCTIONS
//======================================================

// Subscribe once MQTT is connected
void onMqttConnect(bool sessionPresent) {
  if(Debug) Serial.println(F("Subscribing:"));
  for (byte i = 0; i < subLen; i++) {
    mqttClient.subscribe(subTopics[i], 2);
    if(Debug) Serial.println(subTopics[i]);
  }
}

// Attempt re-connect to MQTT when connection is lost
void onmqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (Debug) { Serial.printf_P(PSTR("MQTT Disconnected: %i \n"), (int)reason); }
  if (WiFi.isConnected()) {
    if (Debug) Serial.println(F("MQTT re-connecting..."));
    mqttReconnectTimerID = timer.setTimeout(3000, []() {mqttClient.connect();});
  }
}

void onmqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (Debug) Serial.println(F("Sub. acknowledged"));
}

void onmqttUnsubscribe(uint16_t packetId) {
  if (Debug) Serial.println(F("Unsub. acknowledged"));
}

void onmqttPublish(uint16_t packetId) {
  if (Debug) Serial.printf_P(PSTR("Pub OK. #%u \n"), packetId);
}

void onmqttMessage(const char* topic, const char* payload, const AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (Debug) {
    static const byte printLen = min(len, size_t(PRINT_LEN));
    char payloadPrint[printLen + 1];
    memcpy(payloadPrint, payload, len);
    payloadPrint[printLen] = '\0';      // Ensure it's a valid C-string
 
    Serial.print(F("Received "));
    Serial.printf_P(PSTR("topic: %s, payload: %s\n"), topic, payloadPrint);
    // More properties available:
    // Serial.printf_P(PSTR("Pub received. \n topic: %s, qos: %i, dup: %i, retain: %i, len: %i, index: %i, total: %i \n"), topic, properties.qos, properties.dup, properties.retain, len, index, total);
  }
  // Process MQTT messages according to handlers
  for (byte i = 0; i < handlerCount; i++) {
    if (strstr(topic, handlers[i].topic)) {
      handlers[i].handler(topic, payload);
      return;
    }
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
  if (Debug) Serial.println(F("init MQTT done."));
}

//======================================================
// WIFI EVENT FUNCTIONS
//======================================================
void onwifiConnect(const WiFiEventStationModeGotIP& event) {
  initMqtt();
  if (Debug) Serial.println(F("MQTT Connecting..."));
  mqttClient.connect();
}

void onwifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  if (Debug)   Serial.println(F("Wifi disconnected."));
  timer.deleteTimer(mqttReconnectTimerID);  // avoid reconnect to MQTT while reconnecting to Wi-Fi
  timer.setTimeout(2000, connectToWifi);    // attempt to reconnect to WiFi
}