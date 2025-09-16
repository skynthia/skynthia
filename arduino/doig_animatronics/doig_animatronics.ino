#include <avr/eeprom.h>

/*==============================================================================
  Call reseedRandom once in setup to start random on a new sequence.  Uses 
  four bytes of EEPROM.
==============================================================================*/
void reseedRandom( uint32_t* address )
{
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  // Read the previous raw value from EEPROM
  raw = eeprom_read_dword( address );

  // Loop until a seed within the valid range is found
  do
  {
    // Incrementing by a prime (except 2) every possible raw value is visited
    raw += HappyPrime;

    // Park-Miller is only 31 bits so ignore the most significant bit
    seed = raw & 0x7FFFFFFF;
  }
  while ( (seed < 1) || (seed > 2147483646) );

  // Seed the random number generator with the next value in the sequence
  srandom( seed );  

  // Save the new raw value for next time
  eeprom_write_dword( address, raw );
}
inline void reseedRandom( unsigned short address )
{
  reseedRandom( (uint32_t*)(address) );
}
uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;

int endstop_pins[3][2] = {
  {9, 10},
  {A2, A3},
  {A4, A5}
};
bool prev_endstop0_val[3] = {false, false, false};
bool prev_endstop1_val[3] = {false, false, false};
long endstop_trig[3];
int endstop_timeout = 500;

/*int motor_pins[3][2] = {
  // speed, direction
  {3, 4},
  {5, 8},
  {6, 7}
};*/

int spd_pins[3] = {3, 11, 5};
int dir_pins[3] = {4, 12, 8};

// variables for tracking current behavior of motors
long motor_move_start[3] = { -1, -1, -1 };
long motor_move_dur[3] = { 0, 0, 0 };
int motor_speed[3] = { 0, 0, 0 };
bool motor_dir[3] = { true, true, true };

// options for speed
int speeds[4] = { 255, 192, 128, 64 };

void setup() {
  Serial.begin(9600);
  reseedRandom( &reseedRandomSeed );

  for (int i = 0; i < 3; i++) {
    pinMode(spd_pins[i], OUTPUT); // speed
    pinMode(dir_pins[i], OUTPUT); // direction
    digitalWrite(spd_pins[i], LOW);

    endstop_trig[i] = millis();

    newMove(i, false);
  }

  pinMode(endstop_pins[0][0], INPUT_PULLUP);
  pinMode(endstop_pins[0][1], INPUT_PULLUP);
}

void loop() {
  for (int i = 0; i < 3; i++) {
    // If we've been moving for as long as we were supposed to...
    if (millis() - motor_move_start[i] > motor_move_dur[i]) {
      newMove(i, false);
    }

    // check end stops; sometimes flickers to 1 on the analog read
    bool endstop0_val = i == 0 ? digitalRead(endstop_pins[i][0]) : analogRead(endstop_pins[i][0]) > 0;
    bool endstop1_val = i == 0 ? digitalRead(endstop_pins[i][1]) : analogRead(endstop_pins[i][1]) > 0;
    //if (i == 1) Serial.println(endstop0_val);
    if (((endstop0_val && !prev_endstop0_val[i])  || (endstop1_val && !prev_endstop1_val[i])) && (millis() - endstop_trig[i]) >= endstop_timeout) {
      newMove(i, true);
      endstop_trig[i] = millis();
      //digitalWrite(spd_pins[i], 0); // immediately stop the motor
    }

    prev_endstop0_val[i] = endstop0_val;
    prev_endstop1_val[i] = endstop1_val;
  }
  delay(10);
}

void newMove(int which, bool end_stop) {
  // if this was an end stop, we go the other direction next
  if (end_stop) {
    Serial.println((String) "Motor " + which + " endstop reached; reversing direction");
    motor_dir[which] = !motor_dir[which];
    motor_speed[which] = 255;
    motor_move_dur[which] = 4000;
  }
  else {
    // randomly generate a direction
    motor_dir[which] = random(2);
    
    // speed: quantized
    int spd = random(4);
    motor_speed[which] = speeds[spd];
  
    // duration: longer for slower speeds
    spd++;
    int dur = (1000 * spd) + random(2000 * spd);
    motor_move_dur[which] = dur;
  }

  motor_move_start[which] = millis();
  Serial.println((String) which + ":\t" + motor_speed[which] + "\t" + motor_dir[which] + "\t" + motor_move_dur[which]);
  digitalWrite(dir_pins[which], motor_dir[which]);
  analogWrite(spd_pins[which], motor_speed[which]);
}
