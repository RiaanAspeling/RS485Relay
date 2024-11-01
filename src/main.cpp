#include <Arduino.h>
#include "WiFi.h"
#include "WiFiClient.h"
#include "LittleFS.h"
#include "WiFiManager.h"
#include "ESPAsyncWebServer.h"
#include "AsyncElegantOTA.h"
#include "PubSubClient.h"
#include "ArduinoJSON.h"
#include "SoftwareSerial.h"
#include "config.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
AsyncWebServer server(80);
AsyncServer *serverRS485 = nullptr;
AsyncClient *wifiClientRS485 = nullptr;  // Use AsyncClient instead of WiFiClient
EspSoftwareSerial::UART RS485Port;
AsyncWebSocket ws("/ws"); // WebSocket instance

unsigned long mustRestartTime = 0;

void DebugLog(String text) {
  Serial.println(text);
}

void WriteJSONToMonitor(String messageType, String message) {
  DynamicJsonDocument doc(1024);
  doc["type"] = messageType;
  doc["message"] = message;
  String jsonString;
  serializeJson(doc, jsonString);
  ws.textAll(jsonString);
}

void BlinkLED(int onTime, int offTime) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(onTime);
  digitalWrite(LED_BUILTIN, LOW);
  delay(offTime);
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  DebugLog("Entered Conf Mode");

  DebugLog("Config SSID: ");
  DebugLog(myWiFiManager->getConfigPortalSSID());

  DebugLog("Config IP Address: ");
  DebugLog(WiFi.softAPIP().toString());
}

Config GetConfigFromString(String serialConfig) {
  Config rtnVal;
  if (serialConfig.equalsIgnoreCase("SWSERIAL_5N1")) { rtnVal = SWSERIAL_5N1; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_6N1")) { rtnVal = SWSERIAL_6N1; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_7N1")) { rtnVal = SWSERIAL_7N1; }
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_8N1")) { rtnVal = SWSERIAL_8N1; }
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_5N2")) { rtnVal = SWSERIAL_5N2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_6N2")) { rtnVal = SWSERIAL_6N2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_7N2")) { rtnVal = SWSERIAL_7N2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_8N2")) { rtnVal = SWSERIAL_8N2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_5E1")) { rtnVal = SWSERIAL_5E1; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_6E1")) { rtnVal = SWSERIAL_6E1; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_7E1")) { rtnVal = SWSERIAL_7E1; }
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_8E1")) { rtnVal = SWSERIAL_8E1; }
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_5E2")) { rtnVal = SWSERIAL_5E2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_6E2")) { rtnVal = SWSERIAL_6E2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_7E2")) { rtnVal = SWSERIAL_7E2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_8E2")) { rtnVal = SWSERIAL_8E2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_5O1")) { rtnVal = SWSERIAL_5O1; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_6O1")) { rtnVal = SWSERIAL_6O1; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_7O1")) { rtnVal = SWSERIAL_7O1; }
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_8O1")) { rtnVal = SWSERIAL_8O1; }
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_5O2")) { rtnVal = SWSERIAL_5O2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_6O2")) { rtnVal = SWSERIAL_6O2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_7O2")) { rtnVal = SWSERIAL_7O2; } 
  else if (serialConfig.equalsIgnoreCase("SWSERIAL_8O2")) { rtnVal = SWSERIAL_8O2; } 
  else {
    rtnVal = SWSERIAL_8N1;
  }
  return rtnVal;
}
String GetConfigFromEnum(Config serialConfig) {
  String rtnVal;
  switch (serialConfig) {
    case SWSERIAL_5N1: rtnVal = "SWSERIAL_5N1"; break;
    case SWSERIAL_6N1: rtnVal = "SWSERIAL_6N1"; break;
    case SWSERIAL_7N1: rtnVal = "SWSERIAL_7N1"; break;
    case SWSERIAL_8N1: rtnVal = "SWSERIAL_8N1"; break;
    case SWSERIAL_5N2: rtnVal = "SWSERIAL_5N2"; break;
    case SWSERIAL_6N2: rtnVal = "SWSERIAL_6N2"; break;
    case SWSERIAL_7N2: rtnVal = "SWSERIAL_7N2"; break;
    case SWSERIAL_8N2: rtnVal = "SWSERIAL_8N2"; break;
    case SWSERIAL_5E1: rtnVal = "SWSERIAL_5E1"; break;
    case SWSERIAL_6E1: rtnVal = "SWSERIAL_6E1"; break;
    case SWSERIAL_7E1: rtnVal = "SWSERIAL_7E1"; break;
    case SWSERIAL_8E1: rtnVal = "SWSERIAL_8E1"; break;
    case SWSERIAL_5E2: rtnVal = "SWSERIAL_5E2"; break;
    case SWSERIAL_6E2: rtnVal = "SWSERIAL_6E2"; break;
    case SWSERIAL_7E2: rtnVal = "SWSERIAL_7E2"; break;
    case SWSERIAL_8E2: rtnVal = "SWSERIAL_8E2"; break;
    case SWSERIAL_5O1: rtnVal = "SWSERIAL_5O1"; break;
    case SWSERIAL_6O1: rtnVal = "SWSERIAL_6O1"; break;
    case SWSERIAL_7O1: rtnVal = "SWSERIAL_7O1"; break;
    case SWSERIAL_8O1: rtnVal = "SWSERIAL_8O1"; break;
    case SWSERIAL_5O2: rtnVal = "SWSERIAL_5O2"; break;
    case SWSERIAL_6O2: rtnVal = "SWSERIAL_6O2"; break;
    case SWSERIAL_7O2: rtnVal = "SWSERIAL_7O2"; break;
    case SWSERIAL_8O2: rtnVal = "SWSERIAL_8O2"; break;
  }
  return rtnVal;
}

