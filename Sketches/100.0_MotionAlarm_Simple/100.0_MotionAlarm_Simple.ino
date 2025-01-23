/**********************************************************************
  Product     : Freenove Robot for Raspberry Pi Pico (W)
  Description : Motion Detection Robot.
  Auther      : StrungSafe
  Modification: 2025/1/22
**********************************************************************/\
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "Freenove_Robot_For_Pico_W.h"
#include "Freenove_Robot_WS2812.h"
#include "Freenove_Robot_Emotion.h"
#include "AudioFileSourceLittleFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include "AudioFileSourceID3.h"


#define LeftLeg 10
#define RightLeg 12
#define LeftFoot 11
#define RightFoot 13

#define RoundingError 5

int backgroundPing = 0;

AudioGeneratorMP3 *mp3;
AudioFileSourceLittleFS *file;
AudioOutputI2SNoDAC *out;

void setupAudio() {
  file = new AudioFileSourceLittleFS("Fart1.mp3");
  out = new AudioOutputI2SNoDAC(6);
  out->SetGain(2);
  mp3 = new AudioGeneratorMP3();
}

void stopAudio() {
  mp3->stop();
  out->flush();
  out->stop();
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  delete file;
  delete mp3;
  delete out;
}

void setupLights() {
  WS2812_Setup();
  WS2812_Show(0);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  EEPROM.begin(512);
  delay(500);

  setupAudio();
  setupLights();
  Ultrasonic_Setup();
  Emotion_Setup();
  
  backgroundPing = Get_Sonar();
  delay(500);
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      stopAudio();
      setupAudio();
      WS2812_Show(5);
      Emotion_Show(2);
      delay(2000);
      Emotion_Show(0);
      WS2812_Show(0);
    }
  } else {
    int ping = Get_Sonar();
    if(ping != backgroundPing && (backgroundPing - ping) > RoundingError) {
      mp3->begin(file, out);
    }
  }
  delay(100);
}
