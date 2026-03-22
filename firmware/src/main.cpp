#include <Arduino.h>
#include "websurface.h"
// === Pin Configuration ===
#define X_LEFT_LIMIT  32
#define X_RIGHT_LIMIT 33
#define Y_LEFT_LIMIT  4
#define Y_RIGHT_LIMIT 5

#define EMERGENCY_STOP_PIN 13
#define XIN1 14
#define XIN2 12
#define YIN1 27
#define YIN2 26
#define SLEEP_PIN 2
#define STATUS_LED 2

// === Motion Settings ===
const int pwmFreq = 10000;
const int pwmResolution = 8;
const int pwmSpeedXRight = 220;
const int pwmSpeedXLeft  = 200;
const int pwmSpeedYUp    = 220;
const int pwmSpeedYDown  = 220;
const int pwmSlowSpeedX = 210;
const int pwmSlowSpeedY = 210;

const int pwmX1 = 0, pwmX2 = 1, pwmY1 = 2, pwmY2 = 3;

String inputString = "";
bool commandReady = false;

long xPosition = 0, yPosition = 0;
long xMaxTravel = 0, yMaxTravel = 0;
bool isHomed = false;
String currentStatus = "Idle"; 

const unsigned long MAX_MOVE_TIME = 15000;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long MOTOR_SETTLE_TIME = 100;
const unsigned long HOMING_BACKOFF = 150;

// === Function Declarations ===
void stopAllMotors(), enableDriver(), disableDriver();
bool isLimitPressed(int pin);
void moveMotorSafe(int ch1, int ch2, bool forward, unsigned long duration, int limitPin = -1);
void moveXYTimed(bool xDir, bool yDir, unsigned long xTime, unsigned long yTime);
void drawSquare(), drawNikolausHouse();
void homeAllAxes(), moveToCenter(), printStatus();
void drawDiagonal(unsigned long xTime, unsigned long yTime, bool xDir, bool yDir);
void handleWebSurface();
void initWebSurface();
bool isEmergencyPressed() {
  return digitalRead(EMERGENCY_STOP_PIN) == LOW;
}


void setup() {
  Serial.begin(115200);
  delay(2000);

  ledcSetup(pwmX1, pwmFreq, pwmResolution);
  ledcSetup(pwmX2, pwmFreq, pwmResolution);
  ledcSetup(pwmY1, pwmFreq, pwmResolution);
  ledcSetup(pwmY2, pwmFreq, pwmResolution);

  ledcAttachPin(XIN1, pwmX1); ledcAttachPin(XIN2, pwmX2);
  ledcAttachPin(YIN1, pwmY1); ledcAttachPin(YIN2, pwmY2);

  pinMode(X_LEFT_LIMIT, INPUT_PULLUP); pinMode(X_RIGHT_LIMIT, INPUT_PULLUP);
  pinMode(Y_LEFT_LIMIT, INPUT_PULLUP); pinMode(Y_RIGHT_LIMIT, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT); pinMode(SLEEP_PIN, OUTPUT);
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);

  enableDriver(); stopAllMotors();
  initWebSurface();

  Serial.println("=== ESP32 CNC Controller v2.0 ===");
  Serial.println("Commands: home, square, nikolaus, center, status, stop, help\nReady...");
}

void loop() {
  handleWebSurface();  // Handle incoming web commands

  // ✅ Physical emergency switch logic
  if (isEmergencyPressed()) {
    stopAllMotors();         // Stop everything immediately
    disableDriver();         // Put driver to sleep
    currentStatus = "EMERGENCY";

    Serial.println("=== ⛔ EMERGENCY BUTTON PRESSED – RESTARTING SYSTEM ===");
    delay(500);              // Small delay to allow motors to settle
    ESP.restart();           // Hard restart
    return;                  // This will never be reached
  }

  // === Serial command listener ===
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputString.length() > 0) commandReady = true;
    } else inputString += c;
  }

  if (commandReady) {
    inputString.trim();
    inputString.toLowerCase();

    Serial.print("Command received: ");
    Serial.println(inputString);

    if (inputString == "reset") ESP.restart();
    else if (inputString == "home") {
      enableDriver();
      homeAllAxes();
      isHomed = true;
    } else if (inputString == "square" && isHomed) {
      drawSquare();
    } else if (inputString == "nikolaus" && isHomed) {
      drawNikolausHouse();
    } else if (inputString == "center" && isHomed) {
      moveToCenter();
    } else if (inputString == "status") {
      printStatus();
    } else if (inputString == "stop") {
      stopAllMotors();
      disableDriver();
    } else if (inputString == "help") {
      Serial.println("home, square, nikolaus, center, status, stop, help");
    } else {
      Serial.println("Unknown command.");
    }

    inputString = "";
    commandReady = false;
    Serial.println();
  }

  delay(10);
}