void saveConfigFile()
{
  DebugLog(F("Saving config"));
  DynamicJsonDocument json(2048);
  json["RS485_BAUD"] = RS485_BAUD;
  json["RS485_OPTIONS"] = GetConfigFromEnum(RS485_OPTIONS);
  json["LISTEN_PORT"] = LISTEN_PORT;
  
  File configFile = LittleFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    DebugLog("Failed to open " + String(JSON_CONFIG_FILE) + " file for writing");
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    DebugLog("Failed to write to file " + String(JSON_CONFIG_FILE));
  }
  configFile.close();
}
bool loadConfigFile()
{
  if (!LittleFS.exists(JSON_CONFIG_FILE)) {
    DebugLog(String(JSON_CONFIG_FILE) + " does not exist.");
    return false;
  }
  File configFile = LittleFS.open(JSON_CONFIG_FILE, "r");
  if (!configFile) {
    DebugLog("Failed to open " + String(JSON_CONFIG_FILE));
    return false;
  }
  DebugLog("Reading " + String(JSON_CONFIG_FILE));
  DynamicJsonDocument json(2048);
  DeserializationError error = deserializeJson(json, configFile);
  serializeJsonPretty(json, Serial);
  if (error) {
    DebugLog("Unable to deserialize " + String(JSON_CONFIG_FILE));
    return false;
  }
  RS485_BAUD = json["RS485_BAUD"].as<unsigned int>();
  RS485_OPTIONS = GetConfigFromString(json["RS485_OPTIONS"].as<String>());
  LISTEN_PORT = json["LISTEN_PORT"].as<unsigned int>();
  return true;
}
void saveStaticIPFile() {
  DebugLog(F("Saving Static IP"));
  DynamicJsonDocument json(2048);
  json["STATIC_IP"] = STATIC_IP.toString();
  json["STATIC_SUB"] = STATIC_SUB.toString();
  json["STATIC_GW"] = STATIC_GW.toString();
  json["STATIC_DNS"] = STATIC_DNS.toString();

  File configFile = LittleFS.open(JSON_STATICIP_FILE, "w");
  if (!configFile)
  {
    DebugLog("Failed to open " + String(JSON_STATICIP_FILE) + " file for writing");
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    DebugLog("Failed to write to file " + String(JSON_CONFIG_FILE));
  }
  configFile.close();
}
bool loadStaticIPFile() {
  if (!LittleFS.exists(JSON_STATICIP_FILE)) {
    DebugLog(String(JSON_STATICIP_FILE) + " does not exist.");
    return false;
  }
  File configFile = LittleFS.open(JSON_STATICIP_FILE, "r");
  if (!configFile) {
    DebugLog("Failed to open " + String(JSON_STATICIP_FILE));
    return false;
  }
  DebugLog("Reading " + String(JSON_STATICIP_FILE));
  DynamicJsonDocument json(2048);
  DeserializationError error = deserializeJson(json, configFile);
  serializeJsonPretty(json, Serial);
  if (error) {
    DebugLog("Unable to deserialize " + String(JSON_STATICIP_FILE));
    return false;
  }
  bool ipIsOk = true;
  ipIsOk &= STATIC_IP.fromString(json["STATIC_IP"].as<String>());
  ipIsOk &= STATIC_SUB.fromString(json["STATIC_SUB"].as<String>());
  ipIsOk &= STATIC_GW.fromString(json["STATIC_GW"].as<String>());
  ipIsOk &= STATIC_DNS.fromString(json["STATIC_DNS"].as<String>());
  return ipIsOk && 
          STATIC_IP != IPAddress(0,0,0,0) && 
          STATIC_SUB != IPAddress(0,0,0,0) && 
          STATIC_GW != IPAddress(0,0,0,0) && 
          STATIC_DNS != IPAddress(0,0,0,0);
}

