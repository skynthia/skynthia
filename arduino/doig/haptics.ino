int haptic_pin = 52;

int pulse_clock = 0;
int pulse_count = 0;
int pulse_total = 8;
long ms;
int pulse_lengths[4][2] = {
  {200, 400},
  {400, 200},
  {100, 100},
  {150, 50}
};
int pulse_length[2] = {300, 300};
bool haptic_value = true;
bool continue_haptics = false;

void setupHaptics() {
  Serial.println("Setting up haptics...");
  pinMode(haptic_pin, OUTPUT);
  digitalWrite(haptic_pin, HIGH); // Because of how the transistor works, this turns the motor off
  ms = millis();
  Serial.println("Haptics setup finished");
}

// two fast, 
void fistingHaptic() {
  haptic_value = false;
  pulse_length[0] = 150; // 150ms on, 150ms off
  pulse_length[1] = 50;
  pulse_count = 4; // on and then off, twice
  pulse_clock = 0; // start at the beginning
  continue_haptics = false;
}

void inputHaptic(int count, int total_pins, int pulse_length_index) {
  pulse_length[0] = pulse_lengths[pulse_length_index][0];//250; // TODO: different pulse lengths for each input, give a different "voice"
  pulse_length[1] = pulse_lengths[pulse_length_index][1];//400;
  pulse_count = count * 2; // multiply by two because of on and then off
  pulse_total = total_pins * 2;
  pulse_clock = 0;
  continue_haptics = true;
}

void stopHaptics() {
  pulse_clock = pulse_total;
  continue_haptics = false;
}

void doHaptics() {
  if (millis() - ms >= pulse_length[haptic_value]) {
    //Serial.println(pulse_length[haptic_value]);
    if (pulse_clock < pulse_count) {
      haptic_value = !haptic_value;
      digitalWrite(haptic_pin, haptic_value);
    }
    else {
      digitalWrite(haptic_pin, LOW); // this turns it off
    }
    
    pulse_clock++;// = (pulse_clock + 1) % pulse_total;
    if (continue_haptics) {
      pulse_clock %= pulse_total;
    }
    ms = millis();
  }
}
