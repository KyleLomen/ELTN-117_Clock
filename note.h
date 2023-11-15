#ifndef __NOTE_H__
#define __NOTE_H__

struct note {
  unsigned int frequency;
  unsigned int time;
};

void playNote(int pin, note n) {
  if(n.frequency > 0) {
    tone(pin, n.frequency);
  }
  else {
    noTone(pin);
  }
  delay(n.time);
}

void playMelody(int pin, const note melody[], int length) {
  for(int i = 0; i < length; i++) {
    playNote(pin, melody[i]);
  }
  noTone(pin);
}

void stopNote(int pin) {
  noTone(pin);
}

#define bpm(x) (60000/(x))

#endif //__NOTE_H__