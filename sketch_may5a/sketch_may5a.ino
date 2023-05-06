#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const int pinRele = 2;
const char* ssid = "ssid";
const char* password = "password";
const String queryParameter = "timer_to_off";
bool status = false;

ESP8266WebServer server(80);

void turn_device(bool new_status) {
  status = new_status;
  if (status) {
    digitalWrite(pinRele, HIGH);
  } else {
    digitalWrite(pinRele, LOW);
  }
}

void autoOff() {
  if (server.hasArg(queryParameter)) {
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == queryParameter) {
        String time_off = String(server.arg(i));
        int time_off_length = time_off.length();
        int qt = 0;
        for (int j = 0; j < time_off_length; j++) {
          if (isDigit(time_off[j])) {
            qt++;
          } else {
            break;
          }
        }
        if (qt == time_off_length) {
          turn_device(true);
          Serial.print("Esperando");
          server.send(200, "text/json", "{\"status\": true}");
          delay(time_off.toInt());
          turn_device(false);
          return;
        }
        break;
      }
    }
  }
  server.send(200, "text/json", "{\"status\": false}");
}

void on() {
  turn_device(true);
  server.send(200, "text/json", "{\"status\": true}");
}

void off() {
  turn_device(false);
  server.send(200, "text/json", "{\"status\": false}");
}

void status_device() {
  if (status) {
    server.send(200, "text/json", "{\"status\": true}");
  } else {
    server.send(200, "text/json", "{\"status\": false}");
  }
}

void routing() {
  server.on(F("/auto_off"), HTTP_GET, autoOff);
  server.on(F("/on"), HTTP_GET, on);
  server.on(F("/off"), HTTP_GET, off);
  server.on(F("/status"), HTTP_GET, status_device);
}

void handleNotFound() {
  String message = "Page Not Found";
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);

  pinMode(pinRele, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  routing();
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
