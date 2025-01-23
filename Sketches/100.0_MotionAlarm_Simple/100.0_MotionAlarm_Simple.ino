/**********************************************************************
  Product     : Freenove Robot for Raspberry Pi Pico (W)
  Description : Motion Detection Robot.
  Auther      : StrungSafe
  Modification: 2023/11/07
**********************************************************************/
/*Check that the servos are properly connected to the corresponding 
interface before uploading the code. */
/************************************
           ---     ---
         --------------- 
        |     O   O     |
        |---------------|
YR 12==>|               | <== YL 10
         --------------- 
            ||     ||
            ||     ||
RR 13==>  -----   ------  <== RL 11
         |-----   ------|
************************************/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "Freenove_Robot_WS2812.h"
#include "Freenove_Robot_For_Pico_W.h"

#include "Bipedal_Robot.h"
Bipedal_Robot Bipedal_Robot;

#include "AudioFileSourceLittleFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include "AudioFileSourceID3.h"


#define LeftLeg 10
#define RightLeg 12
#define LeftFoot 11
#define RightFoot 13

#define RoundingError 5

int initialSonar = 0;

AudioGeneratorMP3 *mp3;
AudioFileSourceLittleFS *file;
AudioOutputI2SNoDAC *out;

void setupAudio() {
  file = new AudioFileSourceLittleFS("Fart1.mp3");
  out = new AudioOutputI2SNoDAC(6);
  out->SetGain(0.1);
  mp3 = new AudioGeneratorMP3();
}

void setupServo() {
  Bipedal_Robot.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true);
  Bipedal_Robot.saveTrimsOnEEPROM();
  Bipedal_Robot.home();
}

void setupLights() {
  WS2812_Setup();
  WS2812_Show(0);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  EEPROM.begin(512);
  setupAudio();
  // setupServo();
  setupLights();
  Ultrasonic_Setup();
  
  // motion detection setup
  initialSonar = Get_Sonar();
  Serial.print("initial sonar: ");
  Serial.println(initialSonar);  

  delay(1000);
}

void loop()
{
  if (mp3->isRunning()) {
    Serial.println("mp3 is running");
    if (!mp3->loop()) {
      Serial.println("mp3 is finished");
      mp3->stop();
      out->flush();
      out->stop();
      pinMode(6, OUTPUT);
      digitalWrite(6, LOW);
      delete file;
      delete mp3;
      delete out;
      setupAudio();
      WS2812_Show(0);
    }
  } else {
    Serial.println("else block");
    int ping = Get_Sonar();
    Serial.print("ping: ");
    Serial.println(ping);
    if(ping != initialSonar && (initialSonar - ping) > RoundingError) {
      Serial.println("starting mp3");
      mp3->begin(file, out);
      WS2812_Show(5);
    }
  }
  delay(100);
}
