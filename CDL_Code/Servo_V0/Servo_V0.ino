/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep
*/

#include <ESP32Servo.h>

Servo myservo;  // create Servo object to control a servo
// twelve Servo objects can be created on most boards

    // variable to store the servo position
int pos = Serial.parseInt();
void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the Servo object
  
}

void loop() {
  Serial.println("Angle?");
  while (Serial.available()==0)
  {

  }
  pos = Serial.parseInt();
  myservo.write(pos);
}
