/*
 *  This creates sound tones by outputting a square wave on a DAC pin. The
 *  volume of the tone is the level of the DAC pin.
 *  
 *  The square wave is created by a timer. The timer runs at 2x the freq, because
 *  it needs to transition high and then low.
 * 
 *   
 */
#ifndef SOUND_H
  #define SOUND_H

#include "driver/ledc.h"

#define PWM_CHANNEL 0
#define PWM_FREQUENCY 5000 // Startfrequenz
#define PWM_RESOLUTION LEDC_TIMER_8_BIT
#define PWM_PIN 25 // GPIO-Pin für den Lautsprecher
#define MIN_FREQ 20
#define MAX_FREQ 16000

void setupPWM();
void playSound(uint16_t freq, uint8_t volume);
void stopSound();
void sound_pause();
void sound_resume();
void soundOff();
void sound_init(int pin);



#endif






