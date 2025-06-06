#include <Servo.h>
#include "Util.h"

#define NUM_INPUTS 3

bool tentacle_on = false;

// inputs
int fisting = 0;
int fisting_new = 0;
int fisting_pin = 53;

int vibe_pins[4] = {5, 4, 3, 2};
int hit_pins[4] = {8, 9, 10, 11};
int voice_pins[3] = {23, 25, 27};
int dyn_pins[2] = {A0, A1};
int dyn_threshold = 900;
int dyn_value = 0;
int dyn_count_arr[2] = {0, 0};
int dyn_count = 0;

int *inputs[NUM_INPUTS] = {vibe_pins, hit_pins, voice_pins};
int input_values[NUM_INPUTS] = {0, 0, 0};            // Will be a binary number calculated from the input pins
int input_pin_count[NUM_INPUTS] = {4, 4, 3};
int input_counts[NUM_INPUTS] = {0, 0, 0};
char input_names[4] = {'B', 'H', 'V', 'D'};
String friendly_input_names[NUM_INPUTS] = {"vibe", "hits", "voice"};

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Booting Doig...");
  Serial1.println("Booting Doig...");

  for (int i = 0; i < NUM_INPUTS; i++) {
    int *input_pins = inputs[i];
    for (int j = 0; j < input_pin_count[i]; j++) {
      pinMode(input_pins[j], INPUT);
    }
  }
  
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
    dyn_value = 0; // always restart dynamism at 0 to make it work with input style
    dyn_count_arr[0] = dyn_count_arr[1] = dyn_count = 0;
  }
  else if (fisting_new != fisting && !fisting_new) {
    Serial.print("dynamism set to ");
    Serial.println(dyn_value);
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
      int reading = !digitalRead(input_pins[j]);
      /*Serial.print("Reading from pin ");
      Serial.print(pin);
      Serial.print(": ");
      Serial.println(reading);*/
      binary_value = binary_value | (reading << j);
      if (reading) {
        count++;
      }
    }

    input_values[i] = binary_value;
    // Value has changed since the last check
    if (count != input_counts[i]) {
      Serial.print("changed sensor ");
      Serial.print(friendly_input_names[i]);
      Serial.print(": ");
      Serial.println(binary_value);
      //Serial.println(input_counts[i]);
      inputHaptic(count, input_pin_count[i], i);
      input_counts[i] = count;
      sendToServer(i, binary_value);
    }
  }

  checkDyn();
}

void sendToServer(int which, int val) {
  char input_name = input_names[which];
  if (which == 0) {
    // formerly known as vibe, now vibe
    if (val > 7) {
      input_name = 'F';
    }
    val = val & 7;
  }

  Serial1.write('D');
  Serial1.write(input_name);
  Serial1.write(val + 65); // val max is 15
  Serial1.write('\n');
}

void checkDyn() {
  for (int j = 0; j < 2; j++) {
    int reading = analogRead(dyn_pins[j]);
    if (reading >= dyn_threshold) {
      dyn_value = dyn_value | (1 << j);
      dyn_count_arr[j] = 1;
    }
    // this is so stupid
    int count = dyn_count_arr[0] + dyn_count_arr[1];
    
    if (count != dyn_count) {
      inputHaptic(count, 2, 3);
      dyn_count = count;
      sendToServer(3, dyn_count);
    }
  }
}
