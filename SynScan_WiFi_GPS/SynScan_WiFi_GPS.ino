#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <stdint.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include "Config.h"
#include "OTA.h"
#include "Functions.h"

void setup() {
  //read the OTA pin state: HIGH = normal mode, LOW = OTA mode
  pinMode(OTA_pin, INPUT_PULLUP);
  if (digitalRead(OTA_pin) == HIGH) OTA_active = false;
  else OTA_active = true;

  //OTA active
  if (OTA_active) {
    setup_OTA();
  }
  //Normal mode, OTA not active
  else {
    //setup connections using baud for GPS mode
    gps.begin(GPS_module_baud);
    controller.begin(controller_baud_GPS_mode);
    CurrentMode = false;
  }
}

void loop() {
  //OTA update mode
  if (OTA_active) {
    ArduinoOTA.handle();
  }

  //Normal mode
  else {
    //process GPS requests and NMEA messages
    if (!GPS_terminated) {
      GPS_handle();
    }

    //treat WiFi requests
    if (WiFi_server_active) {
      if (!WiFi_initialized) {
        WiFi_setup();
        WiFi_initialized = true;
      }
      WiFi_handle();
    }

    //if no WiFi requests have been received for WiFi_client_timeout seconds, reactivate GPS mode
    if (millis() - WiFi_client_timer >= WiFi_client_timeout * 1000) GPS_terminated = false;
  }
}
