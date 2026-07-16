/*
  project7_part1_pwm.ino

  Reads the rotary on A0 and sets a PWM output accordingly.
  Uses a PWM-capable pin (default D9). If you need PWM on pin 4,
  follow the README instruction to install the MD_PWM library and
  adapt the `setPWMDuty()` function below to call the MD_PWM API.

  This sketch prints the raw ADC value and the mapped PWM duty (0-255).
*/
/*************************************************** 
  This is a library for the Adafruit PT100/P1000 RTD Sensor w/MAX31865

  Designed specifically to work with the Adafruit RTD Sensor
  ----> https://www.adafruit.com/products/3328

  This sensor uses SPI to communicate, 4 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_MAX31865.h>

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(4, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(10);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0
const int analogPin = A0;
const int pwmPin = 10; // change to a PWM-capable pin. Use MD_PWM for pin 4.

const unsigned long sampleIntervalMs = 50;

unsigned long lastSample = 0;

void setup() {
  Serial.begin(115200);
  delay(50);
  pinMode(pwmPin, OUTPUT);
  // Ensure pin 4 is not driven by the MCU so it can be safely jumpered to D9.
  pinMode(4, INPUT);
  Serial.println("project7_part1_pwm: reading A0 and writing PWM on pin 9");
  Serial.println("Adafruit MAX31865 PT100 Sensor Test!");

  thermo.begin(MAX31865_2WIRE);  // set to 2WIRE or 4WIRE as necessary
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
    uint16_t rtd = thermo.readRTD();

  Serial.print("RTD value: "); Serial.println(rtd);
  float ratio = rtd;
  ratio /= 32768;
  Serial.print("Ratio = "); Serial.println(ratio,8);
  Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
  Serial.print("Temperature = "); Serial.println(thermo.temperature(RNOMINAL, RREF));

  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    thermo.clearFault();
  }
  Serial.println();
  delay(1000);
}
