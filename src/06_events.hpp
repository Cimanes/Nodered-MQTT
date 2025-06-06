//======================================================
// VARIABLES
//======================================================
const size_t PRINT_LEN = 8;       // Size limit for mqtt received "payload" (used to limit serial.print)

struct Handler {                  // Handler structure to manage handlers
  const char* topic;    
  void (*handler)(const char* topic, const char* payload);
};
const Handler handlers[] = {      // topic:
  { "gpio/", handleGPIO },        // gpio/#
  { "debug", handleDebug },       // esp/debug
  { "interval", handleInterval }, // bme/interval
  { "read", handleRead },         // bme/read
  { "espIP", handleIP},           // esp/espIP
  { "reboot", handleReboot },     // esp/reboot
  #ifdef WIFI_MANAGER
    { "wifi", handleWifi },       // esp/wifi
  #endif
  #ifdef USE_OTA
    { "OTA", handleOTA }
  #endif
};
const byte handlerCount = sizeof(handlers) / sizeof(handlers[0]);
const uint16_t wifiReconnectTimer = 3000; // Delay to reconnect to Wifi after failed
const uint16_t mqttReconnectTimer = 15000; // Delay to reconnect to Wifi after failed

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
  timer.setTimeout(1000, publishBME);
}

// Attempt re-connect to MQTT when connection is lost
void onmqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (Debug) { Serial.printf_P(PSTR("MQTT Disconnected: %i \n"), (int)reason); }
  if (WiFi.isConnected()) {
    mqttReconnectTimerID = timer.setTimeout(mqttReconnectTimer, []() { mqttClient.connect();});
    if (Debug) Serial.println(F("MQTT re-connecting..."));
  }
}

void onmqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (Debug) Serial.printf_P(PSTR("Sub. #%u \n"), packetId);
}

void onmqttUnsubscribe(uint16_t packetId) {
  if (Debug) Serial.printf_P(PSTR("Unsub. #%u \n"), packetId);
}

void onmqttPublish(uint16_t packetId) {
  if (Debug) Serial.printf_P(PSTR("Pub. #%u \n"), packetId);
}

// Note: expected payload is a C-string. MQTT payload is typically "not pure" --> clean it
// Empty payload or numbers will create problems with serial.print and with handlers.
void onmqttMessage(const char* topic, char* payload, const AsyncMqttClientMessageProperties properties, const size_t len, const size_t index, const size_t total) {
  if (Debug) {
    memcpy(payload + min(len, PRINT_LEN), "\0", 1);  // Ensure it's a valid C-string
    Serial.printf_P(PSTR(">[MQTT] %s: %s\n"), topic, payload);
    // Serial.printf_P(PSTR("qos: %i, dup: %i, retain: %i, len: %i, index: %i, total: %i \n"), properties.qos, properties.dup, properties.retain, len, index, total);
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
  mqttClient.setKeepAlive(MQTT_PING_INTERVAL);
  if (Debug) Serial.println(F("init MQTT done."));
}

//======================================================
// WIFI EVENT FUNCTIONS
//======================================================
void onwifiConnect(const WiFiEventStationModeGotIP& event) {
  if (Debug) { 
    Serial.println(WiFi.localIP());
    Serial.println(F("initWiFi done"));
  }    
  initMqtt();
  if (Debug) Serial.println(F("MQTT Connecting..."));
  mqttClient.connect();
}

void onwifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  if (Debug)   Serial.println(F("Wifi disconnected."));
  timer.deleteTimer(mqttReconnectTimerID);  // avoid reconnect to MQTT while reconnecting to Wi-Fi
  timer.setTimeout(wifiReconnectTimer, connectToWifi);    // attempt to reconnect to WiFi
}