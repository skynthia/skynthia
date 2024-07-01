bool haptic_pin = 52;

int pulse_clock = 0;
int pulse_count = 0;
int pulse_total = 8;
long ms;
int pulse_length[2] = {300, 300};
bool haptic_value = true;

void setupHaptics() {
  pinMode(haptic_pin, OUTPUT);
  digitalWrite(haptic_pin, HIGH); // Because of how the transistor works, this turns the motor off
  ms = millis();
}

// two fast, 
void fistingHaptic() {
  pulse_length[0] = 150; // 150ms on, 150ms off
  pulse_length[1] = 150;
  pulse_count = 4; // on and then off, twice
  pulse_total = 16;
  pulse_clock = 0; // start at the beginning
}

void inputHaptic(int count, int total_pins) {
  pulse_length[0] = 250; // TODO: different pulse lengths for each input, give a different "voice"
  pulse_length[1] = 400;
  pulse_count = count * 2; // multiply by two because of on and then off
  pulse_total = total_pins * 2;
  pulse_clock = 0;
}

void doHaptics() {
  if (millis() - ms >= pulse_length[haptic_value]) {
    if (pulse_clock < pulse_count) {
      haptic_value = !haptic_value;
      digitalWrite(haptic_pin, haptic_value);
    }
    else {
      digitalWrite(haptic_pin, HIGH); // this turns it off
    }
    pulse_clock = (pulse_clock + 1) % pulse_total;
    ms = millis();
  }
}
