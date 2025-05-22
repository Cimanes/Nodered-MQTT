// =============================================
// LIBRARIES
// =============================================
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#if defined(ESP32)
  #include <WiFi.h>
  // #include <AsyncTCP.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  // #include <ESPAsyncTCP.h>
#endif
#ifdef useOTA
  #include <AsyncElegantOTA.h>
#endif

// =============================================
// GLOBAL VARIABLES
// =============================================
// Create AsyncWebServer object on port 80 and attach event handlers:
AsyncWebServer server(80)               ;   // Required for HTTP 
WiFiEventHandler wifiConnectHandler     ;   // Event handler for wifi connection
WiFiEventHandler wifiDisconnectHandler  ;   // Event handler for wifi disconnection
#define WIFI_MANAGER 

#if defined(WIFI_MANAGER)
  // =============================================
  // Wifi Manager Variables: set SSID, Password and IP address
  // =============================================
  // Search for parameter in HTTP POST request received from user
  // const char* PARAM_INPUTS[5] = { "ssid", "pass", "ip", "router", "host" };
  const char* PARAM_INPUT_1 = "ssid"  ;
  const char* PARAM_INPUT_2 = "pass"  ;
  const char* PARAM_INPUT_3 = "ip"    ;
  const char* PARAM_INPUT_4 = "router";
  const char* PARAM_INPUT_5 = "host"  ;

  //Variables to save values from HTML form
  char ssid[paramSize]  ;
  char pass[paramSize]  ;
  char esp_ip[paramSize];
  char router[paramSize];
  char host[paramSize]  ;

  // File paths to save input values permanently
  // const char* paramPaths[5] = { "/ssid.txt", "/pass.txt", "/ip.txt", "/router.txt", "/host.txt" };
  const char* ssidPath = "/ssid.txt"    ;
  const char* passPath = "/pass.txt"    ;
  const char* ipPath = "/ip.txt"        ;
  const char* routerPath = "/router.txt";
  const char* hostPath = "/host.txt"    ;

  IPAddress hostIP;             // User entry for node-red server IP

#else
  // =============================================
  // Hardcoded Wifi Variables: Credentials. 
  // =============================================
  #define TOLEDO      // OPTIONAL: Choose Wifi credentials [CIMANES, TOLEDO, TRAVEL]
  #if defined(CIMANES)
    const char ssid[] = "Pepe_Cimanes";
    const char pass[] = "Cimanes7581" ;
  #elif defined(TOLEDO)
    const char ssid[] = "MIWIFI_HtR7" ;
    const char pass[] = "TdQTDf3H"    ;
  #elif defined(TRAVEL)
    const char ssid[] = "John-Rs-Foodhall_EXT" ;
    const char pass[] = "sive2017"    ;
  #endif

  const char* esp_ip = "192.168.1.213";
  const IPAddress hostIP(192, 168, 1, 133);   // Had coded Node-red server IP    WiFi.mode(WIFI_STA);
#endif

#if defined (WIFI_MANAGER)
// =============================================
// Wifi Manager Functions
// =============================================

