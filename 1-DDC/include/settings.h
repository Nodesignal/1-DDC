#ifndef SETTINGS_H
	#define SETTINGS_H

#include <EEPROM.h>
#include "sound.h"

// Version 2 adds the number of LEDs

// change this whenever the saved settings are not compatible with a change
// It forces a reset from defaults.
#define SETTINGS_VERSION 2 
#define EEPROM_SIZE				   256

// LEDS
#define NUM_LEDS        287
#define MIN_LEDS				60



#define DEFAULT_BRIGHTNESS	50
#define MIN_BRIGHTNESS			10
#define MAX_BRIGHTNESS 			255

// PLAYER
const uint8_t MAX_PLAYER_SPEED = 10;     // Max move speed of the player
const uint8_t LIVES_PER_LEVEL = 3;      // default lives per level
#define MIN_LIVES_PER_LEVEL 3
#define MAX_LIVES_PER_LEVEL 9

// JOYSTICK
#define JOYSTICK_ORIENTATION 1     // 0, 1 or 2 to set the axis of the joystick
#define JOYSTICK_DIRECTION   1     // 0/1 to flip joystick direction
#define DEFAULT_ATTACK_THRESHOLD     30000 // The threshold that triggers an attack
#define MIN_ATTACK_THRESHOLD     20000
#define MAX_ATTACK_THRESHOLD     30000


#define DEFAULT_JOYSTICK_DEADZONE    8     // Angle to ignore
#define MIN_JOYSTICK_DEADZONE 3
#define MAX_JOYSTICK_DEADZONE 12

// AUDIO
#define DEFAULT_VOLUME          20     // 0 to 255
#define MIN_VOLUME							0
#define MAX_VOLUME							255

// DIFFICULTY
#define DEFAULT_DIFFICULTY 1 // 1 = Easy, 2 = Medium, 3 = Hard
#define MIN_DIFFICULTY 1
#define MAX_DIFFICULTY 3

#define DAC_AUDIO_PIN 		25     // should be 25 or 26 only

enum ErrorNums{
	ERR_SETTING_NUM,
	ERR_SETTING_RANGE
};

long lastInputTime = 0;


//TODO ... move all the settings to this file.

// Function prototypes
//void reset_settings();
void settings_init();
void show_game_stats();
void settings_eeprom_write();
void settings_eeprom_read();
void change_setting_serial(char *line);
void processSerial(char inChar);
void printError(int reason);
void show_settings_menu();
void reset_settings();
void change_setting(char paramCode, uint16_t newValue);

SemaphoreHandle_t xMutex;


typedef struct {
	uint8_t settings_version; // stores the settings format version 	
	
	uint16_t led_count;
	uint8_t led_brightness; 	
	
	uint8_t joystick_deadzone;
	uint16_t attack_threshold;
	
	uint8_t audio_volume;
	
	uint8_t lives_per_level;	
	
	uint8_t difficulty; // Game difficulty level
	
	// saved statistics
	uint16_t games_played;
	uint32_t total_points;
	uint16_t high_score = 0;
	uint16_t boss_kills = 0;
	struct {
		uint16_t score;
		char name[10];
	} leaderboard[5] = {{0, ""}}; // Top 5 high scores with player names
	
}settings_t;

settings_t user_settings;

#define READ_BUFFER_LEN 10
#define CARRIAGE_RETURN 13
char readBuffer[READ_BUFFER_LEN];
uint8_t readIndex = 0;

void settings_init() {	
	reset_settings(); // Ensure settings are initialized to default values
	settings_eeprom_read();
	show_settings_menu();	
	show_game_stats();
}

void checkSerialInput() {
	if (Serial.available()) {
		processSerial(Serial.read());
	}
}