void stopAllMotors() {
  ledcWrite(pwmX1, 0); ledcWrite(pwmX2, 0);
  ledcWrite(pwmY1, 0); ledcWrite(pwmY2, 0);
}

void enableDriver() {
  digitalWrite(SLEEP_PIN, HIGH);
  delay(10);
}

void disableDriver() {
  stopAllMotors();
  digitalWrite(SLEEP_PIN, LOW);
}

bool isLimitPressed(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(DEBOUNCE_DELAY);
    return (digitalRead(pin) == LOW);
  }
  return false;
}

void moveMotorSafe(int ch1, int ch2, bool forward, unsigned long duration, int limitPin) {
  enableDriver(); stopAllMotors(); delay(100);

  int speed = (ch1 == pwmX1 || ch2 == pwmX2) 
              ? (forward ? pwmSpeedXRight : pwmSpeedXLeft)
              : (forward ? pwmSpeedYUp : pwmSpeedYDown);

  if (forward) {
    ledcWrite(ch1, speed);
    ledcWrite(ch2, 0);
  } else {
    ledcWrite(ch1, 0);
    ledcWrite(ch2, speed);
  }

  unsigned long startTime = millis();
  while ((millis() - startTime) < duration) {
    if (isEmergencyPressed()) {
      stopAllMotors();
      disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY STOP TRIGGERED ===");

      // ⚠️ Return immediately but allow recovery
      return;
    }

    if (limitPin != -1 && isLimitPressed(limitPin)) break;
    delay(10);
  }

  stopAllMotors();
  delay(MOTOR_SETTLE_TIME);
}

void moveXYTimed(bool xDir, bool yDir, unsigned long xTime, unsigned long yTime) {
  enableDriver(); stopAllMotors(); delay(100);

  if (xDir) {
    ledcWrite(pwmX1, pwmSpeedXRight); ledcWrite(pwmX2, 0);
  } else {
    ledcWrite(pwmX1, 0); ledcWrite(pwmX2, pwmSpeedXLeft);
  }

  if (yDir) {
    ledcWrite(pwmY1, pwmSpeedYUp); ledcWrite(pwmY2, 0);
  } else {
    ledcWrite(pwmY1, 0); ledcWrite(pwmY2, pwmSpeedYDown);
  }

  unsigned long duration = max(xTime, yTime);
  unsigned long startTime = millis();
  while ((millis() - startTime) < duration) {
    if (isEmergencyPressed()) {
      stopAllMotors();
      disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING DIAGONAL ===");
      return;
    }
    delay(10);
  }

  stopAllMotors();
  delay(MOTOR_SETTLE_TIME);
}


void drawSquare() {
  moveMotorSafe(pwmX1, pwmX2, true, 358, X_RIGHT_LIMIT); delay(1500);
  moveMotorSafe(pwmY1, pwmY2, true, 259, Y_RIGHT_LIMIT); delay(1500);
  moveMotorSafe(pwmX1, pwmX2, false, 473, X_LEFT_LIMIT); delay(1500);
  moveMotorSafe(pwmY1, pwmY2, false, 222, Y_LEFT_LIMIT); delay(1500);
  stopAllMotors();
}

void drawNikolausHouse() {
  drawSquare();
  delay(1500);
  if (isEmergencyPressed()) return;

  drawDiagonal(358, 200, true, true);
  delay(2000);
  if (isEmergencyPressed()) return;

  drawDiagonal(237, 130, false, true);
  delay(2000);
  if (isEmergencyPressed()) return;

  drawDiagonal(237, 111, false, false);
  delay(2000);
  if (isEmergencyPressed()) return;

  drawDiagonal(358, 170, true, false);
  Serial.println("=== Nikolaus House Complete ===");
}


void drawDiagonal(unsigned long xTime, unsigned long yTime, bool xDir, bool yDir) {
  Serial.println("== Drawing Diagonal ==");
  enableDriver();
  stopAllMotors();
  delay(100);

  if (xDir) {
    ledcWrite(pwmX1, pwmSpeedXRight);
    ledcWrite(pwmX2, 0);
  } else {
    ledcWrite(pwmX1, 0);
    ledcWrite(pwmX2, pwmSpeedXLeft);
  }

  if (yDir) {
    ledcWrite(pwmY1, pwmSpeedYUp);
    ledcWrite(pwmY2, 0);
  } else {
    ledcWrite(pwmY1, 0);
    ledcWrite(pwmY2, pwmSpeedYDown);
  }

  unsigned long duration = max(xTime, yTime);
  unsigned long startTime = millis();
  while ((millis() - startTime) < duration) {
    if (isEmergencyPressed()) {
      stopAllMotors();
      disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING DIAGONAL DRAWING ===");
      return;
    }
    delay(10);
  }

  stopAllMotors();
  delay(MOTOR_SETTLE_TIME);
}

