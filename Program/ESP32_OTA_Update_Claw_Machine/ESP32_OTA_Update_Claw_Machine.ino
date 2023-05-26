#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>

const char* ssid = "YOUR_SSID";
const char* password = "SSID_PASS";

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


// JSON:
//  id: "gpiostates"
//  gpios: 
//    x:
//      output: "data"
//      state:  "data"

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

// Check if client is asking for GPIO states
bool askingForGPIOStates(JSONVar Obj){
  return (strcmp((const char*)Obj["id"], "gpiostates") == 0);
}

// JSON:
//  id: "pin"
//  number: "data"
//  action: "data"

// Does the Obj have any pin data
bool couldHavePinData(JSONVar Obj){
  bool hasPinID = (strcmp((const char*)Obj["id"], "pin") == 0)
  return hasPinID && Obj.hasOwnProperty("number") && Obj.hasOwnProperty("action");
}

// Perform action received to pin, then update GPIO states
void handlePinMessage(JSONVar Obj) {
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

// Check and change GPIO state, made for testing...
void checkAndChangeOutput(int pin, bool newState) {
  bool oldState = (bool)digitalRead(pin);
  Serial.print("Old State ");
  Serial.print(oldState);
  Serial.print("New State ");
  Serial.println(newState);
  if(oldState != newState)
  {
    Serial.print("Writing ");
    Serial.println(pin);
    digitalWrite(pin, newState);
  }
}

// JSON:
//  id: "joystick"
//  controller: "data"
//  directionY: "data"
//  directionX: "data"

// Does the Obj have any joystick data
bool couldHaveJoystickData(JSONVar Obj){
  bool hasJoystickID = (strcmp((const char*)Obj["id"], "joystick") == 0)
  bool isQuad = false;
  if(Obj.hasOwnProperty("controller")){
    isQuad = ((const char*)Obj["controller"], "quad") == 0);
  }
  if(isQuad){
    return hasJoystickID && Obj.hasOwnProperty("directionY") && Obj.hasOwnProperty("directionX");
  }
  return hasJoystickID && Obj.hasOwnProperty("directionY");
}

//Handle both joystick messages
void handleJoystickMessage(JSONVar Obj) {
  //Serial.println(Obj);
  Serial.print((const char*)Obj["controller"]);
  Serial.print(", ");
  Serial.println((const char*)Obj["directionY"]);
  if(strcmp((const char*)Obj["controller"], "bi") == 0) {
    biJoystick(Obj);
    Serial.println((const char*)Obj["directionX"]);
  }
  if(strcmp((const char*)Obj["controller"], "quad") == 0) {
    quadJoystick(Obj);
  }
}

// Quad joystick outputs
void quadJoystick(JSONVar Obj) {
  if(strcmp((const char*)Obj["directionY"], "up") == 0) {
    changeDirection(rightUp, true, rightDown, false);
  } else if(strcmp((const char*)Obj["directionY"], "down") == 0) {
    changeDirection(rightUp, false, rightDown, true);
  } else {
    changeDirection(rightUp, false, rightDown, false);
  }

  if(strcmp((const char*)Obj["directionX"], "left") == 0) {
    changeDirection(rightLeft, true, rightRight, false);
  } else if (strcmp((const char*)Obj["directionX"], "right") == 0) {
    changeDirection(rightLeft, false, rightRight, true);
  } else {
    changeDirection(rightLeft, false, rightRight, false);
  }
}

// Bi joystick outputs
void biJoystick(JSONVar Obj) {
  if(strcmp((const char*)Obj["directionY"], "up") == 0) {
    changeDirection(leftUp, true, leftDown, false);
    return;
  }
  if(strcmp((const char*)Obj["directionY"], "down") == 0) {
    changeDirection(leftUp, false, leftDown, true);
    return;
  }
  changeDirection(leftUp, false, leftDown, false);
}

// Change pin output
void changeDirection(int pin, bool state, int pin2, bool state2) {
  digitalWrite(pin, state);
  digitalWrite(pin2, state2);
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
    
    if(askingForGPIOStates(myObj)) {
      handleGpiostatesMessage();
      return;
    }

    if(couldHavePinData(myObj)) {
      handlePinMessage(myObj);
      return;
    }

    if(scouldHaveJoystickData(myObj)) {
      handleJoystickMessage(myObj);
      return;
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