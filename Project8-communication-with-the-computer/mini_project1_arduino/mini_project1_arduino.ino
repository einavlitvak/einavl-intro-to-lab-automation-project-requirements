#include <MsTimer2.h>
#include <ctype.h>
#include <stdlib.h>

const uint8_t LED_PIN = 4;
const uint8_t BUTTON_PIN = 2;
const unsigned long DEFAULT_DURATION_MS = 1000;
const unsigned long MIN_DURATION_MS = 1;
const unsigned long MAX_DURATION_MS = 600000;
const unsigned long DEBOUNCE_MS = 30;

volatile bool press_event = false;
volatile bool release_event = false;
volatile bool timer_expired = false;
volatile unsigned long last_interrupt_ms = 0;

unsigned long led_on_duration_ms = DEFAULT_DURATION_MS;
bool led_is_on = false;
bool waiting_for_release_status = false;

void button_isr() {
  const unsigned long now = millis();
  if (now - last_interrupt_ms < DEBOUNCE_MS) {
    return;
  }
  last_interrupt_ms = now;

  const bool is_pressed = (digitalRead(BUTTON_PIN) == LOW);
  if (is_pressed) {
    press_event = true;
  } else {
    release_event = true;
  }
}

void turn_off() {
  MsTimer2::stop();
  timer_expired = true;
}

bool parse_duration_ms(const String &line, unsigned long &out_value) {
  if (line.length() == 0) {
    return false;
  }

  for (unsigned int i = 0; i < line.length(); ++i) {
    if (!isdigit(line[i])) {
      return false;
    }
  }

  char buffer[24];
  line.toCharArray(buffer, sizeof(buffer));
  char *end_ptr = nullptr;
  const unsigned long parsed = strtoul(buffer, &end_ptr, 10);
  if (*end_ptr != '\0') {
    return false;
  }

  if (parsed < MIN_DURATION_MS || parsed > MAX_DURATION_MS) {
    return false;
  }

  out_value = parsed;
  return true;
}

void handle_serial_input() {
  if (Serial.available() <= 0) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();

  unsigned long parsed_ms = 0;
  if (!parse_duration_ms(line, parsed_ms)) {
    Serial.println("ERR:invalid_duration");
    return;
  }

  led_on_duration_ms = parsed_ms;
  Serial.print("RECEIVED:");
  Serial.println(led_on_duration_ms);
}

void handle_press_event() {
  if (!press_event) {
    return;
  }

  noInterrupts();
  press_event = false;
  interrupts();

  if (led_is_on) {
    return;
  }

  led_is_on = true;
  waiting_for_release_status = true;
  digitalWrite(LED_PIN, HIGH);
  Serial.println("1");

  MsTimer2::stop();
  MsTimer2::set(led_on_duration_ms + 1, turn_off);
  MsTimer2::start();
}

void handle_release_event() {
  if (!release_event) {
    return;
  }

  noInterrupts();
  release_event = false;
  interrupts();

  if (!waiting_for_release_status) {
    return;
  }

  waiting_for_release_status = false;
  Serial.println("2");
}

void handle_timer_expiration() {
  if (!timer_expired) {
    return;
  }

  noInterrupts();
  timer_expired = false;
  interrupts();

  digitalWrite(LED_PIN, LOW);
  led_is_on = false;
  Serial.println("0");
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_isr, CHANGE);

  Serial.begin(9600);
  while (!Serial) {
  }

  Serial.println("READY");
}

void loop() {
  handle_serial_input();
  handle_press_event();
  handle_release_event();
  handle_timer_expiration();
}
