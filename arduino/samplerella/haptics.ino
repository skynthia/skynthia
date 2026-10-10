

void checkHaptics() {
  if (haptics_dur > -1 && millis() - haptics_clock >= haptics_dur) {
    digitalWrite(motor, HIGH);
    haptics_dur = -1;
  }
}

void sendHaptics(int dur) {
  digitalWrite(motor, LOW);
  haptics_dur = dur;
  haptics_clock = millis();
}
