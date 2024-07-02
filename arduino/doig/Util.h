#ifndef Util_h
#define Util_h

struct mode {
  int new_dest_prob;      // The probability of choosing new destinations, per move_freq, once destination is reached. Out of 100.
  int min_dest_change;    // The minimum difference between the previous destination position and the new one, in degrees
  int max_dest_change;    // Max ""
  int move_freq;          // How often this mode takes a step, in milliseconds
  int min_step_size;      // The minimum size of step it takes each time it moves, in degrees
  int max_step_size;      // Max ""
  String mode_name;
};

static int signedRandom(int min_val, int max_val) {
  return random(2) == 0 ? random(min_val, max_val) : 0 - random(min_val, max_val);
}

#endif
