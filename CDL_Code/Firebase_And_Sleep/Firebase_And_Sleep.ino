#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h" // Provides the functions used in the examples.

#define WIFI_SSID "Kailalaland"
#define WIFI_PASSWORD "japolalalander"

#define API_KEY "AIzaSyBNCjZAfq7LwryjZ5ZIhv0WpttO4jXwO7M"
#define USER_EMAIL "cscdlfirebase@gmail.com"
#define USER_PASSWORD "Ineffable!"
#define DATABASE_URL "https://trapsync-a8239-default-rtdb.firebaseio.com/"

#define WAKEUP_GPIO 2
RTC_DATA_ATTR int bootCount = 0;

// Variables
int GREEN_LED_PIN = 6;
int YELLOW_LED_PIN = 7;

// Firebase
SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
UserAuth user_auth(API_KEY, "your_email", "your_password", 3000);
FirebaseApp app;
RealtimeDatabase Database;

// Firebase variable functions
void set_mouse_caught(bool mouseCaught);
void set_battery_level(double batteryLevel);
void set_release_mouse(bool releaseMouse);
bool get_mouse_caught();
double get_battery_level();
bool get_release_mouse();

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);

  if (bootCount == 0) {
    Serial.println("First Boot");
    digitalWrite(YELLOW_LED_PIN, HIGH);
    bootCount++;
    delay(3000);
    digitalWrite(YELLOW_LED_PIN, LOW);

    // Setup wakeup trigger
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_GPIO, 0);  // Wake on LOW
    gpio_pullup_dis((gpio_num_t)WAKEUP_GPIO);
    Serial.println("Going to sleep...");
    esp_deep_sleep_start();
  }
  else {
    Serial.println("Woken up by GPIO event!");

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(300);
    }
    Serial.println("\nWi-Fi Connected");

    // Setup Firebase
    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    set_ssl_client_insecure_and_buffer(ssl_client);
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "authTask");

    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    // Example of sending to Firebase
    set_mouse_caught(true);
    double battery = 85.0;  // Example battery level
    set_battery_level(battery);
    set_release_mouse(false);

    // Optionally get data back
    bool releaseNow = get_release_mouse();
    if (releaseNow) {
      Serial.println("Releasing mouse now...");
    }

    // Blinking Green to show action done
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(GREEN_LED_PIN, LOW);

    // Reset bootCount to allow next trigger
    bootCount = 0;

    delay(2000);  // Small pause
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);
    rtc_gpio_pullup_dis(WAKEUP_GPIO);
    Serial.println("Going back to sleep...");
    esp_deep_sleep_start();
  }
}

void loop() {
  // empty
}