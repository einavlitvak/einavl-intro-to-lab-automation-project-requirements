// Use MsTimer2 (interrupt-driven) to turn the LED off 5 seconds after button press.
#include <MsTimer2.h>

const byte buttonPin = 6;
const byte ledPin = 4;
const byte interruptPin = 2; // external interrupt pin

// LED on-time in milliseconds. Set to 30 for the "30 ms" measurement task.
const unsigned int LED_DURATION_MS = 30;

// Timer callback: turns the LED off. Called from timer ISR context.
void turnOffLED() {
  digitalWrite(ledPin, LOW);
  MsTimer2::stop();
}

// External interrupt: called on pin state change. When button is pressed
// (we detect HIGH here) turn the LED on and start a 5s one-shot timer.
void handleButtonChange() {
  if (digitalRead(interruptPin) == HIGH) {
    digitalWrite(ledPin, HIGH);
    MsTimer2::stop();
    MsTimer2::set(LED_DURATION_MS, turnOffLED);
    MsTimer2::start();
  }
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT);
  Serial.begin(9600);

  // Attach external interrupt on the interrupt pin. CHANGE will call
  // handleButtonChange() on both edges; the handler only responds to presses.
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleButtonChange, CHANGE);

  // Ensure LED initial state
  digitalWrite(ledPin, LOW);
}

void loop() {
  // Simulate long processing (keep this here to show timer works without polling).
  for (int i = 0; i < 1000; i++) {
    Serial.println("calculating...");
  }
}
