bool fisting_haptic_pin = 52;
bool voices_haptic_pin = 7;
bool hits_haptic_pin = 6;
int haptic_pins[NUM_INPUTS] = {voices_haptic_pin, hits_haptic_pin};

int pulse_clock[NUM_INPUTS] = {0, 0};
int pulse_count[NUM_INPUTS] = {0, 0};
long ms[NUM_INPUTS];
int pulse_length = 300;
bool haptic_value[NUM_INPUTS] = {true, true};

void setupHaptics() {
  for (int i = 0; i < NUM_INPUTS; i++) {
    pinMode(haptic_pins[i], OUTPUT);
    digitalWrite(haptic_pins[i], HIGH); // Because of how the transistor works, this turns the motor off
    ms[i] = millis();
  }
  
  pinMode(fisting_haptic_pin, OUTPUT);
  digitalWrite(fisting_haptic_pin, HIGH);
}

void doHaptics() {
  // fisting
  digitalWrite(fisting_haptic_pin, !fisting);

  for (int i = 0; i < NUM_INPUTS; i++) {
    if (millis() - ms[i] >= pulse_length) {
      if (pulse_clock[i] < pulse_count[i]) {
        haptic_value[i] = !haptic_value[i];
        digitalWrite(haptic_pins[i], haptic_value[i]);
      }
      else {
        digitalWrite(haptic_pins[i], HIGH); // this turns it off
      }
      pulse_clock[i] = (pulse_clock[i] + 1) % 8;
      ms[i] = millis();
    }
  }
}
