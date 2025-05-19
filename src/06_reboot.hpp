// =============================================
// VARIABLES 
// =============================================
unsigned int rebootTimer;

// =============================================
// Periodic function to check if a reboot is requested
// =============================================
void checkReboot() {
  if(reboot) { 
    if (Debug) Serial.println(F("rebooting"));
    reboot = false;
  #if defined(ESP32)  
    timer.setTimeout(5000, []() { esp_restart(); } );
  #elif defined(ESP8266)
    timer.setTimeout(5000, []() { ESP.restart(); } );
  #endif
  }
}

void initReboot() { // Request to delete data file
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    reboot = true;
    request->send(200);
  });
  rebootTimer = timer.setInterval(1000, checkReboot);
}