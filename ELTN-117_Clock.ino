/**
   \file ELTN-117_Clock.ino
   \author Kyle Lomen (klomen2@go.pasadena.edu)
   \brief
   \version 1.1
   \date 11/14/2023
*/
#include "pitches.h"
#include "note.h"

// Pin Definitions
const int clockIn = 2;
const int minSet = 3;
const int hourSet = 4;
const int alarmSet = 5;
const int pmLED = 6;
const int speaker = 7;
const int minDisplay[] = {8, 9, 10, 11};
const int loadTens = 13;
const int loadOnes = 12;
const int hourDisplay[] = {A0, A1, A2, A3};

// Constants
const int ticksPerMinute = 3600; // Set to 60 for testing

// Westminster Quarters
#define t bpm(100)
const note westminsterQuarters[5][4] = {
  {{NOTE_GS4,t}, {NOTE_FS4,t}, {NOTE_E4,t }, {NOTE_B3,2*t}},
  {{NOTE_E4 ,t}, {NOTE_GS4,t}, {NOTE_FS4,t}, {NOTE_B3,2*t}},
  {{NOTE_E4 ,t}, {NOTE_FS4,t}, {NOTE_GS4,t}, {NOTE_E4,2*t}},
  {{NOTE_GS4,t}, {NOTE_E4, t}, {NOTE_FS4,t}, {NOTE_B3,2*t}},
  {{NOTE_B3 ,t}, {NOTE_FS4,t}, {NOTE_GS4,t}, {NOTE_E4,2*t}}
};
const note hourStrike[] = {{NOTE_FS5, t}, {NOTE_SILENT, 3*t}};
#undef t

// Alarm sound
#define t (int)bpm(130)
const note alarmMelody[] = {
  {NOTE_E5 , t/2}, {NOTE_D5, t/2}, {NOTE_FS4, t}, {NOTE_GS4, t},
  {NOTE_CS5, t/2}, {NOTE_B4, t/2}, {NOTE_D4 , t}, {NOTE_E4 , t},
  {NOTE_B4 , t/2}, {NOTE_A4, t/2}, {NOTE_CS4, t}, {NOTE_E4 , t},
  {NOTE_A4 , 3*t}, {NOTE_SILENT, t}
};
const int alarmRepeats = 3;
#undef t

#define arrLen(arr) (sizeof(arr)/sizeof(arr[0]))

// Main Time
int hour = 12;
int minute = 0;
bool isPM = false;

// Alarm Time
int alarmHour = 12;
int alarmMinute = 0;
bool alarmIsPM = false;

volatile int tickCount = 0;

// 60Hz interrupt function
void tick() {
  tickCount++;
}

void setup() {
  pinMode(clockIn, INPUT);
  pinMode(minSet, INPUT_PULLUP);
  pinMode(hourSet, INPUT_PULLUP);
  pinMode(alarmSet, INPUT_PULLUP);
  pinMode(pmLED, OUTPUT);
  pinMode(speaker, OUTPUT);
  for (int p : minDisplay) {
    pinMode(p, OUTPUT);
  }
  pinMode(loadTens, OUTPUT);
  pinMode(loadOnes, OUTPUT);
  for (int p : hourDisplay) {
    pinMode(p, OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(clockIn), tick, RISING);
}

void playWestminsterQuarters() {
  if(minute == 15) {
    playMelody(speaker, westminsterQuarters[0], 4);
  }
  else if(minute == 30) {
    playMelody(speaker, westminsterQuarters[1], 4);
    playMelody(speaker, westminsterQuarters[2], 4);
  }
  else if(minute == 45) {
    playMelody(speaker, westminsterQuarters[3], 4);
    playMelody(speaker, westminsterQuarters[4], 4);
    playMelody(speaker, westminsterQuarters[0], 4);
  }
  else if(minute == 0) {
    playMelody(speaker, westminsterQuarters[1], 4);
    playMelody(speaker, westminsterQuarters[2], 4);
    playMelody(speaker, westminsterQuarters[3], 4);
    playMelody(speaker, westminsterQuarters[4], 4);
    playNote(speaker, (note){NOTE_SILENT, 1200});
    for(int i = 0; i < hour; i++) {
      playMelody(speaker, hourStrike, arrLen(hourStrike));
    }
  }
}

void playAlarm() {
  for(int j = 0; j < alarmRepeats; j++) {
    playMelody(speaker, alarmMelody, arrLen(alarmMelody));
  }
  stopNote(speaker);
}

void handleButtons() {
  if(!digitalRead(hourSet)) {
    if(digitalRead(alarmSet)) { // Alarm set is not pressed
      tickCount = 0;
      hour++;
      if(hour == 12) {
        isPM = !isPM;
      }
      else if(hour > 12) {
        hour = 1;
      }
    }
    else { // Alarm set is pressed
      alarmHour++;
      if(alarmHour == 12) {
        alarmIsPM = !alarmIsPM;
      }
      else if(alarmHour > 12) {
        alarmHour = 1;
      }
    }
    displayTime();
    while(!digitalRead(hourSet)) {
      delay(35);
    }
  }
  if(!digitalRead(minSet)) {
    if(digitalRead(alarmSet)) { // Alarm set is not pressed
      tickCount = 0;
      minute++;
      if(minute >= 60) {
        minute = 0;
      }
    }
    else { // Alarm set is pressed
      alarmMinute++;
      if(alarmMinute >= 60) {
        alarmMinute = 0;
      }
    }
    displayTime();
    while(!digitalRead(minSet)) {
      delay(35);
    }
  }
}

void handleTime() {
  if(tickCount >= ticksPerMinute) {
    tickCount -= ticksPerMinute; // Subtract instead of setting to zero to avoid losing missed ticks
    minute++;
    if(minute >= 60) {
      hour++;
      minute = 0;
      if(hour == 12) {
        isPM = !isPM;
      }
      else if(hour > 12) {
        hour = 1;
      }
    }
    // Runs only once per minute
    playWestminsterQuarters();
    handleAlarm(); 
  }
}

void handleAlarm() {
  if(isPM = alarmIsPM && hour == alarmHour && minute == alarmMinute) {
    playAlarm();
  }
}

// Writes a value to the bus of the given size
// Note: only works for size less than 8
void writeBus(const int bus[], int size, byte value) {
  for(int i = 0; i < size; i++) {
    if(value & (1 << i)) {
      digitalWrite(bus[i], HIGH);
    }
    else {
      digitalWrite(bus[i], LOW);
    }
  }
}

// Displays the hour
void displayHour(int h, bool pm) {
  writeBus(hourDisplay, 4, h - 1);
  digitalWrite(pmLED, pm);
}

// Displays the minute
void displayMinute(int m) {
  writeBus(minDisplay, 4, m / 10);
  digitalWrite(loadTens, HIGH);
  digitalWrite(loadTens, LOW);
  writeBus(minDisplay, 4, m % 10);
  digitalWrite(loadOnes, HIGH);
  digitalWrite(loadOnes, LOW);
}

// Displays the time
void displayTime() {
  if(digitalRead(alarmSet)) { // Alarm Set is not pressed
    displayHour(hour, isPM);
    displayMinute(minute);
  }
  else {
    displayHour(alarmHour, alarmIsPM);
    displayMinute(alarmMinute);
  }
}

void loop() {
  handleTime();
  handleButtons();
  displayTime();
}
