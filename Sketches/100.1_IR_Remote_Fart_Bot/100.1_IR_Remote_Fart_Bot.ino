/**********************************************************************
  Product     : Freenove Robot for Raspberry Pi Pico (W)
  Description : Motion Detection Robot.
  Auther      : StrungSafe
  Modification: 2025/1/25
**********************************************************************/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <IRremote.hpp>
#include "Freenove_Robot_For_Pico_W.h"
#include "Freenove_Robot_WS2812.h"
#include "Freenove_Robot_Emotion.h"
#include "AudioFileSourceLittleFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include "AudioFileSourceID3.h"

#define IR_Pin 3  // Infrared receiving pin
#define ENABLE_LED_FEEDBACK true
#define DISABLE_LED_FEEDBACK false

#define LeftLeg 10
#define RightLeg 12
#define LeftFoot 11
#define RightFoot 13

AudioGeneratorMP3 *mp3;
AudioFileSourceLittleFS *file;
AudioOutputI2SNoDAC *out;

const int RoundingError = 5;

int backgroundPing = 0;
float audioVolume = 2.0f;
float voltage = 0.0f;
bool disabled = false;

void setVolume(float volume) {
  audioVolume = volume;
  out->SetGain(volume);
}

void setupAudio() {
  file = new AudioFileSourceLittleFS("Fart1.mp3");
  out = new AudioOutputI2SNoDAC(6);
  setVolume(audioVolume);
  mp3 = new AudioGeneratorMP3();
}

void stopAudio() {
  mp3->stop();
  out->flush();
  out->stop();
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  delete out;
  delete mp3;
  delete file;
}

void setupLights() {
  WS2812_Setup();
  WS2812_Show(0);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  EEPROM.begin(512);
  delay(500);

  setupAudio();
  setupLights();
  Ultrasonic_Setup();
  Emotion_Setup();
  IrReceiver.begin(IR_Pin, DISABLE_LED_FEEDBACK);
  delay(500);

  backgroundPing = Get_Sonar();
  delay(500);
}

void showWS2812(int mode, int delayMs) {
  WS2812_Show(mode);
  delay(delayMs);
  WS2812_Show(0);
}

// Power - BA45FF00
// Menu - B847FF00
// Test - BB44FF00
// + - BF40FF00
// <- - BC43FF00
// <<- (reverse) - F807FF00
// play - EA15FF00
// ->> (forward) - F609FF00
// 0 - E916FF00
// - - E619FF00
// C - F20DFF00
// 1 - F30CFF00
// 2 - E718FF00
// 3 - A15EFF00
// 4 - F708FF00
// 5 - E31CFF00
// 6 - A55AFF00
// 7 - BD42FF00
// 8 - AD52FF00
// 9 - B54AFF00
void handleIrRequest(unsigned long value) {
  switch (value) {
    case 0xBA45FF00: // POWER
      disabled = !disabled;
      if (disabled) {
        Serial.println("IR disabled bot");
      } else {
        Serial.println("IR enabled bot");
        backgroundPing = Get_Sonar();
      }
      break;
    case 0xBF40FF00:  // PLUS
      setVolume(min(audioVolume + 0.5f, 4.0f));
      break;
    case 0xE619FF00:  // MINUS
      setVolume(max(audioVolume - 0.5f, 0.0f));
      break;
    case 0xEA15FF00:  // PLAY
      if (!mp3->isRunning()) {
        mp3->begin(file, out);
      }
      break;
    case 0xBB44FF00:  // TEST
      voltage = Get_Battery_Voltage();
      Serial.print("voltage: ");
      Serial.print(voltage);
      Serial.println("V");
      if (voltage >= 8.5f) {
        showWS2812(8, 2000);
      } else if (voltage >= 8.0f) {
        showWS2812(7, 2000);
      } else if (voltage >= 7.5f) {
        showWS2812(6, 2000);
      } else {
        for (int i = 0; i < 20; ++i) {
          showWS2812(6, 100);
        }
      }
      break;
    default:
      break;
  }
}

void loop() {
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      stopAudio();
      setupAudio();
      WS2812_Show(6);
      Emotion_Show(2);
      delay(2000);
      Emotion_Show(0);
      WS2812_Show(0);
    }
  } else {
    if (IrReceiver.decode()) {
      handleIrRequest(IrReceiver.decodedIRData.decodedRawData);
      IrReceiver.resume();
    }

    if (disabled) {
      delay(500);
      return;
    }

    int ping = Get_Sonar();
    if (ping != backgroundPing && (backgroundPing - ping) > RoundingError) {
      mp3->begin(file, out);
    }
  }
  delay(50);
}
