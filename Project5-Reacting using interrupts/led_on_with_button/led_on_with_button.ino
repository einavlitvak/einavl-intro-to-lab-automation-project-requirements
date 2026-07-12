const byte buttonPin = 6;
const byte ledPin = 4;
const byte interruptPin = 2;

void handleButtonChange() {
  digitalWrite(ledPin, digitalRead(interruptPin));
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT);
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(interruptPin), handleButtonChange, CHANGE);

  handleButtonChange();
}

void loop() {
  for (int i = 0; i< 1000; i++){
    Serial.println("calculating...");
  }
}
