int count = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("DH");
  Serial.println((char) ((count % 16) + 65));

  if (count % 4 == 0) {
    Serial.print("DV");
    Serial.println((char) (((count / 4) % 4) + 66));
  }
  
  count++;
  
  delay(10000);
}
