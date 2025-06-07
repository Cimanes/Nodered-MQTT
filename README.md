## Introduction

### Components
1) Node-red server running in PC / Raspberry pi.
2) ESP8266 with BME280 and digital outputs available.

### Communication
Connection between the ESP and Nodered via MQTT, using port 1883 of server.
The PC / Raspberry pi acts as server; The ESP is a client.
Ensure ports 1883 is open; if required, add a new inbound rule in Windows Defender Firewall - Advanced settings.

## ESP8266
Coded in Arduino enviroment using VS code.
Following libraries are used: 

- [SimpleTimer](https://github.com/jfturcot/SimpleTimer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [ESP Async Webserver](https://github.com/ESP32Async/ESPAsyncWebServer) 
- [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [AsyncMqttClient](https://github.com/marvinroger/async-mqtt-client)
- [AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA)
- [ESPAsyncTCP](https://github.com/ESP32Async/ESPAsyncTCP) --> Used by AsyncMqttClient and ESP Asynch Webserver
  
### Wifi Connection
- Option 1: Wifi Manager (enable line "#define WIFI_MANAGER")
  Wifi credentials are stored as text files in LittleFS.

  If files do not exist, the ESP will reboot as server and prompt the user for credentials.
  The user needs to connect to "ESP WIFI MANAGER". 
  Then open the url 192.168.4.1 in a browser and enter the correct credentials.
  Once the credentials are entered, the ESP will reboot and connect to the Wifi as client.

* Option 2: Fixed Wifi credentials (comment line "#define WIFI_MANAGER")
Wifi credentials need to be hard coded in wifi.hpp.
  
## Node-red
Tree flows are present:
- BME readings: displays current and historic BME signals (P, S.L.P., T, RH, time).
- Console: user can monitor and operate GPIO's, thermostat and ESP (Reboot, OTA update...).
- Console feedback: receive actual status of GPIO's and BME. 

### BME Readings
The ESP periodically reads pressure, temperature and humidity and sends those values to Node-red.
When Node red receives the signals, and each of them feeds a chart and a dial gauge.
Pressure is converted to Sea Level Pressure (SLP)- Local elevation needed.

## MQTT Broker
Using mosquitto broker. Configuration file "mosquitto.conf" with following settings:
  per_listener_settings true        # Each listener has its own credentials
  allow_anonymous false             # Request user and password (true to skip)
  listener 1883                     # Connection port
  password_file /mosquitto/passwd   # Password file to store credentials
