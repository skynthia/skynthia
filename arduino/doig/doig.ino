#include <Servo.h>
#include "Modes.h"

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A15));
  setupTentacle();
}

void loop() {
  loopTentacle();
}
