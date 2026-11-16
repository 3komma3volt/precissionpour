
#include "sounds.h"
#include <Arduino.h>

void playTone(int frequency, int duration) {
 /* float period = 1000000.0 / frequency; 
  int cycles = (duration * frequency) / 1000; 
  int samplesPerCycle = 36;
  float delayPerSample = period / samplesPerCycle;
  
  for (int i = 0; i < cycles; i++) {
    for (int deg = 0; deg < 360; deg += 10) { 
      int dacValue = 128 + 64 * sin(deg * PI / 180);
      dacWrite(SPEAKER, dacValue);
      delayMicroseconds(delayPerSample); 
    }
  }
  dacWrite(SPEAKER, 0);
  */
   ledcWriteTone(2, frequency);
  delay(duration);
  ledcWriteTone(2, 0);
}
void soundSetup() {
  pinMode(SPEAKER, OUTPUT);
 // ledcSetup(2, 0, 8);
  ledcAttachPin(SPEAKER, 2);
} 


void playOKSound() {
  playTone(659, 80);  
  playTone(784, 80); 
}

void playFailSound() {
  playTone(400, 130); 
  playTone(300, 130); 
}

void playTouchSound() {
  playTone(1200, 30);
}

void playWinningMelody() {

playOKSound();
delay(100);
playOKSound();
delay(100);
playOKSound();
}