void processSerial(char inChar)
{
	readBuffer[readIndex] = inChar;
		switch(readBuffer[readIndex]){
			case '?':
				readIndex = 0;
				show_game_stats();
				show_settings_menu();
				return;
			break;
			
			case 'R':
				readIndex = 0;
				reset_settings();
				settings_eeprom_write();
				return;
			break;
			
			case 'P':
				user_settings.games_played = 0;
				user_settings.total_points = 0;
				user_settings.high_score = 0;	
				user_settings.boss_kills = 0;
				settings_eeprom_write();
				return;
			break;		
			
			case '!':
				ESP.restart();			
			default:
			
			break;
		}
		
		if (readBuffer[readIndex] == CARRIAGE_RETURN) {
			if (readIndex < 3) {
				// not enough characters
				readIndex = 0;
			}
			else {				
				readBuffer[readIndex] = 0; // mark it as the end of the string
				change_setting_serial(readBuffer);	
				readIndex = 0;
			}
		}
		else if (readIndex >= READ_BUFFER_LEN) {
			readIndex = 0; // too many characters. Reset and try again
		}
		else
			readIndex++;
}

void change_setting_serial(char *line) {
  // line formate should be ss=nn
  // ss is always a 2 character integer
  // nn starts at index 3 and can be up to a 5 character unsigned integer
  
  //12=12345
  //01234567
  
  char setting_val[6];
  char param;
  uint16_t newValue;
  //Serial.print("Read Buffer: "); Serial.println(readBuffer); 
  
  if (readBuffer[1] != '='){  // check if the equals sign is there
	Serial.print("Missing '=' in command");
	readIndex = 0;
	return;
  }
  
  // move the value characters into a char array while verifying they are digits
  for(int i=0; i<5; i++) {
	if (i+2 < readIndex) {
		if (isDigit(readBuffer[i+2]))
			setting_val[i] = readBuffer[i+2];
		else {
			Serial.println("Invalid setting value");
			return;
			
		}			
	}
	else
		setting_val[i] = 0;
  }
  
  param = readBuffer[0];
  newValue = atoi(setting_val); // convert the val section to an integer
  
  memset(readBuffer,0,sizeof(readBuffer));
	
	change_setting(param, newValue);
	
  
}

