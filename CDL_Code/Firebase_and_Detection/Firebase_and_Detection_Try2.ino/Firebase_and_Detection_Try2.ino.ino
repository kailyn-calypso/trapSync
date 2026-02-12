/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino

  NEEDS TO BE AT 2.4GHz for the internet
*/
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Kailalaland"
#define WIFI_PASSWORD "japolalalander"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBJO0ddbR5fF9Kz3KZvBhdXeSNQGPLf2Fo"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp32-firebase-demo-b35ad-default-rtdb.firebaseio.com/" 

#define SENSOR_PIN 5

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup(){
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  static bool lastSensorState = LOW;

  // Read sensor pin state
  bool sensorState = digitalRead(SENSOR_PIN);

  if (Firebase.ready() && signupOK) {

    // Detect rising edge: LOW -> HIGH
    if (sensorState == HIGH && lastSensorState == LOW) {
      Serial.println("Mouse detected!");

      // Send "mouse_detected" = true to Firebase
      if (Firebase.RTDB.setBool(&fbdo, "mouse_detected", true)) {
        Serial.println("mouse_detected set to true!");
      } else {
        Serial.println("FAILED to set mouse_detected: " + fbdo.errorReason());
      }

      // Wait for "Open_door" to become true
      bool doorOpen = false;
      Serial.println("Waiting for Open_door to be true...");
      while (!doorOpen) {
        if (Firebase.RTDB.getBool(&fbdo, "Open_door")) {
          doorOpen = fbdo.boolData();
          Serial.println("Open_door: " + String(doorOpen));
        } else {
          Serial.println("FAILED to read Open_door: " + fbdo.errorReason());
        }
        delay(1000);  // Avoid spamming requests
      }

      Serial.println("Door is open! Proceeding...");
    }

    lastSensorState = sensorState;
  }
}