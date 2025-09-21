unsigned long clock;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  clock = 0;
}

void loop() {
  checkPing();
}

void checkPing() {
  if (millis() - clock > 5000) {
    
    Serial1.write('P');
    Serial1.write('0');
    Serial1.write('\n');
    Serial.println("P0");
    //Serial1.print("P0");
    clock = millis();
    digitalWrite(13, HIGH);
  }
  else if (millis() - clock > 500) {
    digitalWrite(13, LOW);
  }
}
