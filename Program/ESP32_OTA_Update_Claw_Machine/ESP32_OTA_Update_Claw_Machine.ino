/*
  Rui Santos
  Complete project details
   - Arduino IDE: https://RandomNerdTutorials.com/esp32-ota-over-the-air-arduino/
   - VS Code: https://RandomNerdTutorials.com/esp32-ota-over-the-air-vs-code/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>

// Replace with your network credentials
const char* ssid = "VIRGIN-telco_D608_EXT";
const char* password = "C55QStsbH4EZA7";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Set number of outputs
#define NUM_OUTPUTS  4

// Assign each GPIO to an output
int outputGPIOs[NUM_OUTPUTS] = {2, 4, 12, 14};
//27 26 | 25 33 32 35
int leftUp = 27;
int leftDown = 26;
int rightUp = 25;
int rightDown = 33;
int rightRight = 32;
int rightLeft = 35;

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

String getOutputStates(){
  JSONVar myArray;
  for (int i =0; i<NUM_OUTPUTS; i++){
    myArray["id"] = "gpiostates";
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  return JSON.stringify(myArray);
}

void notifyClients(String state) {
  ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    JSONVar myObj = JSON.parse((char*)data);
    //JSONVar myObject = JSON.parse(input);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObj) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }
    if (!myObj.hasOwnProperty("id")) {
      Serial.println("No id property...");
      return;
    }
    //Serial.println("JSON OBJECT: ");
    Serial.println("-------------");
    //Serial.println(myObj);
    /*
    Serial.println((const char*)myObj["id"]);
    Serial.print("gpiostates: ");
    Serial.println(strcmp((const char*)myObj["id"], "gpiostates") == 0);
    Serial.print("pin: ");
    Serial.println(strcmp((const char*)myObj["id"], "pin") == 0);
    Serial.print("controller: ");
    Serial.println(strcmp((const char*)myObj["id"], "controller") == 0);
    */
    
    if(strcmp((const char*)myObj["id"], "gpiostates") == 0) {
      Serial.println("CHECKING GPIO STATES");
      notifyClients(getOutputStates());
      return;
    }

    if(strcmp((const char*)myObj["id"], "pin") == 0) {
      if(myObj.hasOwnProperty("number")){
        int gpio = atoi(myObj["number"]);
        Serial.print("On pin: ");
        Serial.print(gpio);
        if(myObj.hasOwnProperty("action")){
          Serial.print(" Action: ");
          Serial.print((const char*)myObj["action"]);
          Serial.print(": ");
          if(strcmp((const char*)myObj["action"], "toggle") == 0) {
            digitalWrite(gpio, !digitalRead(gpio));
            Serial.println("Toggled");
            notifyClients(getOutputStates());
          }
        }
      }
      return;
    }

    if(strcmp((const char*)myObj["id"], "joystick") == 0) {
      Serial.println(myObj);
      if(myObj.hasOwnProperty("controller")){
        if(strcmp((const char*)myObj["controller"], "bi") == 0) {
          if(myObj.hasOwnProperty("directionY")){
            Serial.print((const char*)myObj["controller"]);
            Serial.print(", ");
            Serial.println((const char*)myObj["directionY"]);
            if(strcmp((const char*)myObj["directionY"], "up") == 0) {
              digitalWrite(leftUp, 1);
              digitalWrite(leftDown, 0);
              return;
            }
            if(strcmp((const char*)myObj["directionY"], "down") == 0) {
              digitalWrite(leftUp, 0);
              digitalWrite(leftDown, 1);
              return;
            }
            digitalWrite(leftUp, 0);
            digitalWrite(leftDown, 0);
            return;
          }
        }
        if(strcmp((const char*)myObj["controller"], "quad") == 0) {
          //Serial.print((const char*)myObj["controller"]);
          //Serial.print(", ");
          if(myObj.hasOwnProperty("directionY")){
            //Serial.print((const char*)myObj["directionY"]);
            if(strcmp((const char*)myObj["directionY"], "up") == 0) {
              Serial.println("ARRIBA");
              digitalWrite(rightUp, 1);
              digitalWrite(rightDown, 0);
            } else if(strcmp((const char*)myObj["directionY"], "down") == 0) {
              Serial.println("ABAJO");
              digitalWrite(rightUp, 0);
              digitalWrite(rightDown, 1);
            } else if (strcmp((const char*)myObj["directionY"], "stop") == 0) {
              Serial.println("STOP");
              digitalWrite(rightUp, 0);
              digitalWrite(rightDown, 0);
            }
          }
          if(myObj.hasOwnProperty("directionX")){
            //Serial.print(", ");
            //Serial.print((const char*)myObj["directionX"]);
            if(strcmp((const char*)myObj["directionX"], "left") == 0) {
              Serial.println("IZQUIERDA");
              digitalWrite(rightLeft, 1);
              digitalWrite(rightRight, 0);
            } else if (strcmp((const char*)myObj["directionX"], "right") == 0) {
              Serial.println("DERECHA");
              digitalWrite(rightLeft, 0);
              digitalWrite(rightRight, 1);
            } else if (strcmp((const char*)myObj["directionX"], "stop") == 0) {
              Serial.println("STOP");
              digitalWrite(rightLeft, 0);
              digitalWrite(rightRight, 0);
            }
          }
          Serial.println();
          return;
        }
      }
    }
  }
}

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
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Set GPIOs as outputs
  for (int i =0; i<NUM_OUTPUTS; i++){
    pinMode(outputGPIOs[i], OUTPUT);
  }

  pinMode(leftUp, OUTPUT);
  pinMode(leftDown, OUTPUT);
  pinMode(rightUp, OUTPUT);
  pinMode(rightDown, OUTPUT);
  pinMode(rightRight, OUTPUT);
  pinMode(rightLeft, OUTPUT);

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