bool done = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(1000);
}

void loop() {
  if (!done){
    Serial1.print("AT+P8");
    delay(200);
    while (HC12.available()) {           // If HC-12 has data (the AT Command response)
      Serial.write(HC12.read());         // Send the data to Serial monitor
    }
    done = true;
  }
}
