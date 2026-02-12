// Mouse Trap ESP32 Code with Sleep Optimization
#include <ESP32Servo.h>
#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"

Servo myservo;
#define OPEN_POS 90
#define CLOSE_POS 0
#define SERVO_PIN 10

#define IR_PIN 8
#define RED_PIN 6
#define YELLOW_PIN 5
#define GREEN_PIN 4
#define BLUE_PIN 3
#define VSENS_PIN A1
#define R2 30000
#define R1 7500

#define WIFI_SSID "Kailalaland"
#define WIFI_PASSWORD "japolalalander"
#define API_KEY "AIzaSyBNCjZAfq7LwryjZ5ZIhv0WpttO4jXwO7M"
#define DATABASE_URL "https://trapsync-a8239-default-rtdb.firebaseio.com/"
#define USER_EMAIL "cscdlfirebase@gmail.com"
#define USER_PASSWORD "Ineffable!"

// Firebase declarations
SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;

// System State
enum State { WAIT_FOR_MOUSE, WAIT_FOR_RELEASE, WAIT_FOR_RESET };
State currentState = WAIT_FOR_MOUSE;

volatile bool mouse_detected = false;
volatile bool interrupt_enabled = true;

void IRAM_ATTR beamBroken() {
  if (interrupt_enabled) {
    mouse_detected = true;
    interrupt_enabled = false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IR_PIN), beamBroken, FALLING);

  WiFi_Setup();
  Initialize();
}

void loop() {
  app.loop();

  if (currentState == WAIT_FOR_MOUSE && mouse_detected) {
    mouse_detected = false;
    myservo.attach(SERVO_PIN);
    myservo.write(CLOSE_POS);
    delay(500);
    myservo.detach();

    digitalWrite(BLUE_PIN, HIGH);
    fireSet_mouse_caught(true);
    fireSet_debug("Mouse caught!");
    currentState = WAIT_FOR_RELEASE;
  } 
  else if (currentState == WAIT_FOR_RELEASE && fireGet_release_mouse()) {
    myservo.attach(SERVO_PIN);
    myservo.write(OPEN_POS);
    delay(500);
    myservo.detach();

    digitalWrite(BLUE_PIN, LOW);
    fireSet_mouse_caught(false);
    fireSet_release_mouse(false);
    fireSet_debug("Mouse released.");
    currentState = WAIT_FOR_RESET;
  } 
  else if (currentState == WAIT_FOR_RESET && fireGet_reset()) {
    Initialize();
    fireSet_debug("Trap reset.");
    currentState = WAIT_FOR_MOUSE;
  }

  Battery_State();
  delay(5);
}

void Initialize() {
  fireSet_mouse_caught(false);
  fireSet_release_mouse(false);
  fireSet_reset(false);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  interrupt_enabled = true;
  myservo.attach(SERVO_PIN);
  myservo.write(OPEN_POS);
  delay(500);
  myservo.detach();
}

void WiFi_Setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
  initializeApp(aClient, app, getAuth(user_auth), 120 * 1000, auth_debug_print);
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
  fireSet_debug("WiFi connected and database initialized.");
}

void Battery_State() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10000) {
    lastUpdate = millis();
    int sensor_value = analogRead(VSENS_PIN);
    double voltage = sensor_value * (3.3 / 4095);
    double batteryLevel = voltage * 5;
    fireSet_battery_level(batteryLevel);

    digitalWrite(RED_PIN, batteryLevel < 6.5 ? HIGH : LOW);
    digitalWrite(YELLOW_PIN, (batteryLevel > 6.5 && batteryLevel <= 7.5) ? HIGH : LOW);
    digitalWrite(GREEN_PIN, (batteryLevel > 7.5 && batteryLevel <= 9) ? HIGH : LOW);
  }
}


// Firebase Setters
void fireSet_mouse_caught(bool value) { Database.set<bool>(aClient, "/trap/mouse_caught", value); }
void fireSet_release_mouse(bool value) { Database.set<bool>(aClient, "/trap/release_mouse", value); }
void fireSet_reset(bool value) { Database.set<bool>(aClient, "/trap/reset", value); }
void fireSet_battery_level(double value) { Database.set<double>(aClient, "/trap/battery_level", value); }
void fireSet_debug(const String& message) {
    if (Database.set<String>(aClient, "/trap/debug", message)) {
        Serial.print("Debug message sent: ");
        Serial.println(message);
    } else {
        Firebase.printf("Failed to set debug: %s\n", aClient.lastError().message().c_str());
    }
}
// Firebase Getters
bool fireGet_release_mouse() { return Database.get<bool>(aClient, "/trap/release_mouse"); }
bool fireGet_reset() { return Database.get<bool>(aClient, "/trap/reset"); }
