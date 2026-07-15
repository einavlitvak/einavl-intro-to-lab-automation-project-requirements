// Project 11: Accelerometer -> Servo, Fan, OLED, and Fail-Safe Buzzer
// Displays angle and alarm state on the OLED.
// Freezes the servo and cuts the fan when safety limits are exceeded (10 to 170 deg).
// On recovery: Instantly kicks the fan to full target duty, then resumes servo tracking.

#include <Servo.h>
#include "Arduino_SensorKit.h"

// Update interval.
const unsigned long UPDATE_INTERVAL_MS = 500;

// Hardware PWM fan output on D7.
const int FAN_PIN = 7;

// Low-pass filter strength for smoother motion.
const float SERVO_SMOOTHING = 0.25f;
const float FAN_SMOOTHING = 0.25f;

// X-axis range used for mapping to servo angle.
const float X_MIN_G = -0.7f;
const float X_MAX_G = 0.7f;

// Y-axis range used for mapping fan speed.
const float Y_MIN_G = -0.7f;
const float Y_MAX_G = 0.7f;

// Deadband range for when the board is resting flat on the desk.
const float DEADBAND_MIN_X = 0.01f;
const float DEADBAND_MAX_X = 0.10f;

// Servo setup.
const int SERVO_PIN = 3;   // fan servo signal on D3
const int SERVO_MIN_DEG = 0;
const int SERVO_MAX_DEG = 180;
const int SERVO_MIN_US = 1000;
const int SERVO_MAX_US = 2000;

// Buzzer setup (Limits updated to 10 and 170 degrees).
const int BUZZER_PIN = 5;
const int ANGLE_LOW_LIMIT_DEG = 10;
const int ANGLE_HIGH_LIMIT_DEG = 170;
const int BUZZER_LOW_TONE_HZ = 800;
const int BUZZER_HIGH_TONE_HZ = 1200;

unsigned long lastUpdateMs = 0;
Servo fanServo;
float filteredServoAngle = 90.0f;
float filteredFanDuty = 0.0f;
float observedMinX = 999.0f;
float observedMaxX = -999.0f;
float observedMinY = 999.0f;
float observedMaxY = -999.0f;

// Tracks if the system was in alarm state on the previous loop
bool wasAlarmed = false; 

int computeServoAngleFromX(float xG) {
  // If sensor sits in our desk-resting range, force it to absolute center (0.0G -> 90 degrees)
  if (xG >= DEADBAND_MIN_X && xG <= DEADBAND_MAX_X) {
    xG = 0.0f;
  }

  float clampedX = xG;
  if (clampedX < X_MIN_G) {
    clampedX = X_MIN_G;
  }
  if (clampedX > X_MAX_G) {
    clampedX = X_MAX_G;
  }

  float normalized = (clampedX - X_MIN_G) / (X_MAX_G - X_MIN_G);
  float angle = SERVO_MIN_DEG + normalized * (SERVO_MAX_DEG - SERVO_MIN_DEG);
  return (int)(angle + 0.5f);
}

int computeFanDutyFromY(float yG) {
  float clampedY = yG;
  if (clampedY < Y_MIN_G) {
    clampedY = Y_MIN_G;
  }
  if (clampedY > Y_MAX_G) {
    clampedY = Y_MAX_G;
  }

  float normalized = (clampedY - Y_MIN_G) / (Y_MAX_G - Y_MIN_G);
  float duty = normalized * 255.0f;
  return (int)(duty + 0.5f);
}

void setup() {
  Serial.begin(9600);
  Accelerometer.begin();

  // Initialize the OLED display
  Oled.begin();
  Oled.setFlipMode(true);
  Oled.setFont(u8x8_font_chroma48medium8_r);
  
  // Draw a permanent header
  Oled.setCursor(0, 0);
  Oled.print("== MONITORING ==");

  fanServo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  fanServo.write(90);

  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, 0);

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  Serial.println("Accelerometer, Servo, and OLED Monitor Initialized.");
}

