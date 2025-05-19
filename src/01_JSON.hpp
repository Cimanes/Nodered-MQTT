//======================================================
// LIBRARIES
//======================================================
#include <ArduinoJson.h>

//======================================================
// VARIABLES    
//======================================================
StaticJsonDocument<100> jsonDoc;  // Dummy JSON document
char msg[100];

//======================================
// Create a JSON object --> send via Websocket
//======================================
/******************************************************
 * Create a JSON object with "topic" and INTEGER "payload" from a key-value pair.
 * The value is an integer.
 * @param topic The key for the JSON object entry.
 * @param value The value corresponding to the key (integer).
 */
void makeJsonInt(const char* key, uint16_t value) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Add key-value pair to the JSON object "jsonDoc"
  jsonDoc["topic"] = key;
  jsonDoc["payload"] = value;

  serializeJson(jsonDoc, msg, sizeof(msg)); // Convert jsonDoc to char array
  if (Debug) Serial.println(msg);                 // Optional debug output
}

/******************************************************
 * Create a JSON object with "topic" and STRING "payload" from a key-value pair.
 * @param topic The key for the JSON object entry.
 * @param value The value corresponding to the key (string).
 */
void makeJsonString(const char* key, const char* value) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  // Add key-value pair to the JSON object "jsonDoc"
  jsonDoc["topic"] = key;
  jsonDoc["payload"] = value;

  serializeJson(jsonDoc, msg, sizeof(msg)); // Convert jsonDoc to char array
  if (Debug) Serial.println(msg);             // Optional debug output
}


/**
 * Create a JSON object with multiple key-value pairs from arrays.
 * The keys array and values array must have the same size (numKeys).
 * The values are integers.
 * @param numKeys Number of key-value pairs.
 * @param keys Array of keys (strings).
 * @param values Array of values (integers) corresponding to the keys.
 */
void makeJsonArray(const byte numKeys, const char* keys[], uint16_t values[]) {
  jsonDoc.clear();  // Clear the document to avoid leftover data

  for (byte i = 0; i < numKeys; i++)  { jsonDoc[keys[i]] = values[i]; }

  serializeJson(jsonDoc, msg, sizeof(msg)); // Convert jsonDoc to char array
  if (Debug) Serial.println(msg);             // Optional debug output
}