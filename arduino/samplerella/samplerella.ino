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

int sample_num = 0;
int change_sample = 0;

int motor = 50;

// photoresistors
int eyes[2] = {A8, A9};
int eye_calibration[2] = {0, 0};

// timing
long eye_active[2] = {-1, -1};
long lt_active = -1;

unsigned long ping_clock;

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
  checkHaptics();
  checkPing();
  delay(10);
}

void checkPing() {
  // ping every 5 seconds  
  if (millis() - ping_clock >= 5000) {
    Serial1.write('G');
    Serial1.write(1);
    Serial1.write('\n');
    ping_clock = millis();
  }
}

void checkEyes() {
  // top eye: next/previous track
  // bottom eye: start/stop backing??
  for (int i = 0; i < 2; i++) {
    int eye_val = analogRead(eyes[i]);
    if ((eye_val - eye_calibration[i]) >= 200 && eye_active[i] == -1) {
      // do a buzz here so I can tell if this is happening
      sendHaptics(250 + (250*i));
      
      // does Samp need eyelids???
      Serial.print("Activate eye ");
      Serial.println(i);
      eye_active[i] = millis();
    }
    else if ((eye_val - eye_calibration[i]) < 100 && eye_active[i] > -1) {
      Serial.print("Deactivate eye ");
      Serial.println(i);
      if (millis() - eye_active[i] > 1000) {
        sendHaptics(250 + (250*i));
        if (i == 0) {
          Serial.println("Next track");
          sample_num = 0;
          Serial1.println("PNT");
        }
        else {
          Serial.print("Play/stop clip ");
          Serial.println(rts_val);
          Serial1.print("PC");
          Serial1.println((char) (rts_val + 65));
        }
      }
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
    sendHaptics(100 + (100*rts_val));
  }
  if (rtl_val != rtl_new) {
    rtl_val = rtl_new;
    Serial.print("Change vocal effects: ");
    Serial.println(rtl_val);
    Serial1.print("PV");
    Serial1.println((char) (rtl_val + 65));
    sendHaptics(200 + (200*rts_val));
  }

  // left tentacle
  int lt_new = digitalRead(left_tentacle);
  if (lt_val != lt_new) {
    lt_val = lt_new;
    if (lt_val && lt_active == -1) {
      lt_active = millis();
      sendHaptics(100);
    }
    else {
      if (change_sample == 0) {
        Serial.println("Send current sample");
        Serial1.print("PS");
        Serial1.println((char) (sample_num + 65));
        sendHaptics(100);
      }
      else {
        sample_num += change_sample;
        Serial.print("Change sample by ");
        Serial.println(sample_num);
        sendHaptics(300 + (sample_num * 100)));
      }
      lt_active = -1;
    }
  }

  if (millis() - lt_active > 2000 && change_sample == 1) {
    change_sample == -1;
    sendHaptics(200);
  }
  else if (millis() - lt_active > 1000 && change_sample == 0) {
    change_sample == 1;
    sendHaptics(200);
  }
}

void checkTouch() {
  currtouched = mpr.touched();

  for (uint8_t i=0; i<3; i++) {
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      touched_count++;
      sendHaptics(100);
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
