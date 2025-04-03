//======================================
// LIBRARIES
//======================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SimpleTimer.h>

//======================================
// VARIABLES
//======================================
// Define wifi credentials
#define WIFI_SSID "InRoomWiFi"
#define WIFI_PASSWORD ""
boolean espDebug = true;

// Define WiFi event handlers and Ticker/SimpleTimer object to manage reconnection
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
SimpleTimer timer;
unsigned long BMETimerID;

// Global variables to control timers
unsigned long mqttReconnectTimerID;   // Timer to reconnect to MQTT after failed
unsigned long wifiReconnectTimerID;   // TImer to reconnect to WiFi after failed
unsigned long interval = 300000; // Interval at which to publish values