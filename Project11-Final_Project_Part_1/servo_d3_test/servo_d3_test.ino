// Project 11 servo test: verify the fan servo moves on D3.

#include <Servo.h>

const int SERVO_PIN = 3;  // servo signal on D3
const unsigned long STEP_DELAY_MS = 700;

Servo testServo;

void setup() {
  Serial.begin(9600);
  testServo.attach(SERVO_PIN, 1000, 2000);

  Serial.println("Servo D3 test started.");
  Serial.println("The servo should sweep 0 -> 90 -> 180 -> 90.");
}

void loop() {
  Serial.println("Move to 0");
  testServo.write(0);
  delay(STEP_DELAY_MS);

  Serial.println("Move to 90");
  testServo.write(90);
  delay(STEP_DELAY_MS);

  Serial.println("Move to 180");
  testServo.write(180);
  delay(STEP_DELAY_MS);

  Serial.println("Move back to 90");
  testServo.write(90);
  delay(STEP_DELAY_MS);
}
