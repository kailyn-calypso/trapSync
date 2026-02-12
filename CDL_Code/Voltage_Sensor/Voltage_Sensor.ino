/*
This code reads the voltage from an Analog Voltage Divider / Sensor with a maximum input voltage of 25V and prints 
the result on the Serial Monitor of the Arduino IDE. The analog value from the sensor is converted to voltage using 
the formula voltage = sensor_value * (5.0 / 1023.0), where 5.0 is the voltage of the Arduino's power supply and 1023 
is the maximum analog value. The actual voltage is then calculated using the voltage divider formula Vout = Vin * R2 / (R1 + R2), 
where Vin is the maximum input voltage, R1 is the resistance of one of the resistors in the voltage divider, 
and R2 is the resistance of the other resistor in the voltage divider.
 
To use this code, connect the Analog Voltage Divider / Sensor to an analog input pin of your Arduino board 
(usually A0 for most Arduino boards), and connect the voltage divider resistors to the input voltage source and ground. 
The maximum input voltage should not exceed 25V. You will need to modify the code to use the specific values of R1 and R2 
used in your voltage divider circuit.
*/
 

//NONE OF THIS HAS BEEN TESTED!!!!
const int sensor_pin = A1; // Pin connected to the output of the voltage divider
int R1 = 30000;
int R2 = 7500;

void setup() {
  Serial.begin(9600);
}
 
void loop() {
  // Read the analog value from the sensor
  int sensor_value = analogRead(sensor_pin);
 
  // Convert the analog value to voltage (5V = 1023)
  float voltage = -1*sensor_value * (5.0 / 1023.0);
 
  // Convert the voltage to the actual voltage using the voltage divider ratio
  float actual_voltage = voltage * (R1 + R2) / R2;
 
  // Print the actual voltage
  Serial.print("Actual voltage: ");
  Serial.print(actual_voltage);
  Serial.println(" V");
 
  // Wait for 1 second before taking the next reading
  delay(1000);
}
