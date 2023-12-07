/**
  \file ELTN.117_Clock_Code.ino
  \author Kyle Lomen (kylelomen@gmail.com)
  \brief
  \version 1.0
  \date 05/26/2023

*/

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
const int HrLdOnes = A4;
const int MinLdTens = A5;

// Constants
const int ticksPerMinute = 60; // Set to 60*60 for the final program

// Main Time
int hour = 12;
int minute = 0;
bool isPM = true;

// Alarm Time
int alarmHour = 12;
int alarmMinute = 0;
bool alarmIsPM = false;

volatile int tickCount = 0;

// 60Hz interrupt function
void tick() {
  tickCount++;
}

// Plays the alarm sound
void playAlarm() {
  const double tempo = 60000 / 130;
  for(int i = 0; i < 5; i++) {
    tone(speaker, 659); // E5
    delay(.5 * tempo); 
    tone(speaker, 587); // D5
    delay(.5 * tempo);
    tone(speaker, 370); // FS4
    delay(tempo);
    tone(speaker, 415); // GS4
    delay(tempo);
    tone(speaker, 554); // CS5
    delay(.5 * tempo);
    tone(speaker, 494); // B4
    delay(.5 * tempo);
    tone(speaker, 294); // D4
    delay(tempo);
    tone(speaker, 330); // E4
    delay(tempo);
    tone(speaker, 494); // B4
    delay(.5 * tempo);
    tone(speaker, 440); // A4
    delay(.5 * tempo);
    tone(speaker, 277); // CS4
    delay(tempo);
    tone(speaker, 330); // E4
    delay(tempo);
    tone(speaker, 440); // A4
    delay(3 * tempo);
    noTone(speaker);
    delay(tempo);
  }
}

// Checks if the alarm should go off
void handleAlarm() {
  if(isPM = alarmIsPM && hour == alarmHour && minute == alarmMinute) {
    playAlarm();
  }
}

// Makes sure the time is set from the ticks
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
    handleAlarm(); // Runs only once per minute
  }
}

// Handles the button presses
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
  SevSegDisHr(h);
}


void SevSegDisHr(int w){
writeBus(minDisplay, 4, w / 10);
digitalWrite(MinLdTens, HIGH);
digitalWrite(MinLdTens, LOW);
writeBus(minDisplay, 4, w % 10);
digitalWrite(HrLdOnes, HIGH);
digitalWrite(HrLdOnes, LOW);
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

  pinMode(HrLdOnes, OUTPUT)
  pinMode(MinLdTens, OUTPUT)
  }

  attachInterrupt(digitalPinToInterrupt(clockIn), tick, RISING);
}

void loop() {
  handleTime();
  handleButtons();
  displayTime();
}
