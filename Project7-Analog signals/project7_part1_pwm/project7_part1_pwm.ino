/*
  project7_part1_pwm.ino

  Reads the rotary on A0 and sets a PWM output accordingly.
  Uses a PWM-capable pin (default D9). If you need PWM on pin 4,
  follow the README instruction to install the MD_PWM library and
  adapt the `setPWMDuty()` function below to call the MD_PWM API.

  This sketch prints the raw ADC value and the mapped PWM duty (0-255).
*/

const int analogPin = A0;
const int pwmPin = 9; // change to a PWM-capable pin. Use MD_PWM for pin 4.

const unsigned long sampleIntervalMs = 50;

unsigned long lastSample = 0;

void setup() {
  Serial.begin(9600);
  delay(50);
  pinMode(pwmPin, OUTPUT);
  // Ensure pin 4 is not driven by the MCU so it can be safely jumpered to D9.
  pinMode(4, INPUT);
  Serial.println("project7_part1_pwm: reading A0 and writing PWM on pin 9");
}

// Replace the body of this function with MD_PWM calls if you set up MD_PWM
void setPWMDuty(uint8_t duty) {
  // Standard Arduino PWM on supported pins
  analogWrite(pwmPin, duty);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSample < sampleIntervalMs) return;
  lastSample = now;

  int raw = analogRead(analogPin); // 0..1023 (typically)
  // Map to 0..255 for 8-bit PWM
  uint8_t duty = map(raw, 0, 1023, 0, 255);

  setPWMDuty(duty);

  Serial.print("ADC: "); Serial.print(raw);
  Serial.print("  PWM(0-255): "); Serial.println(duty);
}
