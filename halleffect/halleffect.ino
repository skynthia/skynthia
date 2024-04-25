void setup() {
  pinMode(2,INPUT);
  Serial.begin(9600);
}

void loop() {
  int val = digitalRead(2);
  Serial.println(!val);
  delay(100);
}
