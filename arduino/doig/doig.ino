#include <Servo.h>
#include "Util.h"

#define NUM_INPUTS 2

bool tentacle_on = false;

// inputs
bool fisting = false;
bool fisting_pin = 53;

int voice_pins[4] = {5, 4, 3, 2};
int hit_pins[4] = {8, 9, 10, 11};

int *inputs[NUM_INPUTS] = {voice_pins, hit_pins};
int input_values[NUM_INPUTS] = {0, 0};            // Will be a binary number calculated from the input pins
int input_pin_count[NUM_INPUTS] = {4, 4};
int input_counts[NUM_INPUTS] = {0, 0};

void setup() {
  Serial.begin(9600);
  setupHaptics();
  
  randomSeed(analogRead(A15));
  setupTentacle();
}

void loop() {
  checkInputs();
  doHaptics();
  
  if (tentacle_on) {
    loopTentacle();
  }
}

void checkInputs() {
  // Are we allowing programming?
  fisting = !digitalRead(fisting_pin);
  tentacle_on = fisting; // for now
  if (!fisting) {
    return;
  }

  for (int i = 0; i < NUM_INPUTS; i++) {
    int *input_pins = inputs[i];
    input_values[i] = 0;
    
    for (int i = 0; i < input_pin_count[i]; i++) {
      input_counts[i] = 0;
      int reading = digitalRead(input_pins[i]);
      input_values[i] = input_values[i] | (!reading << i);
      if (!reading) {
        input_count[i]++;
      }
    }
  
    pulse_count[i] = input_counts[i] * 2; // multiply by two because of on and then off
  }
}
