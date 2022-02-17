#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>

const char* ssid = "VIRGIN-telco_D608_EXT";
const char* password = "C55QStsbH4EZA7";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Set number of outputs
#define NUM_OUTPUTS  10

// GPIO for Joysticks
int leftUp = 27, leftDown = 26,
    rightUp = 25, rightDown = 33, rightRight = 32, rightLeft = 23;

// Assign each GPIO to an output, (should be on html to toggle)
int outputGPIOs[NUM_OUTPUTS] = {2, 4, 12, 14, leftUp, leftDown, rightUp, rightDown, rightRight, rightLeft};

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// GPIO State as String
String getOutputStates(){
  JSONVar myArray;
  for (int i=0; i<NUM_OUTPUTS; i++){
    myArray["id"] = "gpiostates";
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  return JSON.stringify(myArray);
}

// Send String through websocket
void notifyClients(String state) {
  ws.textAll(state);
}

// Update GPIO states through websocket
void handleGpiostatesMessage() {
  Serial.println("Sending GPIO States");
  notifyClients(getOutputStates());
}

// Perform action received to pin, then update GPIO states
void handlePinMessage(JSONVar Obj) {
  if(Obj.hasOwnProperty("number") && Obj.hasOwnProperty("action")){
    int gpio = atoi(Obj["number"]);
    Serial.print("On pin: ");
    Serial.print(gpio);
    Serial.print(" Action: ");
    Serial.print((const char*)Obj["action"]);
    Serial.print(": ");
    if(strcmp((const char*)Obj["action"], "toggle") == 0) {
      digitalWrite(gpio, !digitalRead(gpio));
      Serial.println("Toggled");
      handleGpiostatesMessage();
    }
  }
}

// Check and change GPIO state, made for testing...
void checkAndChangeOutput(int pin, bool state) {
  if((bool)digitalRead(pin) != state)
  {
    Serial.print("Writing ");
    Serial.println(pin);
    digitalWrite(pin, state);
  }
}

// Quad joystick outputs
void quadJoystick(JSONVar Obj) {
  if(Obj.hasOwnProperty("directionY")){
    if(strcmp((const char*)Obj["directionY"], "up") == 0) {
      checkAndChangeOutput(rightUp, true);
      checkAndChangeOutput(rightDown, false);
    } else if(strcmp((const char*)Obj["directionY"], "down") == 0) {
      checkAndChangeOutput(rightUp, false);
      checkAndChangeOutput(rightDown, true);
    } else {
      checkAndChangeOutput(rightUp, false);
      checkAndChangeOutput(rightDown, false);
    }
  }
  if(Obj.hasOwnProperty("directionX")){
    if(strcmp((const char*)Obj["directionX"], "left") == 0) {
      checkAndChangeOutput(rightLeft, true);
      checkAndChangeOutput(rightRight, false);
    } else if (strcmp((const char*)Obj["directionX"], "right") == 0) {
      checkAndChangeOutput(rightLeft, false);
      checkAndChangeOutput(rightRight, true);
    } else {
      checkAndChangeOutput(rightLeft, false);
      checkAndChangeOutput(rightRight, false);
    }
  }
}

// Bi joystick outputs
void biJoystick(JSONVar Obj) {
  if(Obj.hasOwnProperty("directionY")){
    Serial.print((const char*)Obj["controller"]);
    Serial.print(", ");
    Serial.println((const char*)Obj["directionY"]);
    if(strcmp((const char*)Obj["directionY"], "up") == 0) {
      checkAndChangeOutput(leftUp, true);
      checkAndChangeOutput(leftDown, false);
      return;
    }
    if(strcmp((const char*)Obj["directionY"], "down") == 0) {
      checkAndChangeOutput(leftUp, false);
      checkAndChangeOutput(leftDown, true);
      return;
    }
    checkAndChangeOutput(leftUp, false);
      checkAndChangeOutput(leftDown, false);
    return;
  }
}

//Handle both joystick messages
void handleJoystickMessage(JSONVar Obj) {
  Serial.println(Obj);
  if(Obj.hasOwnProperty("controller")){
    if(strcmp((const char*)Obj["controller"], "bi") == 0) {
      biJoystick(Obj);
    }
    if(strcmp((const char*)Obj["controller"], "quad") == 0) {
      quadJoystick(Obj);
    }
  }
}

//Handle all messages
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    JSONVar myObj = JSON.parse((char*)data);

    if (JSON.typeof(myObj) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    if (!myObj.hasOwnProperty("id")) {
      Serial.println("No id property...");
      return;
    }

    Serial.println("-------------");
    
    if(strcmp((const char*)myObj["id"], "gpiostates") == 0) {
      handleGpiostatesMessage();
      //return;
    }

    if(strcmp((const char*)myObj["id"], "pin") == 0) {
      handlePinMessage(myObj);
      //return;
    }

    if(strcmp((const char*)myObj["id"], "joystick") == 0) {
      handleJoystickMessage(myObj);
      //return;
    }
  }
}

//Handle websocket events
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup(){
  Serial.begin(115200);

  // Set GPIO as outputs
  for (int i =0; i<NUM_OUTPUTS; i++){
    pinMode(outputGPIOs[i], OUTPUT);
  }

  initSPIFFS();
  initWiFi();
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html",false);
  });

  server.serveStatic("/", SPIFFS, "/");

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  
  // Start server
  server.begin();
}

void loop() {
  AsyncElegantOTA.loop();
  ws.cleanupClients();
}