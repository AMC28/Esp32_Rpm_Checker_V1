#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= PIN =================
#define SENSOR_PIN 18
#define BTN_MENU   25     // ganti menu
#define BTN_LAST   26     // last result
#define BTN_REC    27     // record start / stop

// ================= SENSOR =================
volatile unsigned long pulseCount = 0;
volatile unsigned long lastTrigger = 0;

// ================= LIVE DATA =================
unsigned long lastMillis = 0;

int rpm = 0;
int maxRPM = 0;

float hp = 0;
float torque = 0;

// ================= MENU =================
// 0 RPM | 1 HP | 2 TORQUE
int menuPage = 0;

bool showLast = false;

// ================= RECORD =================
bool recording = false;

unsigned long recStart = 0;
unsigned long recSec = 0;

long rpmTotal = 0;
int rpmSamples = 0;

int recMaxRPM = 0;
float recAvgRPM = 0;
float recMaxHP = 0;
float recMaxTorque = 0;

// ================= BUTTON =================
bool lastMenu = HIGH;
bool lastLast = HIGH;
bool lastRec  = HIGH;

// ==========================================
void IRAM_ATTR countPulse() {
  unsigned long now = micros();

  if (now - lastTrigger > 300) {
    pulseCount++;
    lastTrigger = now;
  }
}

// ==========================================
void setup() {
  pinMode(SENSOR_PIN, INPUT);

  pinMode(BTN_MENU, INPUT_PULLUP);
  pinMode(BTN_LAST, INPUT_PULLUP);
  pinMode(BTN_REC, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), countPulse, FALLING);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
}

// ==========================================
void loop() {

  // ===== BUTTON MENU =====
  if (digitalRead(BTN_MENU) == LOW && lastMenu == HIGH) {

    menuPage++;
    if (menuPage > 2) menuPage = 0;

    showLast = false;

    delay(180);
  }
  lastMenu = digitalRead(BTN_MENU);

  // ===== BUTTON LAST RESULT =====
  if (digitalRead(BTN_LAST) == LOW && lastLast == HIGH) {

    showLast = !showLast;

    delay(180);
  }
  lastLast = digitalRead(BTN_LAST);

  // ===== BUTTON RECORD =====
  if (digitalRead(BTN_REC) == LOW && lastRec == HIGH) {

    recording = !recording;

    if (recording) {

      recStart = millis();

      rpmTotal = 0;
      rpmSamples = 0;

      recMaxRPM = 0;
      recAvgRPM = 0;
      recMaxHP = 0;
      recMaxTorque = 0;
      recSec = 0;
    }
    else {

      recSec = (millis() - recStart) / 1000;

      if (rpmSamples > 0) {
        recAvgRPM = (float)rpmTotal / rpmSamples;
      }
    }

    delay(180);
  }
  lastRec = digitalRead(BTN_REC);

  // ===== HITUNG RPM =====
  if (millis() - lastMillis >= 1000) {

    noInterrupts();
    unsigned long count = pulseCount;
    pulseCount = 0;
    interrupts();

    rpm = count * 60;

    if (rpm > maxRPM) maxRPM = rpm;

    hp = rpm / 10000.0;
    torque = rpm / 5000.0;

    if (recording) {

      rpmTotal += rpm;
      rpmSamples++;

      if (rpm > recMaxRPM) recMaxRPM = rpm;
      if (hp > recMaxHP) recMaxHP = hp;
      if (torque > recMaxTorque) recMaxTorque = torque;
    }

    lastMillis = millis();
  }

  // ======================================
  // DISPLAY
  // ======================================
  display.clearDisplay();

  // ===== LAST RESULT =====
  if (showLast) {

    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("LAST RESULT");

    display.setCursor(0,12);
    display.print("MAX:");
    display.println(recMaxRPM);

    display.print("AVG:");
    display.println(recAvgRPM,0);

    display.print("HP:");
    display.println(recMaxHP,2);

    display.print("TQ:");
    display.println(recMaxTorque,2);

    display.print("SEC:");
    display.println(recSec);
  }

  // ===== RPM PAGE =====
  else if (menuPage == 0) {

    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("RPM PAGE");

    display.setTextSize(2);
    display.setCursor(0,24);
    display.print(rpm);
  }

  // ===== HP PAGE =====
  else if (menuPage == 1) {

    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("HORSE POWER");

    display.setTextSize(2);
    display.setCursor(0,24);
    display.print(hp,2);
    display.print("HP");
  }

  // ===== TORQUE PAGE =====
  else if (menuPage == 2) {

    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("TORQUE");

    display.setTextSize(2);
    display.setCursor(0,24);
    display.print(torque,2);
  }

  // ===== REC ICON =====
  if (recording) {
    display.setTextSize(1);
    display.setCursor(96,0);
    display.print("REC");
  }

  display.display();
}