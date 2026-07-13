/*
  project7_part1.ino

  Reads a rotary potentiometer connected to A0 and prints the raw ADC
  values to Serial. Also tracks and periodically prints the observed
  minimum and maximum values so you can determine the range.

  Wiring (as requested):
  - rotary middle (wiper) -> A0 (orange)
  - rotary one end -> 5V
  - rotary other end -> GND (connect GND on Arduino first to the orange with stripe on ADALM)

  Usage:
  - Open Serial Monitor at 9600 baud
  - Turn the rotary and observe values and the reported min/max

  Expected ADC range on standard Arduino UNO (10-bit ADC): 0..1023
  The actual observed range depends on wiring and reference voltage.
*/

const int analogPin = A0;
const unsigned long sampleIntervalMs = 100; // sample every 100 ms
const unsigned long reportIntervalMs = 2000; // report min/max every 2s

int lastValue = 0;
int minValue = 1023;
int maxValue = 0;

unsigned long lastSampleMillis = 0;
unsigned long lastReportMillis = 0;

void setup() {
  Serial.begin(9600);
  // Give Serial a moment to start
  delay(50);
  Serial.println("project7_part1: reading A0. Turn the rotary and watch values.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleMillis >= sampleIntervalMs) {
    lastSampleMillis = now;
    int v = analogRead(analogPin);
    lastValue = v;
    if (v < minValue) minValue = v;
    if (v > maxValue) maxValue = v;
    Serial.print("Value: ");
    Serial.println(v);
  }

  if (now - lastReportMillis >= reportIntervalMs) {
    lastReportMillis = now;
    Serial.print("Observed range: ");
    Serial.print(minValue);
    Serial.print(" .. ");
    Serial.println(maxValue);
  }
}