void change_setting(char paramCode, uint16_t newValue)
{
	switch (paramCode) {
		 
		
		lastInputTime = millis(); // reset screensaver count				
		
		case 'C': // LED Count
				user_settings.led_count = constrain(newValue, MIN_LEDS, MAX_LEDS);
				settings_eeprom_write();
		break;	
			
		case 'B': // brightness
			user_settings.led_brightness = constrain(newValue, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
			FastLED.setBrightness(user_settings.led_brightness);
			settings_eeprom_write();			
		break;
		
		case 'S': // sound
			user_settings.audio_volume = constrain(newValue, MIN_VOLUME, MAX_VOLUME);
			settings_eeprom_write();
		break;
		
		case 'D': // deadzone, joystick
			user_settings.joystick_deadzone = constrain(newValue, MIN_JOYSTICK_DEADZONE, MAX_JOYSTICK_DEADZONE);
			settings_eeprom_write();		
		break;
		
		case 'A': // attack threshold, joystick
			user_settings.attack_threshold = constrain(newValue, MIN_ATTACK_THRESHOLD, MAX_ATTACK_THRESHOLD);
			settings_eeprom_write();
		break;
		
		case 'L': // lives per level
			user_settings.lives_per_level = constrain(newValue, MIN_LIVES_PER_LEVEL, MAX_LIVES_PER_LEVEL);
			settings_eeprom_write();
		break;	
		
		case 'G': // game difficulty
			user_settings.difficulty = constrain(newValue, MIN_DIFFICULTY, MAX_DIFFICULTY);
			settings_eeprom_write();
		break;
			Serial.print("Command Error: ");
			Serial.println(readBuffer[0]);
			return;
		break;
	
  } 
	
	show_settings_menu();
	
}

void reset_settings() {
	
	
	user_settings.settings_version = SETTINGS_VERSION;	
	
	user_settings.led_count = NUM_LEDS;
	user_settings.led_brightness = DEFAULT_BRIGHTNESS;	
	
	user_settings.joystick_deadzone = DEFAULT_JOYSTICK_DEADZONE;
	user_settings.attack_threshold = DEFAULT_ATTACK_THRESHOLD;
	
	user_settings.audio_volume = DEFAULT_VOLUME;
	
	user_settings.lives_per_level = LIVES_PER_LEVEL;
	
	user_settings.difficulty = DEFAULT_DIFFICULTY;
	user_settings.total_points = 0;
	user_settings.high_score = 0;
	user_settings.boss_kills = 0;
	for (int i = 0; i < 5; i++) {
		user_settings.leaderboard[i].score = 0;
		strncpy(user_settings.leaderboard[i].name, "", sizeof(user_settings.leaderboard[i].name));
	}
	
	Serial.println("Settings reset...");
	
	settings_eeprom_write();	
}

void show_settings_menu() {
	Serial.println("\r\n====== TWANG Settings Menu ========");
	Serial.println("=    Current values are shown     =");
	Serial.println("=   Send new values like B=150    =");
	Serial.println("=     with a carriage return      =");
	Serial.println("===================================");
	
	Serial.print("\r\nC=");	
	Serial.print(user_settings.led_count);
	
	Serial.print(" (LED Count 60-");
	Serial.print(MAX_LEDS);
	Serial.println(")");
	
	Serial.print("B=");	
	Serial.print(user_settings.led_brightness);
	Serial.println(" (LED Brightness 5-255)");
	
	Serial.print("S=");
	Serial.print(user_settings.audio_volume);
	Serial.println(" (Sound Volume 0-255)");
	
	Serial.print("D=");
	Serial.print(user_settings.joystick_deadzone);
	Serial.println(" (Joystick Deadzone 3-12)");
	
	Serial.print("A=");
	Serial.print(user_settings.attack_threshold);
	Serial.println(" (Attack Sensitivity 20000-35000)");	
	
	Serial.print("L=");
	Serial.print(user_settings.lives_per_level);
	Serial.println(" (Lives per Level (3-9))");
	
	Serial.print("G=");
	Serial.print(user_settings.difficulty);
	Serial.println(" (Game Difficulty 1=Easy, 2=Medium, 3=Hard)");
	
	Serial.println("\r\n(Send...)");
	Serial.println("  ? to show current settings");
	Serial.println("  R to reset everything to defaults)");
	Serial.println("  P to reset play statistics)");
}

void show_game_stats()
{
	Serial.println("\r\n===== Play statistics ======");
	Serial.print("Games played: ");Serial.println(user_settings.games_played);
	if (user_settings.games_played > 0)	{
		Serial.print("Average Score: ");Serial.println(user_settings.total_points / user_settings.games_played);
	}
	Serial.print("High Score: ");Serial.println(user_settings.high_score);
	Serial.print("Boss kills: ");Serial.println(user_settings.boss_kills);	
}

void settings_eeprom_read() {
    EEPROM.begin(EEPROM_SIZE);

    uint8_t ver = EEPROM.read(0);
    if (ver != SETTINGS_VERSION) {
        Serial.print("Error: EEPROM settings read failed: "); Serial.println(ver);
        Serial.println("Loading defaults...");
        EEPROM.end();
        reset_settings();
        return;
    } else {
        Serial.print("Settings version: "); Serial.println(ver);
    }

    uint8_t temp[sizeof(user_settings)];
    for (int i = 0; i < sizeof(user_settings); i++) {
        temp[i] = EEPROM.read(i + 1); // Start reading from address 1
    }

    EEPROM.end();
    memcpy((char*)&user_settings, temp, sizeof(user_settings));
}

void settings_eeprom_write() {
    sound_pause(); // prevent interrupt from causing crash

    EEPROM.begin(EEPROM_SIZE);

    EEPROM.write(0, SETTINGS_VERSION); // Write version at address 0

    uint8_t temp[sizeof(user_settings)];
    memcpy(temp, (uint8_t*)&user_settings, sizeof(user_settings));

    for (int i = 0; i < sizeof(user_settings); i++) {
        EEPROM.write(i + 1, temp[i]); // Start writing from address 1
    }

    EEPROM.commit();
    EEPROM.end();

    sound_resume(); // restore sound interrupt
}

void printError(int reason) {
	
	switch(reason) {
		case ERR_SETTING_NUM:
			Serial.print("Error: Invalid setting number");
		break;
		case ERR_SETTING_RANGE:
			Serial.print("Error: Setting out of range");
		break;
		default:
			Serial.print("Error:");Serial.println(reason);
		break;
	}	
}

#endif
