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

void setupPWM() {
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void playSound(uint16_t freq, uint8_t volume) {
    if (volume == 0) {
        ledcWrite(PWM_CHANNEL, 0); // PWM ausschalten
        return;
    }
    ledcWriteTone(PWM_CHANNEL, freq); // Frequenz einstellen
    ledcWrite(PWM_CHANNEL, volume); // Lautstärke einstellen
}

void stopSound() {
    ledcWrite(PWM_CHANNEL, 0); // PWM ausschalten
}



#endif






