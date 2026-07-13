/*
  project7_servo.ino

  Reads a rotary potentiometer on A0 and controls a servo on D7.
  Prints the raw ADC value, mapped angle, and microsecond pulse width
  so you can observe the signal with ADALM (connect Analog 2 to D7).

  Wiring:
  - Rotary wiper -> A0
  - Rotary ends -> 5V and GND
  - Servo signal -> D7
  - Servo V+ -> external 5V (do NOT power servo from Arduino 3.3V)
  - Common GND between Arduino and servo

  Notes:
  - Uses the built-in Servo library (install via Library Manager if missing).
  - Typical servo pulse widths: ~1000..2000 us (1ms..2ms) at ~50 Hz.
  - ADALM scope: connect Analogue 2 (dark blue) to D7 to view the PWM.
*/

#include <Servo.h>

const int analogPin = A0;
const int servoPin = 7; // digital pin connected to servo signal

Servo myServo;

const unsigned long sampleIntervalMs = 100; // update every 100 ms
unsigned long lastSample = 0;

// Use observed pulse width range (in microseconds).
// Measured values: 1000 (min) .. 1995 (max)
const int minPulse = 1000;
const int maxPulse = 1995;

void setup() {
  Serial.begin(9600);
  delay(50);
  // Attach servo using observed pulse-width limits so 0..180 maps correctly
  myServo.attach(servoPin, minPulse, maxPulse);
  Serial.println("project7_servo: reading A0 -> controlling servo on D7");
  Serial.println("Turn the rotary to move the servo. ADALM Analogue2 -> D7 to observe PWM.");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSample < sampleIntervalMs) return;
  lastSample = now;

  int raw = analogRead(analogPin); // eg 0..1023

  // Map ADC to pulse width
  int pulse = map(raw, 0, 1023, minPulse, maxPulse);

  // Option A: use writeMicroseconds for precise control
  myServo.writeMicroseconds(pulse);

  // For readability, also compute an approximate angle (0..180)
  int angle = map(raw, 0, 1023, 0, 180);

  Serial.print("ADC: "); Serial.print(raw);
  Serial.print("  Angle(~): "); Serial.print(angle);
  Serial.print("  Pulse(us): "); Serial.println(pulse);
}
