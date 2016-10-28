#include "TDA7439.h"

TDA7439 audio = TDA7439();

void setup() {
  delay(1000);
  audio.inputGain(0);
  audio.setInput(1);
  audio.setVolume(48);
  audio.spkAtt(0);
  
  // bass
  audio.setSnd(7, 1);
  // mid
  audio.setSnd(7, 2);
  // treble
  audio.setSnd(7, 3);

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
}

void loop() {


}