void homeAllAxes() {
  Serial.println("=== Full Homing Start ===");

  Serial.println("Step 1: Move X to RIGHT limit...");
  while (!isLimitPressed(X_RIGHT_LIMIT)) {
    if (isEmergencyPressed()) {
      stopAllMotors(); disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING HOMING STEP 1 ===");
      return;
    }
    ledcWrite(pwmX1, pwmSlowSpeedX);
    ledcWrite(pwmX2, 0);
    delay(10);
  }
  stopAllMotors(); delay(200);

  Serial.println("Step 2: Move X to LEFT limit...");
  while (!isLimitPressed(X_LEFT_LIMIT)) {
    if (isEmergencyPressed()) {
      stopAllMotors(); disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING HOMING STEP 2 ===");
      return;
    }
    ledcWrite(pwmX1, 0);
    ledcWrite(pwmX2, pwmSlowSpeedX);
    delay(10);
  }
  stopAllMotors(); delay(200);

  Serial.println("Step 3: Move Y to BOTTOM limit...");
  while (!isLimitPressed(Y_LEFT_LIMIT)) {
    if (isEmergencyPressed()) {
      stopAllMotors(); disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING HOMING STEP 3 ===");
      return;
    }
    ledcWrite(pwmY1, 0);
    ledcWrite(pwmY2, pwmSlowSpeedY);
    delay(10);
  }
  stopAllMotors(); delay(200);

  Serial.println("Step 4: Move Y to TOP limit...");
  while (!isLimitPressed(Y_RIGHT_LIMIT)) {
    if (isEmergencyPressed()) {
      stopAllMotors(); disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING HOMING STEP 4 ===");
      return;
    }
    ledcWrite(pwmY1, pwmSlowSpeedY);
    ledcWrite(pwmY2, 0);
    delay(10);
  }
  stopAllMotors(); delay(200);

  Serial.println("Step 5: Move to bottom-left corner...");
  ledcWrite(pwmX1, 0); ledcWrite(pwmX2, pwmSpeedXLeft);
  ledcWrite(pwmY1, 0); ledcWrite(pwmY2, pwmSpeedYDown);
  unsigned long tStart = millis();
  while (millis() - tStart < 2000) {
    if (isEmergencyPressed()) {
      stopAllMotors(); disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING HOMING STEP 5 ===");
      return;
    }
    delay(10);
  }
  stopAllMotors(); delay(200);

  Serial.println("Step 6: Offset right from X-left by 1 second...");
  ledcWrite(pwmX1, pwmSpeedXRight); ledcWrite(pwmX2, 0);
  tStart = millis();
  while (millis() - tStart < 1000) {
    if (isEmergencyPressed()) {
      stopAllMotors(); disableDriver();
      currentStatus = "EMERGENCY";
      Serial.println("=== ⛔ EMERGENCY DURING HOMING STEP 6 ===");
      return;
    }
    delay(10);
  }
  stopAllMotors(); delay(MOTOR_SETTLE_TIME);

  isHomed = true; xPosition = 0; yPosition = 0;
  Serial.println("✅ Homing Complete and Offset Right from X-Left");
}

void moveToCenter() {
  if (xMaxTravel > 0 && yMaxTravel > 0) {
    moveMotorSafe(pwmX1, pwmX2, false, xMaxTravel / 4);
    moveMotorSafe(pwmY1, pwmY2, false, yMaxTravel / 4);
    xPosition = 0; yPosition = 0;
  } else Serial.println("ERROR: Axes not homed.");
}

void printStatus() {
  Serial.println("=== System Status ===");
  Serial.print("Homed: "); Serial.println(isHomed ? "YES" : "NO");
  Serial.print("X Max Travel: "); Serial.print(xMaxTravel); Serial.println(" ms");
  Serial.print("Y Max Travel: "); Serial.print(yMaxTravel); Serial.println(" ms");
  Serial.print("Position: X="); Serial.print(xPosition); Serial.print(", Y="); Serial.println(yPosition);
}
