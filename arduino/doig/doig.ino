#include "Tentacle.h"

Tentacle tentacle;

void setup() {
  tentacle = *new Tentacle();
}

void loop() {
  tentacle.loopTentacle();
}

void setup_tentacles() {
}

void check_change_mode() {
  
}
