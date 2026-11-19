// PROJECT B - Website -> Arduino ("Remote Lamp")
// Subscribes to feed "lamp". Values: red, green, blue, off, beep,
// "r,g,b" (color picker), "bri:0-100" (brightness), "msg:text" (LED matrix).

#include <WiFiS3.h>
#include <Arduino_Modulino.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h"

const char* ssid   = SECRET_SSID;
const char* pass   = SECRET_PASS;
const char* ioUser = IO_USERNAME;
const char* ioKey  = IO_KEY;

const char* host = "10.0.0.156";
const int port = 8000;
String lastId = "";
unsigned long lastPoll = 0;

WiFiClient     wifiClient;
ModulinoPixels pixels;
ModulinoBuzzer buzzer;
ArduinoLEDMatrix matrix;

int r = 0, g = 0, b = 0, bri = 50;

void showColor() {
  for (int i = 0; i < 8; i++) pixels.set(i, ModulinoColor(r, g, b), bri);
  pixels.show();
}

void scrollText(const String& s) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(60);
  matrix.textFont(Font_5x7);
  matrix.beginText(12, 1, 0xFFFFFF);
  matrix.print(s);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void handleCommand(String v) {
  v.trim();
  Serial.print("Got: ");
  Serial.println(v);

  if      (v == "red")   { r = 255; g = 0;   b = 0;   showColor(); }
  else if (v == "green") { r = 0;   g = 255; b = 0;   showColor(); }
  else if (v == "blue")  { r = 0;   g = 0;   b = 255; showColor(); }
  else if (v == "off")   { r = g = b = 0; showColor(); }
  else if (v == "beep")  { buzzer.tone(440, 300); }
  else if (v == "chime") { buzzer.tone(659, 200); delay(220); buzzer.tone(880, 300); }
  else if (v.startsWith("bri:")) { bri = constrain(v.substring(4).toInt(), 0, 100); showColor(); }
  else if (v.startsWith("msg:")) { scrollText(v.substring(4)); }
  else {
    int c1 = v.indexOf(','), c2 = v.indexOf(',', c1 + 1);
    if (c1 > 0 && c2 > c1) {
      r = constrain(v.substring(0, c1).toInt(), 0, 255);
      g = constrain(v.substring(c1 + 1, c2).toInt(), 0, 255);
      b = constrain(v.substring(c2 + 1).toInt(), 0, 255);
      showColor();
    }
  }
}

void pollLamp() {
  if (!wifiClient.connect(host, port)) return;
  wifiClient.print(String("GET /raw/lamp HTTP/1.1\r\nHost: ") + host + "\r\nConnection: close\r\n\r\n");
  unsigned long t = millis();
  while (wifiClient.connected() && millis() - t < 2000) delay(5);
  String body = "";
  while (wifiClient.available()) body += (char)wifiClient.read();
  wifiClient.stop();
  int i = body.indexOf("\r\n\r\n");
  if (i < 0) return;
  body = body.substring(i + 4); body.trim();
  int bar = body.indexOf('|');
  if (bar < 0) return;
  String id = body.substring(0, bar);
  if (id == lastId || id == "none") return;
  lastId = id;
  handleCommand(body.substring(bar + 1));
}

void setup() {
  Serial.begin(9600);
  delay(1500);
  Modulino.begin();
  pixels.begin();
  buzzer.begin();
  matrix.begin();
  connectWiFi();
}

void loop() {
  if (millis() - lastPoll > 1000) { lastPoll = millis(); pollLamp(); }
}

void connectWiFi() {
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  Serial.print("\nConnected. IP: ");
  Serial.println(WiFi.localIP());
}
