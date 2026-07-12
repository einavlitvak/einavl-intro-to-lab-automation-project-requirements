/*
  my_blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/
//exercise 1
int random_pin = 12;
unsigned int delay_time = 1;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin random_pin as an output.
  pinMode(random_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // change state of the LED by setting the pin to the HIGH voltage level
  digitalWrite(random_pin, HIGH);  // change state of the LED by setting the pin to the HIGH voltage level
  delay(delay_time);          // wait for a microsecond
  digitalWrite(random_pin, LOW);   // change state of the LED by setting the pin to the LOW voltage level
  digitalWrite(LED_BUILTIN, LOW);  // change state of the LED by setting the pin to the HIGH voltage level
  delay(delay_time);          // wait for a microsecond
}


//exercise 2
int random_pin = 12;
unsigned int delay_time = 1;
int i = 1

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin random_pin as an output.
  pinMode(random_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // change state of the LED by setting the pin to the HIGH voltage level
  i = i+1
  digitalWrite(random_pin, HIGH);  // change state of the LED by setting the pin to the HIGH voltage level
  delay(delay_time);          // wait for a microsecond
  digitalWrite(random_pin, LOW);   // change state of the LED by setting the pin to the LOW voltage level
  digitalWrite(LED_BUILTIN, LOW);  // change state of the LED by setting the pin to the HIGH voltage level
  delay(delay_time);          // wait for a microsecond
}


//exercise 3

unsigned int delay_time = 1;

// the setup function runs once when you press reset or power the board
void setup() {
  DDRB |= _BV(DDB4) | _BV(DDB5);
}

// the loop function runs over and over again forever
void loop() {
  PORTB = B00110000;  // pins 12 and 13 HIGH
  delay(delay_time);            // wait for a millisecond
  PORTB = B00000000;  // pins 12 and 13 LOW  // set pins 12 and 13 HIGH at the same time
  delay(delay_time);            // wait for a millisecond
}
