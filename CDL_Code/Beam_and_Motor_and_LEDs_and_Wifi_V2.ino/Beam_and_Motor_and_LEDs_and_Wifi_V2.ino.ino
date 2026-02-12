
#include <ESP32Servo.h>  //change to this one for the esp32
#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h" // Provides the functions used in the examples.
///////////////////////////////////////////////////////
Servo myservo;  // create Servo object to control a servo
#define OPEN_POS 90
#define CLOSE_POS 0

#define IR_PIN 8 // the pin that the pushsensor is attached to

#define RED_PIN  6
#define YELLOW_PIN 5
#define GREEN_PIN 4
#define BLUE_PIN 3

#define R2 30000 //resistor 1 of voltage sensor
#define R1 7500  //resistor 2 of voltage sensor
#define VSENS_PIN A1 // Pin connected to the output of the voltage divider

//////////////////////////////////////////////////////////////
#define WIFI_SSID "Kailalaland"
#define WIFI_PASSWORD "japolalalander"
#define API_KEY "AIzaSyBNCjZAfq7LwryjZ5ZIhv0WpttO4jXwO7M"
#define DATABASE_URL "https://trapsync-a8239-default-rtdb.firebaseio.com/"
#define USER_EMAIL "cscdlfirebase@gmail.com"
#define USER_PASSWORD "Ineffable!"

//////////////////////////////////////////////////////////////////

//function declarations
void WiFi_Setup();
void Battery_State();
bool Release_Mouse();
void Initialize();
// Setters
void fireSet_mouse_caught(bool mouseCaught);
void fireSet_battery_level(double batteryLevel);
void fireSet_release_mouse(bool releaseMouse);
void fireSet_mouse_still_in(bool mouse_still_in);
void fireSet_reset(bool reset);

// Getters
bool fireGet_mouse_caught();
double fireGet_battery_level();
bool fireGet_release_mouse();
bool fireGet_reset();
///////////////////////////////////////////////////////////////////////////
//filler variables that NEED TO BE ADDED
int batteryLevel;
bool mouse_detected = false;

// Variables will change:
int lastState = 1; 
int sensorState = 1;        // current state of the sensor
int mouseInState = 0;

unsigned long lastBeamBreakTime = 0;
bool mouseStillIn = false;

volatile bool interrupt_enabled = true; 
void IRAM_ATTR beamBroken() {
    if (interrupt_enabled && !mouseInState) {
        mouse_detected = true;
        interrupt_enabled = false; // Disable further interrupts until reset
        digitalWrite(BLUE_PIN, HIGH);
        myservo.write(CLOSE_POS); // Trap the mouse
        mouseInState = 1;
    }
    else if (interrupt_enabled)
    {
        mouse_detected = true;
        interrupt_enabled = false; // Disable further interrupts until reset
    }
}

enum State { WAIT_FOR_MOUSE, WAIT_FOR_RELEASE, WAIT_FOR_RESET };
State currentState = WAIT_FOR_MOUSE;


SSL_CLIENT ssl_client;
////////////////////////////////////////////////////////////////////////////////
// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000 /* expire period in seconds (<3600) */);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;

bool taskComplete = false;
/////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  pinMode(RED_PIN, OUTPUT); 
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // attaches the servo on pin 10 to the Servo object
  myservo.attach(10);

  // initialize the button pin as a input:
  pinMode(IR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), beamBroken, FALLING); // react when beam broken (LOW)
  // initialize serial communication:
  Serial.begin(115200);
  WiFi_Setup();
  Initialize();
}

void loop()
{
    app.loop(); // Firebase background operations

    // Check if beam broken (interrupt triggered)
    if (currentState == WAIT_FOR_MOUSE && mouse_detected)
    {
        mouse_detected = false; // Clear flag
        
        Serial.println("Mouse Caught!");
        Serial.println("Beam Broken (interrupt)");
        fireSet_mouse_caught(true);


        currentState = WAIT_FOR_RELEASE;
    }
    else if (currentState == WAIT_FOR_RELEASE && Release_Mouse())//when mouse release command is true
    {
        currentState = WAIT_FOR_RESET;
        fireSet_debug("Mouse Released.");
        lastBeamBreakTime = millis();
        interrupt_enabled = true;
        
    }
    else if (currentState == WAIT_FOR_RESET )//while waiting for reset, in case mouse is still in
    {
        if (fireGet_reset())
        {
            Initialize();
            fireSet_debug("Trap reset.");
            currentState = WAIT_FOR_MOUSE;
        
        }
        if (mouse_detected) {
        lastBeamBreakTime = millis();
        mouse_detected = false;

            if (!mouseStillIn) {
                fireSet_mouse_still_in(true);
                fireSet_debug("Mouse is still in the trap!");
                mouseStillIn = true;
            }
        }

        if (mouseStillIn && millis() - lastBeamBreakTime >= 10000) {
            fireSet_mouse_still_in(false);
            fireSet_debug("Mouse should be out! Waited 10s.");
            mouseStillIn = false;
        }

    }

    Battery_State(); // Still update battery voltage (internally slowed to 10s)

    delay(5); // Small delay to let WiFi and other tasks breathe
}


