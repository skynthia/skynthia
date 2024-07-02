#include <Servo.h>
#include "Util.h"

#define NUM_INPUTS 2

bool tentacle_on = false;

// inputs
int fisting = 0;
int fisting_new = 0;
int fisting_pin = 53;

int voice_pins[4] = {5, 4, 3, 2};
int hit_pins[4] = {8, 9, 10, 11};

int *inputs[NUM_INPUTS] = {voice_pins, hit_pins};
int input_values[NUM_INPUTS] = {0, 0};            // Will be a binary number calculated from the input pins
int input_pin_count[NUM_INPUTS] = {4, 4};
int input_counts[NUM_INPUTS] = {0, 0};

void setup() {
  Serial.begin(9600);
  Serial.println("Booting Doig...");
  
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
  delay(100);
}

void checkInputs() {
  // Are we allowing programming?
  fisting_new = !digitalRead(fisting_pin);
  // Signal that we're reading to receive programming via a haptic signal
  if (fisting_new != fisting && fisting_new) {
    Serial.println("Accepting sensor input");
    fistingHaptic();
  }
  fisting = fisting_new;
  if (!fisting) {
    stopHaptics();
    return;
  }

  for (int i = 0; i < NUM_INPUTS; i++) {
    int *input_pins = inputs[i];
    int binary_value = 0;
    int count = 0;
  
    for (int j = 0; j < input_pin_count[i]; j++) {
      int reading = digitalRead(input_pins[j]);
      //Serial.print("Reading from pin ");
      //Serial.println(input_pins[i]);
      binary_value = binary_value | (!reading << j);
      if (!reading) {
        count++;
      }
    }

    input_values[i] = binary_value;
    // Value has changed since the last check
    if (count != input_counts[i]) {
      Serial.print("changed sensor ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(binary_value);
      inputHaptic(count, input_pin_count[i]);
      input_counts[i] = count;
    }
  }
}
