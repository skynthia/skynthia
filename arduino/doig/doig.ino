#include "Util.h"

#define NUM_INPUTS 3

// inputs
int fisting = 0;
int fisting_new = 0;
int fisting_pin = 53;

int vibe_pins[4] = {5, 4, 3, 2};
int hit_pins[4] = {8, 9, 10, 11};
int voice_pins[3] = {23, 25, 27};

int photo_threshold = 900;
int dyn_pin = A0;
int dyn_count = 0;
int last_dyn_value = 0;
bool dyn_changed = false;
unsigned long dyn_start;

int tempo_pin = A1;
double tempo = 0;
int last_tempo_value = 0;
int tempo_diffs[4];
int tempo_index = 0;
bool tempo_changed = false;
unsigned long tempo_start;

int *inputs[NUM_INPUTS] = {vibe_pins, hit_pins, voice_pins};
int input_values[NUM_INPUTS] = {0, 0, 0};            // Will be a binary number calculated from the input pins
int input_pin_count[NUM_INPUTS] = {4, 4, 3};
int input_counts[NUM_INPUTS] = {0, 0, 0};
char input_names[5] = {'B', 'H', 'V', 'D', 'T'};
String friendly_input_names[5] = {"vibe", "hits", "voice", "dynamism", "tempo"};

unsigned long ping_clock;

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
  dyn_start = tempo_start = ping_clock = millis();
  
  randomSeed(analogRead(A15));
}

void loop() {
  checkPing();
  checkInputs();
  doHaptics();
}

void checkPing() {
  // ping every 5 seconds  
  if (millis() - ping_clock >= 5000) {
    Serial1.write('P');
    Serial1.write(0);
    Serial1.write('\n');
    ping_clock = millis();
  }
}

void checkInputs() {
  // Are we allowing programming?
  fisting_new = !digitalRead(fisting_pin);
  // Signal that we're reading to receive programming via a haptic signal
  if (fisting_new != fisting && fisting_new) {
    Serial.println("Accepting sensor input");
    fistingHaptic();
  }
  else if (fisting_new != fisting && !fisting_new) {
    if (dyn_changed) {
      Serial.print("dynamism set to ");
      Serial.println(dyn_count);
      dyn_changed = false;
      dyn_count = 0;
      sendToServer(3, dyn_count);
    }
    if (tempo_changed) {
      Serial.print("tempo set to ");
      Serial.println(tempo);
      tempo_changed = false;
      tempo_index = 0;
      sendToServer(4, tempo);
    }
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
  checkTempo();
}

void sendToServer(int which, int val) {
  char input_name = input_names[which];
  if (which == 0) {
    // formerly known as root_voice, now vibe
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
  bool reading = analogRead(dyn_pin) >= photo_threshold;
  // if this is newly over the threshold, AND it's been at least 50ms since the last such reading
  if (reading != last_dyn_value && reading && millis() - dyn_start > 50) {
    Serial.println("dynamism change detected");
    dyn_changed = true; // Dynamism was changed
    dyn_count++;
    dyn_start = millis();
    inputHaptic(dyn_count, 4, 3);
  }
  
  last_dyn_value = reading;
}

void checkTempo() {
  bool reading = analogRead(tempo_pin) >= photo_threshold;
  long ms = millis();
  if (reading != last_tempo_value && reading && ms - tempo_start > 50) {
    Serial.println("beat detected");
    tempo_changed = true;
    tempo_diffs[tempo_index] = ms - tempo_start;
    tempo_start = ms;

    if (tempo_index == 0) {
      inputHaptic(1, 4, 5);
    }
    else if (tempo_index == 3) {
      tempo = calculateTempo();
      pulse_lengths[6][0] = tempo * 0.75;
      pulse_lengths[6][1] = tempo * 0.25;
      inputHaptic(4, 4, 6);
    }

    tempo_index = (tempo_index + 1) % 4; // mod 4 so in case this triggers more than 4 times we don't go out of bounds
  }

  last_tempo_value = reading;
}

double calculateTempo() {
  double tempo_sum = 0.0;
  for (int i = 0; i < 4; i++) {
    tempo_sum += tempo_diffs[i];
  }
  return tempo_sum / 4.0;
}
