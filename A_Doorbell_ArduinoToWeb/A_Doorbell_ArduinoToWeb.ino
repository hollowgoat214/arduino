// PROJECT A - Arduino -> Website ("Doorbell")
// Buttons A/B/C publish "A"/"B"/"C" to the Adafruit IO feed "doorbell".
// Rate limit: free IO = 30 msgs/min, so MIN_GAP_MS must stay >= 2000.

#include <WiFiS3.h>
#include <Arduino_Modulino.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h"

ArduinoLEDMatrix matrix;
void show(const char* t) {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(t);
  matrix.endText();
  matrix.endDraw();
}

const char* ssid   = SECRET_SSID;
const char* pass   = SECRET_PASS;
const char* ioUser = IO_USERNAME;
const char* ioKey  = IO_KEY;

const char* host = "10.0.0.156";
const int port = 8000;

const unsigned long MIN_GAP_MS = 2000;

WiFiClient      wifiClient;
ModulinoButtons buttons;

unsigned long lastSendMs = 0;

void setup() {
  Serial.begin(9600);
  matrix.begin();
  show("1");
  delay(1500);
  Serial.println("boot");

  Modulino.begin();
  show("2");
  buttons.begin();
  buttons.setLeds(true, true, true);

  show("W");
  connectWiFi();
  show(WiFi.status() == WL_CONNECTED ? "K" : "N");
  Serial.println("Ready. Press A, B or C.");
}

void loop() {
  if (buttons.update()) {
    if (buttons.isPressed('A')) { show("A"); sendEvent("A"); }
    if (buttons.isPressed('B')) { show("B"); sendEvent("B"); }
    if (buttons.isPressed('C')) { show("C"); sendEvent("C"); }
  }
}

void sendEvent(const char* value) {
  unsigned long now = millis();
  if (now - lastSendMs < MIN_GAP_MS) {
    Serial.println("Too soon - ignored (rate limit protection).");
    return;
  }
  lastSendMs = now;

  Serial.print("Sending: ");
  Serial.println(value);

  if (wifiClient.connect(host, port)) {
    String body = String("{\"value\":\"") + value + "\"}";
    wifiClient.print(String("POST /api/feeds/doorbell HTTP/1.1\r\nHost: ") + host + "\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length: " + body.length() + "\r\n\r\n" + body);
    delay(300);
    wifiClient.stop();
    show("S");
  } else {
    show("X");
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  Serial.print("\nConnected. IP address: ");
  Serial.println(WiFi.localIP());
}
