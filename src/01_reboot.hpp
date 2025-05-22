//==============================================
// VARIABLES
//==============================================
boolean reboot = false;
unsigned int rebootTimer; // Timer for reboot

//==============================================
// FUNCTIONS
//==============================================

// Periodic callback function to check if a reboot is requested
void checkReboot() {
  if(reboot) {
    if (Debug) Serial.println(F("rebooting"));
    reboot = false;
  #if defined(ESP32)  
    timer.setTimeout(5000, []() { esp_restart(); } );
  #elif defined(ESP8266)
    timer.setTimeout(4000, []() { ESP.restart(); } );
  #endif
  }
}

// Setup timer for reboot check and server response when reboot is requested (optional)
void initReboot() {
  rebootTimer = timer.setInterval(4000, checkReboot);  
  // server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   reboot = true;
  //   request->send(200);
  // });
  if (Debug)  { Serial.println(F("Reboot ready")); }
}