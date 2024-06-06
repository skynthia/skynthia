#ifndef Tentacle_h
#define Tentacle_h

#include <Servo.h>
#include "Modes.h"

class Tentacle {
private:
  // how many tentacle servos?
  const static int NUM_TSERV = 3;

  // Tentacle modes
  const static int WAVE = 0;
  const static int TWITCH = 1;
  const static int SUBTLE = 2;
  
  Servo servos[NUM_TSERV];
  int servo_pins[NUM_TSERV] = {22, 24, 26};
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

  void checkModeChange(bool force);
  void checkNewDestinations(bool force);
  void checkMove();

public:
  Tentacle();
  void loopTentacle();
};

#endif
