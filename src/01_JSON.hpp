//======================================================
// LIBRARIES
//======================================================
#include <ArduinoJson.h>

//======================================================
// VARIABLES    
//======================================================
StaticJsonDocument<100> jsonDoc;  // Dummy JSON document
static char strBuffer[100];      // Dummy char array to send MQTT message

//======================================================
// FUNCTIONS    
//======================================================
/**
 * Create a JSON object with multiple key-value pairs from arrays.
 * The keys array and values array must have the same size (numKeys).
 * The values are integers.
 * @param numKeys Number of key-value pairs.
 * @param keys Array of keys (strings).
 * @param values Array of values (integers) corresponding to the keys.
 */
void makeJsonArray(const byte numKeys, const char* keys[], int16_t values[]) {
  jsonDoc.clear();  // Clear the document to avoid leftover data
  for (byte i = 0; i < numKeys; i++)  { jsonDoc[keys[i]] = values[i]; }
  serializeJson(jsonDoc, strBuffer, sizeof(strBuffer)); // Convert jsonDoc to char array
  if (Debug) Serial.printf_P(PSTR("[MQTT]> %s\n"), strBuffer);  // Optional debug output
}

// ============================= Not used in this project
// void makeJsonInt(const char* key, uint16_t value) {
//   jsonDoc.clear();  // Clear the document to avoid leftover data
//   // Add key-value pair to the JSON object "jsonDoc"
//   jsonDoc["topic"] = key;
//   jsonDoc["payload"] = value;
//   serializeJson(jsonDoc, strBuffer, sizeof(strBuffer)); // Convert jsonDoc to char array
//   if (Debug) Serial.printf_P(PSTR("[MQTT]> %s\n"), strBuffer);  // Optional debug output
// }

// ============================= Not used in this project
// void makeJsonString(const char* key, const char* value) {
//   jsonDoc.clear();  // Clear the document to avoid leftover data

//   // Add key-value pair to the JSON object "jsonDoc"
//   jsonDoc["topic"] = key;
//   jsonDoc["payload"] = value;

//   serializeJson(jsonDoc, strBuffer, sizeof(strBuffer)); // Convert jsonDoc to char array
//   if (Debug) Serial.printf_P(PSTR("[MQTT]> %s\n"), strBuffer);  // Optional debug output
// }