void Initialize()
{
    Serial.println("Initializing...");
    fireSet_mouse_caught(false);
    fireSet_release_mouse(false);
    fireSet_mouse_still_in(false);
    fireSet_reset(false);
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    myservo.write(OPEN_POS);
    interrupt_enabled = true; // Re-enable interrupt for next mouse
    mouseStillIn = false;
    lastBeamBreakTime = millis();
    mouse_detected = false;
    mouseInState = 0;
}

void WiFi_Setup()
{
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
    //initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");

    // Or intialize the app and wait.
    initializeApp(aClient, app, getAuth(user_auth), 120 * 1000, auth_debug_print);

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
    fireSet_debug("Wi-Fi Set!!");
}

void Battery_State()
{
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 10000) { // update every 10 seconds
        lastUpdate = millis();
        int sensor_value = analogRead(VSENS_PIN);
        // Convert the analog value to voltage (3.3V = 1023)
        double voltage = sensor_value * (3.3 / 4095);
        // Convert the voltage to the actual voltage using the voltage divider ratio
        double batteryLevel = voltage * 5;
        fireSet_battery_level(batteryLevel);
        //status indication
        digitalWrite(RED_PIN, batteryLevel < 6.5 ? HIGH : LOW);
        digitalWrite(YELLOW_PIN, (batteryLevel > 6.5 && batteryLevel <= 7.5) ? HIGH : LOW);
        digitalWrite(GREEN_PIN, (batteryLevel > 7.5 && batteryLevel <= 9) ? HIGH : LOW);
        fireSet_debug("Battery updated: " + String(batteryLevel, 2) + "V");

    }
}


bool Release_Mouse()
{
    //mouse indication LED
    if (fireGet_release_mouse())
    {
        digitalWrite(BLUE_PIN, LOW);
        myservo.write(OPEN_POS);
        fireSet_mouse_caught(false);
        fireSet_release_mouse(false);
        Serial.println("Mouse Released!");
        fireSet_debug("Mouse Released!");
        return true;
    }
    else
    {
        return false;
    }
}



// Set mouse_caught
void fireSet_mouse_caught(bool mouseCaught) {
    if (Database.set<bool>(aClient, "/trap/mouse_caught", mouseCaught)) {
        Serial.println("Set mouse_caught successfully");
        fireSet_debug("Set mouse_caught successfully");
    } else {
        Firebase.printf("Failed to set mouse_caught: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to set mouse_caught");
    }
}

// Set battery_level
void fireSet_battery_level(double batteryLevel) {
    if (Database.set<double>(aClient, "/trap/battery_level", batteryLevel)) {
        Serial.println("Set battery_level successfully");
        fireSet_debug("Set battery_level successfully");
    } else {
        Firebase.printf("Failed to set battery_level: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to set battery_level");
    }
}

// Set release_mouse
void fireSet_release_mouse(bool releaseMouse) {
    if (Database.set<bool>(aClient, "/trap/release_mouse", releaseMouse)) {
        Serial.println("Set release_mouse successfully");
        fireSet_debug("Set release_mouse successfully");
    } else {
        Firebase.printf("Failed to set release_mouse: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to set release_mouse");
    }
}

// Set reset
void fireSet_reset(bool reset) {
    if (Database.set<bool>(aClient, "/trap/reset", reset)) {
        Serial.println("Set reset successfully");
        fireSet_debug("Set reset successfully");
    } else {
        Firebase.printf("Failed to set reset: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to set reset");
    }
}

//Set mouse still in during release phase
void fireSet_mouse_still_in(bool mouse_still_in) {
    if (Database.set<bool>(aClient, "/trap/mouse_still_in", mouse_still_in)) {
        Serial.println("Set mouse_still_in successfully");
        fireSet_debug("Set mouse_still_in successfully");
    } else {
        Firebase.printf("Failed to set mouse_still_in: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to set mouse_still_in");
    }
}


void fireSet_debug(const String& message) {
    if (Database.set<String>(aClient, "/trap/debug", message)) {
        Serial.print("Debug message sent: ");
        Serial.println(message);
    } else {
        Firebase.printf("Failed to set debug: %s\n", aClient.lastError().message().c_str());
    }
}


// Get mouse_caught
bool fireGet_mouse_caught() {
    bool value = false;
    value = Database.get<bool>(aClient, "/trap/mouse_caught");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got mouse_caught successfully");
        fireSet_debug("Got mouse_caught successfully");
    } else {
        Firebase.printf("Failed to get mouse_caught: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to get mouse_caught");
    }
    return value;
}

// Get battery_level
double fireGet_battery_level() {
    double value = 0.0;
    value = Database.get<double>(aClient, "/trap/battery_level");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got battery_level successfully");
        fireSet_debug("Got battery_level successfully");
    } else {
        Firebase.printf("Failed to get battery_level: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to get battery_level");
    }
    return value;
}

// Get release_mouse
bool fireGet_release_mouse() {
    bool value = false;
    value = Database.get<bool>(aClient, "/trap/release_mouse");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got release_mouse successfully");
        fireSet_debug("Got release_mouse successfully");
    } else {
        Firebase.printf("Failed to get release_mouse: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to get release_mouse");
    }
    return value;
}

// Get release_mouse
bool fireGet_reset() {
    bool value = false;
    value = Database.get<bool>(aClient, "/trap/reset");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got reset successfully");
        fireSet_debug("Got reset successfully");
    } else {
        Firebase.printf("Failed to get reset: %s\n", aClient.lastError().message().c_str());
        fireSet_debug("Failed to get reset");
    }
    return value;
}