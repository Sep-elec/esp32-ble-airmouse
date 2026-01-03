#include <BleMouse.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

BleMouse bleMouse("ESP32 BLE Mouse");
Adafruit_MPU6050 mpu;

// ================= PIN DEFINITIONS =================
#define LEFT_CLICK_PIN   18
#define RIGHT_CLICK_PIN  19
#define SCROLL_UP_PIN    4
#define SCROLL_DOWN_PIN  27

// ================= TUNING PARAMETERS =================
float baseSensitivity = 18.0;
float threshold       = 0.005;

#define DRAG_HOLD_TIME 180   // ms (150–220 is the sweet spot)
#define DOUBLE_CLICK_TIME 300   // ms
#define DRAG_THRESHOLD   2.5    // cursor movement to start drag

// ================= STATE VARIABLES =================
bool dragging = false;
bool lastLeftState = HIGH;
bool clickPending = false;
unsigned long clickTimer = 0;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LEFT_CLICK_PIN, INPUT_PULLUP);
  pinMode(RIGHT_CLICK_PIN, INPUT_PULLUP);
  pinMode(SCROLL_UP_PIN, INPUT_PULLUP);
  pinMode(SCROLL_DOWN_PIN, INPUT_PULLUP);
  pinMode(2, OUTPUT);

  Wire.begin();

  if (!mpu.begin()) {
    Serial.println("MPU6050 NOT FOUND");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  bleMouse.begin();
  Serial.println("BLE Mouse Ready");
}

// ================= LOOP =================
void loop() {

  if (bleMouse.isConnected()) {

    // -------- READ GYRO --------
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float rawX = g.gyro.z;
    float rawY = g.gyro.x;

    if (fabs(rawX) < threshold) rawX = 0;
    if (fabs(rawY) < threshold) rawY = 0;

    float speed = sqrt(rawX * rawX + rawY * rawY);
    float sensitivity = baseSensitivity * (1 + speed * 0.6);

    float moveX = rawX * sensitivity;
    float moveY = rawY * sensitivity;

    // -------- MOVE CURSOR --------
    if ((int)moveX || (int)moveY) {
      bleMouse.move(-(int)moveX, -(int)moveY);
    }

    bool isMoving = (fabs(moveX) > DRAG_THRESHOLD || fabs(moveY) > DRAG_THRESHOLD);
    bool leftPressed = (digitalRead(LEFT_CLICK_PIN) == LOW);

    // -------- LEFT BUTTON HOLD DETECTION --------
    static bool leftHeld = false;
    static unsigned long leftPressTime = 0;

    if (leftPressed && !leftHeld) {
      leftHeld = true;
      leftPressTime = millis();
    }

    if (!leftPressed) {
      leftHeld = false;
    }

    // -------- DRAG LOGIC (DELAYED) --------
    if (leftHeld && !dragging) {
      if ((millis() - leftPressTime > DRAG_HOLD_TIME) && isMoving) {
        bleMouse.press(MOUSE_LEFT);
        dragging = true;
      }
    }

    if (!leftPressed && dragging) {
      bleMouse.release(MOUSE_LEFT);
      dragging = false;
    }

    // -------- CLICK / DOUBLE CLICK --------
    if (!dragging) {

      if (lastLeftState == LOW && leftPressed == HIGH) {

        if (!clickPending) {
          clickPending = true;
          clickTimer = millis();
        } else {
          bleMouse.click(MOUSE_LEFT);
          delay(120);
          bleMouse.click(MOUSE_LEFT);
          clickPending = false;
        }
      }

      if (clickPending && (millis() - clickTimer > DOUBLE_CLICK_TIME)) {
        bleMouse.click(MOUSE_LEFT);
        clickPending = false;
      }
    }

    lastLeftState = leftPressed;

    // -------- RIGHT CLICK --------
    if (digitalRead(RIGHT_CLICK_PIN) == LOW) {
      bleMouse.click(MOUSE_RIGHT);
      delay(250);
    }

    // -------- SCROLL --------
    if (digitalRead(SCROLL_UP_PIN) == LOW) {
      bleMouse.move(0, 0, 1);
      delay(120);
    }

    if (digitalRead(SCROLL_DOWN_PIN) == LOW) {
      bleMouse.move(0, 0, -1);
      delay(120);
    }

  } 
  // -------- NOT CONNECTED --------
  else {
    static unsigned long lastBlink = 0;
    static bool ledState = false;

    if (millis() - lastBlink > 500) {
      ledState = !ledState;
      digitalWrite(2, ledState);
      lastBlink = millis();
    }
  }

  delay(8);
}



