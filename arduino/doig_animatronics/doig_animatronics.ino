int endstop_pins[3][2] = {
  {A0, A1},
  {A2, A3},
  {A4, A5}
};

int motor_pins[3][2] = {
  // speed, direction
  {3, 4},
  {5, 8},
  {6, 7}
};

// variables for tracking current behavior of motors
long motor_move_start[3] = { -1, -1, -1 };
long motor_move_dur[3] = { 0, 0, 0 };
int motor_speed[3] = { 0, 0, 0 };
bool motor_dir[3] = { true, true, true };

// options for speed
int speeds[5] = { 0, 64, 128, 192, 255 };

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 3; i++) {
    pinMode(motor_pins[i][0], OUTPUT); // speed
    pinMode(motor_pins[i][1], OUTPUT); // direction
    digitalWrite(motor_pins[i][0], LOW);
  }
}

void loop() {
  for (int i = 0; i < 3; i++) {
    // If we've been moving for as long as we were supposed to...
    if (millis() - motor_move_start[i] > motor_move_dur[i]) {
      newMove(i, false);
    }

    // check end stops
    
    // move
  }
}

void newMove(int which, bool end_stop) {
  // if this was an end stop, we go the other direction next
  if (end_stop) {
    motor_dir[which] = !motor_dir[which];
  }
  else {
    // randomly generate a direction
    motor_dir[which] = random(2);
  }

  // speed: quantized
  motor_speed = speeds[random(6)];

  // duration: longer for slower speeds
  
}

