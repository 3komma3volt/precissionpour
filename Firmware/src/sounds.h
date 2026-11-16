#ifndef SOUNDS_H
#define SOUNDS_H


const int SPEAKER = 26;

void playTone(int frequency, int duration);
void playOKSound();
void playFailSound();
void playWinningMelody();
void playTouchSound();
void soundSetup();
#endif // SOUNDS_H