// Get SSID, Password and IP address from LittleFS files
  void getWiFi() {
    fileToCharPtr(LittleFS, ssidPath, ssid)     ; // Search for stored SSID
    fileToCharPtr(LittleFS, passPath, pass)     ; // Search for stored Password
    fileToCharPtr(LittleFS, ipPath, esp_ip)     ; // Search for stored local IP
    fileToCharPtr(LittleFS, routerPath, router) ; // Search for stored router IP
    fileToCharPtr(LittleFS, hostPath, host)     ; // Search for stored host IP
  }
  
  // Function to Initialize Wifi
  bool initWiFi() {
    if(strcmp(ssid, "") == 0 || strcmp(esp_ip, "") == 0 || strcmp(router, "") == 0 ){
      Serial.println(F("Undefined WiFi"));
      return false;
    }
    
    const IPAddress subnet(255, 255, 0, 0);   // Subnet mask required for static IP address
    IPAddress gateway;          // IP address of the router
    IPAddress dns;              // IP address of the DNS (= router)
    IPAddress localIP;          // IP address of the ESP

    localIP.fromString(esp_ip);
    gateway.fromString(router);
    dns.fromString(router);
    hostIP.fromString(host);

    if (!WiFi.config(localIP, gateway, subnet, dns)){
      if (Debug) Serial.println(F("STA config Failed"));
      return false;
    }
    WiFi.begin(ssid, pass);   // STA mode is default

    Serial.print(F("Connecting .."));
    while (WiFi.status() != WL_CONNECTED) { 
      Serial.print('.'); delay(1000);
    }
    if (Debug) Serial.println(WiFi.localIP());

    #ifdef useOTA
      AsyncElegantOTA.begin(&server); // Start OTA
    #endif
    Serial.println(F("initWiFi done"));
    return true;
  }


  // Function to allow user to enter ssid and password
  // @details Connect to an ESP Wi-Fi network with a given SSID and password. 
  //          Starts a web server to allow the user to input these values.
  //          The values are stored in the LittleFS file system of the ESP. 
  //          If the values are not defined, the ESP will start an open Wi-Fi network "ESP-WIFI-MANAGER".
  //          The user can connect to this network and open the web page at the IP address of the ESP (usually 192.168.4.1).
  //          In that page, the user can input the required fields for Wifi connection. 
  //          The values are then stored in the LittleFS file system of the ESP.
  //          The ESP will then reboot and connect to the Wi-Fi network with the given values.
  void defineWiFi() {
    Serial.println(F("Setting AP")); 
    // Remove the password parameter (=NULL), if you want the AP (Access Point) to be open 
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print(F("AP local IP: "));
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            strncpy(ssid, p->value().c_str(), paramSize - 1);
            ssid[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.printf_P(PSTR("SSID: %s\n"), ssid);
            writeFile(LittleFS, ssidPath, ssid);
          }
          // HTTP POST pass value
          else if (p->name() == PARAM_INPUT_2) {
            strncpy(pass, p->value().c_str(), paramSize - 1);
            pass[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.printf_P(PSTR("Password: %s\n"), pass);
            writeFile(LittleFS, passPath, pass);
          }
          // HTTP POST esp_ip value
          else if (p->name() == PARAM_INPUT_3) {
            strncpy(esp_ip, p->value().c_str(), paramSize - 1);
            esp_ip[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.printf_P(PSTR("ESP IP: %s\n"), esp_ip);
            writeFile(LittleFS, ipPath, esp_ip);
          }
          // HTTP POST router IP value
          else if (p->name() == PARAM_INPUT_4) {
            strncpy(router, p->value().c_str(), paramSize - 1);
            router[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.printf_P(PSTR("Router IP: %s\n"), router);
            writeFile(LittleFS, routerPath, router);
          }
          // HTTP POST node-red host IP value
          else if (p->name() == PARAM_INPUT_5) {
            strncpy(host, p->value().c_str(), paramSize - 1);
            host[paramSize - 1] = '\0'; // Ensure null-termination
            Serial.printf_P(PSTR("Host IP: %s\n"), host);
            writeFile(LittleFS, hostPath, host);
          }           
          if (Debug) Serial.printf_P(PSTR("POST[%s]: %s\n"), p->name().c_str(), p->value().c_str()); 
        }
      }
      request->send(200, "text/plain", "Done. ESP rebooting, connect to your router. ESP IP address: " + String(esp_ip));
      reboot = true;
    });
    
    // Serve files (HTML, JS, CSS and favicon) from LittleFS when requested by the root URL. 
    server.serveStatic("/", LittleFS, "/");
    server.begin(); // Start the server.
    Serial.println(F("defineWifi done"));
  }

#else
//==================================================
// Hard coded Wifi initialization 
//==================================================
  void initWiFi() {
    const IPAddress subnet(255, 255, 0, 0);   // Subnet mask required for static IP address
    const IPAddress gateway(192, 168, 1, 1);  // Had coded router IP
    const IPAddress dns(192, 168, 1, 1);      // Had coded router IP
    IPAddress localIP;                        // IP address of the ESP
    localIP.fromString(esp_ip);

    if (!WiFi.config(localIP, gateway, subnet, dns)){
      if (Debug) Serial.println(F("STA config Failed"));
      return;
    }
    WiFi.begin(ssid, pass);   // STA mode is default

    Serial.print(F("Connecting to WiFi .."));
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    #ifdef useOTA
      AsyncElegantOTA.begin(&server); // Start OTA
    #endif
    Serial.println(WiFi.localIP());
  }
#endif

//==================================================
// Connect to WiFi (common function)
//==================================================
void connectToWifi() {
  #if defined(WIFI_MANAGER)  // Initialize Wifi, optional use WIFI_MANAGER 
    getWiFi();              // Get SSID, Password and IP from files
    if(!initWiFi()) {       // If SSID or Password were not stored, manage them and reboot
      defineWiFi();
      return; 
    }
  #else
    initWiFi();     // Initialize Wifi with hardcoded values
  #endif
}