#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <ResponsiveAnalogRead.h>
#include "TDA7439.h"

#define SND_TREBLE 3
#define SND_MID 2
#define SND_BASS 1

// *** Targeting information ***
// Core: https://github.com/SpenceKonde/ATTinyCore
// Board: ATTiny24/44/84
// Mapping: clockwise
// Chip: ATTiny84
// Clock: 1 Mhz (internal)

// Pin assignments
const int volumePin = A0;
const int treblePin = A1;
const int midPin = A2;
const int bassPin = A3;
const int ledPin = 8;

// Input controls
// If you don't have a potentiometer connected, set the "use" flag
// to false and set the level value to whatever default you want.
// If you do have a pot connected to an input, enable the reading of
// that pot by setting the corresponding "use" flag to true. In that
// case the default level value doesn't matter because it'll be immediately
// overridden by the pot reading.  

// Valid value ranges: volume 0 to 48, tones -7 to 7

bool useVolumeControl = true;
int volumeLevel = 48;

bool useTrebleControl = true;
int trebleLevel = 1;

bool useMidControl = true;
int midLevel = 0;

bool useBassControl = true;
int bassLevel = 2;

ResponsiveAnalogRead volumeReader(volumePin, true);
ResponsiveAnalogRead trebleReader(treblePin, true);
ResponsiveAnalogRead midReader(midPin, true);
ResponsiveAnalogRead bassReader(bassPin, true);

// Audio device
TDA7439 audio = TDA7439();

void setup()
{
  #define BODS 7 //BOD Sleep bit in MCUCR
  #define BODSE 2 //BOD Sleep enable bit in MCUCR
  MCUCR |= _BV(BODS) | _BV(BODSE); //turn off the brown-out detector

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);

  audio.inputGain(0);
  audio.setInput(2);
  audio.spkAtt(0); 
  
  if (!useVolumeControl)
  {
    audio.setVolume(volumeLevel);
  }

  if (!useTrebleControl)
  {
    audio.setSnd(trebleLevel, SND_TREBLE);
  }
  
  if (!useMidControl)
  {
    audio.setSnd(midLevel, SND_MID);
  }
  
  if (!useBassControl)
  {
    audio.setSnd(bassLevel, SND_BASS);
  }
}

void loop()
{  
  if (useVolumeControl)
  {
    UpdateVolume();
  }
  
  if (useTrebleControl)
  {
    UpdateTreble();
  }

  if (useMidControl)
  {
    UpdateMid();
  }

  if (useBassControl)
  {
    UpdateBass();
  }

  // sleep for 64 ms to save power
  system_sleep();
}

void UpdateVolume()
{
  volumeReader.update();
  int volumePinValue = volumeReader.getValue();
  int newVolume = potMap(volumePinValue, 0, 1023, -15, 48);
  if (newVolume != volumeLevel)
  {
    volumeLevel = newVolume;

    // The volume control by itself has a rather abrupt step between 1 and 0 (mute) when
    // the source is loud. We further extend the lower volume range to get a smooth
    // ramp to inaudibility by using the attenuation feature once we reach minimum
    // volume (volume values 0 and below invoke increasing attenuation)
    
    if (volumeLevel > 0)
    {
      audio.setVolume(volumeLevel);
      audio.spkAtt(0);
    }
    else
    {
      audio.setVolume(1);
      audio.spkAtt(abs(volumeLevel) + 1);
    }
    
    blinkLed();
  }
}

void UpdateTreble()
{
  trebleReader.update();
  int treblePinValue = trebleReader.getValue();
  int newTreble = potMap(treblePinValue, 0, 1023, -7, 7);
  if (newTreble != trebleLevel)
  {
    trebleLevel = newTreble;
    audio.setSnd(trebleLevel, SND_TREBLE);
    blinkLed();
  }  
}

void UpdateMid()
{
  midReader.update();
  int midPinValue = midReader.getValue();
  int newMid = potMap(midPinValue, 0, 1023, -7, 7);
  if (newMid != midLevel)
  {
    midLevel = newMid;
    audio.setSnd(midLevel, SND_MID);
    blinkLed();
  }  
}

void UpdateBass()
{
  bassReader.update();
  int bassPinValue = bassReader.getValue();
  int newBass = potMap(bassPinValue, 0, 1023, -7, 7);
  if (newBass != bassLevel)
  {
    bassLevel = newBass;
    audio.setSnd(bassLevel, SND_BASS);
    blinkLed();
  }  
}

void blinkLed()
{
  digitalWrite(ledPin, HIGH);
  delay(5);
  digitalWrite(ledPin, LOW);  
}

long potMap(long x, long in_min, long in_max, long out_min, long out_max)
{
  // The map function will map only the absolute in_max value to the out_max value,
  // because it truncates (https://www.arduino.cc/en/Reference/Map). When we're
  // trying to map the value of a potentiometer we usually don't see the absolute
  // max value come out of the ADC so we bump up both in_max and out_max here which
  // has the effect of mapping the upper end of the realistic pot range to the
  // maximum output value we want to get.

  return map(x, in_min, in_max + 1, out_min, out_max + 1);
}

// ATTiny84 sleep code, below, from https://www.marcelpost.com/wiki/index.php/ATtiny84_WDT_sleep

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect)
{
 // nothing here
}

void setup_watchdog(int ii) 
{
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  if (ii > 9 )
  {
    ii = 9;
  }

  uint8_t bb;
  bb = ii & 7;
  if (ii > 7)
  {
    bb |= (1 << 5);
  }
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

void system_sleep() 
{
  ADCSRA &= ~ bit(ADEN); // disable the ADC
  bitSet(PRR, PRADC); // power down the ADC

  setup_watchdog(2);
 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sei();                               // Enable the Interrupts so the wdt can wake us up

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 

  bitClear(PRR, PRADC); // power up the ADC
  ADCSRA |= bit(ADEN); // enable the ADC
}