void connectLittleFS() {
  if (LittleFS.begin(false) || LittleFS.begin(true))
  {
    DebugLog("LittleFS Connected!");
  } else {
    DebugLog("LittleFS FAILED to mount!");
    delay(5000);
    ESP.restart();
  }
}
void connectWifi(bool forceConfig) {
  // Setup wifi and manager
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setBreakAfterConfig(true);
  wm.setAPCallback(configModeCallback);

  if (forceConfig) {
    wm.startConfigPortal();
    DebugLog("Restarting and resettings Static IP!");
    STATIC_IP = IPAddress(0,0,0,0);
    STATIC_SUB = IPAddress(0,0,0,0);
    STATIC_GW = IPAddress(0,0,0,0);
    STATIC_DNS = IPAddress(0,0,0,0);
    saveStaticIPFile();
    delay(2000);
    ESP.restart();
    return;
  }

  if (wm.getWiFiIsSaved()) {
    DebugLog("Connecting to " + wm.getWiFiSSID());
  }
    else
  {
    DebugLog("Connect to configure:");
    DebugLog(wm.getDefaultAPName());
  }

  bool canConnect = wm.autoConnect();
  if (loadStaticIPFile()) {
    DebugLog("Configure IP: " + STATIC_IP.toString() + "/" + STATIC_SUB.toString() + " GW:" + STATIC_GW.toString() + " DNS:" + STATIC_DNS.toString());
    WiFi.config(STATIC_IP, STATIC_GW, STATIC_SUB, STATIC_DNS);
  }

  if (!canConnect)
  {
    DebugLog("Failed to connect, restarting!");
    delay(5000);
    ESP.restart();
  }
  // Allow to reconnect to WiFi if signal is lost
  WiFi.setAutoReconnect(true);
}

// Read data from RS485 and send it to the connected PC client
void readRS485() {
  // Collect all available data in one go
  String rs485Data = "";
  while (RS485Port.available() > 0) {
    rs485Data += (char)RS485Port.read();
    BlinkLED(10, 2);
  }
  if (rs485Data.length() > 0) {
    if (wifiClientRS485 && wifiClientRS485->connected()) {
      wifiClientRS485->write(rs485Data.c_str(), rs485Data.length());
    }
    DebugLog("RS485=>PORT: " + rs485Data);
    WriteJSONToMonitor("rs485", "RS485=>PORT: " + rs485Data);
  }
}

// Callback for when data is received from the PC client
void onClientData(void *arg, AsyncClient *tcpClient, void *data, size_t len) {
  if (tcpClient && tcpClient->connected()) {
    RS485Port.write((uint8_t*)data, len); // Send the data from the PC to RS485
    String clientData((char*)data, len);
    DebugLog("PORT=>RS485: " + clientData);
    WriteJSONToMonitor("port", "PORT=>RS485: " + clientData);
    BlinkLED(10, 5);
  }
}

// Modified to handle WebSocket events
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        WriteJSONToMonitor("port", "Client connected: " + client->remoteIP().toString());
    }
}

String buildJavaScriptForDefaults() {
  String rtnVal = "*/"; 
  rtnVal += "selectValue('RS485_BAUD','" + String(RS485_BAUD) + "');";
  rtnVal += "selectValue('RS485_OPTIONS','" + GetConfigFromEnum(RS485_OPTIONS) + "');";
  rtnVal += "/*";
  return rtnVal;
}

