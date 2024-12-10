// Created by Daniel from Saber C++ in 2024. MIT License
// Video Tutorial (to make the physical part): 

#include <FastLED.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>


// PINS
#define LED_PIN 2
#define BUTTON_PIN 13
#define DFPLAYER_RX 10
#define DFPLAYER_TX 11

// LED SETTINGS
#define NUM_LEDS 108
#define BRIGHTNESS 10 // 0-255
#define HEIGHT 12
#define WIDTH 9
// NOTE: This is because I was 1 LED short so the first LED is missing, so we have to start at LED 1 instead of 0
#define STARTING_LED 1

// Set your alarm in 24-hour time (YYYY, MM, DD, HH, MM, SS) [year, month, day do not matter]
DateTime alarmTime(0, 0, 0, 22, 42, 0);


struct point {
  byte x;
  byte y;
};

point digit1[15] = {
  { 7, 11 },
  { 6, 11 },
  { 5, 11 },
  { 7, 10 },
  { 6, 10 },
  { 5, 10 },
  { 7, 9 },
  { 6, 9 },
  { 5, 9 },
  { 7, 8 },
  { 6, 8 },
  { 5, 8 },
  { 7, 7 },
  { 6, 7 },
  { 5, 7 },
};
point digit2[15] = {
  { 3, 11 },
  { 2, 11 },
  { 1, 11 },
  { 3, 10 },
  { 2, 10 },
  { 1, 10 },
  { 3, 9 },
  { 2, 9 },
  { 1, 9 },
  { 3, 8 },
  { 2, 8 },
  { 1, 8 },
  { 3, 7 },
  { 2, 7 },
  { 1, 7 }
};
point digit3[15] = {
  { 7, 4 },
  { 6, 4 },
  { 5, 4 },
  { 7, 3 },
  { 6, 3 },
  { 5, 3 },
  { 7, 2 },
  { 6, 2 },
  { 5, 2 },
  { 7, 1 },
  { 6, 1 },
  { 5, 1 },
  { 7, 0 },
  { 6, 0 },
  { 5, 0 },
};
point digit4[15] = {
  { 3, 4 },
  { 2, 4 },
  { 1, 4 },
  { 3, 3 },
  { 2, 3 },
  { 1, 3 },
  { 3, 2 },
  { 2, 2 },
  { 1, 2 },
  { 3, 1 },
  { 2, 1 },
  { 1, 1 },
  { 3, 0 },
  { 2, 0 },
  { 1, 0 }
};

int numbers[10][15] = {
  // 0
  {
    1, 1, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
    1, 1, 1
  },

  // 1
  {
    0, 1, 0,
    1, 1, 0,
    0, 1, 0,
    0, 1, 0,
    1, 1, 1
  },

  // 2
  {
    1, 1, 1,
    0, 0, 1,
    1, 1, 1,
    1, 0, 0,
    1, 1, 1
  },

  // 3
  {
    1, 1, 1,
    0, 0, 1,
    1, 1, 1,
    0, 0, 1,
    1, 1, 1
  },

  // 4
  {
    1, 0, 1,
    1, 0, 1,
    1, 1, 1,
    0, 0, 1,
    0, 0, 1
  },

  // 5
  {
    1, 1, 1,
    1, 0, 0,
    1, 1, 1,
    0, 0, 1,
    1, 1, 1
  },

  // 6
  {
    1, 1, 1,
    1, 0, 0,
    1, 1, 1,
    1, 0, 1,
    1, 1, 1
  },

  // 7
  {
    1, 1, 1,
    0, 0, 1,
    0, 1, 0,
    1, 0, 0,
    1, 0, 0
  },

  // 8
  {
    1, 1, 1,
    1, 0, 1,
    1, 1, 1,
    1, 0, 1,
    1, 1, 1
  },

  // 9
  {
    1, 1, 1,
    1, 0, 1,
    1, 1, 1,
    0, 0, 1,
    1, 1, 1
  }
};

// Define RX, TX pins for communication with DFPlayer
SoftwareSerial softwareSerial(DFPLAYER_RX, DFPLAYER_TX);
DFRobotDFPlayerMini dfPlayer;

RTC_DS3231 rtc;

