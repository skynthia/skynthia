int dov[4] = {5, 4, 3, 2};
int dov_count = 0;

int pulse_clock = 0;
int pulse_count = 0;
long ms;
int pulse_length = 300;
bool motor_value = true;
int motor_pin = 7;

void setup() {
  Serial.begin(9600);
  pinMode(motor_pin, OUTPUT);
  pinMode(13, OUTPUT); // use built in LED for debugging
  digitalWrite(motor_pin, motor_value);
  ms = millis();
}

void loop() {
  int binary_value = 0;
  dov_count = 0;
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(dov[i]);
    binary_value = binary_value | (!reading << i);
    if (!reading) {
      dov_count++;
    }
  }

  //Serial.println(binary_value);

  pulse_count = dov_count * 2; // multiply by two because of on and then off
  
  pulse();
  delay(10);
}

void pulse() {
  if (millis() - ms >= pulse_length) {
    if (pulse_clock < pulse_count) {
      motor_value = !motor_value;
      digitalWrite(13, motor_value); // blink the internal LED for now
      digitalWrite(motor_pin, motor_value);
      //Serial.println(motor_value);
    }
    pulse_clock = (pulse_clock + 1) % 8;
    ms = millis();
  }
}
