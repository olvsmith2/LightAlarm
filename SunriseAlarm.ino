//Speaker notes file
#include "pitches.h"

//LEDs
int LEDPin = 15;
int fadeTime = 30000; //Time between the fading increments of the LED. Set to 15/51
unsigned long lastFadeIncrement = 0; //Used to check the time between fading the LED

//RTC Libs
#include <RTClib.h>
#include <Wire.h>

//Display Lib
#include <SevSeg.h>
SevSeg sevseg;

//Setting time Lib
#include <iostream>

//RTC setup
int currentTime;
RTC_DS3231 rtc;
char timeString[5]; //Makes string to hold the time

//Setting time Variables
int minIncrement = 5; // Sets the increments between times
int times[288]; // Declare an array to hold all the times. Change length to 1440/96
int timeIndex;
int startTime;

//Encoder Variables
int encoderCLKPin = 19;
int encoderDTPin = 18;
int encoderSWPin = 17; //Encoder Pins
int encoderCounter = 0;
int encoderCountMax = ((1440/minIncrement)-1)*4;
int encoderCountMin = 0;
unsigned long lastRunEncoder=0; //Used to debouce encoder signals
bool settingAlarm = false;

//On Off Switch 
int onOffPin = 8;
bool alarmOn = true;

//Speaker Variables
int tempo = 140;
int buzzerPin = 16;

//Music
int melody[] = { 
  // Notes of the moledy followed by the duration.
  // Minuet in G - Petzold
  // Score available at https://musescore.com/user/3402766/scores/1456391
  NOTE_D5,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,8, //1
  NOTE_D5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8,
  NOTE_G5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_C5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8,
  
  NOTE_B4,4, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,//6
  NOTE_FS4,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8,
  NOTE_A4,-2,
  NOTE_D5,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,8, 
  NOTE_D5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8,
  
  NOTE_G5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_C5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, //12
  NOTE_B4,4, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
  NOTE_A4,4, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8, NOTE_FS4,8,
  NOTE_G4,-2,

  //repeats from 1

  NOTE_D5,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,8, //1
  NOTE_D5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8,
  NOTE_G5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_C5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8,
  
  NOTE_B4,4, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,//6
  NOTE_FS4,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8,
  NOTE_A4,-2,
  NOTE_D5,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,8, 
  NOTE_D5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8,
  
  NOTE_G5,4, NOTE_G4,4, NOTE_G4,4,
  NOTE_C5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, //12
  NOTE_B4,4, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
  NOTE_A4,4, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8, NOTE_FS4,8,
  NOTE_G4,-2,

  //continues from 17

  NOTE_B5,4, NOTE_G5,8, NOTE_A5,8, NOTE_B5,8, NOTE_G5,8,//17
  NOTE_A5,4, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_G5,4, NOTE_E5,8, NOTE_FS5,8, NOTE_G5,8, NOTE_D5,8,
  NOTE_CS5,4, NOTE_B4,8, NOTE_CS5,8, NOTE_A4,4,
  NOTE_A4,8, NOTE_B4,8, NOTE_CS5,8, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8,

  NOTE_G5,4, NOTE_FS5,4, NOTE_E5,4, //22
  NOTE_FS5,4, NOTE_A4,4, NOTE_CS5,4,
  NOTE_D5,-2,
  NOTE_D5,4, NOTE_G4,8, NOTE_FS5,8, NOTE_G4,4,
  NOTE_E5,4,  NOTE_G4,8, NOTE_FS4,8, NOTE_G4,4,
  NOTE_D5,4, NOTE_C5,4, NOTE_B4,4,

  NOTE_A4,8, NOTE_G4,8, NOTE_FS4,8, NOTE_G4,8, NOTE_A4,4, //28
  NOTE_D4,8, NOTE_E4,8, NOTE_FS4,8, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8,
  NOTE_C5,4, NOTE_B4,4, NOTE_A4,4,
  NOTE_B4,8, NOTE_D5,8, NOTE_G4,4, NOTE_FS4,4,
  NOTE_G4,-2,
};
int notes = sizeof(melody) / sizeof(melody[0]) / 2; //Number of notes used in the for loop to play melody
int wholenote = (60000 * 4) / tempo; //Calculates duration of a whole note based on tempo
int divider = 0, noteDuration = 0;

//Play Melody function
void play_melody(){
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    if (alarmOn == false) {
      break;      
    }
    divider = melody[thisNote + 1]; // calculates the duration of the note
    if (divider > 0) {
      noteDuration = (wholenote) / divider; // regular note, just proceed
    } else if (divider < 0) {
      noteDuration = (wholenote) / abs(divider); // dotted notes are represented with negative durations
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
    tone(buzzerPin, melody[thisNote], noteDuration * 0.9); //The note is played for 90% of the duration, leaving 10% as a pause
    
    delay(noteDuration); //Wait for the specief duration before playing the next note.
    
    noTone(buzzerPin); // stop the waveform generation before the next note.

    
  }
}

