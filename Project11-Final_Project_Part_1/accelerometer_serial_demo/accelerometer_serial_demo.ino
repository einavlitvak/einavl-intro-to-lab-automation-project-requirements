// Project 11 starter sketch: read the Grove accelerometer and print X/Y/Z to Serial.

#include "Arduino_SensorKit.h"

// Print sensor readings every half second.
const unsigned long PRINT_INTERVAL_MS = 500;

// LIS3DHTR readings are in g. Use a simple range for a first pass.
const float ACCEL_MIN_G = -1.5f;
const float ACCEL_MAX_G = 1.5f;
// Servo angle range we will use later for the fan.
const int SERVO_MIN_DEG = 0;
const int SERVO_MAX_DEG = 180;



unsigned long lastPrintMs = 0;

int computeAngleFromX(float xG) {
  float clampedG = xG;

  if (clampedG < ACCEL_MIN_G) {
    clampedG = ACCEL_MIN_G;
  }

  if (clampedG > ACCEL_MAX_G) {
    clampedG = ACCEL_MAX_G;
  }

  float normalized = (clampedG - ACCEL_MIN_G) / (ACCEL_MAX_G - ACCEL_MIN_G);
  float angle = SERVO_MIN_DEG + normalized * (SERVO_MAX_DEG - SERVO_MIN_DEG);

  return (int)(angle + 0.5f);
}

void setup() {
  Serial.begin(9600);

  // Start the Grove 3-axis accelerometer.
  Accelerometer.begin();

  Serial.println("Accelerometer serial demo started.");
}

void loop() {
  if (millis() - lastPrintMs < PRINT_INTERVAL_MS) {
    return;
  }
  lastPrintMs = millis();

  // Read the 3-axis acceleration values.
  float x = Accelerometer.readX();
  float y = Accelerometer.readY();
  float z = Accelerometer.readZ();
  int angle = computeAngleFromX(x);

  // Print in a simple format that is easy to read in Serial Monitor.
  Serial.print("X axis: ");
  Serial.print(x);
  Serial.print("   Y axis: ");
  Serial.print(y);
  Serial.print("   Z axis: ");
  Serial.print(z);
  Serial.print("   Angle: ");
  Serial.println(angle);
}
