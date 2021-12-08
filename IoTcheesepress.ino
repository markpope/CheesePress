#define _USE_AZURE_IOT
// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HX711.h>
#include <TaskScheduler.h>

#include "cheesepress.h"
#include "consoleoutput.h"
#include "iot_configs.h"

void readPressureCallback(); // Read pressure and update global var.
void monitorAndStopCallback(); // Monitor flag and press to target pressure.
void returnToTargetPressureCallback(); // Monitor for pressure drop and increase to target.
void broadcastPressureCallback(); // Notify clients of scale weight.
void pubToIoTCallback(); // Publish pressure to IoT Hub.
Task t1(10, TASK_FOREVER, &readPressureCallback);
Task t2(10, TASK_FOREVER, &monitorAndStopCallback);
Task t3(100, TASK_FOREVER, &returnToTargetPressureCallback);
Task t4(100, TASK_FOREVER, &broadcastPressureCallback);
#ifdef _USE_AZURE_IOT
Task t5(2000, TASK_FOREVER, &pubToIoTCallback);
#endif


Scheduler runner;

static unsigned long next_telemetry_send_time_ms = 0;

IPAddress local_IP(192, 168, 12, 128);
IPAddress gateway(192, 168, 12, 1);
IPAddress subnet(255, 255, 0, 0);

bool ledState = 0;
const int ledPin = 2;
int targetPressure = 0 ;
float currentPressure = 0;
int variance = 3;
bool pressFlag = 0;
bool underPressure = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocket console("/console");

//HX711 Stuff
#define calibration_factor -21305.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT D5
#define CLK  D6
#define PIN_SWITCH_1 D1
#define PIN_SWITCH_2 D2

HX711 scale;

// Moved to readPressureCallback to remove Alarm calls
void notifyClients() {
  ws.textAll(String(currentPressure));
}

void engagePress() {
  Serial.println("Engaging Press");
  console.textAll("Engaging Press<br>");
  digitalWrite(PIN_SWITCH_1, HIGH);
  digitalWrite(PIN_SWITCH_2, LOW);
}

void releasePress() {
  Serial.println("Releasing Press");
  console.textAll("Releasing Press<br>");
  digitalWrite(PIN_SWITCH_1, LOW);
  digitalWrite(PIN_SWITCH_2, HIGH);
}

void stopPress() {
  //Serial.println("Stopping Press");
  digitalWrite(PIN_SWITCH_1, LOW);
  digitalWrite(PIN_SWITCH_2, LOW);
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "release") == 0) {
      Serial.println("Released pressed");
      releasePress();
      pressFlag = 0;
      underPressure = 0;
    }
    else if (strcmp((char*)data, "tare") == 0) {
      Serial.println("Tare Pressed");
      //console.textAll("Tare Pressed <br>");
      if (!underPressure || !pressFlag) {
        scale.tare();
      }
      else {
        Serial.println("Can't tare: Under Pressure");
      }
    }
    else {
      Serial.printf("Set pressure: %s\n", data);
      console.textAll("Set pressure: ");
      console.textAll((char*)data);
      console.textAll("<br>");
      targetPressure = atoi((char*)data);
      //Start pressing if target pressure is not met

      if (targetPressure > currentPressure) {
        Serial.println("Setting pressFlag");
        engagePress();
        pressFlag = 1;
      }
      else {
        Serial.println("Current higher than target");
      }
    }
  }
}

void readPressureCallback() {
  //Serial.println("T1 called");
  currentPressure = scale.get_units() * 2.20462;
  if ((currentPressure > -0.05) && (currentPressure < 0.05)) {
    currentPressure = 0.00;
  }
  //ws.textAll(String(currentPressure));
}

void monitorAndStopCallback() {
  //Serial.println("T2 called");
  if (pressFlag && !underPressure) {

    if (currentPressure > 55) {
      releasePress();
      underPressure = 0;
      pressFlag = 0;
    }

    if ((currentPressure > (targetPressure * .90))) {
      Serial.printf("Pressure is %.2f", currentPressure);
      stopPress();
      underPressure = 1;
      pressFlag = 1;
    }
  }
}
static char msg[100];

void returnToTargetPressureCallback() {

  if (underPressure && pressFlag) {
    //Serial.println("T3 Checking Pressure");

    if ( currentPressure > (targetPressure * .90)) {
      sprintf(msg, "Stopping press at %.2f<b>", getCurrentLocalTimeString(), currentPressure, targetPressure);

      //Serial.println("T3 stopping press");  // Always getting here under normal pressure
      stopPress();
    }

    if ( currentPressure < (targetPressure - 2)) {
      sprintf(msg, "%s Restoring pressure: from %.2f to %d <b>", getCurrentLocalTimeString(), currentPressure, targetPressure);
      console.textAll(msg);
      Serial.println("T3 increasing pressure");
      engagePress();
    }
  }
}

void broadcastPressureCallback() {
  ws.textAll(String(currentPressure));
}

static char payload[100];

void pubToIoTCallback() {

  if (!mqtt_client.connected())
  {
    establishConnection();
  }



  sprintf(payload, "{ \"time\" : \"%s\" , \"Pressure\" : %.2f }", getCurrentLocalTimeString(), currentPressure < 0 ? 0 : currentPressure);
  sendTelemetry(payload);

  sprintf(payload, "%s\" Pressure: %.2f <br>", getCurrentLocalTimeString(), currentPressure < 0 ? 0 : currentPressure);
  console.textAll(payload);

  // MQTT loop must be called to process Device-to-Cloud and Cloud-to-Device.
  mqtt_client.loop();
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
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

void onConsoleEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                    void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      //handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  console.onEvent(onConsoleEvent);
  server.addHandler(&console);
}

String processor(const String& var) {
  Serial.println(var);
  if (var == "STATE") {
    if (ledState) {
      return "ON";
    }
    else {
      return "OFF";
    }
  }
}


void setup() {
  // Serial port for debugging purposes
  Serial.begin(74880);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // IoT setup
  establishConnection();


  // Pin setup
  pinMode (PIN_SWITCH_1, OUTPUT);
  pinMode (PIN_SWITCH_2, OUTPUT);

  //Disable relays
  //  digitalWrite(PIN_SWITCH_1, HIGH);
  //  digitalWrite(PIN_SWITCH_2, HIGH);

  // In case ESP was powered down with scale under presse. Release pressure.
  releasePress();

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0


  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Route for root /console web page
  server.on("/console", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", console_html, processor);
  });


  runner.init();
  Serial.println("Initialized scheduler");

  runner.addTask(t1);
  runner.addTask(t2);
  runner.addTask(t3);
  runner.addTask(t4);
#ifdef _USE_AZURE_IOT
  runner.addTask(t5);
#endif
  t1.enable();
  t2.enable();
  t3.enable();
  t4.enable();
#ifdef _USE_AZURE_IOT
  t5.enable();
#endif
  // Start server
  server.begin();

}


void loop() {
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
  runner.execute();

}
