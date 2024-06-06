#include <Arduino.h>
#include "Tentacle.h"

Tentacle::Tentacle() {
  wave = {50, 30, 80, 100, 1, 5};
  twitch = {30, 50, 100, 1000, 50, 180};
  subtle = {80, 10, 30, 200, 2, 8};
  modes[0] = &wave;
  modes[1] = &twitch;
  modes[2] = &subtle;
  
  for (int i = 0; i < NUM_TSERV; i++) {
    servos[i].attach(servo_pins[i]);
  }
  last_move = millis();
  checkModeChange(true);
}

void Tentacle::loopTentacle() {
  long ms = millis();
  if (ms - last_move >= time_diff) {
    checkMove();
    checkNewDestinations(false);
    checkModeChange(false);
    last_move = ms;
    time_diff = curr_mode->move_freq + signedRandom(0, curr_mode->move_freq / 20);
  }
}

void Tentacle::checkModeChange(bool force) {
  if (!force) {
    long diff = millis() - last_mode_change;
    if (diff >= 2000) {                             // start checking if we're gonna change at 2 seconds
      float prob = 20 + ((diff - 2000) / 100);      // 20% probability going up to a 100% prob after 10 seconds
      if (random(100) >= prob) {
        return;
      }
    }
  }
  
  // if we get down here, it's because we're changing modes
  // either forced or chance
  mode_num = random(3);
  curr_mode = modes[mode_num];
  last_mode_change = millis();
  time_diff = curr_mode->move_freq + signedRandom(0, curr_mode->move_freq / 20);
  checkNewDestinations(true); // force new destinations
}

void Tentacle::checkNewDestinations(bool force) {
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
    dest[i] = dest[i] > 180 ? 180 : dest[i];
    dest[i] = dest[i] < 0 ? 0 : dest[i];
    step_size[i] = random(curr_mode->min_step_size, curr_mode->max_step_size);
  }
}

void Tentacle::checkMove() {
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
  }
}
