
#include <ESP32Servo.h>  //change to this one for the esp32

///////////////////////////////////////////////////////
Servo myservo;  // create Servo object to control a servo
#define OPEN_POS 0;
#define CLOSE_POS 90;

#define IR_PIN = 8; // the pin that the pushsensor is attached to

#define RED_PIN  6
#define YELLOW_PIN 5
#define GREEN_PIN 4
#define BLUE_PIN 3

#define R1 30000 //resistor 1 of voltage sensor
#define R2 7500  //resistor 2 of voltage sensor
#define VSENS_PIN A1 // Pin connected to the output of the voltage divider

//////////////////////////////////////////////////////////////
#define WIFI_SSID "Kailalaland"
#define WIFI_PASSWORD "japolalalander"

#define API_KEY "AIzaSyBNCjZAfq7LwryjZ5ZIhv0WpttO4jXwO7M"
#define USER_EMAIL "cscdlfirebase@gmail.com"
#define USER_PASSWORD "Ineffable!"
#define DATABASE_URL "https://trapsync-a8239-default-rtdb.firebaseio.com/"
//////////////////////////////////////////////////////////////////


//filler variables that NEED TO BE ADDED
int batteryLevel;
bool detectMouse = false;

// Variables will change:
int lastState = 1; 
int sensorState = 1;        // current state of the sensor

//function declarations
void WiFi_Setup()
void Battery_State(double voltage);
void Catch_Mouse();
void Release_Mouse();

// Setters
void fireSet_mouse_caught(bool mouseCaught);
void fireSet_battery_level(double batteryLevel);
void fireSet_release_mouse(bool releaseMouse);

// Getters
bool fireGet_mouse_caught();
double fireGet_battery_level();
bool fireGet_release_mouse();



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
        // initialize the LED as an output:
        //pinMode(ledPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(9600);
  WiFi_Setup()
}


void loop() 
{
  //battery detection
    int sensor_value = analogRead(VSENS_PIN);
    Battery_State(sensor_value);

    Catch_Mouse();
    Release_Mouse();




  delay(1000);
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
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");

    // Or intialize the app and wait.
    // initializeApp(aClient, app, getAuth(user_auth), 120 * 1000, auth_debug_print);

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
}

void Battery_State(double sensor_value)
{
      // Convert the analog value to voltage (5V = 1023)
    float voltage = -1*sensor_value * (5.0 / 1023.0);
    // Convert the voltage to the actual voltage using the voltage divider ratio
    float batteryLevel = voltage * (R1 + R2) / R2;
    fireSet_battery_level(batteryLevel);
  //status indication
    digitalWrite(RED_PIN, batteryLevel > 12 ? HIGH : LOW);
    digitalWrite(YELLOW_PIN, (batteryLevel > 9 && batteryLevel <= 12) ? HIGH : LOW);
    digitalWrite(GREEN_PIN, (batteryLevel > 6 && batteryLevel <= 9) ? HIGH : LOW);
}

void Catch_Mouse()
{
  sensorState = digitalRead(IR_PIN);
  while ((sensorState==lastState)&&(sensorState != LOW))
  {
    if ((sensorState!=lastState)&&(sensorState == LOW))//Wait for sensor break
    {
      delay(50);
    }
  }
  // If the sensor is triggered (beam broken)  
  Serial.println("Broken");
  fireSet_mouse_caught(true); //set database mouse caught
  digitalWrite(BLUE_PIN, HIGH); //set blue LED
  myservo.write(CLOSE_POS); //close trap door

  // Update the lastState variable to the current state
  lastState = sensorState;
  
}


void Release_Mouse()
{
    //mouse indication LED
    while (~fireGet_release_mouse())
    {//Wait for release command  
    }
    digitalWrite(BLUE_PIN, LOW);
    myservo.write(OPEN_POS);
    fireSet_mouse_caught(false);
    fireSet_release_mouse(false);
    
}


// Set mouse_caught
void fireSet_mouse_caught(bool mouseCaught) {
    if (Database.set<bool>(aClient, "/trap/mouse_caught", mouseCaught)) {
        Serial.println("Set mouse_caught successfully");
    } else {
        Firebase.printf("Failed to set mouse_caught: %s\n", aClient.lastError().message().c_str());
    }
}

// Set battery_level
void fireSet_battery_level(double batteryLevel) {
    if (Database.set<double>(aClient, "/trap/battery_level", batteryLevel)) {
        Serial.println("Set battery_level successfully");
    } else {
        Firebase.printf("Failed to set battery_level: %s\n", aClient.lastError().message().c_str());
    }
}

// Set release_mouse
void fireSet_release_mouse(bool releaseMouse) {
    if (Database.set<bool>(aClient, "/trap/release_mouse", releaseMouse)) {
        Serial.println("Set release_mouse successfully");
    } else {
        Firebase.printf("Failed to set release_mouse: %s\n", aClient.lastError().message().c_str());
    }
}


// Get mouse_caught
bool fireGet_mouse_caught() {
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
double fireGet_battery_level() {
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
bool fireGet_release_mouse() {
    bool value = false;
    value = Database.get<bool>(aClient, "/trap/release_mouse");
    if (aClient.lastError().code() == 0) {
        Serial.println("Got release_mouse successfully");
    } else {
        Firebase.printf("Failed to get release_mouse: %s\n", aClient.lastError().message().c_str());
    }
    return value;
}