#include <ResponsiveAnalogRead.h>
#include "TDA7439.h"

const int volumePin = A0;
int volume = 0;

TDA7439 audio = TDA7439();
ResponsiveAnalogRead volumeReader(volumePin, true);

void setup() {
  delay(1000);
  audio.inputGain(0);
  audio.setInput(1);
  //audio.setVolume(48);
  audio.spkAtt(0);
  
  // bass
  audio.setSnd(4, 1);
  // mid
  audio.setSnd(0, 2);
  // treble
  audio.setSnd(2, 3);

  pinMode(2, OUTPUT);
}

void loop() {
  volumeReader.update();
  
  int volumePinValue = volumeReader.getValue();
  int newVolume = map(volumePinValue, 0, 1023, 0, 48);
  if (newVolume != volume) {
    volume = newVolume;
    audio.setVolume(volume);
  
    digitalWrite(2, HIGH);
    delay(20);
    digitalWrite(2, LOW);
  }

  delay(1);
}
