// how many tentacle servos?
const static int NUM_TSERV = 3;

// Tentacle modes
static int WAVE = 0;
static int TWITCH = 1;
static int SUBTLE = 2;

Servo servos[NUM_TSERV];
int servo_pins[NUM_TSERV] = {35, 37, 39};
int pos[NUM_TSERV] = {0, 0, 0};
int dest[NUM_TSERV] = {0, 0, 0};
int step_size[NUM_TSERV] = {0, 0, 0};
long last_move;
int mode_num;
mode wave, twitch, subtle;
mode *modes[3];
mode *curr_mode;

long last_mode_change;
int time_diff;

void setupTentacle() {
  Serial.println("Setting up tentacle...");
  wave = {50, 20, 40, 50, 1, 5, "WAVE"};
  twitch = {30, 50, 80, 1000, 20, 50, "TWCH"};
  subtle = {80, 4, 10, 50, 2, 8, "SBTL"};
  modes[0] = &wave;
  modes[1] = &twitch;
  modes[2] = &subtle;
  
  for (int i = 0; i < NUM_TSERV; i++) {
    servos[i].attach(servo_pins[i]);
    servos[i].write(0);
  }
  last_move = millis();
  checkModeChange(true);
  Serial.print("Tentacle setup finished. Mode: ");
  Serial.println(curr_mode->mode_name);
}

void loopTentacle() {
  if (millis() - last_move >= time_diff) {
    last_move = millis();
    checkMove();
    checkNewDestinations(false);
    checkModeChange(false);
    time_diff = curr_mode->move_freq + signedRandom(0, curr_mode->move_freq / 2.0);
  }
}

void checkModeChange(bool force) {
  if (!force) {
    long diff = millis() - last_mode_change;
    if (diff >= 4000) {                             // start checking if we're gonna change at 2 seconds
      double prob = 20.0 + ((diff - 4000.0) / 100.0);      // 20% probability going up to a 100% prob after 10 seconds
      if (random(100) >= prob) {
        return;
      }
    }
    else {
      return;
    }
  }
  
  // if we get down here, it's because we're changing modes
  // either forced or chance
  mode_num = random(3);
  curr_mode = modes[mode_num];
  last_mode_change = millis();
  time_diff = curr_mode->move_freq + signedRandom(0, curr_mode->move_freq / 2.0);
  checkNewDestinations(true); // force new destinations
}

void checkNewDestinations(bool force) {
  if (!force) {
    // If we haven't arrived at the destinations yet, don't set a new destination
    for (int i = 0; i < NUM_TSERV; i++) {
      if (pos[i] != dest[i]) {
        return;
      }
    }

    // We've arrived at the destinations
    if (random(100) >= curr_mode->new_dest_prob) {
      return;
    }
  }

  for (int i = 0; i < NUM_TSERV; i++) {
    int diff = signedRandom(curr_mode->min_dest_change, curr_mode->max_dest_change);
    dest[i] += diff;
    // Can't go above 180 or below 0
    dest[i] = dest[i] > 90 ? 90 : dest[i];
    dest[i] = dest[i] < 0 ? 0 : dest[i];
    step_size[i] = random(curr_mode->min_step_size, curr_mode->max_step_size);
  }
}

void checkMove() {
  for (int i = 0; i < NUM_TSERV; i++) {
    if (pos[i] > dest[i]) {
      pos[i] -= step_size[i];
      // if we've overshot, go to the destination
      if (pos[i] < dest[i]) {
        pos[i] = dest[i];
      }
    }
    else if (pos[i] < dest[i]) {
      pos[i] += step_size[i];
      if (pos[i] > dest[i]) {
        pos[i] = dest[i];
      }
    }
    servos[i].write(pos[i]);
    /*
    Serial.print(pos[i]);
    Serial.print(":");
    Serial.print(dest[i]);
    Serial.print("\t");*/
  }
  //Serial.println();
}
