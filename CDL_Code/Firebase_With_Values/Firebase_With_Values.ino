#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h" // Provides the functions used in the examples.

#define WIFI_SSID "Kailalaland"
#define WIFI_PASSWORD "japolalalander"

#define API_KEY "AIzaSyBNCjZAfq7LwryjZ5ZIhv0WpttO4jXwO7M"
#define USER_EMAIL "cscdlfirebase@gmail.com"
#define USER_PASSWORD "Ineffable!"
#define DATABASE_URL "https://trapsync-a8239-default-rtdb.firebaseio.com/"

void processData(AsyncResult &aResult);
// Setters
void set_mouse_caught(bool mouseCaught);
void set_battery_level(double batteryLevel);
void set_release_mouse(bool releaseMouse);

// Getters
bool get_mouse_caught();
double get_battery_level();
bool get_release_mouse();


void create_dummy_data();

void simulate_user_updates();


SSL_CLIENT ssl_client;

// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000 /* expire period in seconds (<3600) */);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;

bool taskComplete = false;

void setup()
{
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    set_ssl_client_insecure_and_buffer(ssl_client);

    Serial.println("Initializing app...");
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");

    // Or intialize the app and wait.
    // initializeApp(aClient, app, getAuth(user_auth), 120 * 1000, auth_debug_print);

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);

    //setting values individually
    set_mouse_caught(true);
    set_battery_level(9);
    set_release_mouse(false);
}


void loop() {
  app.loop();

    if (app.ready() && !taskComplete)
    {
      simulate_user_updates();

      bool isMouseCaught = get_mouse_caught();
      double currentBattery = get_battery_level();
      bool shouldReleaseMouse = get_release_mouse();

      // Now you can use these variables in your logic
      if (isMouseCaught) {
          Serial.println("Mouse was caught!");
      }

      if (currentBattery < 6.5) {
          Serial.println("Battery is low!");
      }

      if (shouldReleaseMouse) {
          Serial.println("Release the mouse!");
      }
    }

}

// Set mouse_caught
void set_mouse_caught(bool mouseCaught) {
    if (Database.set<bool>(aClient, "/trap/mouse_caught", mouseCaught)) {
        Serial.println("Set mouse_caught successfully");
    } else {
        Firebase.printf("Failed to set mouse_caught: %s\n", aClient.lastError().message().c_str());
    }
}

// Set battery_level
void set_battery_level(double batteryLevel) {
    if (Database.set<double>(aClient, "/trap/battery_level", batteryLevel)) {
        Serial.println("Set battery_level successfully");
    } else {
        Firebase.printf("Failed to set battery_level: %s\n", aClient.lastError().message().c_str());
    }
}

// Set release_mouse
void set_release_mouse(bool releaseMouse) {
    if (Database.set<bool>(aClient, "/trap/release_mouse", releaseMouse)) {
        Serial.println("Set release_mouse successfully");
    } else {
        Firebase.printf("Failed to set release_mouse: %s\n", aClient.lastError().message().c_str());
    }
}


// Get mouse_caught
bool get_mouse_caught() {
    bool value = false;
    value = Database.get<bool>(aClient, "/trap/mouse_caught");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got mouse_caught successfully");
    } else {
        Firebase.printf("Failed to get mouse_caught: %s\n", aClient.lastError().message().c_str());
    }
    return value;
}

// Get battery_level
double get_battery_level() {
    double value = 0.0;
    value = Database.get<double>(aClient, "/trap/battery_level");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got battery_level successfully");
    } else {
        Firebase.printf("Failed to get battery_level: %s\n", aClient.lastError().message().c_str());
    }
    return value;
}

// Get release_mouse
bool get_release_mouse() {
    bool value = false;
    value = Database.get<bool>(aClient, "/trap/release_mouse");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got release_mouse successfully");
    } else {
        Firebase.printf("Failed to get release_mouse: %s\n", aClient.lastError().message().c_str());
    }
    return value;
}

void simulate_user_updates() {
    // First: Update battery_level
    Serial.println("Enter new battery level (double): ");
    while (Serial.available() == 0) {
        // wait for user input
    }
    double newBatteryLevel = Serial.parseFloat();
    set_battery_level(newBatteryLevel);
    Serial.print("Battery level updated to: ");
    Serial.println(newBatteryLevel);

    delay(500); // short pause before next

    // Second: Update mouse_caught
    Serial.println("Enter mouse caught state (0 for false, 1 for true): ");
    while (Serial.available() == 0) {
        // wait for user input
    }
    int mouseCaughtInput = Serial.parseInt();
    bool newMouseCaught = (mouseCaughtInput != 0);
    set_mouse_caught(newMouseCaught);
    Serial.print("Mouse caught state updated to: ");
    Serial.println(newMouseCaught);

    delay(500); // short pause before next

    // Third: Update release_mouse
    Serial.println("Enter release mouse state (0 for false, 1 for true): ");
    while (Serial.available() == 0) {
        // wait for user input
    }
    int releaseMouseInput = Serial.parseInt();
    bool newReleaseMouse = (releaseMouseInput != 0);
    set_release_mouse(newReleaseMouse);
    Serial.print("Release mouse state updated to: ");
    Serial.println(newReleaseMouse);

    delay(500);
}