CRGB leds[NUM_LEDS];
CRGB colorOptions[7] = {CRGB(255,0,0), CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Purple, CRGB::Orange};
CRGB currentColor = colorOptions[0];

// When the alarm goes off, both of these will be true
// They will either disappear after the time of the alarm runs out, or alarm will stop sounding at the first button press and stop flashing at the second
bool alarmSounding;
bool alarmFlashing;

// Read button state
bool buttonIsDown;
bool buttonWasDown;


void setup() {
  Serial.begin(115200);

  softwareSerial.begin(9600);
  dfPlayer.begin(softwareSerial);
  dfPlayer.volume(15);

  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(BUTTON_PIN, OUTPUT);
  digitalWrite(BUTTON_PIN, HIGH);

  setWholePanelToColor(CRGB::Black);
  currentColor = CRGB(255,0,0);//colorOptions[EEPROM.read(0)];
  displayNumbers(559);
  delay(5000);
  FastLED.setBrightness(255);
  for (int i = 0; i < 10; i++) {
    setWholePanelToColor(CRGB(255,5,1));
    delay(1000);
    setWholePanelToColor(CRGB::Black);
    delay(1000);
  }
  setWholePanelToColor(CRGB::Black);
  displayNumbers(600);
}

void loop() {
  return;
  DateTime now = rtc.now();
  int currentHour = now.hour() == 0 || now.hour() == 12 ? 12 : now.hour() % 12;
  displayNumbers(currentHour * 100 + now.minute());

  if (now.hour() == alarmTime.hour() && now.minute() == alarmTime.minute() && now.second() == alarmTime.second()) {
    alarm();
  }
  else if (!digitalRead(BUTTON_PIN)) {
    alarm();
  }

  delay(100);
}

void alarm() {
  alarmSounding = true;
  alarmFlashing = true;

  // Play a sound effect
  dfPlayer.play();

  // Show some light effects
  for (int i = 0; i < 60; i++) {
    buttonIsDown = !digitalRead(BUTTON_PIN);
    // If button was just pressed
    if (buttonIsDown && !buttonWasDown) {
      // If the sound is still playing, stop it
      if (alarmSounding) {
        Serial.println("sound off");
        alarmSounding = false;
        dfPlayer.stop();
        delay(10);
        dfPlayer.stop();
      }
      // If the sound has already been deactivated (by the user), deactivate the lights as well
      else {
        Serial.println("light off");
        break;
      }
      buttonIsDown = true;
    }
    buttonWasDown = buttonIsDown;
    setWholePanelToColor(CRGB::Red);
    delay(500);
    setWholePanelToColor(CRGB::Black);
    delay(500);
  }

  alarmSounding = false;
  alarmFlashing = false;
  // Set brightness back to normal
  FastLED.setBrightness(BRIGHTNESS);
}

/// SIMPLE DISPLAY FUNCTIONS

void setWholePanelToColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void displayNumbers(int toDisplay) {
  int firstDigit = toDisplay / 1000;
  int secondDigit = (toDisplay % 1000) / 100;
  int thirdDigit = (toDisplay % 100) / 10;
  int fourthDigit = toDisplay % 10;

  for (int i = 0; i < 15; i++) {
    leds[pointToIndex(digit1[i]) - STARTING_LED] = numbers[firstDigit][i] == 1 ? currentColor : CRGB::Black;
    leds[pointToIndex(digit2[i]) - STARTING_LED] = numbers[secondDigit][i] == 1 ? currentColor : CRGB::Black;
    leds[pointToIndex(digit3[i]) - STARTING_LED] = numbers[thirdDigit][i] == 1 ? currentColor : CRGB::Black;
    leds[pointToIndex(digit4[i]) - STARTING_LED] = numbers[fourthDigit][i] == 1 ? currentColor : CRGB::Black;
  }

  FastLED.show();
}

/// UTILITIES

int pointToIndex(point p) {
  // If it's an even row, treat it like a regular 2D->1D array conversion
  // where each y moves us an entire row worth of units, but each x only moves us 1 unit
  if (p.y % 2 == 0) {
    return p.y * WIDTH + p.x;
  }
  // If it's an odd row, the max value of the row is on the right, so calculate that
  // and then subtract the x from it
  else {
    int rowMax = (p.y + 1) * WIDTH - 1;
    return rowMax - p.x;
  }
}
