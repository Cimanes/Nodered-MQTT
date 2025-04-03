#include "01_globals.hpp"
#include "02_bme.hpp"
#include "03_mqtt.hpp"

//=======================================
//Handle WiFi events
//=======================================
void connectToWifi() {
  if(espDebug) Serial.println(F("Connecting to Wi-Fi..."));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onwifiConnect(const WiFiEventStationModeGotIP& event) {
  if(espDebug) {
    if(espDebug) {    
      Serial.print(F("Connected to Wi-Fi. IP: "));
      Serial.println(WiFi.localIP());
    }
  }
  connectToMqtt();
}

void onwifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  if (espDebug)   Serial.println(F("Disconnected from Wi-Fi."));
  timer.deleteTimer(mqttReconnectTimerID);  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimerID = timer.setTimeout(2000, connectToWifi); // attempt to reconnect to WiFi
}
//============================================



void setup() {
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  wifiConnectHandler = WiFi.onStationModeGotIP(onwifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onwifiDisconnect);

  initMqtt();
  initBME();
  connectToWifi();
  BMETimerID = timer.setInterval(interval, []() {
    readBME();
    publishMqtt(pubTopics, values, sizeof(pubTopics)/sizeof(pubTopics[0]));    
  });
}

void loop() {
  timer.run();
}