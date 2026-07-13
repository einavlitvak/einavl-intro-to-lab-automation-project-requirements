// Millis-based timer triggered by an external interrupt.
// LED turns on when button is pressed and turns off 5 seconds later
// without using delay().

const byte buttonPin = 6;
const byte ledPin = 4;
const byte interruptPin = 2; // external interrupt pin

volatile bool buttonPressed = false; // set in ISR, handled in loop
bool ledOn = false;
unsigned long ledOnMillis = 0;
const unsigned long ledDuration = 5000UL; // 5000 ms = 5 seconds

// ISR: only set a flag (keep ISR short). We check pin state here
// to only trigger on a press (HIGH). If using INPUT_PULLUP, invert.
void handleButtonChange() {
  if (digitalRead(interruptPin) == HIGH) {
    buttonPressed = true;
  }
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT);
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(interruptPin), handleButtonChange, CHANGE);

  digitalWrite(ledPin, LOW);
}

void loop() {
  // Handle button press set by ISR
  if (buttonPressed) {
    buttonPressed = false; // clear flag as soon as possible
    digitalWrite(ledPin, HIGH);
    ledOn = true;
    ledOnMillis = millis();
  }

  // Non-blocking check to turn LED off after ledDuration
  if (ledOn && (millis() - ledOnMillis >= ledDuration)) {
    digitalWrite(ledPin, LOW);
    ledOn = false;
  }

  // // Simulate long processing (from the exercise). This can delay
  // // response if it is very long or uses slow I/O like Serial prints.
  // for (int i = 0; i < 1000; i++) {
  //   Serial.println("calculating...");
  // }
}
