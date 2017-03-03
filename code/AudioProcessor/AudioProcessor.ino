#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <ResponsiveAnalogRead.h>
#include "TDA7439.h"

const int volumePin = A0;
const int treblePin = A1;
const int midPin = A2;
const int bassPin = A3;
const int ledPin = 2;
int volumeLevel = 0;
int trebleLevel = 0;
int midLevel = 0;
int bassLevel = 0;

TDA7439 audio = TDA7439();
ResponsiveAnalogRead volumeReader(volumePin, true);
ResponsiveAnalogRead trebleReader(treblePin, true);
ResponsiveAnalogRead midReader(midPin, true);
ResponsiveAnalogRead bassReader(bassPin, true);

void setup() {
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
}

void loop() {  
  volumeReader.update();
  int volumePinValue = volumeReader.getValue();
  int newVolume = potMap(volumePinValue, 0, 1023, 0, 48);
  if (newVolume != volumeLevel) {
    volumeLevel = newVolume;
    audio.setVolume(volumeLevel);
    blinkLed();
  }

  trebleReader.update();
  int treblePinValue = trebleReader.getValue();
  int newTreble = potMap(treblePinValue, 0, 1023, -7, 7);
  if (newTreble != trebleLevel) {
    trebleLevel = newTreble;
    audio.setSnd(trebleLevel, 3);
    blinkLed();
  }  

  midReader.update();
  int midPinValue = midReader.getValue();
  int newMid = potMap(midPinValue, 0, 1023, -7, 7);
  if (newMid != midLevel) {
    midLevel = newMid;
    audio.setSnd(midLevel, 2);
    blinkLed();
  }  

  bassReader.update();
  int bassPinValue = bassReader.getValue();
  int newBass = potMap(bassPinValue, 0, 1023, -7, 7);
  if (newBass != bassLevel) {
    bassLevel = newBass;
    audio.setSnd(bassLevel, 1);
    blinkLed();
  }  

  // sleep for 256 ms
  system_sleep();
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

// ATTiny84 sleep code from https://www.marcelpost.com/wiki/index.php/ATtiny84_WDT_sleep

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect)
{
 // nothing here
}

void setup_watchdog(int ii) 
{
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  uint8_t bb;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

// system wakes up when watchdog is timed out
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

