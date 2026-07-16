// DEBUG code for the P controller
int set_point = 35;
int Kp = 1.0;

int ReadTemperature() {
  return 15;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  int current_temperature = ReadTemperature();
  int error = set_point - current_temperature;
  int power = Kp * error;
  Serial.print("Setpoint: "); Serial.println(set_point);
  Serial.print("Current Temperature: "); Serial.println(current_temperature);
  Serial.print("Power: "); Serial.println(power);
  delay(1000);
}
