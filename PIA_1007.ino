#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "SevSeg.h"

#define ALARM_TEMPERATURE 30
#define ALARM_PRESSURE 76
#define ALARM_HUMIDITY 60

int BUZZER_PIN = 12;
bool oldAlarm = false;
bool newAlarm = false;
int displayData = 0;
int oldDisplayValue = 0;
int newDisplayValue = 0;
bool oldKeyState = false;
bool newKeyState = false;
int counter = 0;

Adafruit_BMP085 bmp;
SevSeg sevseg;  //Instantiate a seven segment controller object

void handleBuzzer() {
  if (newAlarm) {
    ledcWriteTone(0, 1000);  //channel 0,  1000Hz
    delay(1);
  } else {
    ledcWriteTone(0, 0);  //channel 0, 0Hz
    delay(1);
  }
  if (newAlarm != oldAlarm) {
    if (newAlarm) {
      Serial.println("Buzzer ON");
    } else {
      Serial.println("Buzzer OFF");
    }
    oldAlarm = newAlarm;
  }
}

int readTemperature() {
  float t1 = bmp.readTemperature();
  int t2 = (int)(t1 + 0.5);  //rounding
  if (t2 >= ALARM_TEMPERATURE) {
    newAlarm = true;
  } else {
    newAlarm = false;
  }
  return t2;
}

int readPressure() {
  float p1 = bmp.readPressure();  //in Pa
  p1 = p1 / 133.322387;           //in mmHg
  p1 = p1 / 10;                   //in cmHg
  int p2 = (int)(p1 + 0.5);       //rounding
  if (p2 >= ALARM_PRESSURE) {
    newAlarm = true;
  } else {
    newAlarm = false;
  }
  return p2;
}

int readHumidity() {
  int u = 100 - 100 * analogRead(13) / 4096;
  if (u >= ALARM_HUMIDITY) {
    newAlarm = true;
  } else {
    newAlarm = false;
  }
  return u;
}

void setup() {
  Serial.begin(115200);
  if (!bmp.begin()) {
    Serial.println("BMP180 Not Found. CHECK CIRCUIT!");
    while (1) {}
  }
  ledcSetup(0, 2000, 8);         // channel 0, freq 2000Hz,  8 bit resolution
  ledcAttachPin(BUZZER_PIN, 0);  // buzzer attached to pin 12, channel 0 PWM
  byte numDigits = 2;
  byte digitPins[] = { 15, 2, 0, 0 };
  byte segmentPins[] = { 4, 16, 17, 5, 18, 19, 25, 26 };
  bool resistorsOnSegments = false;      // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE;  // See README.md for options
  bool updateWithDelays = false;         // Default 'false' is Recommended
  bool leadingZeros = false;             // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = true;           // Use 'true' if your decimal point doesn't exist or isn't connected
  sevseg.begin(hardwareConfig, numDigits,
               digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(90);
  pinMode(23, INPUT);  // button
}

void loop() {
  if (counter == 0) {
    switch (displayData) {
      case 0:
        newDisplayValue = readTemperature();
        if (newDisplayValue != oldDisplayValue) {
          Serial.print("Temperature = ");
          Serial.print(newDisplayValue);
          Serial.println(" *C");
          oldDisplayValue = newDisplayValue;
        }
        break;
      case 1:
        newDisplayValue = readPressure();
        if (newDisplayValue != oldDisplayValue) {
          Serial.print("Pressure = ");
          Serial.print(newDisplayValue);
          Serial.println(" cmHg");
          oldDisplayValue = newDisplayValue;
        }
        break;
      case 2:
        newDisplayValue = readHumidity();
        if (newDisplayValue != oldDisplayValue) {
          Serial.print("Humidity = ");
          Serial.print(newDisplayValue);
          Serial.println("%");
          oldDisplayValue = newDisplayValue;
        }
        break;
    }
  }
  counter = (counter + 1) % 1000;
  sevseg.setNumber(newDisplayValue);
  handleBuzzer();
  newKeyState = digitalRead(23);
  if (newKeyState != oldKeyState) {
    if (newKeyState == 1) {
      displayData = (displayData + 1) % 3;
      Serial.print("Changing display to ");
      switch (displayData) {
        case 0:
          Serial.println("temperature");
          break;
        case 1:
          Serial.println("pressure");
          break;
        case 2:
          Serial.println("humidity");
          break;
      }
    }
    oldKeyState = newKeyState;
    delay(100);
  }
  sevseg.refreshDisplay();  // Must run repeatedly
}