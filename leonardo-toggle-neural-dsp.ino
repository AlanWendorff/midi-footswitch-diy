#include "MIDIUSB.h"

// BUTTONS
const int N_BUTTONS = 8;
const int BUTTONS[N_BUTTONS] = {2, 3, 4, 5, 6, 7, 8, 9};

// Button states
int buttonCurrentState[N_BUTTONS] = {};       
int buttonPrevState[N_BUTTONS] = {};  
boolean buttonStates[N_BUTTONS] = {false, false, false, false, false, false, false, false}; 

// LEDS
const int N_LEDS = 8;
const int LEDS[N_LEDS] = {A0, A1, A2, A3, A4, A5, 12, 11};

// debounce
unsigned long lastDebounceTime[N_BUTTONS] = {0};
unsigned long debounceDelay = 50;  

// MIDI
const byte MIDI_CHANNEL = 0; //* MIDI channel to be used
const byte MIDI_NOTES[N_BUTTONS] = {0, 12, 24, 36, 48, 60, 72, 84};

void setup() {
  //Serial.begin(115200); 
  
  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTONS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < N_LEDS; i++) {
    pinMode(LEDS[i], OUTPUT);
  }
}

void loop() {
    for (int i = 0; i < N_BUTTONS; i++) {
      buttonCurrentState[i] = digitalRead(BUTTONS[i]);
      if ((millis() - lastDebounceTime[i]) > debounceDelay) {
        if (buttonPrevState[i] != buttonCurrentState[i]) {
          lastDebounceTime[i] = millis();
          if(buttonCurrentState[i] == LOW) {
            //Serial.println(BUTTONS[i]);
            handleLedStatus(i);
            handleNote(MIDI_CHANNEL, MIDI_NOTES[i], 64);
            MidiUSB.flush();       
          }
          buttonPrevState[i] = buttonCurrentState[i];
      }
    }
  }
}


void handleLedStatus(int index) {
  if (index < 5) {
    if (buttonStates[index] == false){ 
      buttonStates[index] = true;
      digitalWrite(LEDS[index], HIGH);
    }else {
      buttonStates[index] = false;
      digitalWrite(LEDS[index], LOW);
    }
  }else {
    digitalWrite(LEDS[index], HIGH);
    delay(100);
     digitalWrite(LEDS[index], LOW);
  }  
}



void handleNote(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}
