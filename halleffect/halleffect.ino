#define NUM_BTNS 4

const int hallSensorPin = 2;
const int btns[NUM_BTNS] = {3, 4, 5, 6};

void setup() {
  pinMode(hallSensorPin, INPUT);
  for (int i = 0; i < NUM_BTNS; i++) {
    pinMode(btns[i], INPUT_PULLUP);
  }
  Serial.begin(9600);
}

void loop() {
  int hallValue = digitalRead(hallSensorPin);
  
  Serial.print("Hall Sensor Value: ");
  //Serial.println(hallValue);
  
  // Use 4 buttons to generate binary value
  int bin = 0;
  for (int i = 0; i < NUM_BTNS; i++) {
    int btn = !digitalRead(btns[i]);
    bin = bin | (btn << i);
  }
  Serial.println(bin);

  delay(100); // Add a small delay to avoid rapid repeated detections
}
