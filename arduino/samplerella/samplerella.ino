#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 mpr = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int touched_count = 0;
bool pause_drums = false;

// hall effect
int right_tentacle_short[2] = {22, 23};
int rts_val = 0;
int right_tentacle_long[2] = {24, 25};
int rtl_val = 0;
int left_tentacle = 26;
int lt_val = 0;

int motor = 50;

// photoresistors
int eyes[2] = {A8, A9};
int eye_calibration[2] = {0, 0};

// timing
long eye_active[2] = {-1, -1};
long lt_active = -1;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Booting Samplerella...");
  
  pinMode(right_tentacle_short[0], INPUT);
  pinMode(right_tentacle_short[1], INPUT);
  pinMode(right_tentacle_long[0], INPUT);
  pinMode(right_tentacle_long[1], INPUT);
  pinMode(left_tentacle, INPUT);
  pinMode(motor, OUTPUT);

  if (!mpr.begin(0x5A)) {
    Serial.println("MPR121 not found");
    while (1);
  }
  Serial.println("MPR121 found, running auto configuration.");
  mpr.setAutoconfig(true);
  
  calibrateEyes();
  Serial.println("Finished booting Samplerella.");
}

void loop() {
  checkEyes();
  checkTentacles();
  checkTouch();
  delay(10);
}

void checkEyes() {
  // top eye: next/previous track
  // bottom eye: start/stop backing??
  for (int i = 0; i < 2; i++) {
    int eye_val = analogRead(eyes[i]);
    if ((eye_val - eye_calibration[i]) >= 200 && eye_active[i] == -1) {
      // do a buzz here so I can tell if this is happening
      // does Samp need eyelids???
      Serial.print("Activate eye ");
      Serial.println(i);
      eye_active[i] = millis();
    }
    else if ((eye_val - eye_calibration[i]) < 100 && eye_active[i] > -1) {
      Serial.print("Deactivate eye ");
      Serial.println(i);
      eye_active[i] = -1;
    }
  }
}

void checkTentacles() {
  // right tentacle
  int rts_new = 0;
  int rtl_new = 0;
  for (int i = 0; i < 2; i++) {
    int rts = digitalRead(right_tentacle_short[i];
    int rtl = digitalRead(right_tentacle_long[i];
    rts_new = rts_new | (rts << i);
    rtl_new = rtl_new | (rtl << i);
  }
  if (rts_val != rts_new) {
    rts_val = rts_new;
    Serial.print("New RTS val: ");
    Serial.println(rts_val);
  }
  if (rtl_val != rtl_new) {
    rtl_val = rtl_new;
    Serial.print("New RTL val: ");
    Serial.println(rtl_val);
  }

  // left tentacle
  int lt_new = digitalRead(left_tentacle);
  if (lt_val != lt_new) {
    lt_val = lt_new;
    if (lt_val) {
      lt_active = millis();
    }
    else {
      if (millis() - lt_active > 1000) {
        // previous sample
      }
      else {
        Serial.println("Send current sample");
        
      }
      lt_active = -1;
    }
    
    Serial.print("New LT val: ");
    Serial.println(lt_val);
  }
}

void checkTouch() {
  currtouched = mpr.touched();

  for (uint8_t i=0; i<3; i++) {
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      touched_count++;
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      touched_count--;
    }
  }
  
  if (touched_count >= 2 && !pause_drums) {
    Serial.println("Pause drums");
    Serial1.println("PDP");
    pause_drums = true;
  }
  else if (touched_count == 0 && pause_drums) {
    Serial.println("Unpause drums");
    Serial1.println("PDU");
    pause_drums = false;
  }

  lasttouched = currtouched;
}

void calibrateEyes() {
  Serial.println("Calibrating photoresistors...");
  
  digitalWrite(motor, LOW);
  
  for (int i = 0; i < 10; i++) {
    eye_calibration[0] += analogRead(A8);
    eye_calibration[1] += analogRead(A9);
    delay(100);
  }
  eye_calibration[0] /= 10;
  eye_calibration[1] /= 10;
  
  Serial.print("Finished calibrating photoresistors: ");
  Serial.print(eye_calibration[0]);
  Serial.print(", ");
  Serial.println(eye_calibration[1]);

  if (eye_calibration[0] > 750 || eye_calibration[1] > 750) {
    Serial.println("TOO MUCH LIGHT");
    return;
  }
  
  digitalWrite(motor, HIGH);
}
