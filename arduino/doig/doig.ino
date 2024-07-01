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
  delay(10);
}

void checkInputs() {
  // Are we allowing programming?
  int fisting_new = !digitalRead(fisting_pin);
  // Signal that we're reading to receive programming via a haptic signal
  if (fisting_new != fisting && fisting_new) {
    fistingHaptic();
  }
  
  fisting = fisting_new;
  if (!fisting) {
    return;
  }

  for (int i = 0; i < NUM_INPUTS; i++) {
    int *input_pins = inputs[i];
    int binary_value = 0;
    int count = 0;
    
    for (int i = 0; i < input_pin_count[i]; i++) {
      int reading = digitalRead(input_pins[i]);
      binary_value = binary_value | (!reading << i);
      if (!reading) {
        count++;
      }
    }

    input_values[i] = binary_value;
    // Value has changed since the last check
    if (count != input_counts[i]) {
      inputHaptic(count, input_pin_count[i]);
    }
  }
}