//Encoder interupt functions
void shaft_moved(){
  if (settingAlarm == true) {
    if (millis()-lastRunEncoder>5){  
      if (digitalRead(encoderDTPin)==1 && digitalRead(encoderCLKPin)==0 || digitalRead(encoderDTPin)==0 && digitalRead(encoderCLKPin)==1){ //Checks direction of the turn
        if(encoderCounter==encoderCountMax){
            encoderCounter = encoderCountMin; //Loops round if the counter is at the max value
        } else{
            encoderCounter++;
        }
      }
      if (digitalRead(encoderDTPin)==0 && digitalRead(encoderCLKPin) == 0 || digitalRead(encoderDTPin)==1 && digitalRead(encoderCLKPin)==1){ //Checks direction of the turn
        if (encoderCounter==0) {
            encoderCounter = encoderCountMax; //Loops round if the counter is at the min value
        } else{
            encoderCounter--;
        }
      }
      lastRunEncoder=millis(); //Used to check when last interupt was run
    }
  }
}

void encoder_pressed() {
  if (millis() - lastRunEncoder > 200) {
    settingAlarm = !settingAlarm;
  }
  lastRunEncoder=millis();
}

void on_off() {
  if (digitalRead(onOffPin) == HIGH) {
    alarmOn = false;
  } else if (digitalRead(onOffPin) == LOW) {
    alarmOn = true;
  }
}

int get_time() {
  DateTime now = rtc.now();
  char timeString[5];
  if (now.minute() < 10) {
    sprintf(timeString, "%02d0%d", now.hour(), now.minute());
  } else {
    sprintf(timeString, "%02d%02d", now.hour(), now.minute());
  }
  int time_int = atoi(timeString);
  char timeStringPadded[5];
  sprintf(timeStringPadded, "%04d", time_int);
  return time_int;
}

int subtract_15_minutes(int time) {
  int hour = time / 100; // Get the hour (first two digits)
  int minute = time % 100; // Get the minute (last two digits)
  
  minute -= 15; // Subtract 15 minutes
  
  // If the minute becomes negative, subtract an hour and add 60 minutes
  if (minute < 0) {
    hour--;
    minute += 60;
  }
  
  // If the hour becomes negative, add 24 hours
  if (hour < 0) {
    hour += 24;
  }
  
  // Combine the hour and minute into a 4-digit integer and return it
  return (hour * 100) + minute;
}

void wake_up() {
  lastFadeIncrement = millis();
  int i = 0;
  while (alarmOn == true) {
    sevseg.setChars("UPUP"); 
    sevseg.refreshDisplay(); //Sets display to say UPUP
    if (i > 255) {
      Serial.println(i);
      sevseg.setNumber(0); 
      sevseg.refreshDisplay(); //Sets display to say UPUP
      for (int j = 0; j<3; j++) { //Repeates the melody 5 times
        play_melody();
      }     
      analogWrite(LEDPin, 0);
      break;
    }else if (millis()-lastFadeIncrement > fadeTime && i < 20) {
      i += 1;
      Serial.println(i);
      analogWrite(LEDPin, i);
      lastFadeIncrement = millis();
    }else if (millis()-lastFadeIncrement > fadeTime && i <= 255 && i >= 20) {
      i += 25;
      Serial.println(i);
      analogWrite(LEDPin, i);
      lastFadeIncrement = millis();
    }      
  }
  analogWrite(LEDPin, 0);
}

void setup() {
  Serial.begin(9600);
  //RTC initialising and setting time
  Wire.begin();
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  //rtc.adjust(DateTime(2019, 1, 21, 9, 59, 0));
  
  //LED pin/ mosfet controler pin
  pinMode(LEDPin, OUTPUT);

  //Display setup
  byte numDigits = 4; //4 digit display
  byte digitPins[] = {3, 2, 1, 26}; //digit pins from 1 to 4
  byte segmentPins[] = {7, 0, 27, 21, 20, 6, 28, 22}; //digit pins from a to g + dp
  bool resistorsOnSegments = true; 
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_ANODE; 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);

  //Encoder setup
  int index = 0;
  for (int hour = 0; hour < 24; hour++) { 
        for (int minute = 0; minute < 60; minute += minIncrement) { 
            int time = hour * 100 + minute;
            times[index] = time; 
            index++; //For loop to create the 4-digit times at the minIncrement and store them to the times array
        }
    }
  attachInterrupt(digitalPinToInterrupt(encoderSWPin),encoder_pressed, FALLING);
  pinMode(encoderSWPin,INPUT_PULLUP); //Interupt for the enoder being pressed
  attachInterrupt(digitalPinToInterrupt(encoderCLKPin), shaft_moved, CHANGE);
  pinMode(encoderDTPin,INPUT); //Interupt for the enoder turning
  
  //Set up on off switch pin
  attachInterrupt(digitalPinToInterrupt(onOffPin), on_off, CHANGE);
  pinMode(onOffPin, INPUT_PULLUP);
  if (digitalRead(onOffPin) == HIGH) {
    alarmOn = false;
  }
}

void loop() {
  timeIndex = encoderCounter/4;
  currentTime = get_time();
  startTime = subtract_15_minutes(times[timeIndex]);
  if (settingAlarm == true) {
    sevseg.setNumber(times[timeIndex], 2); 
    sevseg.refreshDisplay(); //Sets display with dp after digit 2
  } else if (alarmOn == true && startTime == currentTime) {
    wake_up();
  } else {
    sevseg.setNumber(currentTime, 2); 
    sevseg.refreshDisplay(); //Sets display with dp after digit 2
  }
}
  
