#include "MIDIUSB.h"

// FOOT INPUTS
const int N_BUTTONS = 8;
const int BUTTONS[N_BUTTONS] = {2, 3, 4, 5, 6, 7, 8, 9};
const int EXPRESSION_PEDAL = A0;

// STATES
int btnCurrentState[N_BUTTONS] = {};
int btnPrevState[N_BUTTONS] = {};
boolean btnVirtualRetention[N_BUTTONS] = {false, false, false, false, false, false, false, false};
boolean expressionVirtualRetention = false;

// LEDS
const int N_LEDS = 8;
const int LEDS[N_LEDS] = {10, A1, A2, A3, A4, A5, 12, 11};

// BUTTON DEBOUNCE
unsigned long lastDebounceTime[N_BUTTONS] = {0};
unsigned long debounceDelay = 50;

// MIDI
const byte MIDI_CHANNEL = 0;
const byte MIDI_NOTES[N_BUTTONS] = {0, 12, 24, 36, 48, 60, 72, 84};
const byte MIDI_EXPRESION_ON_OFF = 96;

void setup(){
  for (int i = 0; i < N_BUTTONS; i++){
    pinMode(BUTTONS[i], INPUT_PULLUP);
    pinMode(EXPRESSION_PEDAL, INPUT_PULLUP);
  }

  for (int i = 0; i < N_LEDS; i++){
    pinMode(LEDS[i], OUTPUT);
  }
}

void loop(){
  for (int index = 0; index < N_BUTTONS; index++){
    btnCurrentState[index] = digitalRead(BUTTONS[index]);

    if ((millis() - lastDebounceTime[index]) > debounceDelay){
      if (btnPrevState[index] != btnCurrentState[index]){
        handleUpdateDeboundeTime(index);
        handleFootCommand(index);
        btnPrevState[index] = btnCurrentState[index];
      }
    }
  }

  handleExpressionChange();
}

void handleFootCommand(int index){
  if (btnCurrentState[index] == LOW){
    if (index < 4){
      handleNote(MIDI_CHANNEL, MIDI_NOTES[index], 64);
      MidiUSB.flush();
      handleLedLatching(index);
    }
    else{
      handleProgramChange(MIDI_CHANNEL, index);
      MidiUSB.flush();
      handleResetLedsStatus();
      handleLedBlink(index);
    }
  }
}

void handleUpdateDeboundeTime(int index){
  lastDebounceTime[index] = millis();
}

void handleLedLatching(int index){
  if (btnVirtualRetention[index] == false){
    btnVirtualRetention[index] = true;
    digitalWrite(LEDS[index], HIGH);
  }
  else{
    btnVirtualRetention[index] = false;
    digitalWrite(LEDS[index], LOW);
  }
}

void handleLedBlink(int index){
  digitalWrite(LEDS[index], HIGH);
  delay(100);
  digitalWrite(LEDS[index], LOW);
}

void handleResetLedsStatus(){
  for (int index = 0; index < N_BUTTONS; index++){
    btnVirtualRetention[index] = false;
    digitalWrite(LEDS[index], LOW);
  }
}

void handleNote(byte channel, byte pitch, byte velocity){
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void handleProgramChange(byte channel, byte program){
  midiEventPacket_t pc = {0x0C, 0xC0 | channel, program, 0};
  MidiUSB.sendMIDI(pc);
}

void handleExpressionChange(){
  int pitch = map(analogRead(EXPRESSION_PEDAL), 220, 787, -1, 127);
  midiEventPacket_t noteOn = {0x09, 0x90 | MIDI_CHANNEL, MIDI_EXPRESION_ON_OFF, 64};

  if (expressionVirtualRetention == false && pitch > 0){
    MidiUSB.sendMIDI(noteOn);
    MidiUSB.flush();
    expressionVirtualRetention = true;
  }

  if (expressionVirtualRetention == true && pitch <= 0){
    MidiUSB.sendMIDI(noteOn);
    MidiUSB.flush();
    expressionVirtualRetention = false;
  }

  midiEventPacket_t cc = {0x0B, 0xB0 | MIDI_CHANNEL, 1, pitch};
  MidiUSB.sendMIDI(cc);
  MidiUSB.flush();
}