String processDefaults(const String& var) {
  if (var == "FIRMWARE_VERSION") {          DebugLog(var + "=" + FIRMWARE_VERSION);           return String(FIRMWARE_VERSION); }
  else if (var == "STATIC-IP") {            DebugLog(var + "=" + STATIC_IP.toString());       return STATIC_IP != IPAddress(0,0,0,0) ? STATIC_IP.toString() : ""; }
  else if (var == "STATIC-SUB") {           DebugLog(var + "=" + STATIC_SUB.toString());      return STATIC_SUB != IPAddress(0,0,0,0) ? STATIC_SUB.toString() : ""; } 
  else if (var == "STATIC-GW") {            DebugLog(var + "=" + STATIC_GW.toString());       return STATIC_GW != IPAddress(0,0,0,0) ? STATIC_GW.toString() : ""; } 
  else if (var == "STATIC-DNS") {           DebugLog(var + "=" + STATIC_DNS.toString());      return STATIC_DNS != IPAddress(0,0,0,0) ? STATIC_DNS.toString() : ""; }
  else if (var == "LISTEN_PORT") {          DebugLog(var + "=" + String(LISTEN_PORT));        return String(LISTEN_PORT); }
  else if (var == "JAVASCRIPT-DEFAULTS") {  
    String js = buildJavaScriptForDefaults();
    DebugLog(var + "=" + js);   
    return js; 
  }
  DebugLog(var +"=[Unknown]");
  return String();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200, SERIAL_8N1);

  pinMode(D2, OUTPUT);
  digitalWrite(D2, LOW);

  Serial.print("Startup in 5..");
  for(int c = 4; c > 0; c--) {
    Serial.print(String(c) + "..");
    BlinkLED(800, 200);
  }
  connectLittleFS();
  bool loadedConfig = loadConfigFile();
  connectWifi(!loadedConfig);

  Serial.println("Setup RS485 serial to " + String(RS485_BAUD) + " baud and " + GetConfigFromEnum(RS485_OPTIONS));
  RS485Port.begin(RS485_BAUD, RS485_OPTIONS, D4, D3, false);

  // Initialize WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html", false, processDefaults);
  });
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/settings.html", "text/html", false, processDefaults);
  });
  server.on("/save", HTTP_GET, [] (AsyncWebServerRequest *request) {
    bool saveIP = false;
    if (request->getParam("STATIC_IP")->value().length() > 0) {
      saveIP = true;
      saveIP &= STATIC_IP.fromString(request->getParam("STATIC_IP")->value());    DebugLog(STATIC_IP.toString());
      saveIP &= STATIC_SUB.fromString(request->getParam("STATIC_SUB")->value());  DebugLog(STATIC_SUB.toString());
      saveIP &= STATIC_GW.fromString(request->getParam("STATIC_GW")->value());    DebugLog(STATIC_GW.toString());
      saveIP &= STATIC_DNS.fromString(request->getParam("STATIC_DNS")->value());  DebugLog(STATIC_DNS.toString());
    }
    if (!saveIP) {
      STATIC_IP = IPAddress(0,0,0,0);
      STATIC_SUB = IPAddress(0,0,0,0);
      STATIC_GW = IPAddress(0,0,0,0);
      STATIC_DNS = IPAddress(0,0,0,0);
    }
    saveStaticIPFile();
    RS485_BAUD = request->getParam("RS485_BAUD")->value().toInt();                DebugLog(String(RS485_BAUD));
    String serialConfig = request->getParam("RS485_OPTIONS")->value();               DebugLog(serialConfig);
    RS485_OPTIONS = GetConfigFromString(serialConfig);
    LISTEN_PORT = request->getParam("LISTEN_PORT")->value().toInt();                 DebugLog(String(LISTEN_PORT));
    saveConfigFile();
    request->send(LittleFS, "/saved.html", "text/html", false, processDefaults);
    mustRestartTime = millis() + 5000;
  });
  server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/monitor.html", "text/html", false, processDefaults);
  });
  

  AsyncElegantOTA.begin(&server, "", "");
  server.serveStatic("/", LittleFS, "/");
  server.onNotFound(notFound);
  server.begin();

  // WebSerial.begin(&server);
  DebugLog("Loaded OTA...");
  DebugLog("Ready!");

  // Set up TCP server
  serverRS485 = new AsyncServer(LISTEN_PORT); // Listen on port 12345
  serverRS485->onClient([](void *arg, AsyncClient *tcpClient) {
    wifiClientRS485 = tcpClient; // Store the client connection
    wifiClientRS485->onData(onClientData, nullptr); // Relay data from PC to RS485
  }, nullptr);
  serverRS485->begin();

  DebugLog("RS485 Server started!");
}

void loop() {
  if (mustRestartTime != 0 && mustRestartTime < millis())
    ESP.restart();
  readRS485();
  ws.cleanupClients();
}