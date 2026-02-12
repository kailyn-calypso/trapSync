/*
  State change detection (edge detection)

  Often, you don't need to know the state of a digital input all the time, but
  you just need to know when the input changes from one state to another.
  For example, you want to know when a sensor goes from OFF to ON. This is called
  state change detection, or edge detection.

  This example shows how to detect when a sensor or sensor changes from off to on
  and on to off.

  The circuit:
  - pushsensor attached to pin 2 from +5V
  - 10 kilohm resistor attached to pin 2 from ground
  - LED attached from pin 13 to ground through 220 ohm resistor (or use the
    built-in LED on most Arduino boards)

  created  27 Sep 2005
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/StateChangeDetection
*/

// this constant won't change:
const int sensorPin = 2;  // the pin that the pushsensor is attached to
const int ledPin = 6;    // the pin that the LED is attached to

// Variables will change:
int lastState=1; 

int sensorState = 1;        // current state of the sensor

void setup() {
  // initialize the button pin as a input:
  pinMode(sensorPin, INPUT_PULLUP);
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(9600);
}


void loop() {
  // Read the sensor input pin:
  sensorState = digitalRead(sensorPin);

  // Detect state change (edge detection):
  if (sensorState != lastState) {  // Check if the state has changed
    if (sensorState == LOW) {      // If the sensor is triggered (beam broken)
      Serial.println("Broken");
      digitalWrite(ledPin, HIGH);  // Turn on the LED
    } else {                      // If the sensor is untriggered (beam unbroken)
      Serial.println("Unbroken");
      digitalWrite(ledPin, LOW);   // Turn off the LED
    }
  }

  // Update the lastState variable to the current state
  lastState = sensorState;

  // Delay a little bit to avoid bouncing
  delay(50);
}