void loop() {
  if (millis() - lastUpdateMs < UPDATE_INTERVAL_MS) {
    return;
  }
  lastUpdateMs = millis();

  float x = Accelerometer.readX();
  float y = Accelerometer.readY();
  float z = Accelerometer.readZ();
  
  int targetAngle = computeServoAngleFromX(x);
  int targetFanDuty = computeFanDutyFromY(y);
  int buzzerToneHz = 0;

  // Track min/max readings for diagnostic logging
  if (x < observedMinX) observedMinX = x;
  if (x > observedMaxX) observedMaxX = x;
  if (y < observedMinY) observedMinY = y;
  if (y > observedMaxY) observedMaxY = y;

  // Calculate what the smoothed servo angle *would* be if we let it update
  float tentativeServoAngle = 
    (SERVO_SMOOTHING * targetAngle) + ((1.0f - SERVO_SMOOTHING) * filteredServoAngle);
  int tentativeAngle = (int)(tentativeServoAngle + 0.5f);

  // Evaluate alarm states based on the physical, smoothed angle tentative value
  if (tentativeAngle <= ANGLE_LOW_LIMIT_DEG) {
    buzzerToneHz = BUZZER_LOW_TONE_HZ;
  } else if (tentativeAngle >= ANGLE_HIGH_LIMIT_DEG) {
    buzzerToneHz = BUZZER_HIGH_TONE_HZ;
  }

  int angle;
  int fanDuty;

  if (buzzerToneHz > 0) {
    // Fail-safe State (Alarm Triggered):
    tone(BUZZER_PIN, buzzerToneHz);
    wasAlarmed = true; 
    
    // 1. Force the fan to a complete stop immediately
    fanDuty = 0;
    filteredFanDuty = 0.0f; 

    // 2. FREEZE: Keep the last safe filtered angle position
    angle = (int)(filteredServoAngle + 0.5f);
  } else {
    // Normal State (Safe):
    noTone(BUZZER_PIN);

    if (wasAlarmed) {
      // --- STAGE 1 RECOVERY: Wake up the fan instantly ---
      filteredFanDuty = (float)targetFanDuty;
      fanDuty = targetFanDuty;

      // Keep the servo completely frozen at its last position for this loop cycle
      angle = (int)(filteredServoAngle + 0.5f);

      // Reset the flag so that the NEXT loop iteration resumes servo tracking
      wasAlarmed = false;
    } else {
      // --- STAGE 2 / NORMAL STATE: Update both normally using low-pass filters ---
      filteredServoAngle = tentativeServoAngle; // Accept the tentative smoothed angle
      angle = tentativeAngle;

      filteredFanDuty =
        (FAN_SMOOTHING * targetFanDuty) + ((1.0f - FAN_SMOOTHING) * filteredFanDuty);
      fanDuty = (int)(filteredFanDuty + 0.5f);
    }
  }

  // Write commands to hardware
  fanServo.write(angle);
  analogWrite(FAN_PIN, fanDuty);

  // --- Update OLED Display ---
  Oled.setCursor(0, 2);
  Oled.print("Angle: ");
  Oled.print(angle);
  Oled.print(" deg   "); 

  Oled.setCursor(0, 4);
  Oled.print("Alarm: ");
  if (buzzerToneHz > 0) {
    Oled.print("ACTIVE   ");
  } else {
    Oled.print("CLEAR    ");
  }

// --- Serial CSV Telemetry (Time, Angle, Fan Duty, Alarm State) ---
  Serial.print(millis());
  Serial.print(",");
  Serial.print(angle);
  Serial.print(",");
  Serial.print(fanDuty);
  Serial.print(",");
  Serial.println(buzzerToneHz > 0 ? 1 : 0); // 1 = Alarm Active, 0 = Alarm Clear
}