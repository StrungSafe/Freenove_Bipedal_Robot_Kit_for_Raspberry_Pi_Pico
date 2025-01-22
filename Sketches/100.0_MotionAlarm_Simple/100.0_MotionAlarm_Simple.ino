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

// TODO: Get and upload the alarm(s) sound
// TODO: Could use the IR or BT controllers?

int initialSonar 0;

AudioGeneratorMP3 *mp3;
AudioFileSourceLittleFS *file;
AudioOutputI2SNoDAC *out;

void setup() {
  Serial.begin(115200);

  // audio setup
  file = new AudioFileSourceLittleFS("Nicetomeetyou.mp3");
  out = new AudioOutputI2SNoDAC(6);
  out->SetGain(2);  //Volume Setup
  mp3 = new AudioGeneratorMP3();
  // ultrasonic setup
  Ultrasonic_Setup();
  // servo setup
  Bipedal_Robot.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true); //Set the servo pins
  Bipedal_Robot.home();
  // motion detection setup
  initialSonar = Get_Sonar();
  // lights setup
  WS2812_Setup();

  delay(50);
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();
      delete file;
      delete mp3;
      mp3 = new AudioGeneratorMP3();
      WS2812_Show(0);
    }
  } else {
    int ping = Get_Sonar();
    if(ping != initialSonar && (initialSonar - ping) > 1) {
      mp3->begin(file, out);
      WS2812_Show(5);
    }
  }
  delay(100);
}
