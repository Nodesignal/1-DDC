#include <Arduino.h> // Füge dies hinzu, um Serial zu verwenden
#include "sound.h"

void setupPWM() {
    // Konfiguriere den PWM-Timer und -Kanal
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void sound_pause() {
    // Implementiere die Logik, um den Sound zu pausieren
}

void sound_resume() {
    // Implementiere die Logik, um den Sound fortzusetzen
}

void soundOff() {
    // Implementiere die Logik, um den Sound auszuschalten
}

void sound_init(int pin) {
    // Initialisiere den PWM-Kanal
    setupPWM();
}
#include "sound.h"

void playSound(uint16_t freq, uint8_t volume) {
    // Überprüfe, ob die Frequenz innerhalb der zulässigen Grenzen liegt
    if (freq < MIN_FREQ || freq > MAX_FREQ) {
        Serial.println("Frequenz außerhalb der zulässigen Grenzen");
        return;
    }
    // Konfiguriere den PWM-Kanal mit der Frequenz und Lautstärke
    ledcWriteTone(PWM_CHANNEL, freq);
    ledcWrite(PWM_CHANNEL, volume);
}
